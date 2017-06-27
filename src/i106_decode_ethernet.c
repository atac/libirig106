/****************************************************************************

 i106_decode_ethernet.c

 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "int.h"

#include "irig106ch10.h"
#include "i106_decode_ethernet.h"


/* Function Declaration */

void FillInMessagePointers(EthernetF0_Message *msg);


I106Status I106_Decode_FirstEthernetF0(I106C10Header *header, void *buffer, EthernetF0_Message *msg){

    // Set pointers to the beginning of the Ethernet buffer
    msg->CSDW = (EthernetF0_CSDW *)buffer;

    // Check for no messages
    msg->FrameNumber = 0;
    if (msg->CSDW->Frames == 0)
        return I106_NO_MORE_DATA;

    msg->Length = header->DataLength;
 
    // Set the pointer to the first Ethernet message
    msg->IPH = (EthernetF0_IPH *)((char *)(buffer) + sizeof(EthernetF0_CSDW));

    // Set the pointer to the ethernet message data
    msg->Data = (uint8_t *)msg->IPH + sizeof(EthernetF0_IPH);

    return I106_OK;
}


I106Status I106_Decode_NextEthernetF0(EthernetF0_Message *msg){

    // Check for no more messages
    msg->FrameNumber++;
    if (msg->FrameNumber >= msg->CSDW->Frames)
        return I106_NO_MORE_DATA;

    // Set pointer to the next ethernet message intrapacket header
    // Note that the next packet header must fall on an even byte boundary
    msg->IPH = (EthernetF0_IPH *)((char *)(msg->IPH) + sizeof(EthernetF0_IPH)
            + msg->IPH->Length + (msg->IPH->Length % 2));

    // Set the pointer to the ethernet message data
    msg->Data = (uint8_t *)msg->IPH + sizeof(EthernetF0_IPH);

    return I106_OK;
}
