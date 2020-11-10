/****************************************************************************

 i106_decode_comp_gen_0.h - Computer generated data format 0
 Created by Bob Baggerman

 ****************************************************************************/

#ifndef _I106_DECODE_COMP_GEN_0_H
#define _I106_DECODE_COMP_GEN_0_H

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

// Channel specific data word
// --------------------------

typedef PUBLIC struct CompGen0_ChanSpec_S
    {
    uint32_t    uReserved;
#if !defined(__GNUC__)
    } SuCompGen0_ChanSpec;
#else
    } __attribute__ ((packed)) SuCompGen0_ChanSpec;
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
