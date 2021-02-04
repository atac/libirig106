/****************************************************************************

 i106_decode_arinc429.c

 ****************************************************************************/

#include "libirig106.h"
#include "i106_util.h"
#include "i106_arinc429.h"


/* Function Declaration */


I106Status I106_Decode_FirstArinc429F0(I106C10Header *header, void * buffer,
        Arinc429F0_Message * msg){

    // Save pointer to channel specific data
    msg->CSDW = (Arinc429F0_CSDW *)buffer;

    msg->Offset = sizeof(Arinc429F0_CSDW);

    // Check for no messages
    msg->MessageNumber = 0;
    if (msg->CSDW->Count == 0)
        return I106_NO_MORE_DATA;

    // Make the time for the current message
    TimeArray2LLInt(header->RTC, &(msg->IPTS));

    // Set pointers the header and data
    msg->IPH  = (Arinc429F0_IPH *)((char *)buffer + sizeof(Arinc429F0_CSDW));
    msg->Data = (Arinc429F0_Data *)((char *)(msg->IPH) + sizeof(Arinc429F0_IPH));

    return I106_OK;
}


I106Status I106_Decode_NextArinc429F0(Arinc429F0_Message *msg){

    // Check for no more messages
    msg->MessageNumber++;
    if (msg->MessageNumber >= msg->CSDW->Count)
        return I106_NO_MORE_DATA;

    // Figure out the offset to the next ARINC 429 message and
    // make sure it isn't beyond the end of the data buffer
    msg->Offset += sizeof(Arinc429F0_IPH) + sizeof(Arinc429F0_Data);

    // Set pointer to the next ARINC 429 header
    msg->IPH = (Arinc429F0_IPH *)((char *)msg->IPH + sizeof(Arinc429F0_IPH) + sizeof(Arinc429F0_Data));

    // Set pointer to the next ARINC 429 data buffer
    msg->Data = (Arinc429F0_Data *)((char *)(msg->IPH) + sizeof(Arinc429F0_IPH));

    // Make the time for the current message
    msg->IPTS += msg->IPH->GapTime;

    return I106_OK;
}
