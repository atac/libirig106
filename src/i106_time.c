/****************************************************************************

 i106_time.c

 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "int.h"
#include "irig106ch10.h"
#include "i106_time.h"
#include "i106_decode_time.h"


/* Macros and definitions */

// Number of leap years from 1970 to `y' (not including `y' itself).
#define nleap(y) (((y) - 1969) / 4 - ((y) - 1901) / 100 + ((y) - 1601) / 400)

// Nonzero if `y' is a leap year, else zero.
#define leap(y) (((y) % 4 == 0 && (y) % 100 != 0) || (y) % 400 == 0)

// Additional leapday in February of leap years.
#define leapday(m, y) ((m) == 1 && leap (y))

#define ADJUST_TM(tm_member, tm_carry, modulus) \
    if ((tm_member) < 0) { \
        tm_carry -= (1 - ((tm_member)+1) / (modulus)); \
        tm_member = (modulus-1) + (((tm_member)+1) % (modulus)); \
    } else if ((tm_member) >= (modulus)) { \
        tm_carry += (tm_member) / (modulus); \
        tm_member = (tm_member) % (modulus); \
    }

// Length of month `m' (0 .. 11)
#define monthlen(m, y) (julian_day[(m)+1] - julian_day[m] + leapday (m, y))


/* Module data */

static TimeRef  time_ref[MAX_HANDLES];  // Relative / absolute time reference


/* Function Declaration */

// Update the current reference time value
I106Status I106_SetRelTime(int handle, I106Time *time, uint8_t rtc[]){

    // Save the absolute time value
    time_ref[handle].IrigTime.Seconds = time->Seconds;
    time_ref[handle].IrigTime.Fraction = time->Fraction;
    time_ref[handle].IrigTime.Format  = time->Format;

    // Save the relative (i.e. the 10MHz counter) value
    time_ref[handle].RTC = 0;
    memcpy((char *)&(time_ref[handle].RTC), (char *)&rtc[0], 6);

    return I106_OK;
}


// Take a 6 byte relative time value (like the one in the IRIG header) and
// turn it into a real time based on the current reference IRIG time.
I106Status I106_Rel2IrigTime(int handle, uint8_t rtc[], I106Time *time){
    int64_t     rel_time;
    I106Status  status;

    // Convert 6 byte time array to 16 bit int.  This only works for 
    // positive time, but that shouldn't be a problem
    rel_time = 0L;
    memcpy(&rel_time, &rtc[0], 6);

    return I106_RelInt2IrigTime(handle, rel_time, time);
}


// Take a 64 bit relative time value and turn it into a real time based on 
// the current reference IRIG time.
I106Status I106_RelInt2IrigTime(int handle, int64_t rel_time, I106Time *time){
    int64_t         time_diff;
    int64_t         frac_diff;
    int64_t         sec_diff;

    int64_t         seconds;
    int64_t         fraction;


    // Figure out the relative time difference
    time_diff = rel_time - time_ref[handle].RTC;
    sec_diff  = time_diff / 10000000;
    frac_diff = time_diff % 10000000;

    seconds   = time_ref[handle].IrigTime.Seconds + sec_diff;
    fraction  = time_ref[handle].IrigTime.Fraction + frac_diff;

    // This seems a bit extreme but it's defensive programming
    while (fraction < 0){
        fraction += 10000000;
        seconds -= 1;
    }
        
    while (fraction >= 10000000){
        fraction -= 10000000;
        seconds += 1;
    }

    // Now add the time difference to the last IRIG time reference
    time->Fraction = (unsigned long)fraction;
    time->Seconds  = (unsigned long)seconds;
    time->Format   = time_ref[handle].IrigTime.Format;

    return I106_OK;
}


// Take a real clock time and turn it into a 6 byte relative time.
I106Status I106_Irig2RelTime(int handle, I106Time *time, uint8_t rtc[]){
    int64_t  diff;
    int64_t  new_rtc;

    // Calculate time difference (LSB = 100 nSec) between the passed time 
    // and the time reference
    diff = (int64_t)(+ time->Seconds - time_ref[handle].IrigTime.Seconds) * 10000000 +
        (int64_t)(+ time->Fraction - time_ref[handle].IrigTime.Fraction);

    // Add this amount to the reference 
    new_rtc = time_ref[handle].RTC + diff;

    // Now convert this to a 6 byte relative time
    memcpy((char *)&rtc[0], (char *)&(new_rtc), 6);

    return I106_OK;
}


