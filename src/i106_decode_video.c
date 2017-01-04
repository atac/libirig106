/****************************************************************************

 i106_decode_1553f1.c - 

 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stdint.h"

#include "irig106ch10.h"
#include "i106_decode_video.h"

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



/* ======================================================================= */

/// Setup reading multiple Video Format 0 messages

EnI106Status I106_CALL_DECL 
    enI106_Decode_FirstVideoF0(SuI106Ch10Header  * psuHeader,
                               void              * pvBuff,
                               SuVideoF0_CurrMsg * psuCurrMsg)
    {

    // Save pointer to channel specific data
    psuCurrMsg->psuChanSpec = (SuVideoF0_ChanSpec *)pvBuff;

    // Set pointers if embedded time used
    if (psuCurrMsg->psuChanSpec->bET == 1)
        {
        psuCurrMsg->psuIPHeader = (SuVideoF0_Header *)
                                  ((char *)pvBuff + sizeof(SuVideoF0_ChanSpec));
        psuCurrMsg->pachTSData  = (uint8_t         *)
                                  ((char *)pvBuff             + 
                                   sizeof(SuVideoF0_ChanSpec) +
                                   sizeof(SuVideoF0_Header));
        }

    // No embedded time
    else
        {
        psuCurrMsg->psuIPHeader = NULL;
        psuCurrMsg->pachTSData  = (uint8_t *)pvBuff + sizeof(SuVideoF0_ChanSpec);
        }

// TAKE CARE OF BYTE SWAPPING BASED ON CH 10 RELEASE AND BA CSDW (NEW IN -09)

    return I106_OK;
    }



/* ----------------------------------------------------------------------- */

EnI106Status I106_CALL_DECL 
    enI106_Decode_NextVideoF0 (SuI106Ch10Header  * psuHeader,
                               SuVideoF0_CurrMsg * psuCurrMsg)
    {
    int     iNextOffset;

    // Calculate the offset to the next video packet
    if (psuCurrMsg->psuChanSpec->bET == 1)
        {
        iNextOffset = 188 + sizeof(SuVideoF0_Header);
        psuCurrMsg->psuIPHeader = (SuVideoF0_Header *)
                                  ((char *)psuCurrMsg->psuIPHeader + iNextOffset);
        psuCurrMsg->pachTSData  = (uint8_t         *)
                                  ((char *)psuCurrMsg->pachTSData  + iNextOffset);
        }
    else
        {
        iNextOffset = 188;
        psuCurrMsg->psuIPHeader = (SuVideoF0_Header *)NULL;
        psuCurrMsg->pachTSData  = (uint8_t         *)
                                  ((char *)psuCurrMsg->pachTSData + iNextOffset);
        }

    // If new data pointer is beyond end of buffer then we're done
    if ((unsigned long)((char *)psuCurrMsg->pachTSData - (char *)psuCurrMsg->psuChanSpec) >= psuHeader->ulDataLen)
        return I106_NO_MORE_DATA;

    return I106_OK;
    }




/* ----------------------------------------------------------------------- */

#ifdef __cplusplus
} // end namespace Irig106
#endif
