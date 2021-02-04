/****************************************************************************

 i106_decode_pcmf1.c
 Author: Hans-Gerhard Flohr, Hasotec GmbH, www.hasotec.de
 2014/04/07 Initial Version 1.0
 2014/04/23 Version 1.1 
 Changes:   Inversing meaning of swap data bytes / words
            Correcting llintpkttime calculation if a new packet was received

 ****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <strings.h>
#endif
#ifndef __APPLE__
#include <malloc.h>
#endif
#include <ctype.h>
#include <assert.h>

#include "libirig106.h"
#include "i106_util.h"
#include "i106_tmats.h"
#include "i106_pcm.h"


/* Module data */

// For test with the data of a known (special) file 
#ifdef _DEBUG
    //#define DEBUG_OTHER_PCM_FILE
    #ifdef DEBUG_OTHER_PCM_FILE
        static FILE *FilePcmTest = NULL;
    #endif
#endif


/* Function Declaration */

// Local functions
I106Status PrepareNextDecodingRun_PCMF1(PCMF1_Message *msg);
void PrepareNewMinorFrameCollection_PCMF1(PCMF1_Attributes *attributes);
void GetNextBit_PCMF1(PCMF1_Message *msg, PCMF1_Attributes *attributes);
int IsSyncWordFound_PCMF1(PCMF1_Attributes *attributes);
void RenewSyncCounters_PCMF1(PCMF1_Attributes *attributes, uint64_t sync_count);


// Note; This code is tested only with Pcm in throughput mode


I106Status I106_Decode_FirstPCMF1(I106C10Header *header, char *buffer,
        PCMF1_Message *msg){
	uint32_t subpacket_length;
    uint32_t remainder;

    // Check for attributes available
    if (msg->Attributes == NULL)
        return I106_UNSUPPORTED;

    // Set pointers to the beginning of the Pcm buffer
    msg->Header = header; 
    msg->CSDW = (PCMF1_CSDW *)buffer; 

    msg->BytesRead = 0;
    msg->Length = header->DataLength;
    msg->BytesRead += sizeof(PCMF1_CSDW);

    // Check for no (more) data
    if (msg->Length <= msg->BytesRead)
        return I106_NO_MORE_DATA;

    // Save the time from the packet header
    TimeArray2LLInt(header->RTC, &(msg->IPTS_Base));

    // Some precalculations inclusive time
    if (msg->CSDW->Throughput){
        // Throughput mode, no intra packet header present
        msg->IPH = NULL;

        // Take the whole remaining data buffer as packet len
        msg->SubPacketLength = msg->Length - msg->BytesRead;

        // The IntPktTime is recalculated later from the bit position
        msg->IPTS = msg->IPTS_Base;
    }
    else {
        // Not throughput mode, an intra packet header must be present
        // NOTE: UNTESTED

        msg->IPH = (PCMF1_IPH *) ((char *)(msg->CSDW) + msg->BytesRead);
        msg->BytesRead += sizeof(PCMF1_IPH);

        // If there is no space for data
        if (msg->Length <= msg->BytesRead)
            return I106_NO_MORE_DATA;

        // Compute the padded framelen for the minor frame
        subpacket_length = msg->Attributes->BitsInMinorFrame;
        
        // 16 bit alignement
        if (!msg->CSDW->Alignment){
            remainder = subpacket_length & 0xf; // %16
            subpacket_length >>= 4; // /= 16;

            if(remainder)
                subpacket_length += 1;
            subpacket_length <<= 1; // * 2
        }
        
        // 32 bit alignement
        else {
            remainder = subpacket_length & 0x1f; // % 32
            subpacket_length >>= 5; // / 32

            if (remainder)
                subpacket_length += 1;
            subpacket_length <<= 2; // * 4
        }
        msg->SubPacketLength = subpacket_length; 

        // Fetch the time from the intra packet header
        FillInTimeStruct(header, (IntraPacketTS *)msg->IPH, &msg->Time);
        // and publish it   
        msg->IPTS = msg->Time.RTC;

    }


    // We continue with the throughput mode

    // Check for the amount of the remaining data including the length of the data
    if (msg->Length < msg->BytesRead + msg->SubPacketLength)
        return I106_NO_MORE_DATA;

    // Set the pointer to the Pcm message data
    msg->Data = (uint8_t *)((char *)(msg->CSDW) + msg->BytesRead);

    msg->BytesRead += msg->SubPacketLength;

    // For troughput mode
    msg->SubPacketBits = msg->SubPacketLength * 8;

    // Prepare the Pcm buffers and load the first bits
    if (msg->Attributes->PrepareNextDecodingRun){

        // Set up the data
        I106Status status = PrepareNextDecodingRun_PCMF1(msg);
        if (status != I106_OK)
            return status;
    }

    // Intra-packet not tested, so return
    if (!msg->CSDW->Throughput)
        return I106_UNSUPPORTED;

    if (!msg->Attributes->DontSwapRawData){
        if (SwapBytes_PCMF1(msg->Data, msg->SubPacketLength))
            return I106_INVALID_DATA;

        // Note: Untested 
        if (msg->CSDW->Alignment){
            if(SwapShortWords_PCMF1((uint16_t *)msg->Data, msg->SubPacketLength))
            return I106_INVALID_DATA; 
        }
    }

    // Now start the decode of this buffer
    msg->Attributes->BitPosition = 0;

    return DecodeMinorFrame_PCMF1(msg);
}