// Take a Irig Ch4 time value (like the one in a secondary IRIG header) and
// turn it into an Irig106 time
I106Status I106_Ch4Binary2IrigTime(I106Ch4_Binary_Time *ch4_time, I106Time *irig_time){
    irig_time->Seconds = (unsigned long)
        ( (double)ch4_time->HighBinTime * CH4BINARYTIME_HIGH_LSB_SEC
        + (unsigned long)ch4_time->LowBinTime * CH4BINARYTIME_LOW_LSB_SEC );
    irig_time->Fraction = (unsigned long)ch4_time->Micro * _100_NANO_SEC_IN_MICRO_SEC;

    return I106_OK;
}


// Take a IEEE-1588 time value (like the one in a secondary IRIG header) and
// turn it into an Irig106 time
I106Status I106_IEEE15882IrigTime(IEEE1588_Time *i1588_time, I106Time  *irig_time){
    irig_time->Seconds = (unsigned long)i1588_time->Seconds;

    //Convert 'nanoseconds' to '100 nanoseconds'
    irig_time->Fraction = (unsigned long)i1588_time->NanoSeconds / 100;     

    return I106_OK;
}


// Warning - array to int / int to array functions are little endian only!

// Create a 6 byte array value from a 64 bit int relative time
void LLInt2TimeArray(int64_t *rel_time, uint8_t rtc[]){
    memcpy((char *)rtc, (char *)rel_time, 6);
}


// Create a 64 bit int relative time from 6 byte array value
void TimeArray2LLInt(uint8_t rtc[], int64_t *rel_time){
    *rel_time = 0L;
    memcpy((char *)rel_time, (char *)rtc, 6);
}


// Read the data file from the current position to try to determine a valid 
// relative time to clock time from a time packet.
I106Status I106_SyncTime(int handle, int sync, int max_seconds){
    int64_t             offset;
    int64_t             time_limit;
    int64_t             current_time;
    I106Status          status;
    I106Status          return_status;
    I106C10Header       header;
    I106Time            time;
    unsigned long       buffer_size = 0;
    void              * buffer = NULL;
    TimeF1_CSDW       * csdw = NULL;

    // Get and save the current file position
    status = I106C10GetPos(handle, &offset);
    if (status != I106_OK)
        return status;

    // Read the next header
    status = I106C10ReadNextHeaderFile(handle, &header);
    if (status == I106_EOF)
        return I106_TIME_NOT_FOUND;

    if (status != I106_OK)
        return status;

    // Calculate the time limit if there is one
    if (max_seconds > 0){
        TimeArray2LLInt(header.RTC, &time_limit);
        time_limit = time_limit + (int64_t)max_seconds * (int64_t)10000000;
    }
    else
        time_limit = 0;

    // Loop, looking for appropriate time message
    while (1){

        // See if we've passed our time limit
        if (time_limit > 0){
            TimeArray2LLInt(header.RTC, &current_time);
            if (time_limit < current_time){
                return_status = I106_TIME_NOT_FOUND;
                break;
            }
        }

        // If IRIG time type then process it
        if (header.DataType == I106CH10_DTYPE_IRIG_TIME){

            // Read header OK, make buffer for time message
            if (buffer_size < header.PacketLength){
                buffer       = realloc(buffer, header.PacketLength);
                csdw         = (TimeF1_CSDW *)buffer;
                buffer_size  = header.PacketLength;
            }

            // Read the data buffer
            status = I106C10ReadData(handle, buffer_size, buffer);
            if (status != I106_OK){
                return_status = I106_TIME_NOT_FOUND;
                break;
            }

            // If external sync OK then decode it and set relative time
            if ((sync == 0) || (csdw->TimeSource == 1)){
                I106_Decode_TimeF1(&header, buffer, &time);
                I106_SetRelTime(handle, &time, header.RTC);
                return_status = I106_OK;
                break;
            }
        }

        // Read the next header and try again
        status = I106C10ReadNextHeaderFile(handle, &header);
        if (status == I106_EOF){
            return_status = I106_TIME_NOT_FOUND;
            break;
        }

        if (status != I106_OK){
            return_status = status;
            break;
        }

    }

    // Restore file position
    status = I106C10SetPos(handle, offset);
    if (status != I106_OK)
        return_status = status;

    // Return the malloc'ed memory
    free(buffer);

    return return_status;
}


