/****************************************************************************

 i106_decode_analogf1.c
 
 Author: Spencer Hatch, Dartmouth College, Hanover, NH, USA
 *STOLEN* from Hans-Gerhard Flohr's i106_decode_analogf1.c
 2014 NOV Initial Version 1.0

 ****************************************************************************/


#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "i106_decode_analogf1.h"
#include "i106_decode_tmats.h"
#include "i106_time.h"
#include "int.h"
#include "util.h"
#include "irig106ch10.h"


/* Function Declaration */

// Local functions
I106Status PrepareNextDecodingRun_AnalogF1(AnalogF1_Message *msg);


// Print out analog information.
I106Status I106_Setup_AnalogF1(I106C10Header *header, void *buffer,
        AnalogF1_Message *msg){

    AnalogF1_Attributes  * attributes;

    // Check for attributes available
    if (msg->Attributes == NULL)
        return I106_INVALID_PARAMETER;

    attributes = msg->Attributes;
    
    // Set pointers to the beginning of the Analog buffer
    msg->Header = header; 
    msg->CSDW = (AnalogF1_CSDW *)buffer; 

    // Set variables for reading
    msg->BytesRead = 0;
    msg->Length = header->DataLength;
    
    // Make sure we're using packed analog data (others unsupported!)
    if(msg->CSDW->Mode != ANALOG_PACKED)
        return I106_UNSUPPORTED;
    
    // Check whether number of subchannels reported by TMATS matches number
    // reported by CSDW
    if(attributes->ChannelsPerPacket != msg->CSDW->Subchannels){
        fprintf(stderr, "TMATS # of subchannels reported does not match CSDW \
total number of channels!\n");
        return I106_INVALID_DATA;
    }
    
    // Based on first CSDWs 'Same' bit, prepare to allocate additional CSDW structs
    for (int i = 0; i < msg->CSDW->Subchannels; i++){

        // Allocate memory for _pointers_ to all CSDWs and allocate memory for
        // CSDWs themselves
        attributes->Subchannels[i] = malloc(sizeof(AnalogF1_Subchannel));

        attributes->Subchannels[i]->CSDW = malloc(sizeof(AnalogF1_CSDW));

        // Open and set name of subchan outfile, allocate subchan buffer
        sprintf(attributes->Subchannels[i]->OutputFilename,
                "%s--Analog_Subchan%i.dmpanalog",
                attributes->DataSourceID, i);
        printf("Opening subchannel output file %s...\n",
                attributes->Subchannels[i]->OutputFilename);

        // TODO: NEED TO FREE ALL MALLOC'ed MEM IF THINGS GO AWRY
        if((attributes->Subchannels[i]->OutputFile =
                    fopen(attributes->Subchannels[i]->OutputFilename, "w")) == NULL)
            return I106_OPEN_ERROR;

        
        if((attributes->Subchannels[i]->Data = calloc(1, msg->Length)) == NULL)
            return I106_BUFFER_TOO_SMALL;
	
        // Copy CSDW into allocated mem for future reference
        // TODO: I have not tested situations where bSame is bFALSE!
        if(msg->CSDW->Same == 0)
     	    memcpy(attributes->Subchannels[i]->CSDW, &(msg->CSDW[i]),
                    sizeof(AnalogF1_CSDW));
        else {
            attributes->Subchannels[i]->CSDW = malloc(sizeof(AnalogF1_CSDW));
	    
            // Copy CSDW into allocated mem for future reference
            memcpy(attributes->Subchannels[i]->CSDW, &(msg->CSDW[0]),
                    sizeof(AnalogF1_CSDW));
        }
 
        // Update bytes read outside do loop
        if (msg->CSDW->Same)
            msg->BytesRead += sizeof(AnalogF1_CSDW);
        else
            msg->BytesRead += sizeof(AnalogF1_CSDW) * attributes->ChannelsPerPacket;
        
        PrintCSDW_AnalogF1(attributes->Subchannels[i]->CSDW);
    };

    // Bubble sort pointers to subchannels
    // This is necessary (for this implementation) in order to comply with the
    // organization of analog data packets as described in Ch.10
    for (int i = 0; i < msg->CSDW->Subchannels; i++){
        AnalogF1_Subchannel *swap;

        for (int j = 0; j < msg->CSDW->Subchannels - j - 1; j++){
            int sub1 = attributes->Subchannels[j]->CSDW->Subchannel;
            int sub2 = attributes->Subchannels[j + 1]->CSDW->Subchannel;

            if (sub1 > sub2){
                swap = attributes->Subchannels[j];
                attributes->Subchannels[j] = attributes->Subchannels[j+1];
                attributes->Subchannels[j + 1] = swap;
            }
        }
    }

    return I106_OK;
}