I106Status I106_Decode_NextPCMF1(PCMF1_Message *msg){

    if (msg->CSDW->Throughput)
        return DecodeMinorFrame_PCMF1(msg);

    // Check for no (more) data
    if (msg->Length < msg->BytesRead)
        return I106_NO_MORE_DATA;
    
    // If not thru mode, we must have an intrapacket header
    // NOTE: UNTESTED
    // May be, it points to outside ...
    msg->IPH = (PCMF1_IPH *) ((char *)(msg->CSDW) + msg->BytesRead);
    msg->BytesRead += sizeof(PCMF1_IPH);
    // ... so check, if it was successful
    if (msg->Length <= msg->BytesRead)
        return I106_NO_MORE_DATA;
            
    // TODO: Check time stamp, alignment, compute the the sub packet len etc
            
    // Fetch the time from the intra packet header
    FillInTimeStruct(msg->Header, (IntraPacketTS *)msg->IPH, &msg->Time);
    // and publish it   
    msg->IPTS = msg->Time.RTC;

    // Check for no more data (including the length of the minor frame)
    if (msg->Length < msg->BytesRead + msg->SubPacketLength)
        return I106_NO_MORE_DATA;

    // Set the pointer to the Pcm message data
    msg->Data = (uint8_t *)((char *)(msg->CSDW) + msg->BytesRead);

    msg->BytesRead += msg->SubPacketLength;

    return I106_OK;
}


