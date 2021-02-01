/****************************************************************************

 i106_decode_discrete.c

 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libirig106.h"
#include "i106_util.h"
#include "i106_decode_discrete.h"


/* Function Declaration */

static void FillInMessagePointers(DiscreteF1_Message *msg);


I106Status I106_Decode_FirstDiscreteF1(I106C10Header * header, void * buffer,
        DiscreteF1_Message *msg, TimeRef * time){

    msg->BytesRead = 0;

    // Set pointers to the beginning of the Discrete buffer
    msg->CSDW = (DiscreteF1_CSDW *)buffer;

    msg->BytesRead += sizeof(DiscreteF1_CSDW);

    // Check for no data
    if (header->DataLength <= msg->BytesRead)
        return I106_NO_MORE_DATA;


    // Get the other pointers
    FillInMessagePointers(msg);

    FillInTimeStruct(header, msg->IPTS, time);

    return I106_OK;
}


I106Status I106_Decode_NextDiscreteF1(I106C10Header *header, DiscreteF1_Message *msg, TimeRef *time){

    // Check for no more data
    if (header->DataLength <= msg->BytesRead)
        return I106_NO_MORE_DATA;

    // Get the other pointers
    FillInMessagePointers(msg);

    FillInTimeStruct(header, msg->IPTS, time);

    return I106_OK;
}


void FillInMessagePointers(DiscreteF1_Message *msg){
    msg->IPTS = (IntraPacketTS *)((char *)(msg->CSDW) + msg->BytesRead);
    msg->BytesRead+=sizeof(msg->IPTS->IPTS);

    msg->Data = *( (uint32_t *) ((char *)(msg->CSDW) + msg->BytesRead));
    msg->BytesRead+=sizeof(msg->Data);
}