I106Status I106_Decode_FirstAnalogF1(I106C10Header *header, void *buffer,
        AnalogF1_Message *msg){
    AnalogF1_Attributes  * attributes;
    
    // Check for attributes available
    if(msg->Attributes == NULL)
        return I106_INVALID_PARAMETER;

    attributes = msg->Attributes;
    
    // Set pointers to the beginning of the Analog buffer
    msg->Header = header;
    msg->CSDW = (AnalogF1_CSDW *)buffer; 

    // Set variables for reading
    msg->BytesRead = 0;
    msg->Length = header->DataLength;
    
    // Make sure we're using packed analog data (others unsupported!)
    if(msg->CSDW->Mode != ANALOG_PACKED){
        fprintf(stderr, "Unpacked analog data not supported!\n");
        return I106_UNSUPPORTED;
    }    

    // Check whether number of subchannels reported by TMATS matches number
    // reported by CSDW
    if(attributes->ChannelsPerPacket != msg->CSDW->Subchannels){
        fprintf(stderr, "TMATS # of subchannels reported does not match CSDW \
total number of channels!\n");
        return I106_INVALID_DATA;
    }

    for (int i = 0; i < msg->CSDW->Subchannels; i++){
        // Check current CSDW against the first one received
        // TODO: I have not tested situations where bSame is bFALSE!
        if((attributes->Subchannels[i]->CSDW->Mode != msg->CSDW[i].Mode)
                || (attributes->Subchannels[i]->CSDW->Length != msg->CSDW[i].Length )
                || (attributes->Subchannels[i]->CSDW->Subchannels !=
                    msg->CSDW[i].Subchannels )){
            fprintf(stderr, "ERROR for subchannel %i on analog channel %s: \
Current CSDW does not match initial CSDW\n",
                    attributes->Subchannels[i]->CSDW->Subchannel,
                    attributes->DataSourceID);
            return I106_INVALID_DATA;
        }
    };

    // Update bytes read outside do loop
    if (msg->CSDW->Same)
        msg->BytesRead += sizeof(AnalogF1_CSDW);
    else
        msg->BytesRead += sizeof(AnalogF1_CSDW) * attributes->ChannelsPerPacket;

    // Check for no (more) data
    if (msg->Length <= msg->BytesRead)
        return I106_NO_MORE_DATA;

    // Set the pointer to the Analog message data
    msg->Data = (uint8_t *)((char *)(msg->CSDW) + msg->BytesRead);
    
    // Prepare the Analog buffers and load the first bits
    if (attributes->PrepareNextDecodingRun){
        // Set up the data
        I106Status status = PrepareNextDecodingRun_AnalogF1(msg);
        if (status != I106_OK)
            return status;
    }

    // Now start the decode of this buffer
    return I106_Decode_NextAnalogF1(msg);
}