// Fill the attributes from TMATS 
// TODO: Check if all needed definitions found
I106Status Set_Attributes_PCMF1(R_DataSource *r_datasource, PCMF1_Attributes *attributes){
    P_Record          * p_record;

    if (attributes == NULL)
        return I106_INVALID_PARAMETER;

    memset(attributes, 0, sizeof(PCMF1_Attributes));
    p_record = r_datasource->P_Record;

    if (p_record == NULL)
        return I106_INVALID_PARAMETER;

    // Collect the TMATS values
    // ------------------------
    // Essential values for throughput mode are: 
    //      szBitsPerSec / ulBitsPerSec // P-x\D2
    //      szCommonWordLen / ulCommonWordLen // P-x\F1
    //      szWordsInMinorFrame / ulWordsInMinorFrame // P-x\MF1
    //      szBitsInMinorFrame / ulBitsInMinorFrame // P-x\MF2
    //          if not existent, it is calculated from  ulCommonWordLen * (ulWordsInMinorFrame - 1) + ulMinorFrameSyncPatLen;
    //          Note: May be over-determined, no check for this
    //      szMinorFrameSyncPatLen / ulMinorFrameSyncPatLen // P-x\MF4
    //          if not existent, it is calculated from the number of bits in ulMinorFrameSyncPat
    //          Note: May be over-determined, no check for this
    //      szMinorFrameSyncPat / ulMinorFrameSyncPa) // P-x\MF5

    // Default values for throughput are taken for  
    //      szWordTransferOrder: "M" P-x\F2
    //      szParityType: "NO" P-x\F3
    //      szParityTransferOrder not "L" P-x/F4
    //      szMinorFrameSyncType P-x\MF3: "FPT"
    // Unneeded values for throughput are

    //      iRecordNum / iRecordNum // P-x
    //      szNumMinorFrames / ulNumMinorFrames  P-x\MF\N

    attributes->R_DataSource = r_datasource; // May be, we need it in the future

    attributes->RecordNumber = p_record->RecordNumber; // P-x

    if (p_record->BitsPerSecond != NULL)
        attributes->BitsPerSecond = atol(p_record->BitsPerSecond); // P-x\D2
    if (p_record->CommonWordLength != NULL)
        attributes->CommonWordLength = atol(p_record->CommonWordLength); // P-x\F1
    
    // P-x\F2 most significant bit "M", least significant bit "L". default: M
    if (p_record->WordTransferOrder != NULL){
        /*
        Measurement Transfer Order. Which bit is being transferred first is specified as Most Significant Bit (M), 
        Least Significant Bit (L), or Default (D). The default is specified in the P-Group - (P-x\F2:M).
        D-1\MN3-1-1:M;
        */
        if (p_record->WordTransferOrder[0] == 'L'){
            attributes->WordTransferOrder = PCM_LSB_FIRST;
            return I106_UNSUPPORTED;
        }
    }
    
    // P-x/F3
    if (p_record->ParityType != NULL){
        //even "EV", odd "OD", or none "NO", default: none
        if (strncasecmp(p_record->ParityType, "EV", 2) == 0) 
            attributes->ParityType = PCM_PARITY_EVEN;
        else if (strncasecmp(p_record->ParityType, "OD", 2) == 0) 
            attributes->ParityType = PCM_PARITY_EVEN; 
        else
            attributes->ParityType = PCM_PARITY_NONE;
    }

    if (p_record->ParityTransferOrder != NULL){
        if (strncasecmp(p_record->ParityType, "L", 1) == 0)    // P-x/F4
            attributes->ParityTransferOrder = 1;
        else
            attributes->ParityTransferOrder = 0;
    }
    if (p_record->NumberMinorFrames != NULL)
        attributes->MinorFrames = atol(p_record->NumberMinorFrames); // P-x\MF\N

    if (p_record->WordsInMinorFrame != NULL)
        attributes->WordsInMinorFrame = atol(p_record->WordsInMinorFrame); // P-x\MF1

    if (p_record->BitsInMinorFrame != NULL)
        attributes->BitsInMinorFrame = atol(p_record->BitsInMinorFrame); // P-x\MF2

    if (p_record->MinorFrameSyncType != NULL){
        // if not "FPT" : Error
        if (strncasecmp(p_record->MinorFrameSyncType, "FPT", 3) != 0) // P-x\MF3
            return I106_UNSUPPORTED;
        attributes->MinorFrameSyncType = 0;
    }

    if (p_record->MinorFrameSyncPatternLength != NULL)
        attributes->MinorFrameSyncPatternLength = atol(p_record->MinorFrameSyncPatternLength); // P-x\MF4

    // to declare that the system is in sync – First good sync (0), Check (1 or greater), or Not specified (NS).
    if (p_record->InSyncCritical != NULL)
        attributes->MinSyncs = 0; // Minimal number of syncs P-x\SYNC1;

    // P-x\MF5
    if (p_record->MinorFrameSyncPattern != NULL){
        uint64_t sync_pattern = 0;
        uint64_t sync_mask = 0;
        uint32_t minor_frame_sync_length = 0;
        char *pattern = p_record->MinorFrameSyncPattern;
        //Example: 0xFE6B2840
        //static char *xx = "11111110011010110010100001000000";
        //pChar = xx;
            
        // Skip leading blanks
        while (*pattern == ' ')
            pattern++;

        // Transfer the sync bits
        while ((*pattern == '0') || (*pattern == '1')){
            minor_frame_sync_length++;
            sync_mask <<= 1;
            sync_mask |= 1;
            
            sync_pattern <<= 1;
            if(*pattern == '1') 
                sync_pattern |= 1;
            pattern++;
        }
        attributes->MinorFrameSyncPattern = sync_pattern;
        attributes->MinorFrameSyncMask = sync_mask;
        if (attributes->MinorFrameSyncPatternLength == 0)
            attributes->MinorFrameSyncPatternLength = minor_frame_sync_length;
    }
        
    // Some post processing
    if (attributes->BitsInMinorFrame == 0){
        attributes->BitsInMinorFrame = attributes->CommonWordLength * (attributes->WordsInMinorFrame - 1)
            + attributes->MinorFrameSyncPatternLength;
    }
    for (uint32_t i = 0; i < attributes->CommonWordLength; i++){
        attributes->CommonWordMask <<= 1;
        attributes->CommonWordMask |= 1;
    }
        
    attributes->Delta100NanoSeconds = d100NANOSECONDS / attributes->BitsPerSecond;
        
    attributes->PrepareNextDecodingRun = 1; // Set_Attributes_PcmF1
        
    return I106_OK;
}


