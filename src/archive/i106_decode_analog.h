/****************************************************************************

 i106_decode_analog.h - 
 Created by Bob Baggerman

 ****************************************************************************/

#ifndef _I106_DECODE_ANALOG_H
#define _I106_DECODE_ANALOG_H

#ifdef __cplusplus
namespace Irig106 {
extern "C" {
#endif


/*
 * Macros and definitions
 * ----------------------
 */

#define ANALOG_MAX_SUBCHANS 256

typedef enum
    {
    ANALOG_PACKED               = 0,
    ANALOG_UNPACKED_LSB_PADDED  = 1,
    ANALOG_RESERVED             = 2,
    ANALOG_UNPACKED_MSB_PADDED  = 3,
    } ANALOG_MODE;

typedef enum 
    {
    ANALOG_MSB_FIRST            = 0,
    ANALOG_LSB_FIRST            = 1,
    } ANALOG_BIT_TRANSFER_ORDER;

typedef enum
    {
    ANALOG_FMT_ONES             = 0,
    ANALOG_FMT_TWOS             = 1,
    ANALOG_FMT_SIGNMAG_0        = 2,
    ANALOG_FMT_SIGNMAG_1        = 3,
    ANALOG_FMT_OFFSET_BIN       = 4,
    ANALOG_FMT_UNSIGNED_BIN     = 5,
    ANALOG_FMT_SINGLE_FLOAT     = 6,
    } ANALOG_FORMAT;   // R-x\AF-n-m


/*
 * Data structures
 * ---------------
 */

#if defined(_MSC_VER)
#pragma pack(push,1)
#endif

// Channel specific data word
// --------------------------

typedef PUBLIC struct AnalogF1_ChanSpec_S
    {
    uint32_t    uMode           :  2;      // 
    uint32_t    uLength         :  6;      // Bits in A/D value
    uint32_t    uSubChan        :  8;      // Subchannel number
    uint32_t    uTotChan        :  8;      // Total number of subchannels
    uint32_t    uFactor         :  4;      // Sample rate exponent
    uint32_t    bSame           :  1;      // One/multiple Channel Specific
    uint32_t    iReserved       :  3;      //
#if !defined(__GNUC__)
    } SuAnalogF1_ChanSpec;
#else
    } __attribute__ ((packed)) SuAnalogF1_ChanSpec;
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