I106Status I106C10SetPosToIrigTime(int handle, I106Time *irig_seek_time){
    uint8_t           rtc_seek_time[6];
    int64_t           seek_time;
    InOrderIndex    * index = &handles[handle].Index;
    int               upper_limit;
    int               lower_limit;

    // If there is no index in memory then barf
    if (index->SortStatus != SORTED)
        return I106_NO_INDEX;

    // We have an index so do a binary search for time

    // Convert clock time to 10 MHz count
    I106_Irig2RelTime(handle, irig_seek_time, rtc_seek_time);
    TimeArray2LLInt(rtc_seek_time, &seek_time);

    // Check time bounds
    if (seek_time < index->Index[0].Time){
        I106C10FirstMsg(handle);
        return I106_TIME_NOT_FOUND;
    };

    if (seek_time > index->Index[index->ArrayUsed - 1].Time){
        I106C10LastMsg(handle);
        return I106_TIME_NOT_FOUND;
    };

    // If we don't already have it, figure out how many search steps
    if (index->NumSearchSteps == 0){
        upper_limit = 1;
        while (upper_limit < index->ArrayUsed){
            upper_limit *= 2;
            index->NumSearchSteps++;
        }
    }

    // Loop prescribed number of times
    lower_limit = 0;
    upper_limit = index->ArrayUsed - 1;
    index->ArrayPos = (upper_limit - lower_limit) / 2;
    for (int i = 0; i < index->NumSearchSteps; i++){
        if (index->Index[index->ArrayPos].Time > seek_time)
            upper_limit = (upper_limit - lower_limit) / 2;
        else if (index->Index[index->ArrayPos].Time < seek_time)
            lower_limit = (upper_limit - lower_limit) / 2;
        else
            break;
    }

    return I106_OK;
}


/* General purpose time utilities */

// Convert IRIG time into an appropriate string
char * IrigTime2String(I106Time *time){
    static char    time_str[30];
    struct tm     *tm;

    // Convert IRIG time into it's components
    tm = gmtime((time_t *)&(time->Seconds));

    // Make the appropriate string
    switch (time->Format){

        // Year / Month / Day format ("2008/02/29 12:34:56.789")
        case I106_DATEFMT_DMY:
            sprintf(time_str, "%4.4i/%2.2i/%2.2i %2.2i:%2.2i:%2.2i.%3.3i",
                tm->tm_year + 1900,
                tm->tm_mon + 1,
                tm->tm_mday,
                tm->tm_hour,
                tm->tm_min,
                tm->tm_sec,
                time->Fraction / 10000);
            break;

        // Day of the Year format ("001:12:34:56.789")
        case I106_DATEFMT_DAY:
        default:
            sprintf(time_str, "%3.3i:%2.2i:%2.2i:%2.2i.%3.3i",
                tm->tm_yday+1,
                tm->tm_hour,
                tm->tm_min,
                tm->tm_sec,
                time->Fraction / 10000);
            break;
    }

    return time_str;
}