// Fill the attributes from an external source
// Replace the correspondent TMATS values, if the argument value is >= 0
I106Status Set_Attributes_Ext_PCMF1(R_DataSource *r_datasource,
        PCMF1_Attributes *attributes,
        int32_t record_number,             // P-x
        int32_t bits_per_second,           // P-x\D2
        int32_t common_word_length,        // P-x\F1
        int32_t word_transfer_order,       // P-x\F2
        int32_t parity_type,               // P-x\F3
        int32_t parity_transfer_order,     // P-x\F4
        int32_t minor_frames,              // P-x\MF\N
        int32_t words_in_minor_frame,      // P-x\MF1
        int32_t bits_in_minor_frame,       // P-x\MF2
        int32_t minor_frame_sync_type,     // P-x\MF3
        int32_t minor_frame_sync_length,   // P-x\MF4
        int64_t minor_frame_sync_pattern,  // P-x\MF5
        int32_t min_syncs,                 // P-x\SYNC1
        int64_t minor_frame_sync_mask,
        int32_t no_byte_swap){

    if(r_datasource == NULL)
        return I106_INVALID_PARAMETER;

    if(attributes == NULL)
        return I106_INVALID_PARAMETER;

    // Transfer the external data
    if(record_number != -1)
        attributes->RecordNumber = record_number;
    if(bits_per_second != -1)
        attributes->BitsPerSecond = bits_per_second;
    if(common_word_length != -1)
        attributes->CommonWordLength = common_word_length;
    if(word_transfer_order != -1)
        attributes->WordTransferOrder = word_transfer_order;
    if(parity_type != -1)
        attributes->ParityType = parity_type;
    if(parity_transfer_order != -1)
        attributes->ParityTransferOrder = parity_transfer_order;
    if(minor_frames != -1)
        attributes->MinorFrames = minor_frames;
    if(words_in_minor_frame != -1)
        attributes->WordsInMinorFrame = words_in_minor_frame;
    if(bits_in_minor_frame != -1)
        attributes->BitsInMinorFrame = bits_in_minor_frame;
    if(minor_frame_sync_type != -1)
        attributes->MinorFrameSyncType = minor_frame_sync_type;
    if(minor_frame_sync_length != -1)
        attributes->MinorFrameSyncPatternLength = minor_frame_sync_length;
    if(minor_frame_sync_pattern != -1)
        attributes->MinorFrameSyncPattern = minor_frame_sync_pattern;
    if(minor_frame_sync_mask != -1)
        attributes->MinorFrameSyncMask = minor_frame_sync_mask;
    if(min_syncs != -1)
        attributes->MinSyncs = min_syncs;
    if(no_byte_swap != -1)
        attributes->DontSwapRawData = no_byte_swap;

    attributes->CommonWordMask = 0;
    for(uint32_t i = 0; i < attributes->CommonWordLength; i++){
         attributes->CommonWordMask <<= 1;
         attributes->CommonWordMask |= 1;
    }

    attributes->CommonWordMask &= attributes->MinorFrameSyncMask;
    attributes->Delta100NanoSeconds = d100NANOSECONDS / attributes->BitsPerSecond;
    attributes->PrepareNextDecodingRun = 1;

    return I106_OK;
}


