/****************************************************************************

 i106_decode_parallel.h - 
 Created by Bob Baggerman

 ****************************************************************************/

#ifndef _I106_DECODE_PARALLEL_H
#define _I106_DECODE_PARALLEL_H

#ifdef __cplusplus
namespace Irig106 {
extern "C" {
#endif


/*
 * Macros and definitions
 * ----------------------
 */

#define PARALLEL_FO_TYPE_DCRSI      254

/*
 * Data structures
 * ---------------
 */

#if defined(_MSC_VER)
#pragma pack(push,1)
#endif

// Channel specific data word
// --------------------------

typedef struct ParallelF0_ChanSpec_S
    {
    uint32_t    uScanNum        : 24;      // Scan number
    uint32_t    uType           :  8;      // Data type
#if !defined(__GNUC__)
    } SuParallelF0_ChanSpec;
#else
    } __attribute__ ((packed)) SuParallelF0_ChanSpec;
#endif

// No Intra-message header!

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
