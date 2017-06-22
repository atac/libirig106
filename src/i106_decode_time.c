/****************************************************************************

 i106_decode_time.c

 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>

#if defined(_WIN32)
#include <windows.h>        // For FILETIME
#endif

#include "stdint.h"

#include "irig106ch10.h"
#include "i106_time.h"
#include "i106_decode_time.h"


/* Data structures */

// Day of Year to Day and Month
typedef struct {
    int  Month;     // Month 0 - 11
    int  Day;       // Day of month 1-31
} DOY2DM;      


/* Module data */

// THIS IS KIND OF A PROBLEM BECAUSE THIS SHOULD BE DONE ON A PER FILE BASIS.
// THAT MEANS THIS REALLY SHOULD BE STORED IN THE HEADER.
// SuTimeRef   m_suCurrRefTime;           // Current value of IRIG reference time


// These structures are used to convert from day of the year format to 
// day and month.  One is for normal years and the other is for leap years.
// The values and index are of the "struct tm" notion.  That is, the day of 
// the year index is number of days since Jan 1st, i.e. Jan 1st = 0.  For
// IRIG time, Jan 1st = 1.  The month value is months since January, i.e. 
// Jan = 0.  Don't get confused!

DOY2DM day_to_day_and_month[] = {
{ 0,  0}, // This is to handle the special case where IRIG DoY is incorrectly set to 000
{ 0,  1}, { 0,  2}, { 0,  3}, { 0,  4}, { 0,  5}, { 0,  6}, { 0,  7}, { 0,  8},
{ 0,  9}, { 0, 10}, { 0, 11}, { 0, 12}, { 0, 13}, { 0, 14}, { 0, 15}, { 0, 16},
{ 0, 17}, { 0, 18}, { 0, 19}, { 0, 20}, { 0, 21}, { 0, 22}, { 0, 23}, { 0, 24},
{ 0, 25}, { 0, 26}, { 0, 27}, { 0, 28}, { 0, 29}, { 0, 30}, { 0, 31}, { 1,  1},
{ 1,  2}, { 1,  3}, { 1,  4}, { 1,  5}, { 1,  6}, { 1,  7}, { 1,  8}, { 1,  9},
{ 1, 10}, { 1, 11}, { 1, 12}, { 1, 13}, { 1, 14}, { 1, 15}, { 1, 16}, { 1, 17},
{ 1, 18}, { 1, 19}, { 1, 20}, { 1, 21}, { 1, 22}, { 1, 23}, { 1, 24}, { 1, 25},
{ 1, 26}, { 1, 27}, { 1, 28}, { 2,  1}, { 2,  2}, { 2,  3}, { 2,  4}, { 2,  5},
{ 2,  6}, { 2,  7}, { 2,  8}, { 2,  9}, { 2, 10}, { 2, 11}, { 2, 12}, { 2, 13},
{ 2, 14}, { 2, 15}, { 2, 16}, { 2, 17}, { 2, 18}, { 2, 19}, { 2, 20}, { 2, 21},
{ 2, 22}, { 2, 23}, { 2, 24}, { 2, 25}, { 2, 26}, { 2, 27}, { 2, 28}, { 2, 29},
{ 2, 30}, { 2, 31}, { 3,  1}, { 3,  2}, { 3,  3}, { 3,  4}, { 3,  5}, { 3,  6},
{ 3,  7}, { 3,  8}, { 3,  9}, { 3, 10}, { 3, 11}, { 3, 12}, { 3, 13}, { 3, 14},
{ 3, 15}, { 3, 16}, { 3, 17}, { 3, 18}, { 3, 19}, { 3, 20}, { 3, 21}, { 3, 22},
{ 3, 23}, { 3, 24}, { 3, 25}, { 3, 26}, { 3, 27}, { 3, 28}, { 3, 29}, { 3, 30},
{ 4,  1}, { 4,  2}, { 4,  3}, { 4,  4}, { 4,  5}, { 4,  6}, { 4,  7}, { 4,  8},
{ 4,  9}, { 4, 10}, { 4, 11}, { 4, 12}, { 4, 13}, { 4, 14}, { 4, 15}, { 4, 16},
{ 4, 17}, { 4, 18}, { 4, 19}, { 4, 20}, { 4, 21}, { 4, 22}, { 4, 23}, { 4, 24},
{ 4, 25}, { 4, 26}, { 4, 27}, { 4, 28}, { 4, 29}, { 4, 30}, { 4, 31}, { 5,  1},
{ 5,  2}, { 5,  3}, { 5,  4}, { 5,  5}, { 5,  6}, { 5,  7}, { 5,  8}, { 5,  9},
{ 5, 10}, { 5, 11}, { 5, 12}, { 5, 13}, { 5, 14}, { 5, 15}, { 5, 16}, { 5, 17},
{ 5, 18}, { 5, 19}, { 5, 20}, { 5, 21}, { 5, 22}, { 5, 23}, { 5, 24}, { 5, 25},
{ 5, 26}, { 5, 27}, { 5, 28}, { 5, 29}, { 5, 30}, { 6,  1}, { 6,  2}, { 6,  3},
{ 6,  4}, { 6,  5}, { 6,  6}, { 6,  7}, { 6,  8}, { 6,  9}, { 6, 10}, { 6, 11},
{ 6, 12}, { 6, 13}, { 6, 14}, { 6, 15}, { 6, 16}, { 6, 17}, { 6, 18}, { 6, 19},
{ 6, 20}, { 6, 21}, { 6, 22}, { 6, 23}, { 6, 24}, { 6, 25}, { 6, 26}, { 6, 27},
{ 6, 28}, { 6, 29}, { 6, 30}, { 6, 31}, { 7,  1}, { 7,  2}, { 7,  3}, { 7,  4},
{ 7,  5}, { 7,  6}, { 7,  7}, { 7,  8}, { 7,  9}, { 7, 10}, { 7, 11}, { 7, 12},
{ 7, 13}, { 7, 14}, { 7, 15}, { 7, 16}, { 7, 17}, { 7, 18}, { 7, 19}, { 7, 20},
{ 7, 21}, { 7, 22}, { 7, 23}, { 7, 24}, { 7, 25}, { 7, 26}, { 7, 27}, { 7, 28},
{ 7, 29}, { 7, 30}, { 7, 31}, { 8,  1}, { 8,  2}, { 8,  3}, { 8,  4}, { 8,  5},
{ 8,  6}, { 8,  7}, { 8,  8}, { 8,  9}, { 8, 10}, { 8, 11}, { 8, 12}, { 8, 13},
{ 8, 14}, { 8, 15}, { 8, 16}, { 8, 17}, { 8, 18}, { 8, 19}, { 8, 20}, { 8, 21},
{ 8, 22}, { 8, 23}, { 8, 24}, { 8, 25}, { 8, 26}, { 8, 27}, { 8, 28}, { 8, 29},
{ 8, 30}, { 9,  1}, { 9,  2}, { 9,  3}, { 9,  4}, { 9,  5}, { 9,  6}, { 9,  7},
{ 9,  8}, { 9,  9}, { 9, 10}, { 9, 11}, { 9, 12}, { 9, 13}, { 9, 14}, { 9, 15},
{ 9, 16}, { 9, 17}, { 9, 18}, { 9, 19}, { 9, 20}, { 9, 21}, { 9, 22}, { 9, 23},
{ 9, 24}, { 9, 25}, { 9, 26}, { 9, 27}, { 9, 28}, { 9, 29}, { 9, 30}, { 9, 31},
{10,  1}, {10,  2}, {10,  3}, {10,  4}, {10,  5}, {10,  6}, {10,  7}, {10,  8},
{10,  9}, {10, 10}, {10, 11}, {10, 12}, {10, 13}, {10, 14}, {10, 15}, {10, 16},
{10, 17}, {10, 18}, {10, 19}, {10, 20}, {10, 21}, {10, 22}, {10, 23}, {10, 24},
{10, 25}, {10, 26}, {10, 27}, {10, 28}, {10, 29}, {10, 30}, {11,  1}, {11,  2},
{11,  3}, {11,  4}, {11,  5}, {11,  6}, {11,  7}, {11,  8}, {11,  9}, {11, 10},
{11, 11}, {11, 12}, {11, 13}, {11, 14}, {11, 15}, {11, 16}, {11, 17}, {11, 18},
{11, 19}, {11, 20}, {11, 21}, {11, 22}, {11, 23}, {11, 24}, {11, 25}, {11, 26},
{11, 27}, {11, 28}, {11, 29}, {11, 30}, {11, 31} };

