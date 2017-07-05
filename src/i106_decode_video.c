/****************************************************************************

 i106_decode_video.c

 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "int.h"

#include "irig106ch10.h"
#include "i106_decode_video.h"

/* Function Declaration */

// Setup reading multiple Video Format 0 messages
I106Status I106_Decode_FirstVideoF0(I106C10Header *header, void *buffer,
        VideoF0_Message *msg){

    // Save pointer to channel specific data
    msg->CSDW = (VideoF0_CSDW *)buffer;

    // Set pointers if embedded time used
    if (msg->CSDW->ET == 1){
        msg->IPH = (VideoF0_IPH *)((char *)buffer + sizeof(VideoF0_CSDW));
        msg->Data = (uint8_t *)((char *)buffer + sizeof(VideoF0_CSDW) + sizeof(VideoF0_IPH));
    }

    // No embedded time
    else {
        msg->IPH = NULL;
        msg->Data  = (uint8_t *)buffer + sizeof(VideoF0_CSDW);
    }

    // TODO: BYTE SWAPPING BASED ON CH 10 RELEASE AND BA CSDW (NEW IN -09)

    return I106_OK;
}


I106Status I106_Decode_NextVideoF0 (I106C10Header *header, VideoF0_Message *msg){
    int next;

    // Calculate the offset to the next video packet
    if (msg->CSDW->ET == 1){
        next = 188 + sizeof(VideoF0_IPH);
        msg->IPH = (VideoF0_IPH *)((char *)msg->IPH + next);
        msg->Data  = (uint8_t *)((char *)msg->Data + next);
    }
    else {
        next = 188;
        msg->IPH = (VideoF0_IPH *)NULL;
        msg->Data = (uint8_t *)((char *)msg->Data + next);
    }

    // If new data pointer is beyond end of buffer then we're done
    if ((unsigned long)((char *)msg->Data - (char *)msg->CSDW) >= header->DataLength)
        return I106_NO_MORE_DATA;

    return I106_OK;
}
