/****************************************************************************

 i106_decode_uart.h - 

 ****************************************************************************/

#ifndef _I106_DECODE_UART_H
#define _I106_DECODE_UART_H

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

/* UART Format 0 */

// Channel specific header
typedef struct 
    {
    uint32_t    Reserved     : 31;      
    uint32_t    bIPH         :  1;      // Intra-Packet Header enabled    
#if !defined(__GNUC__)
    } SuUartF0_ChanSpec;
#else
    } __attribute__ ((packed)) SuUartF0_ChanSpec;
#endif

// Intra-message header
typedef struct 
    {    
    uint16_t    uDataLength     : 16;    // Length of the UART data in bytes
    uint16_t    uSubchannel     : 14;    // Subchannel for the following data
    uint16_t    uReserved       : 1;
    uint16_t    bParityError    : 1;     //Parity Error    
#if !defined(__GNUC__)
    } SuUartF0_Header;
#else
    } __attribute__ ((packed)) SuUartF0_Header;
#endif

// Current UART message
typedef struct
    {
    SuI106Ch10Header      * psuHeader;      // Pointer to the current header
    unsigned int            uBytesRead;
    SuUartF0_ChanSpec     * psuChanSpec;    // Pointer to the Channel Specific Data Word
    SuIntraPacketTS       * psuIPTimeStamp; // Pointer to the Intra-Packet time stamp
    SuUartF0_Header       * psuUartHdr;     // Pointer to the Intra-Packet header
    uint8_t               * pauData;        // Pointer to the data
    SuTimeRef               suTimeRef;
    } SuUartF0_CurrMsg;


#if defined(_MSC_VER)
#pragma pack(pop)
#endif


/*
 * Function Declaration
 * --------------------
 */

EnI106Status I106_CALL_DECL 
    enI106_Decode_FirstUartF0(SuI106Ch10Header         * psuHeader,
                              void                     * pvBuff,
                              SuUartF0_CurrMsg         * psuCurrMsg);

EnI106Status I106_CALL_DECL 
    enI106_Decode_NextUartF0(SuUartF0_CurrMsg          * psuCurrMsg);

#ifdef __cplusplus
}
}
#endif

#endif