DOY2DM day_to_day_and_month_leapyear[] = {
{ 0,  0}, // This is to handle the special case where IRIG DoY is incorrectly set to 000
{ 0,  1}, { 0,  2}, { 0,  3}, { 0,  4}, { 0,  5}, { 0,  6}, { 0,  7}, { 0,  8},
{ 0,  9}, { 0, 10}, { 0, 11}, { 0, 12}, { 0, 13}, { 0, 14}, { 0, 15}, { 0, 16},
{ 0, 17}, { 0, 18}, { 0, 19}, { 0, 20}, { 0, 21}, { 0, 22}, { 0, 23}, { 0, 24},
{ 0, 25}, { 0, 26}, { 0, 27}, { 0, 28}, { 0, 29}, { 0, 30}, { 0, 31}, { 1,  1},
{ 1,  2}, { 1,  3}, { 1,  4}, { 1,  5}, { 1,  6}, { 1,  7}, { 1,  8}, { 1,  9},
{ 1, 10}, { 1, 11}, { 1, 12}, { 1, 13}, { 1, 14}, { 1, 15}, { 1, 16}, { 1, 17},
{ 1, 18}, { 1, 19}, { 1, 20}, { 1, 21}, { 1, 22}, { 1, 23}, { 1, 24}, { 1, 25},
{ 1, 26}, { 1, 27}, { 1, 28}, { 1, 29}, { 2,  1}, { 2,  2}, { 2,  3}, { 2,  4},
{ 2,  5}, { 2,  6}, { 2,  7}, { 2,  8}, { 2,  9}, { 2, 10}, { 2, 11}, { 2, 12},
{ 2, 13}, { 2, 14}, { 2, 15}, { 2, 16}, { 2, 17}, { 2, 18}, { 2, 19}, { 2, 20},
{ 2, 21}, { 2, 22}, { 2, 23}, { 2, 24}, { 2, 25}, { 2, 26}, { 2, 27}, { 2, 28},
{ 2, 29}, { 2, 30}, { 2, 31}, { 3,  1}, { 3,  2}, { 3,  3}, { 3,  4}, { 3,  5},
{ 3,  6}, { 3,  7}, { 3,  8}, { 3,  9}, { 3, 10}, { 3, 11}, { 3, 12}, { 3, 13},
{ 3, 14}, { 3, 15}, { 3, 16}, { 3, 17}, { 3, 18}, { 3, 19}, { 3, 20}, { 3, 21},
{ 3, 22}, { 3, 23}, { 3, 24}, { 3, 25}, { 3, 26}, { 3, 27}, { 3, 28}, { 3, 29},
{ 3, 30}, { 4,  1}, { 4,  2}, { 4,  3}, { 4,  4}, { 4,  5}, { 4,  6}, { 4,  7},
{ 4,  8}, { 4,  9}, { 4, 10}, { 4, 11}, { 4, 12}, { 4, 13}, { 4, 14}, { 4, 15},
{ 4, 16}, { 4, 17}, { 4, 18}, { 4, 19}, { 4, 20}, { 4, 21}, { 4, 22}, { 4, 23},
{ 4, 24}, { 4, 25}, { 4, 26}, { 4, 27}, { 4, 28}, { 4, 29}, { 4, 30}, { 4, 31},
{ 5,  1}, { 5,  2}, { 5,  3}, { 5,  4}, { 5,  5}, { 5,  6}, { 5,  7}, { 5,  8},
{ 5,  9}, { 5, 10}, { 5, 11}, { 5, 12}, { 5, 13}, { 5, 14}, { 5, 15}, { 5, 16},
{ 5, 17}, { 5, 18}, { 5, 19}, { 5, 20}, { 5, 21}, { 5, 22}, { 5, 23}, { 5, 24},
{ 5, 25}, { 5, 26}, { 5, 27}, { 5, 28}, { 5, 29}, { 5, 30}, { 6,  1}, { 6,  2},
{ 6,  3}, { 6,  4}, { 6,  5}, { 6,  6}, { 6,  7}, { 6,  8}, { 6,  9}, { 6, 10},
{ 6, 11}, { 6, 12}, { 6, 13}, { 6, 14}, { 6, 15}, { 6, 16}, { 6, 17}, { 6, 18},
{ 6, 19}, { 6, 20}, { 6, 21}, { 6, 22}, { 6, 23}, { 6, 24}, { 6, 25}, { 6, 26},
{ 6, 27}, { 6, 28}, { 6, 29}, { 6, 30}, { 6, 31}, { 7,  1}, { 7,  2}, { 7,  3},
{ 7,  4}, { 7,  5}, { 7,  6}, { 7,  7}, { 7,  8}, { 7,  9}, { 7, 10}, { 7, 11},
{ 7, 12}, { 7, 13}, { 7, 14}, { 7, 15}, { 7, 16}, { 7, 17}, { 7, 18}, { 7, 19},
{ 7, 20}, { 7, 21}, { 7, 22}, { 7, 23}, { 7, 24}, { 7, 25}, { 7, 26}, { 7, 27},
{ 7, 28}, { 7, 29}, { 7, 30}, { 7, 31}, { 8,  1}, { 8,  2}, { 8,  3}, { 8,  4},
{ 8,  5}, { 8,  6}, { 8,  7}, { 8,  8}, { 8,  9}, { 8, 10}, { 8, 11}, { 8, 12},
{ 8, 13}, { 8, 14}, { 8, 15}, { 8, 16}, { 8, 17}, { 8, 18}, { 8, 19}, { 8, 20},
{ 8, 21}, { 8, 22}, { 8, 23}, { 8, 24}, { 8, 25}, { 8, 26}, { 8, 27}, { 8, 28},
{ 8, 29}, { 8, 30}, { 9,  1}, { 9,  2}, { 9,  3}, { 9,  4}, { 9,  5}, { 9,  6},
{ 9,  7}, { 9,  8}, { 9,  9}, { 9, 10}, { 9, 11}, { 9, 12}, { 9, 13}, { 9, 14},
{ 9, 15}, { 9, 16}, { 9, 17}, { 9, 18}, { 9, 19}, { 9, 20}, { 9, 21}, { 9, 22},
{ 9, 23}, { 9, 24}, { 9, 25}, { 9, 26}, { 9, 27}, { 9, 28}, { 9, 29}, { 9, 30},
{ 9, 31}, {10,  1}, {10,  2}, {10,  3}, {10,  4}, {10,  5}, {10,  6}, {10,  7},
{10,  8}, {10,  9}, {10, 10}, {10, 11}, {10, 12}, {10, 13}, {10, 14}, {10, 15},
{10, 16}, {10, 17}, {10, 18}, {10, 19}, {10, 20}, {10, 21}, {10, 22}, {10, 23},
{10, 24}, {10, 25}, {10, 26}, {10, 27}, {10, 28}, {10, 29}, {10, 30}, {11,  1},
{11,  2}, {11,  3}, {11,  4}, {11,  5}, {11,  6}, {11,  7}, {11,  8}, {11,  9},
{11, 10}, {11, 11}, {11, 12}, {11, 13}, {11, 14}, {11, 15}, {11, 16}, {11, 17},
{11, 18}, {11, 19}, {11, 20}, {11, 21}, {11, 22}, {11, 23}, {11, 24}, {11, 25},
{11, 26}, {11, 27}, {11, 28}, {11, 29}, {11, 30}, {11, 31} };


