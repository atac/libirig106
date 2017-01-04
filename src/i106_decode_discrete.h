/****************************************************************************

 i106_decode_discrete.h -

 ****************************************************************************/

#ifndef _I106_DECODE_DISCRETE_H
#define _I106_DECODE_DISCRETE_H

#ifdef __cplusplus
namespace Irig106 {
extern "C" {
#endif

/*
 * Macros and definitions
 * ----------------------
 */
#define I106CH10_NUM_DISCRETE_INPUTS_PER_STATE    (uint16_t)32


/*
 * Data structures
 * ---------------
 */

/* Discrete Format 1 */

#if defined(_MSC_VER)
#pragma pack(push)
#pragma pack(1)
#endif

// Channel specific header
typedef struct
   {
    uint32_t    uReserved1  :  24;
    uint32_t    uLength     :  5;      // Number of bits in the event
    uint32_t    uReserved2  :  1;
    uint32_t    uAlignment  :  1;      // 0 = lsb, 1 = msb
    uint32_t    uRecState   :  1;      // 0 = date recorded on change, 1 = recorded at time interval
#if !defined(__GNUC__)
    } SuDiscreteF1_ChanSpec;
#else
    } __attribute__ ((packed)) SuDiscreteF1_ChanSpec;
#endif

// Current discrete message
typedef struct
    {
    unsigned int            uBytesRead;
    SuDiscreteF1_ChanSpec * psuChanSpec;
    SuIntraPacketTS       * psuIPTimeStamp;
    uint32_t                uDiscreteData;
#if !defined(__GNUC__)
    } SuDiscreteF1_CurrMsg;
#else
    } __attribute__ ((packed)) SuDiscreteF1_CurrMsg;
#endif

#if defined(_MSC_VER)
#pragma pack(pop)
#endif


/*
 * Function Declaration
 * --------------------
 */

EnI106Status I106_CALL_DECL
    enI106_Decode_FirstDiscreteF1(SuI106Ch10Header     * psuHeader,
                                  void                 * pvBuff,
                                  SuDiscreteF1_CurrMsg * psuCurrMsg,
                                  SuTimeRef            * psuTimeRef);

EnI106Status I106_CALL_DECL
    enI106_Decode_NextDiscreteF1(SuI106Ch10Header     * psuHeader,
                                 SuDiscreteF1_CurrMsg * psuCurrMsg,
                                 SuTimeRef            * psuTimeRef);

#ifdef __cplusplus
} // end extern "C"
} // end namespace Irig106
#endif

#endif
