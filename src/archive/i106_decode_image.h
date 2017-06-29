/****************************************************************************

 i106_decode_image.h - 
 Created by Bob Baggerman

 ****************************************************************************/

#ifndef _I106_DECODE_IMAGE_H
#define _I106_DECODE_IMAGE_H

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

// Image packet Format 0

typedef PUBLIC struct ImageF0_ChanSpec_S
    {
    uint32_t    uLength         : 27;      // Segment byte length
    uint32_t    bIPH            :  1;      // Intra-packet header flag
    uint32_t    uSum            :  2;      // 
    uint32_t    uPart           :  2;      //
#if !defined(__GNUC__)
    } SuImageF0_ChanSpec;
#else
    } __attribute__ ((packed)) SuImageF0_ChanSpec;
#endif


// Image packet Format 1

typedef PUBLIC struct ImageF1_ChanSpec_S
    {
    uint32_t    uReserved       : 23;      //
    uint32_t    uLength         :  4;      // Image format
    uint32_t    bIPH            :  1;      // Intra-packet header flag
    uint32_t    uSum            :  2;      // 
    uint32_t    uPart           :  2;      //
#if !defined(__GNUC__)
    } SuImageF1_ChanSpec;
#else
    } __attribute__ ((packed)) SuImageF1_ChanSpec;
#endif

// Intra-message header
typedef struct ImageF1_Header
    {
    uint64_t    suIntraPckTime;            // Reference time
    uint32_t    uMsgLength;                // Message length
#if !defined(__GNUC__)
    } SuMessageF1_Header;
#else
    } __attribute__ ((packed)) SuMessageF1_Header;
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