// This function fills in the SuTimeRef structure with the "best" relative 
// and/or absolute time stamp available from the packet header and intra-packet 
// header (if available).
I106Status FillInTimeStruct(I106C10Header *header, IntraPacketTS * ipts, TimeRef * timeref){
    int secondary_time_format;

    // Get the secondary header time format
    secondary_time_format = header->PacketFlags & I106CH10_PFLAGS_TIMEFMT_MASK;
    timeref->RTCValid = 0;
    timeref->TimeValid = 0;
    
    // Set the relative time from the packet header
    TimeArray2LLInt(header->RTC, &(timeref->RTC));
    timeref->RTCValid = 1;

    // If secondary header is available, use that time for absolute
    if ((header->PacketFlags & I106CH10_PFLAGS_SEC_HEADER) != 0){
        switch(secondary_time_format){
            case I106CH10_PFLAGS_TIMEFMT_IRIG106:
                I106_Ch4Binary2IrigTime((I106Ch4_Binary_Time *)header->RTC, &(timeref->IrigTime));
                timeref->TimeValid = 1;
                break;
            case I106CH10_PFLAGS_TIMEFMT_IEEE1588:
                I106_IEEE15882IrigTime((IEEE1588_Time *)header->RTC, &(timeref->IrigTime));
                timeref->TimeValid = 1;
                break;
            default:
                //Currently reserved, should we have a default way to decode?
                break;
        }
    }

    // Now process values from the intra-packet headers if available
    if (ipts != NULL){
        
        // If relative time
        if ((header->PacketFlags & I106CH10_PFLAGS_IPTIMESRC) == 0){
            TimeArray2LLInt(ipts->IPTS, &(timeref->RTC));
            timeref->RTCValid = 1;
        }

        // else is absolute time
        else {
            switch(secondary_time_format){
                case I106CH10_PFLAGS_TIMEFMT_IRIG106:
                    I106_Ch4Binary2IrigTime((I106Ch4_Binary_Time *)ipts, &(timeref->IrigTime));
                    timeref->TimeValid = 1;
                    break;
                case I106CH10_PFLAGS_TIMEFMT_IEEE1588:                  
                    I106_IEEE15882IrigTime((IEEE1588_Time *)ipts, &(timeref->IrigTime));
                    timeref->TimeValid = 1;
                    break;
                default:
                    //Current reserved, should we have a default way to decode
                    break;
            }
        }
    }
    
    return I106_OK;
}


/* Return the equivalent in seconds past 12:00:00 a.m. Jan 1, 1970 GMT
   of the Greenwich Mean time and date in the exploded time structure `tm'.

   The standard mktime() has the annoying "feature" of assuming that the 
   time in the tm structure is local time, and that it has to be corrected 
   for local time zone.  In this library time is assumed to be UTC and UTC
   only.  To make sure no timezone correction is applied this time conversion
   routine was lifted from the standard C run time library source.  Interestingly
   enough, this routine was found in the source for mktime().

   This function does always put back normalized values into the `tm' struct,
   parameter, including the calculated numbers for `tm->tm_yday',
   `tm->tm_wday', and `tm->tm_isdst'.

   Returns -1 if the time in the `tm' parameter cannot be represented
   as valid `time_t' number. 
 */
uint32_t mkgmtime(struct tm *time){

    // Accumulated number of days from 01-Jan up to start of current month.
    static short julian_day[] = {
        0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
    };

    int years, months, days, hours, minutes, seconds;

    years   = time->tm_year + 1900;  // year - 1900 -> year
    months  = time->tm_mon;          // 0..11
    days    = time->tm_mday - 1;     // 1..31 -> 0..30
    hours   = time->tm_hour;         // 0..23
    minutes = time->tm_min;          // 0..59
    seconds = time->tm_sec;          // 0..61 in ANSI C.

    ADJUST_TM(seconds, minutes, 60)
    ADJUST_TM(minutes, hours,   60)
    ADJUST_TM(hours,   days,    24)
    ADJUST_TM(months,  years,   12)

    while (days < 0) {
        if (--months < 0) {
            --years;
            months = 11;
        }
        days += monthlen(months, years);
    } ;

    while (days >= monthlen(months, years)){
        days -= monthlen(months, years);
        if (++months >= 12) {
            ++years;
            months = 0;
        }
    }

    // Restore adjusted values in tm structure
    time->tm_year = years - 1900;
    time->tm_mon  = months;
    time->tm_mday = days + 1;
    time->tm_hour = hours;
    time->tm_min  = minutes;
    time->tm_sec  = seconds;

    // Set `days' to the number of days into the year.
    days += julian_day[months] + (months > 1 && leap (years));
    time->tm_yday = days;

    // Now calculate `days' to the number of days since Jan 1, 1970.
    days = (unsigned)days + 365 * (unsigned)(years - 1970) +
           (unsigned)(nleap (years));
    time->tm_wday = ((unsigned)days + 4) % 7; /* Jan 1, 1970 was Thursday. */
    time->tm_isdst = 0;

    if (years < 1970)
        return (uint32_t)-1;

    return (uint32_t)(86400L * days  + 3600L * hours + 60L * minutes + seconds);
}
