/****************************************************************************

 i106_decode_can.c
 Created by: Tommaso Falchi Delitala <volalto86@gmail.com>

 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libirig106.h"
#include "i106_time.h"

#include "i106_decode_can.h"


/* Function Declaration */

static void FillInMessagePointers(CAN_Message *msg);


I106Status I106_Decode_FirstCAN(I106C10Header *header, void * buffer,
        CAN_Message *msg){

    msg->BytesRead = 0;

    // Keep a pointer to the current header
    msg->Header = header;

    // Set pointers to the beginning of the Message buffer
    msg->CSDW = (CAN_CSDW *)buffer;
    msg->BytesRead += sizeof(CAN_CSDW);

    // Check for no messages
    msg->MessageNumber = 0;
    if (msg->CSDW->Count == 0)
        return I106_NO_MORE_DATA;

    // Get the other pointers
    FillInMessagePointers(msg);

    FillInTimeStruct(header, msg->IPTS, &msg->Time);

    return I106_OK;
}


I106Status I106_Decode_NextCAN(CAN_Message *msg){

    // Check for no more messages
    msg->MessageNumber++;
    if (msg->MessageNumber >= msg->CSDW->Count)
        return I106_NO_MORE_DATA;

    // Check for packet overrun
    if (msg->Header->DataLength <= msg->BytesRead)
        return I106_NO_MORE_DATA;

    // Get the other pointers
    FillInMessagePointers(msg);

    FillInTimeStruct(msg->Header, msg->IPTS, &msg->Time);

    return I106_OK;
}


void FillInMessagePointers(CAN_Message *msg){
    // Set the pointer to the intra-packet time stamp
    msg->IPTS = (IntraPacketTS *)((char *)(msg->CSDW) + msg->BytesRead);

    // Set the pointer to the intra-packet header
    msg->IPH = (CAN_IPH *)((char *)(msg->CSDW) + msg->BytesRead);
    msg->BytesRead += sizeof(CAN_IPH);

    // Set pointer to the CAN ID word
    msg->ID = (CAN_ID*)((char *)(msg->CSDW) + msg->BytesRead);

    // Do not increment uBytesRead as MsgLength already include IdWord size

    // Set the pointer to the data
    // MsgLength = IdWord (4 bytes) + CAN Payload (0-4 bytes)
    msg->Data = (uint8_t *)((char *)(msg->CSDW) + msg->BytesRead);

    // Add the data length, if it is odd, account for the filler byte we will skip
    msg->BytesRead+=  msg->IPH->Length + (msg->IPH->Length % 2);
}
