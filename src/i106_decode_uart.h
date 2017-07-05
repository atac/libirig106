/****************************************************************************

 i106_decode_uart.h

 ****************************************************************************/

#ifndef _I106_DECODE_UART_H
#define _I106_DECODE_UART_H

#include "irig106ch10.h"
#include "i106_time.h"


/* Data structures */

#if defined(_MSC_VER)
#pragma pack(push)
#pragma pack(1)
#endif

/* UART Format 0 */

// Channel specific header
typedef struct {
    uint32_t    Reserved    : 31;      
    uint32_t    IPH         :  1;      // Intra-Packet Header enabled    
} PACKED UARTF0_CSDW;

// Intra-message header
typedef struct {    
    uint16_t    Length     : 16;    // Length of the UART data in bytes
    uint16_t    Subchannel     : 14;    // Subchannel for the following data
    uint16_t    Reserved       : 1;
    uint16_t    ParityError    : 1;     //Parity Error    
} PACKED UARTF0_IPH;

// Current UART message
typedef struct {
    I106C10Header  * Header;     // Pointer to the current header
    unsigned int     BytesRead;
    UARTF0_CSDW    * CSDW;       // Pointer to the Channel Specific Data Word
    IntraPacketTS  * IPTS;       // Pointer to the Intra-Packet time stamp
    UARTF0_IPH     * IPH;        // Pointer to the Intra-Packet header
    uint8_t        * Data;       // Pointer to the data
    TimeRef          Time;
} UARTF0_Message;


#if defined(_MSC_VER)
#pragma pack(pop)
#endif


/* Function Declaration */
I106Status I106_Decode_FirstUARTF0(I106C10Header *header, void *buffer, UARTF0_Message *msg);
I106Status I106_Decode_NextUARTF0(UARTF0_Message *msg);

#endif
