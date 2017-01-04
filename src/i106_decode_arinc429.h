/****************************************************************************

 i106_decode_arinc429.h - 
 Created by Bob Baggerman

 ****************************************************************************/

#ifndef _I106_DECODE_ARINC429_H
#define _I106_DECODE_ARINC429_H

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

typedef struct Arinc429F0_ChanSpec_S
    {
    uint32_t    uMsgCount       : 16;      // Message count
    uint32_t    Reserved        : 16;      //
#if !defined(__GNUC__)
    } SuArinc429F0_ChanSpec;
#else
    } __attribute__ ((packed)) SuArinc429F0_ChanSpec;
#endif

// Intra-message header
typedef struct Arinc429F0_Header_S
    {
    uint32_t    uGapTime        : 20;      // Gap Time
    uint32_t    Reserved        :  1;      // 
    uint32_t    uBusSpeed       :  1;      // Bus Speed
    uint32_t    bParityError    :  1;      // Parity Error
    uint32_t    bFormatError    :  1;      // Data type
    uint32_t    uBusNum         :  8;      // Bus number
#if !defined(__GNUC__)
    } SuArinc429F0_Header;
#else
    } __attribute__ ((packed)) SuArinc429F0_Header;
#endif


// ARINC 429 data format
typedef struct Arinc429F0_Data_S
    {
    uint32_t    uLabel          :  8;      // Label
    uint32_t    uSDI            :  2;      // Source/Destination Identifiers
    uint32_t    uData           : 19;      // Data
    uint32_t    uSSM            :  2;      // Sign/Status Matrix
    uint32_t    uParity         :  1;      // Parity
#if !defined(__GNUC__)
    } SuArinc429F0_Data;
#else
    } __attribute__ ((packed)) SuArinc429F0_Data;
#endif


// Current ARINC 429 message
typedef struct
    {
    unsigned int            uMsgNum;
    uint32_t                ulCurrOffset;   // Offset into data buffer
    SuArinc429F0_ChanSpec * psuChanSpec;
    int64_t                 llIntPktTime;   // Intrapacket message relative time
    SuArinc429F0_Header   * psu429Hdr;      // Pointer to the 429 header
    SuArinc429F0_Data     * psu429Data;     // Pointer to the 429 data
#if !defined(__GNUC__)
    } SuArinc429F0_CurrMsg;
#else
    } __attribute__ ((packed)) SuArinc429F0_CurrMsg;
#endif



#if defined(_MSC_VER)
#pragma pack(pop)
#endif

/*
 * Function Declaration
 * --------------------
 */

EnI106Status I106_CALL_DECL 
    enI106_Decode_FirstArinc429F0(SuI106Ch10Header * psuHeader,
                              void                 * pvBuff,
                              SuArinc429F0_CurrMsg * psuMsg);

EnI106Status I106_CALL_DECL 
    enI106_Decode_NextArinc429F0(SuArinc429F0_CurrMsg * psuMsg);



#ifdef __cplusplus
}
}
#endif

#endif
