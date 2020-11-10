/****************************************************************************

 i106_decode_events.h - Computer generated data format 2 events
 Created by Bob Baggerman

 ****************************************************************************/

#ifndef _I106_DECODE_EVENTS_H
#define _I106_DECODE_EVENTS_H

#ifdef __cplusplus
namespace Irig106 {
extern "C" {
#endif


/*
 * Macros and definitions
 * ----------------------
 */


/*
 * Data structures
 * ---------------
 */

#if defined(_MSC_VER)
#pragma pack(push)
#pragma pack(1)
#endif

// Channel specific data word
// --------------------------

typedef PUBLIC struct Events_ChanSpec_S
    {
    uint32_t    uEventCount     : 12;   // Total number of events
    uint32_t    uReserved       : 19;
    uint32_t    bIntraPckHdr    :  1;   // Intra-packet header present
#if !defined(__GNUC__)
    } SuEvents_ChanSpec;
#else
    } __attribute__ ((packed)) SuEvents_ChanSpec;
#endif

// Event data
// ----------

typedef PUBLIC struct Events_Data_S
    {
    uint32_t    uNumber         : 12;   // Event identification number
    uint32_t    uCount          : 16;   // Event count index
    uint32_t    bEventOccurence :  1;   // 
    uint32_t    uReserved       :  3;   // Time tag bits
#if !defined(__GNUC__)
    } SuEvents_Data;
#else
    } __attribute__ ((packed)) SuEvents_Data;
#endif

// The various intra-packet headers
// --------------------------------

// Event message, with RTC format time, without optional data
typedef PUBLIC struct Events_RTC_S
    {
    SuIntraPacketRtc        suRtcTime;  // RTC format time stamp
    SuEvents_Data           suData;     // Data about the event
#if !defined(__GNUC__)
    } SuEvents_RTC;
#else
    } __attribute__ ((packed)) SuEvents_RTC;
#endif

// Event message, with Ch 4 format time, without optional data
typedef PUBLIC struct Events_Ch4Time_S
    {
    SuI106Ch4_Binary_Time   suCh4Time;  // Ch 4 format time stamp
    SuEvents_Data           suData;     // Data about the event
#if !defined(__GNUC__)
    } SuEvents_Ch4Time;
#else
    } __attribute__ ((packed)) SuEvents_Ch4Time;
#endif

// Event message, with IEEE-1588 format time, without optional data
typedef PUBLIC struct Events_1588Time_S
    {
    SuIEEE1588_Time         su1588Time; // IEEE-1588 format time stamp
    SuEvents_Data           suData;     // Data about the event
#if !defined(__GNUC__)
    } SuEvents_1588Time;
#else
    } __attribute__ ((packed)) SuEvents_1588Time;
#endif


#if defined(_MSC_VER)
#pragma pack(pop)
#endif


/*
 * Function Declaration
 * --------------------
 */


#ifdef __cplusplus
}
}
#endif

#endif
