/****************************************************************************

 i106_decode_discrete.c -

 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "stdint.h"

#include "irig106ch10.h"
#include "i106_time.h"
#include "i106_decode_discrete.h"

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

static void vFillInMsgPtrs(SuDiscreteF1_CurrMsg * psuCurrMsg);

/* ======================================================================= */

EnI106Status I106_CALL_DECL
    enI106_Decode_FirstDiscreteF1(SuI106Ch10Header     * psuHeader,
                                  void                 * pvBuff,
                                  SuDiscreteF1_CurrMsg * psuCurrMsg,
                                  SuTimeRef            * psuTimeRef)
    {

    psuCurrMsg->uBytesRead = 0;

    // Set pointers to the beginning of the Discrete buffer
    psuCurrMsg->psuChanSpec = (SuDiscreteF1_ChanSpec *)pvBuff;

    psuCurrMsg->uBytesRead+=sizeof(SuDiscreteF1_ChanSpec);

    // Check for no data
    if (psuHeader->ulDataLen <= psuCurrMsg->uBytesRead)
        return I106_NO_MORE_DATA;


    // Get the other pointers
    vFillInMsgPtrs(psuCurrMsg);

    vFillInTimeStruct(psuHeader, psuCurrMsg->psuIPTimeStamp, psuTimeRef);

    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL
    enI106_Decode_NextDiscreteF1(SuI106Ch10Header     * psuHeader,
                                 SuDiscreteF1_CurrMsg * psuCurrMsg,
                                 SuTimeRef            * psuTimeRef)
    {

   // Check for no more data
    if (psuHeader->ulDataLen <= psuCurrMsg->uBytesRead)
        return I106_NO_MORE_DATA;

    // Get the other pointers
    vFillInMsgPtrs(psuCurrMsg);

    vFillInTimeStruct(psuHeader, psuCurrMsg->psuIPTimeStamp, psuTimeRef);

    return I106_OK;
    }

/* ----------------------------------------------------------------------- */

void vFillInMsgPtrs(SuDiscreteF1_CurrMsg * psuCurrMsg)
{

    psuCurrMsg->psuIPTimeStamp = (SuIntraPacketTS *)
                              ((char *)(psuCurrMsg->psuChanSpec) +
                               psuCurrMsg->uBytesRead);
    psuCurrMsg->uBytesRead+=sizeof(psuCurrMsg->psuIPTimeStamp->aubyIntPktTime);

    psuCurrMsg->uDiscreteData = *( (uint32_t *) ((char *)(psuCurrMsg->psuChanSpec) +
                              psuCurrMsg->uBytesRead));
    psuCurrMsg->uBytesRead+=sizeof(psuCurrMsg->uDiscreteData);

}

#ifdef __cplusplus
} // end namespace Irig106
#endif
