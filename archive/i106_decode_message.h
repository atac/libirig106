/****************************************************************************

 i106_decode_message.h - 
 Created by Bob Baggerman

 ****************************************************************************/

#ifndef _I106_DECODE_MESSAGE_H
#define _I106_DECODE_MESSAGE_H

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
#pragma pack(push,1)
#endif

// Channel specific data word
// --------------------------

typedef PUBLIC struct MessageF0_ChanSpec_S
    {
    uint32_t    uCounter        : 16;      // Message/segment counter
    uint32_t    uType           :  2;      // Complete/segment type
    uint32_t    uReserved       : 14;
#if !defined(__GNUC__)
    } SuMessageF0_ChanSpec;
#else
    } __attribute__ ((packed)) SuMessageF0_ChanSpec;
#endif


// Intra-message header
typedef struct MessageF0_Header
    {
    uint64_t    suIntraPckTime;            // Reference time
    uint32_t    uMsgLength      : 16;      // Message length
    uint32_t    uSubChannel     : 14;      // Subchannel number
    uint32_t    bFmtError       :  1;      // Format error flag
    uint32_t    bDataError      :  1;      // Data error flag
#if !defined(__GNUC__)
    } SuMessageF0_Header;
#else
    } __attribute__ ((packed)) SuMessageF0_Header;
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
