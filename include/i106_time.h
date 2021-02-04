/****************************************************************************

 i106_decode_time.h

 ****************************************************************************/

#ifndef _I106_DECODE_TIME_H
#define _I106_DECODE_TIME_H

#include "i106_util.h"
#include "libirig106.h"


/* Macros and definitions */

typedef enum {
    I106_TIME_FORMAT_IRIG_B      =  0x00,
    I106_TIME_FORMAT_IRIG_A      =  0x01,
    I106_TIME_FORMAT_IRIG_G      =  0x02,
    I106_TIME_FORMAT_INT_RTC     =  0x03,
    I106_TIME_FORMAT_GPS_UTC     =  0x04,
    I106_TIME_FORMAT_GPS_NATIVE  =  0x05,
} I106TimeFormat;

typedef enum {
    I106_TIMESOURCE_INTERNAL      =  0x00,
    I106_TIMESOURCE_EXTERNAL      =  0x01,
    I106_TIMESOURCE_INTERNAL_RMM  =  0x02,
    I106_TIMESOURCE_NONE          =  0x0F    
} I106TimeSource;


/* Data structures */

/* Time Format 1 */

#pragma pack(push, 1)

// Channel specific header
typedef struct {
    uint32_t    TimeSource  :  4;      // Time source    
    uint32_t    TimeFormat  :  4;      // Time format
    uint32_t    LeapYear    :  1;      // Leap year
    uint32_t    DateFormat  :  1;      // Date format
    uint32_t    Reserved2   :  2;
    uint32_t    Reserved3   : 20;
} TimeF1_CSDW;

// Time message - Day format
typedef struct {
    uint16_t    Tmn         :  4;      // Tens of milliseconds
    uint16_t    Hmn         :  4;      // Hundreds of milliseconds
    uint16_t    Sn          :  4;      // Units of seconds
    uint16_t    TSn         :  3;      // Tens of seconds
    uint16_t    Reserved1   :  1;      // 0

    uint16_t    Mn          :  4;      // Units of minutes
    uint16_t    TMn         :  3;      // Tens of minutes
    uint16_t    Reserved2   :  1;      // 0
    uint16_t    Hn          :  4;      // Units of hours
    uint16_t    THn         :  2;      // Tens of Hours
    uint16_t    Reserved3   :  2;      // 0

    uint16_t    Dn          :  4;      // Units of day number
    uint16_t    TDn         :  4;      // Tens of day number
    uint16_t    HDn         :  2;      // Hundreds of day number
    uint16_t    Reserved4   :  6;      // 0
} Time_MessageDayFormat;

// Time message - DMY format
typedef struct {
    uint16_t    Tmn         :  4;      // Tens of milliseconds
    uint16_t    Hmn         :  4;      // Hundreds of milliseconds
    uint16_t    Sn          :  4;      // Units of seconds
    uint16_t    TSn         :  3;      // Tens of seconds
    uint16_t    Reserved1   :  1;      // 0

    uint16_t    Mn          :  4;      // Units of minutes
    uint16_t    TMn         :  3;      // Tens of minutes
    uint16_t    Reserved2   :  1;      // 0
    uint16_t    Hn          :  4;      // Units of hours
    uint16_t    THn         :  2;      // Tens of Hours
    uint16_t    Reserved3   :  2;      // 0

    uint16_t    Dn          :  4;      // Units of day number
    uint16_t    TDn         :  4;      // Tens of day number
    uint16_t    On          :  4;      // Units of month number
    uint16_t    TOn         :  1;      // Tens of month number
    uint16_t    Reserved4   :  3;      // 0

    uint16_t    Yn          :  4;      // Units of year number
    uint16_t    TYn         :  4;      // Tens of year number
    uint16_t    HYn         :  4;      // Hundreds of year number
    uint16_t    OYn         :  2;      // Thousands of year number
    uint16_t    Reserved5   :  2;      // 0
} Time_MessageDMYFormat;

// Time message Format 1 structure
typedef struct MessageTimeF1 MessageTimeF1;
struct MessageTimeF1 {
    TimeF1_CSDW  CSDW;
    union {
        Time_MessageDayFormat    DayFormat;
        Time_MessageDMYFormat    DMYFormat;
    } Message;
};

#pragma pack(pop)


/* Function Declaration */

I106Status I106_Decode_TimeF1(I106C10Header *header, void *raw_buffer, I106Time *time);
void I106_Decode_TimeF1_Buffer(int data_format, int leap_year, void *buffer, I106Time *time);
I106Status I106_Encode_TimeF1(I106C10Header *header, unsigned int time_source,
        unsigned int time_format, unsigned int date_format, I106Time *time, void *buffer);

#endif
