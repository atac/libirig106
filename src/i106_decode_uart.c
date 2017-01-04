/****************************************************************************

 i106_decode_uart.c - 

 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stdint.h"

#include "irig106ch10.h"
#include "i106_time.h"

#include "i106_decode_uart.h"

#ifdef __cplusplus
namespace Irig106 {
#endif


/*
 * Macros and definitions
 * ----------------------
 */


/*
 * Data structures
 * ---------------
 */


/*
 * Module data
 * -----------
 */



/*
 * Function Declaration
 * --------------------
 */

static void vFillInMsgPtrs(SuUartF0_CurrMsg * psuCurrMsg);

/* ======================================================================= */

EnI106Status I106_CALL_DECL 
    enI106_Decode_FirstUartF0(SuI106Ch10Header         * psuHeader,
                              void                     * pvBuff,
                              SuUartF0_CurrMsg         * psuCurrMsg)

    {

    // Keep a pointer to the current header    
    psuCurrMsg->psuHeader = psuHeader;

    psuCurrMsg->uBytesRead = 0;

    // Set pointers to the beginning of the UART buffer
    psuCurrMsg->psuChanSpec = (SuUartF0_ChanSpec *)pvBuff;

    psuCurrMsg->uBytesRead += sizeof(SuUartF0_ChanSpec);

    // Check for no data
    if (psuHeader->ulDataLen <= psuCurrMsg->uBytesRead)
        return I106_NO_MORE_DATA;

    // Get the other pointers
    vFillInMsgPtrs(psuCurrMsg);

    vFillInTimeStruct(psuHeader, psuCurrMsg->psuIPTimeStamp, &psuCurrMsg->suTimeRef);

    return I106_OK;
    
    }



/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    enI106_Decode_NextUartF0(SuUartF0_CurrMsg         * psuCurrMsg)
    {
    
    // Check for no more data
    if (psuCurrMsg->psuHeader->ulDataLen <= psuCurrMsg->uBytesRead)
        return I106_NO_MORE_DATA;

    // Get the other pointers
    vFillInMsgPtrs(psuCurrMsg);

    vFillInTimeStruct(psuCurrMsg->psuHeader, psuCurrMsg->psuIPTimeStamp, &psuCurrMsg->suTimeRef);

    return I106_OK;

    }

/* ----------------------------------------------------------------------- */

void vFillInMsgPtrs(SuUartF0_CurrMsg * psuCurrMsg)
{
     // Set the pointer to the intra-packet time stamp if available
    if(psuCurrMsg->psuChanSpec->bIPH == 1)
    {
        psuCurrMsg->psuIPTimeStamp = (SuIntraPacketTS *)
                              ((char *)(psuCurrMsg->psuChanSpec) +
                               psuCurrMsg->uBytesRead);
        psuCurrMsg->uBytesRead += sizeof(psuCurrMsg->psuIPTimeStamp->aubyIntPktTime);

    }
    else
        psuCurrMsg->psuIPTimeStamp = NULL;

    // Set the pointer to the intra-packet header
    psuCurrMsg->psuUartHdr = (SuUartF0_Header *)
                             ((char *)(psuCurrMsg->psuChanSpec) + 
                              psuCurrMsg->uBytesRead); 
    psuCurrMsg->uBytesRead += sizeof(SuUartF0_Header);
    
    // Set the pointer to the data
    psuCurrMsg->pauData = (uint8_t *)((char *)(psuCurrMsg->psuChanSpec) + psuCurrMsg->uBytesRead);

    // Add the data length, if it is odd, account for the filler byte we will skip   
    psuCurrMsg->uBytesRead+=
        psuCurrMsg->psuUartHdr->uDataLength + (psuCurrMsg->psuUartHdr->uDataLength % 2);    

}



#ifdef __cplusplus
}
#endif

