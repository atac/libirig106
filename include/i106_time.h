/****************************************************************************

 i106_time.h

 ****************************************************************************/

#ifndef _I106_TIME_H
#define _I106_TIME_H

#include <time.h>
#include "irig106ch10.h"

/* Macros and definitions */

#define CH4BINARYTIME_HIGH_LSB_SEC  655.36
#define CH4BINARYTIME_LOW_LSB_SEC     0.01
#define _100_NANO_SEC_IN_MICRO_SEC   10

typedef enum {
    I106_DATEFMT_DAY  = 0,
    I106_DATEFMT_DMY  = 1,
} I106DateFormat;


/* Data structures */

// Time has a number of representations in the IRIG 106 spec.
// The structure below is used as a convenient standard way of
// representing time.  The nice thing about standards is that there 
// are so many to choose from, and time is no exception. But none of 
// the various C time representations really fill the bill. So I made 
// a new time representation.  So there.
typedef struct {
    time_t           Seconds;
    uint32_t         Fraction;    // LSB = 100ns
    I106DateFormat   Format;      // Day or DMY format
} I106Time;


// Relative time to absolute time reference
typedef struct {
    int64_t    RTC;       // Relative time from header
    I106Time   IrigTime;  // Clock time from IRIG source
    uint16_t   RTCValid   :  1;
    uint16_t   TimeValid  :  1;
    uint16_t   Reserved   :  14;
} TimeRef;


#pragma pack(push, 1)

// IRIG 106 secondary header time in Ch 4 BCD format
typedef struct {
    uint16_t      Minute1   : 4;    // High order time
    uint16_t      Minute10  : 3;
    uint16_t      Hour1     : 4;
    uint16_t      Hour10    : 2;
    uint16_t      Day1      : 3;
    uint16_t      Sec0_01   : 4;    // Low order time
    uint16_t      Sec0_1    : 4;
    uint16_t      Sec1      : 4;
    uint16_t      Sec10     : 2;
    uint16_t      Reserved  : 2;
    uint16_t      Micro;           // Microsecond time
} I106Ch4_BCD_Time;


// IRIG 106 secondary header time in Ch 4 binary format
typedef struct {
    uint16_t      HighBinTime;     // High order time
    uint16_t      LowBinTime;      // Low order time
    uint16_t      Micro;           // Microsecond time
} I106Ch4_Binary_Time;


// IRIG 106 secondary header time in IEEE-1588 format
typedef struct {
    uint32_t      NanoSeconds;     // Nano-seconds
    uint32_t      Seconds;         // Seconds
} IEEE1588_Time;


// Intra-packet header relative time counter format
typedef struct {
    uint8_t       RTC[6];   // Reference time
    uint16_t      Reserved;
} IntraPacketRTC;

// Intra-packet header time stamp - raw data
typedef struct {
    uint8_t  IPTS[8];   // Time Stamp    
} IntraPacketTS;

#pragma pack(pop)


/* Function Declaration */

I106Status I106_SetRelTime(int handle, I106Time *time, uint8_t rtc[]);
I106Status I106_Rel2IrigTime(int handle, uint8_t rtc[], I106Time *time);
I106Status I106_RelInt2IrigTime(int handle, int64_t rtc, I106Time *time);
I106Status I106_Irig2RelTime(int handle, I106Time  *time, uint8_t rtc[]);
I106Status I106_Ch4Binary2IrigTime(I106Ch4_Binary_Time *ch4_time, I106Time *irig_time);
I106Status I106_IEEE15882IrigTime(IEEE1588_Time *i1588_time, I106Time *irig_time);
I106Status FillInTimeStruct(I106C10Header *header, IntraPacketTS *ipts, TimeRef *time_ref);

// Warning - array to int / int to array functions are little endian only!
void LLInt2TimeArray(int64_t * rel_time, uint8_t rtc[]);
void TimeArray2LLInt(uint8_t rtc[], int64_t *rel_time);
I106Status I106_SyncTime(int handle, int sync, int time_limit);
I106Status I106C10SetPosToIrigTime(int handle, I106Time *seek_time);


/* General purpose time utilities */

// Convert IRIG time into an appropriate string
char * IrigTime2String(I106Time *time);

// This is handy enough that we'll go ahead and export it to the world
uint32_t mkgmtime(struct tm *time);

#endif