/* Function Declaration */

// Take an IRIG F1 time packet and decode it into something we can use
I106Status I106_Decode_TimeF1(I106C10Header  *header, void *raw_buffer, I106Time *time){
    TimeF1_CSDW   * csdw;
    void          * buffer;

    csdw = (TimeF1_CSDW *)raw_buffer;
    buffer = (char *)raw_buffer + sizeof(TimeF1_CSDW);

    I106_Decode_TimeF1_Buffer(csdw->DateFormat, csdw->LeapYear, buffer, time);

    return I106_OK;
}


void I106_Decode_TimeF1_Buffer(int date_format, int leap_year, void *buffer, I106Time *time){
    struct tm                tm_time;
    Time_MessageDMYFormat  * time_dmy;
    Time_MessageDayFormat  * time_day;

    if (date_format == 0){
        // Make time
        time_day = (Time_MessageDayFormat *)buffer;
        tm_time.tm_sec   = time_day->TSn * 10 + time_day->Sn;
        tm_time.tm_min   = time_day->TMn * 10 + time_day->Mn;
        tm_time.tm_hour  = time_day->THn * 10 + time_day->Hn;

        // Legal IRIG DoY numbers are from 1 to 365 (366 for leap year). Some vendors however
        // will use 000 for DoY.  Not legal but there it is.
        tm_time.tm_yday  = time_day->HDn * 100 + time_day->TDn * 10 + time_day->Dn;

        // Make day
        if (leap_year){
            tm_time.tm_mday  = day_to_day_and_month_leapyear[tm_time.tm_yday].Day;
            tm_time.tm_mon   = day_to_day_and_month_leapyear[tm_time.tm_yday].Month;
            tm_time.tm_year  = 72;  // i.e. 1972, a leap year
        }
        else {
            tm_time.tm_mday  = day_to_day_and_month[tm_time.tm_yday].Day;
            tm_time.tm_mon   = day_to_day_and_month[tm_time.tm_yday].Month;
            tm_time.tm_year  = 71;  // i.e. 1971, not a leap year
        }
        tm_time.tm_isdst = 0;
        time->Seconds   = mkgmtime(&tm_time);
        time->Fraction  = time_day->Hmn * 1000000L + time_day->Tmn * 100000L;
        time->Format    = I106_DATEFMT_DAY;
    }

    // Time in DMY format
    else {
        time_dmy = (Time_MessageDMYFormat *)buffer;
        tm_time.tm_sec   = time_dmy->TSn *   10 + time_dmy->Sn;
        tm_time.tm_min   = time_dmy->TMn *   10 + time_dmy->Mn;
        tm_time.tm_hour  = time_dmy->THn *   10 + time_dmy->Hn;
        tm_time.tm_yday  = 0;
        tm_time.tm_mday  = time_dmy->TDn *   10 + time_dmy->Dn;
        tm_time.tm_mon   = time_dmy->TOn *   10 + time_dmy->On - 1;
        tm_time.tm_year  = time_dmy->OYn * 1000 + time_dmy->HYn * 100 + 
                           time_dmy->TYn *   10 + time_dmy->Yn - 1900;
        tm_time.tm_isdst = 0;
        time->Seconds   = mkgmtime(&tm_time);
        time->Fraction  = time_dmy->Hmn * 1000000L + time_dmy->Tmn * 100000L;
        time->Format    = I106_DATEFMT_DMY;
    }
}