// Fill the attributes from TMATS 
// TODO: implement needed attributes parsing in TMATS module
I106Status Set_Attributes_AnalogF1(R_DataSource *r_datasource,
        AnalogF1_Attributes *attributes){

    if (attributes == NULL)
        return I106_INVALID_PARAMETER;

    memset(attributes, 0, sizeof(AnalogF1_Attributes));

    // Collect the TMATS values
    
    // May be, we need it in the future
    attributes->R_Datasource      = r_datasource; 
    attributes->DataSourceID      = r_datasource->DataSourceID;
    attributes->DataSourceNumber  = r_datasource->DataSourceNumber; // R-x

    // Get number of chans per packet
    if (r_datasource->AnalogChannelsPerPacket != NULL)
        attributes->ChannelsPerPacket = atoi(r_datasource->AnalogChannelsPerPacket);

    // Get sample rate
    if (r_datasource->AnalogSampleRate != NULL)
        attributes->SampleRate = strtoull(r_datasource->AnalogSampleRate, NULL, 10);
    
    // Get whether data is packed
    if (r_datasource->AnalogDataPacking != NULL)
        attributes->Packed = (uint32_t) *r_datasource->AnalogDataPacking;

    // Get size of a data sample on this channel
    /* if (r_datasource->AnalogDataLength != NULL) */
    /*    attributes->ulAnalogDataLength = strtoul(psuRDataSrc->szAnalogDataLength, NULL, 10); */

    /* if (r_datasource->szAnalogMeasTransfOrd != NULL)    // R-x\AMTO-n-m most significant bit "M", least significant bit "L". default: M */
    /* { */
    /*     /1* Measurement Transfer Order. Which bit is being transferred first is specified as – Most Significant Bit (M), */ 
    /*     Least Significant Bit (L), or Default (D). */
    /*     D-1\MN3-1-1:M; */
    /*     *1/ */
    /*     if(psuRDataSrc->szAnalogMeasTransfOrd[0] == 'L') */
    /*     { */
    /*         psuAnalogF1_Attributes->ulAnalogMeasTransfOrd = ANALOG_LSB_FIRST; */
    /*         return(I106_UNSUPPORTED); */
    /*     } */
    /* } */

    //Get Sample Filter 3dB Bandwidth (in Hz)
    /* if(r_datasource->szAnalogSampleFilter != NULL) */
    /*    psuAnalogF1_Attributes->ullAnalogSampleFilter = strtoull(psuRDataSrc->szAnalogSampleFilter, NULL, 10); */

    //Get whether AC/DC Coupling
    /* if(r_datasource->szAnalogIsDCCoupled != NULL) */
    /*   psuAnalogF1_Attributes->bAnalogIsDCCoupled = psuRDataSrc->bAnalogIsDCCoupled; */

    //Get Recorder Input Impedance 
    /* if(r_datasource->szAnalogRecImpedance != NULL) */
    /*    psuAnalogF1_Attributes->ulAnalogRecImpedance = strtoul(psuRDataSrc->szAnalogRecImpedance, NULL, 10); */

    //Get Channel Gain in milli units (10x = 010000)
    /* if(r_datasource->szAnalogChanGain != NULL) */
    /*    psuAnalogF1_Attributes->ulAnalogChanGain = strtoul(psuRDataSrc->szAnalogChanGain, NULL, 10); */

    //Get Full-Scale Range (in milliVolts)
    /* if(r_datasource->szAnalogFullScaleRange != NULL) */
    /*    psuAnalogF1_Attributes->ulAnalogFullScaleRange = strtoul(psuRDataSrc->szAnalogFullScaleRange, NULL, 10); */

    //Get Offset Voltage (in milliVolts)
    /* if(r_datasource->szAnalogOffsetVoltage != NULL) */
    /*    psuAnalogF1_Attributes->lAnalogOffsetVoltage = strtoul(psuRDataSrc->szAnalogOffsetVoltage, NULL, 10); */

    //Get LSB Value
    /* if(r_datasource->szAnalogLSBValue != NULL) */
    /*    psuAnalogF1_Attributes->lAnalogLSBValue = strtoul(psuRDataSrc->szAnalogLSBValue, NULL, 10); */

    //Get Analog Format 
    //"1" = One's comp. 
    //"2" = Two's comp.
    //"3" = Sign and magnitude binary [+=0]
    //"4" = Sign and magnitude binary [+=1]
    //"B" = Offset binary
    //"U" = Unsigned binary
    //"F" = IEEE 754 single-precision [IEEE 32] floating point
    /* if(psuRDataSrc->szAnalogFormat != NULL) */
    /*   switch ( psuRDataSrc->szAnalogFormat[0] ) */
    /*   { */
    /*   case '1': */
	/* psuAnalogF1_Attributes->ulAnalogFormat = ANALOG_FMT_ONES; */
	/* break; */
    /*   case '2': */
	/* psuAnalogF1_Attributes->ulAnalogFormat = ANALOG_FMT_TWOS; */ 
	/* break; */
    /*   case '3': */
	/* psuAnalogF1_Attributes->ulAnalogFormat = ANALOG_FMT_SIGNMAG_0; */
	/* break; */
    /*   case '4': */
	/* psuAnalogF1_Attributes->ulAnalogFormat = ANALOG_FMT_SIGNMAG_1; */
	/* break; */
    /*   case 'B': */
	/* psuAnalogF1_Attributes->ulAnalogFormat = ANALOG_FMT_OFFSET_BIN; */
	/* break; */
    /*   case 'U': */
	/* psuAnalogF1_Attributes->ulAnalogFormat = ANALOG_FMT_UNSIGNED_BIN; */
	/* break; */
    /*   case 'F': */
	/* psuAnalogF1_Attributes->ulAnalogFormat = ANALOG_FMT_SINGLE_FLOAT; */
	/* break; */
    /*   default: */
	/* return(I106_UNSUPPORTED); */
    /*     break; */
    /*   } */

    //Get analog input type; 'D' = differential, 'S' = single-ended
    /* if(psuRDataSrc->szAnalogDifferentialInp != NULL) */
    /*    psuAnalogF1_Attributes->bAnalogDifferentialInp = psuRDataSrc->bAnalogDifferentialInp; */
 
    //Get whether audio
    /* if(psuRDataSrc->szAnalogIsAudio != NULL) */
    /*    psuAnalogF1_Attributes->bAnalogIsAudio = psuRDataSrc->bAnalogIsAudio; */
        
    attributes->PrepareNextDecodingRun = 1; // Set_Attributes_AnalogF1
        
    return I106_OK;
}