// Create the output buffers for a minor frame (data and error flags)
I106Status CreateOutputBuffers_PCMF1(PCMF1_Attributes *attributes){

    // Allocate the Pcm output buffer for a minor frame
    attributes->BufferSize = attributes->WordsInMinorFrame;
    attributes->Buffer = (uint64_t *)calloc(attributes->BufferSize, sizeof(uint64_t));
    if (attributes->Buffer == NULL)
        return I106_BUFFER_TOO_SMALL;
    
    attributes->BufferError = (uint8_t *)calloc(attributes->BufferSize, sizeof(uint8_t));
    if (attributes->BufferError == NULL){
        free(attributes->Buffer);
        attributes->Buffer = NULL;
        return I106_BUFFER_TOO_SMALL;
    }

    return I106_OK;
}


// Free the output buffers for a minor frame
I106Status FreeOutputBuffers_PCMF1(PCMF1_Attributes *attributes){

    if (attributes->Buffer){
        free(attributes->Buffer);
        attributes->Buffer = NULL;
    }
    if (attributes->BufferError){
        free(attributes->BufferError);
        attributes->BufferError = NULL;
    }
    attributes->PrepareNextDecodingRun = 1; 

    return I106_OK;
}


// Prepare a new decoding run 
// Creates the output buffers and resets values and counters
I106Status PrepareNextDecodingRun_PCMF1(PCMF1_Message *msg){
    PCMF1_Attributes *attributes = msg->Attributes;

    I106Status status = CreateOutputBuffers_PCMF1(attributes);
    if(status != I106_OK)
        return status;

    attributes->PrepareNextDecodingRun = 0;
    
    // If not throughput mode, the work is done 
    if(!msg->CSDW->Throughput)
        return I106_OK;

    // Prepare the variables for bit decoding in throughput mode
    // --------------------------------------------------------
    attributes->SyncCount = -1; // -1 sets all bits to 1
    attributes->SyncErrors = 0;
    attributes->TestWord = 0; 
    attributes->BitPosition = 0; 
    attributes->BitsLoaded = 0;
    // Nearly the same as in RenewSyncCounter...
    attributes->MinorFrameBitCount = 0;
    attributes->MinorFrameWordCount = 0;
    attributes->DataWordBitCount = 0;
    attributes->SaveData = 0;

    return I106_OK;
}


I106Status DecodeMinorFrame_PCMF1(PCMF1_Message *msg){
    PCMF1_Attributes * attributes = msg->Attributes;

    while (attributes->BitPosition < msg->SubPacketBits){

        GetNextBit_PCMF1(msg, attributes);

        // Check for a sync word

        if (IsSyncWordFound_PCMF1(attributes)){   
            // Prevent an overflow after a terabyte of bits
            if (attributes->BitsLoaded > 1000000000000)
                attributes->BitsLoaded = 1000000000;

            attributes->SyncCount++;

            if (attributes->MinorFrameBitCount == attributes->BitsInMinorFrame){
                // A sync word found at the correct offset to the previous one

                RenewSyncCounters_PCMF1(attributes, attributes->SyncCount); // with the current sync counter
                
                // If there are enough syncs, release the previous filled buffer
                // Note: a minor frame is released only, if it is followed by a sync word at the correct offset. 
                // i.e. the sync word are used as brackets
                if((attributes->SyncCount >= attributes->MinSyncs) && (attributes->SaveData > 1)){

                    // Compute the intrapacket time of the start sync bit position in the current buffer
                    int64_t bit_position = (int64_t)attributes->BitPosition - (int64_t)attributes->BitsInMinorFrame;

                    double ipts_offset = (double)bit_position * attributes->Delta100NanoSeconds;   

                    msg->IPTS = msg->IPTS_Base + (int64_t)ipts_offset; // Relative time, omit rounding

                    // Prepare for the next run
                    PrepareNewMinorFrameCollection_PCMF1(attributes);
                    return I106_OK;
                }
            }
            else {
                // Note: a wrong offset is also at the first sync in the whole decoding run

                // Save the sync error for statistics
                if(attributes->SyncCount > 0)
                    attributes->SyncErrors++;

                // RenewSyncCounters_PcmF1 with a sync counter of zero
                RenewSyncCounters_PCMF1(attributes, 0);
            }

            PrepareNewMinorFrameCollection_PCMF1(attributes);
            continue;
        }

        // Collect the data

        if (attributes->SaveData == 1){
            attributes->DataWordBitCount++;
            if (attributes->DataWordBitCount >= attributes->CommonWordLength){
                attributes->Buffer[attributes->MinorFrameWordCount - 1] = attributes->TestWord;
                attributes->DataWordBitCount = 0;
                attributes->MinorFrameWordCount++;
            }
        }

        // Don't release the data here but wait for a trailing sync word. 
        if (attributes->MinorFrameWordCount >= attributes->WordsInMinorFrame)
            attributes->SaveData = 2;

    }

    // Preset for the next run
    attributes->BitPosition = 0;

    return I106_NO_MORE_DATA;
}


