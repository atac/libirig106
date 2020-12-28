/****************************************************************************

 i106_decode_index.c

 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libirig106.h"
#include "i106_time.h"
#include "i106_decode_index.h"


/* Function Declaration */

I106Status FillInMsgPtrs(IndexMsg *msg);


/* These Decode_First / Decode_Next routines take an index packet and return
 * successive root or node index entries.  Indexes are complicated with a lot
 * of optional components.  The IndexMsg will have pointers to packet
 * data that exists.  Optional index components that aren't in the current 
 * index packet will have their data pointers set to NULL.  It is important
 * to check the IndexMsg data pointers for the NULL value before trying
 * to use them.
 
 * Calls to decode a root index packet will fill in a non-NULL pointer for the 
 * RootInfo pointer until the end of the packet is reached.  The last call to 
 * decode a root index packet will fill in a NULL pointer for psuRootInfo, and 
 * will fill in a non-NULL pointer for psuNextRootOffset, the pointer to the 
 * offset of the next root packet.
 
 * Calls to decode a node index packet will fill in a non-NULL pointer for the
 * psuNodeInfo pointer. */

I106Status I106_Decode_FirstIndex(I106C10Header *header, void *buffer, IndexMsg *msg){
    int64_t offset = 0;

    // Null everything out to start
    memset(msg, 0 , sizeof(IndexMsg));

    // Set pointers to the beginning of the index buffer
    msg->CSDW = (IndexCSDW *)buffer;
    offset += sizeof(IndexCSDW);

    // If there is a file size then make a pointer to it
    if (msg->CSDW->FileSize == 0)
        msg->FileSize = NULL;
    else {
        msg->FileSize = (int64_t *)((char *)(buffer) + offset);
        offset += sizeof(int64_t);
    }
        
    // Set pointer to the beginning of the index array
    msg->IndexArray = (char *)(buffer) + offset;
    offset += sizeof(int64_t);

    // Check for no messages
    msg->MessageNumber = 0;
    if (msg->CSDW->Count == 0)
        return I106_NO_MORE_DATA;

    // Make sure the offset to the first index isn't beyond the end of the data buffer
    msg->DataLength = header->DataLength;
    if (offset >= msg->DataLength)
        return I106_BUFFER_OVERRUN;

    // Get the index data pointers
    return FillInMsgPtrs(msg);
}


I106Status I106_Decode_NextIndex(IndexMsg *msg){
    // Check for no more messages
    msg->MessageNumber++;
    if (msg->MessageNumber >= msg->CSDW->Count)
        return I106_NO_MORE_DATA;

    // Get the index data pointers
    return FillInMsgPtrs(msg);
}


/* Fill in the pointers to the various index packet message data items. There
 * are three different kinds of messages, a node index message, a root index
 * message, and an offset to the next root index packet. */
I106Status FillInMsgPtrs(IndexMsg *msg){
    I106Status status;

    // Process node index message
    if (msg->CSDW->IndexType == 1){
        // With optional secondary header time
        if (msg->CSDW->IPH == 1){
            msg->Time        = &(((IndexNodeMsgTime *)msg->IndexArray)[msg->MessageNumber].Time);
            msg->IPTS        = &(((IndexNodeMsgTime *)msg->IndexArray)[msg->MessageNumber].SecondaryTime);
            msg->FileOffset  = &(((IndexNodeMsgTime *)msg->IndexArray)[msg->MessageNumber].Offset);
            msg->NodeData    = &(((IndexNodeMsgTime *)msg->IndexArray)[msg->MessageNumber].Data);
        }

        // Without optional secondary header time
        else {
            msg->Time        = &(((IndexNodeMsg *)msg->IndexArray)[msg->MessageNumber].Time);
            msg->IPTS        = NULL;
            msg->FileOffset  = &(((IndexNodeMsg *)msg->IndexArray)[msg->MessageNumber].Offset);
            msg->NodeData    = &(((IndexNodeMsg *)msg->IndexArray)[msg->MessageNumber].Data);
        }
        status = I106_INDEX_NODE;
    }

    // Process root index messages
    else {
        // With optional secondary header time
        if (msg->CSDW->IPH == 1){
            msg->Time        = &(((IndexRootMsgTime *)msg->IndexArray)[msg->MessageNumber].Time);
            msg->IPTS        = &(((IndexRootMsgTime *)msg->IndexArray)[msg->MessageNumber].SecondaryTime);
            msg->FileOffset  = &(((IndexRootMsgTime *)msg->IndexArray)[msg->MessageNumber].Offset);
            msg->NodeData    = NULL;
        }

        // Without optional secondary header time
        else {
            msg->Time        = &(((IndexRootMsg *)msg->IndexArray)[msg->MessageNumber].Time);
            msg->IPTS        = NULL;
            msg->FileOffset  = &(((IndexRootMsg *)msg->IndexArray)[msg->MessageNumber].Offset);
            msg->NodeData    = NULL;
        }

        // If not the last message then it's a root index message
        if (msg->MessageNumber < msg->CSDW->Count - 1)
            status = I106_INDEX_ROOT;

        // Since it is the last message it's a pointer to next root packet
        else
            status = I106_INDEX_ROOT_LINK;
    }

    return status;
}