I106Status I106_Encode_TimeF1(I106C10Header *header,
                         unsigned int  time_source,
                         unsigned int  time_format,
                         unsigned int  date_format,
                         I106Time     *time,
                         void         *buffer){

    // A temporary integer to decimate to get BCD factors
    uint32_t                 int_time;
    struct tm              * tm_time;
    MessageTimeF1          * message;
    Time_MessageDayFormat  * day_format;
    Time_MessageDMYFormat  * dmy_format;

    // Now, after creating this ubertime-structure above, create a 
    // couple of pointers to make the code below simpler to read.
    message = (MessageTimeF1 *)buffer;
    day_format = &(message->Message.DayFormat);
    dmy_format = &(message->Message.DMYFormat);

    // Zero out all the time fields
    memset(message, 0, sizeof(TimeF1_CSDW));

    // Break time down to DMY HMS
    tm_time = gmtime((time_t *)&(time->Seconds));

    // Make channel specific data word
    message->CSDW.TimeSource    = time_source;
    message->CSDW.TimeFormat    = time_format;
    message->CSDW.DateFormat    = date_format;
    if (tm_time->tm_year % 4 == 0)
        message->CSDW.LeapYear = 1;
    else
        message->CSDW.LeapYear = 0;

/* #pragma message("WARNING - Don't zero out the whole packet") */

    // Fill in day of year format
    if (date_format == 0){
        // Zero out all the time fields
        memset(day_format, 0, sizeof(Time_MessageDayFormat));

        // Set the various time fields
        int_time = time->Fraction / 100000L;
        day_format->Tmn = (uint16_t)(int_time  % 10);
        int_time = (int_time - day_format->Hmn) / 10;
        day_format->Hmn = (uint16_t)(int_time  % 10);

        int_time = tm_time->tm_sec;
        day_format->Sn  = (uint16_t)(int_time  % 10);
        int_time = (int_time - day_format->Sn)  / 10;
        day_format->TSn = (uint16_t)(int_time  % 10);

        int_time = tm_time->tm_min;
        day_format->Mn  = (uint16_t)(int_time  % 10);
        int_time = (int_time - day_format->Mn)  / 10;
        day_format->TMn = (uint16_t)(int_time  % 10);

        int_time = tm_time->tm_hour;
        day_format->Hn  = (uint16_t)(int_time  % 10);
        int_time = (int_time - day_format->Hn)  / 10;
        day_format->THn = (uint16_t)(int_time  % 10);

        int_time = tm_time->tm_yday + 1;
        day_format->Dn = (uint16_t)(int_time   % 10);
        int_time = (int_time - day_format->Dn)  / 10;
        day_format->TDn = (uint16_t)(int_time  % 10);
        int_time = (int_time - day_format->TDn) / 10;
        day_format->HDn  = (uint16_t)(int_time  % 10);

        // Set the data length in the header
        header->DataLength = sizeof(TimeF1_CSDW) + sizeof(Time_MessageDayFormat);
    }

    // Fill in day, month, year format
    else {
        // Zero out all the time fields
        memset(dmy_format, 0, sizeof(Time_MessageDMYFormat));

        // Set the various time fields
        int_time = time->Fraction / 100000L;
        dmy_format->Tmn = (uint16_t)(int_time  % 10);
        int_time = (int_time - dmy_format->Hmn) / 10;
        dmy_format->Hmn = (uint16_t)(int_time  % 10);

        int_time = tm_time->tm_sec;
        dmy_format->Sn  = (uint16_t)(int_time  % 10);
        int_time = (int_time - dmy_format->Sn)  / 10;
        dmy_format->TSn = (uint16_t)(int_time  % 10);

        int_time = tm_time->tm_min;
        dmy_format->Mn  = (uint16_t)(int_time  % 10);
        int_time = (int_time - dmy_format->Mn)  / 10;
        dmy_format->TMn = (uint16_t)(int_time  % 10);

        int_time = tm_time->tm_hour;
        dmy_format->Hn  = (uint16_t)(int_time  % 10);
        int_time = (int_time - dmy_format->Hn)  / 10;
        dmy_format->THn = (uint16_t)(int_time  % 10);

        int_time = tm_time->tm_mday;
        dmy_format->Dn  = (uint16_t)(int_time  % 10);
        int_time = (int_time - dmy_format->Dn)  / 10;
        dmy_format->TDn = (uint16_t)(int_time  % 10);

        int_time = tm_time->tm_mon + 1;
        dmy_format->On  = (uint16_t)(int_time  % 10);
        int_time = (int_time - dmy_format->On)  / 10;
        dmy_format->TOn = (uint16_t)(int_time  % 10);

        int_time = tm_time->tm_year + 1900;
        dmy_format->Yn  = (uint16_t)(int_time  % 10);
        int_time = (int_time - dmy_format->Yn)  / 10;
        dmy_format->TYn = (uint16_t)(int_time  % 10);
        int_time = (int_time - dmy_format->TYn) / 10;
        dmy_format->HYn = (uint16_t)(int_time  % 10);
        int_time = (int_time - dmy_format->HYn) / 10;
        dmy_format->OYn = (uint16_t)(int_time  % 10);

        // Set the data length in the header
        header->DataLength = 
            sizeof(TimeF1_CSDW) + sizeof(Time_MessageDMYFormat);
    }

    // Make the data buffer checksum and update the header
    // This is the job of the caller
    //AddDataFillerChecksum(header, (unsigned char *)buffer);

    return I106_OK;
}