// Prepare a new minor frame collection
void PrepareNewMinorFrameCollection_PCMF1(PCMF1_Attributes *attributes){
    attributes->DataWordBitCount = 0;
    attributes->SaveData = 1;
}


// Get the next bit
void GetNextBit_PCMF1(PCMF1_Message *msg, PCMF1_Attributes *attributes){
    attributes->TestWord <<= 1;
    if(IsBitSetL2R(msg->Data, attributes->BitPosition))
        attributes->TestWord |= 1;
    attributes->BitsLoaded++;
    attributes->MinorFrameBitCount++;
    attributes->BitPosition++;
}


// Check for a sync word
int IsSyncWordFound_PCMF1(PCMF1_Attributes *attributes){
    return
        (attributes->BitsLoaded >= attributes->MinorFrameSyncPatternLength) && 
        (attributes->TestWord & attributes->MinorFrameSyncMask) == attributes->MinorFrameSyncPattern;
}


// RenewSyncCounters_PcmF1
void RenewSyncCounters_PCMF1(PCMF1_Attributes *attributes, uint64_t sync_count){
    attributes->MinorFrameBitCount = 0; 
    attributes->MinorFrameWordCount = 1; // Note the 1: this is the sync word
    attributes->DataWordBitCount = 0;
    attributes->SyncCount = sync_count;
}


// Returns I106_OK on success, I106_INVALID_DATA on error
I106Status CheckParity_PCMF1(uint64_t test_word, int word_length, int parity_type,
        int parity_transfer_order){
    uint64_t test_bit    = 1;
    unsigned int bit_sum = 0;

    switch(parity_type){
        case PCM_PARITY_NONE:
            break;
        case PCM_PARITY_EVEN:
            while (word_length-- > 0){
                if (test_word & test_bit)
                    bit_sum++;
                test_bit <<= 1;
            }
            if (bit_sum & 1)
                return I106_INVALID_DATA;
            break;
        case PCM_PARITY_ODD:
            while (word_length-- > 0) {
                if (test_word & test_bit)
                    bit_sum++;
                test_bit <<= 1;
            }
            if (!(bit_sum & 1))
                return I106_INVALID_DATA;
            break;
        default:
            break;
    }
    return I106_OK;
}


// Swaps "bytes" bytes in place
I106Status SwapBytes_PCMF1(uint8_t *buffer, long bytes){
    uint32_t data = 0x03020100;
    uint8_t tmp;

    if (bytes & 1)
        return I106_BUFFER_OVERRUN; // May be also an underrun ...
    while ((bytes -= 2) >= 0){
        tmp = *buffer;
        *buffer = *(buffer + 1);
        *++buffer = tmp;
        buffer++;
    }
    SwapShortWords_PCMF1((uint16_t *)&data, 4);

    return I106_OK;
}


// Swaps "bytes" bytes of 16 bit words in place
I106Status SwapShortWords_PCMF1(uint16_t *buffer, long bytes){
    uint16_t tmp;

    if (bytes & 3)
        return I106_BUFFER_OVERRUN; // May be also an underrun ...

    bytes >>= 1;
    while ((bytes -= 2) >= 0){
        tmp = *buffer;
        *buffer = *(buffer + 1);
        *++buffer = tmp;
        buffer++;
    }
    return I106_OK;
}
