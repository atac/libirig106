/****************************************************************************

 i106_decode_1553f1.c

 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stdint.h"

#include "libirig106.h"
#include "i106_decode_1553f1.h"


/* Function Declaration */

static void FillInMessagePointers(MS1553F1_Message *msg);


I106Status I106_Decode_First1553F1(I106C10Header *header, void *buffer,
        MS1553F1_Message *msg){

    // Set pointers to the beginning of the 1553 buffer
    msg->CSDW = (MS1553F1_CSDW *)buffer;

    // Check for no messages
    msg->MessageNumber = 0;
    if (msg->CSDW->MessageCount == 0)
        return I106_NO_MORE_DATA;

    // Check for too many messages. There was a problem with a recorder
    // that produced bad 1553 packets.  These bad packets showed *huge*
    // message counts, and all the data was garbage.  This test catches
    // this case.  Sorry.  8-(
    if (msg->CSDW->MessageCount > 100000)
        return I106_BUFFER_OVERRUN;

    // Figure out the offset to the first 1553 message and
    // make sure it isn't beyond the end of the data buffer
    msg->DataLength = header->DataLength;
    msg->Offset = sizeof(MS1553F1_CSDW);
    if (msg->Offset >= msg->DataLength)
        return I106_BUFFER_OVERRUN;

    // Set the pointer to the first 1553 message
    msg->IPH = (MS1553F1_IPH *) ((char *)(buffer) + msg->Offset);

    // Check to make sure the data does run beyond the end of the buffer
    if ((msg->Offset + sizeof(MS1553F1_IPH) + msg->IPH->Length) > msg->DataLength)
        return I106_BUFFER_OVERRUN;

    // Get the other pointers
    FillInMessagePointers(msg);

    return I106_OK;
}


I106Status I106_Decode_Next1553F1(MS1553F1_Message *msg){

    // Check for no more messages
    msg->MessageNumber++;
    if (msg->MessageNumber >= msg->CSDW->MessageCount)
        return I106_NO_MORE_DATA;

    // Figure out the offset to the next 1553 message and
    // make sure it isn't beyond the end of the data buffer
    msg->Offset += sizeof(MS1553F1_IPH) + msg->IPH->Length;

    if (msg->Offset >= msg->DataLength)
        return I106_BUFFER_OVERRUN;

    // Set pointer to the next 1553 data buffer
    msg->IPH = (MS1553F1_IPH *)
        ((char *)(msg->IPH) + sizeof(MS1553F1_IPH) + msg->IPH->Length);

    // Check to make sure the data does run beyond the end of the buffer
    if ((msg->Offset + sizeof(MS1553F1_IPH) + msg->IPH->Length) > msg->DataLength)
        return I106_BUFFER_OVERRUN;

    // Get the other pointers
    FillInMessagePointers(msg);

    return I106_OK;
}


void FillInMessagePointers(MS1553F1_Message *msg){
    msg->CommandWord1  = (CommandWordUnion *)((char *)(msg->IPH) +
            sizeof(MS1553F1_IPH));

    // Position of data and status response differ between transmit and receive
    // If not RT to RT
    if ((msg->IPH)->RT2RT == 0){
        // Second command and status words not available
        msg->CommandWord2  = NULL;
        msg->StatusWord2   = NULL;

        // Figure out the word count
        msg->WordCount = MS1553WordCount(msg->CommandWord1);

        // Receive
        if (msg->CommandWord1->CommandWord.TR == 0){
            msg->Data        = (uint16_t *)msg->CommandWord1 + 1;
            msg->StatusWord1 = msg->Data + msg->WordCount;
        }

        // Transmit
        else {
            msg->StatusWord1 = (uint16_t *)msg->CommandWord1 + 1;
            msg->Data        = (uint16_t *)msg->CommandWord1 + 2;
        }
    }

    // RT to RT
    else {
        msg->CommandWord2 = msg->CommandWord1 + 1;
        msg->WordCount    = MS1553WordCount(msg->CommandWord2);
        msg->StatusWord2  = (uint16_t *)msg->CommandWord1 + 2;
        msg->Data         = (uint16_t *)msg->CommandWord1 + 3;
        msg->StatusWord1  = (uint16_t *)msg->Data + msg->WordCount;
    }
}


char * GetCommandWord(unsigned int raw){
    static char    string[16];
    CommandWord  * command_word = (CommandWord *)&raw;

    sprintf(string, "%2d-%c-%2d-%2d",
         command_word->RT,
         command_word->TR ? 'T' : 'R',
         command_word->SubAddress,
         command_word->WordCount==0 ? 32 : command_word->WordCount);

    return &string[0];
}


/* Return the number of word in a 1553 message taking into account mode codes */
int MS1553WordCount(const CommandWordUnion *command_word){

    // If the subaddress is a mode code then find out number of data words
    if ((command_word->CommandWord.SubAddress == 0x0000) || (command_word->CommandWord.SubAddress == 0x001f)){
        return (int)command_word->CommandWord.WordCount & 0x0010;
    }

    // If regular subaddress find out number of data words
    else {
        if (command_word->CommandWord.WordCount == 0)
            return 32;
        else
            return command_word->CommandWord.WordCount;
    }
}