// Create the output buffers (data and error flags)
I106Status CreateOutputBuffers_AnalogF1(AnalogF1_Attributes *attributes,
        uint32_t data_length){

    // Allocate the Analog output buffer
    attributes->BufferSize = data_length;
    attributes->Buffer = (uint8_t *)calloc(sizeof(uint8_t), data_length);
    if (attributes->Buffer == NULL){
        free(attributes->Buffer);
        attributes->Buffer = NULL;
        return I106_BUFFER_TOO_SMALL;
    }

    return I106_OK;
}


// Free the output buffers
I106Status FreeOutputBuffers_AnalogF1(AnalogF1_Attributes *attributes){
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
I106Status PrepareNextDecodingRun_AnalogF1(AnalogF1_Message *msg){
    AnalogF1_Attributes *attributes = msg->Attributes;

    I106Status status = CreateOutputBuffers_AnalogF1(attributes, msg->Length);
    if (status != I106_OK)
        return status;

    attributes->PrepareNextDecodingRun = 0;
    attributes->SaveData = 0;

    return I106_OK;
}


// TODO: Implement reading of MSB- or LSB-padded packets
// Currently only supports packed data
I106Status I106_Decode_NextAnalogF1(AnalogF1_Message *msg){
    //Use these arrays to save on time
    AnalogF1_Attributes  * attributes = msg->Attributes;
    uint32_t               sample_factors[ANALOG_MAX_SUBCHANS];
    uint32_t               sample_sizes[ANALOG_MAX_SUBCHANS];
    uint32_t               subchannels = attributes->ChannelsPerPacket;
    AnalogF1_Subchannel  * subchannel;
    int32_t                max_simultaneous = 0;

    if (attributes->Packed) {
        // Need to distinguish between subchannels if not identical
        // TODO: finish implementation and testing of Same == 0
        if (msg->CSDW->Same == 0){
        
            // Calculate all factors for each channel (done here for speed)
            // Also get max number of simultaneous samples (See pp 56-57 in
            // IRIG-106 Ch10 June 2013 rev.) Also ensure all sample sizes an
            // integer factor of 8
            for (int i = 0; i < subchannels; i++){
                subchannel = msg->Attributes->Subchannels[i];
                sample_factors[i] = 0;
                if (subchannel->CSDW->Factor > 0){
                    for (int j = 0; j < subchannel->CSDW->Factor; j++){
                        sample_factors[i] *= 2;
                    }
                    if (max_simultaneous < sample_factors[i]){
                        max_simultaneous = sample_factors[i];
                    }
                }
                // Now get sample sizes for each subchannel
                sample_sizes[i] = subchannel->CSDW->Length;
                if (sample_sizes[i] % 8 != 0)
                    return I106_UNSUPPORTED;
            }
            
            while (msg->BytesRead < msg->Length){
                for (int32_t i = 1; i < max_simultaneous; i++){
                    for (int j = 0; j < subchannels; j++){
                        if (max_simultaneous == sample_factors[j]){
                            subchannel = msg->Attributes->Subchannels[j];
                            subchannel->BytesRead += sample_sizes[j];
                            msg->BytesRead += sample_sizes[j];
                        }
                    }
                }
            }
        }
        else {
            if (subchannels > 1){
                while (msg->BytesRead < msg->Length ){
                    for (int i = 0; i < subchannels; i++){
                        subchannel = msg->Attributes->Subchannels[i];
                        subchannel->BytesRead += sample_sizes[i];
                        msg->BytesRead += sample_sizes[i];
                    }
                }      
            }
            else {
                subchannel = msg->Attributes->Subchannels[0];
                // Code to write all data to sampbuff
                if (((msg->Length - msg->BytesRead ) %
                            (subchannel->CSDW->Length / 8)) != 0 ){

                    printf("Remaining databuff doesn't allow for clean copy!\n");
                    return I106_INVALID_DATA;
                }

                // TODO: why are we writing here?
                /* int bytes_written = fwrite(msg->Data + msg->BytesRead, 1, */
                /*         msg->Length - msg->BytesRead, subchannel->OutputFile); */
                /* printf("Wrote %i bytes to %s\n", bytes_written, subchannel->OutputFilename); */

                msg->BytesRead = msg->Length;
            }
        }
    
    }
    else {
        fprintf(stderr,
                "Unpacked analog data is currently unsupported!\nEnding...\n");
        return I106_UNSUPPORTED;
    }

    return I106_NO_MORE_DATA;
}


I106Status PrintCSDW_AnalogF1(AnalogF1_CSDW *csdw){
    printf("Subchannel number:\t\t %" PRIu32 "\n", csdw->Subchannel);
    printf("Mode:\t\t\t\t %" PRIu32 "\n", csdw->Mode);
    printf("Total number of subchannels:\t %" PRIu32 "\n", csdw->Subchannels);
    printf("Sampling factor:\t\t %" PRIu32 "\n", csdw->Factor);
    printf("Same bit:\t\t\t %" PRIu32 "\n", csdw->Same);
    printf("Reserved:\t\t\t %" PRIu32 "\n", csdw->Reserved);

    return I106_OK;
}


// TODO: implement missing TMATS attributes and re-enable
I106Status PrintAttributesfromTMATS_AnalogF1(R_DataSource *r_datasource,
        AnalogF1_Attributes *attributes, FILE *output){

    if ((r_datasource == NULL) || (attributes == NULL))
        return I106_INVALID_PARAMETER;

    printf("\n");
    printf("========================================\n");  
    printf("TMATS Attributes, Data Source %s\n", r_datasource->DataSourceID);
    printf("========================================\n");
    printf("\n");
    //  printf("Data source ID\t\t\t:\t%s\n", r_datasource->szDataSourceID);
    printf("Data source number\t\t:\t%i\n", r_datasource->DataSourceNumber);
    printf("Channel Data type\t\t:\t%s\n", r_datasource->ChannelDataType);
    printf("Channel Enabled\t\t\t:\t%i\n", r_datasource->Enabled);
    printf("\n");
    if(r_datasource->AnalogChannelsPerPacket != NULL)
        printf("Analog Channels/Packet\t\t:\t%i\n", attributes->ChannelsPerPacket);
    if(r_datasource->AnalogSampleRate != NULL)
        printf("Analog Sample Rate\t\t:\t%" PRIu64 "\tHz\n", attributes->SampleRate);
    if(r_datasource->AnalogDataPacking != NULL)
        printf("Analog Data Packed\t\t:\t%s\n", r_datasource->AnalogDataPacking);
    /* if(r_datasource->szAnalogDataLength != NULL) */
    /*     printf("Analog Data Length\t\t:\t%" PRIu32 "-bit\n", attributes->ulAnalogDataLength); */
    /* if(r_datasource->szAnalogMeasTransfOrd != NULL)    // R-x\AMTO-n-m most significant bit "M", least significant bit "L". default: M */
    /*     printf("Analog Meas Transfer Order\t:\t%c\n", r_datasource->szAnalogMeasTransfOrd[0]); */
    /* if(r_datasource->szAnalogSampleFactor != NULL) */
    /*     printf("Analog Sample Factor\t\t:\t%" PRIu32 "\n", attributes->ulAnalogSampleFactor); */
    /* if(r_datasource->szAnalogSampleFilter != NULL) */
    /*     printf("Analog 3dB Sample Filter\t:\t%" PRIu64 "\tHz\n", attributes->ullAnalogSampleFilter); */
    /* if(r_datasource->szAnalogIsDCCoupled != NULL) */
    /*     printf("Analog AC/DC Coupling\t\t:\t%cC\n", r_datasource->szAnalogIsDCCoupled[0]); */
    /* if(r_datasource->szAnalogRecImpedance != NULL) */
    /*     printf("Analog Recorder Input Impedance\t:\t%" PRIu32 "\t\tOhms\n", attributes->ulAnalogRecImpedance); */
    /* if(r_datasource->szAnalogChanGain != NULL) */
    /*     printf("Analog Channel Gain\t\t:\t%" PRIu32 "\t\tmilliunits\n", attributes->ulAnalogChanGain); */
    /* if(r_datasource->szAnalogFullScaleRange != NULL) */
    /*     printf("Analog Full-Scale Range\t\t:\t%" PRIu32 "\t\tmV\n", attributes->ulAnalogFullScaleRange); */
    /* if(r_datasource->szAnalogOffsetVoltage != NULL) */
    /*     printf("Analog Offset Voltage\t\t:\t%" PRIi32 "\t\tmV\n", attributes->lAnalogOffsetVoltage); */
    /* if(r_datasource->szAnalogLSBValue != NULL) */
    /*     printf("Analog LSB Value\t\t:\t%" PRIi32 "\n", attributes->lAnalogLSBValue); */
    /* if(r_datasource->szAnalogFormat != NULL) */
    /*     printf("Analog Data Format\t\t:\t%c\n", r_datasource->szAnalogFormat[0]); */
    /* if(r_datasource->szAnalogDifferentialInp != NULL) */
    /*     printf("Analog Data Format\t\t:\t%c\n", r_datasource->szAnalogDifferentialInp[0]); */
    /* if(r_datasource->szAnalogIsAudio != NULL) */
    /*     printf("Analog Chan Is Audio\t\t:\t%" PRIi32 "\n", attributes->bAnalogIsAudio); */

    return I106_OK;
}
