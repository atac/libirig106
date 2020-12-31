
#include <string.h>

#include "util.h"


// Swaps "bytes" bytes in place
I106Status SwapBytes(uint8_t *buffer, long bytes){
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

    SwapShortWords((uint16_t *)&data, 4);

    return I106_OK;
}


// Swaps "bytes" bytes of 16 bit words in place
I106Status SwapShortWords(uint16_t *buffer, long bytes){
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

/* -----------------------------------------------------------------------
 * Utilities
 * TODO: move these to a separate file
 * ----------------------------------------------------------------------- */

/* Set packet header to some sane values. Be sure to fill in proper values for:
    ulPacketLen
    ulDataLen
    ubyHdrVer
    aubyRefTime
    uChecksum
 */
int HeaderInit(
        I106C10Header * header,
        unsigned int channel_id,
        unsigned int data_type,
        unsigned int flags,
        unsigned int sequence_number){

    // Make a legal, valid header
    header->SyncPattern     = IRIG106_SYNC;
    header->ChannelID       = channel_id;
    header->PacketLength    = HEADER_SIZE;
    header->DataLength      = 0;
    header->HeaderVersion   = 0x02;  // <-- NEED TO PASS THIS IN!!!
    header->SequenceNumber  = sequence_number;
    header->PacketFlags     = flags;
    header->DataType        = data_type;
    memset(&(header->RTC), 0, 6);
    header->Checksum        = 0;

    return 0;
}


int GetHeaderLength(I106C10Header *header){
    if ((header->PacketFlags & I106CH10_PFLAGS_SEC_HEADER) == 0)
        return HEADER_SIZE;
    
    return HEADER_SIZE + SEC_HEADER_SIZE;
}


// Figure out data length including padding and any data checksum
uint32_t GetDataLength(I106C10Header *header){
    return header->PacketLength - GetHeaderLength(header);
}


uint16_t HeaderChecksum(I106C10Header *header){
    uint16_t   header_sum = 0;
    uint16_t  *header_array = (uint16_t *)header;

    for (int i=0; i<(HEADER_SIZE-2)/2; i++)
        header_sum += header_array[i];

    return header_sum;
}


uint16_t SecondaryHeaderChecksum(I106C10Header *header){
    uint16_t   sum = 0;
    uint16_t  *secondary_array = (uint16_t *)(header + HEADER_SIZE);

    for (int i=0; i<(SEC_HEADER_SIZE-2)/2; i++)
        sum += secondary_array[i];

    return sum;
}


// Add the filler and appropriate checksum to the end of the data buffer
// It is assumed that the buffer is big enough to hold additional filler 
// and the checksum. Also fill in the header with the correct packet length.
I106Status AddFillerAndChecksum(I106C10Header *header, unsigned char data[]){
    uint32_t    i;
    uint32_t    buffer_size;
    uint32_t    fill_size;
    int         checksum_type;

    uint8_t    *sum_8;
    uint8_t    *data_8;
    uint16_t   *sum_16;
    uint16_t   *data_16;
    uint32_t   *sum_32;
    uint32_t   *data_32;

    // Extract the checksum type
    checksum_type = header->PacketFlags & 0x03;

    // Figure out how big the final packet will be
    uint32_t  buffer_length = header->DataLength;

    // Add in enough for the selected checksum
    if (checksum_type == I106CH10_PFLAGS_CHKSUM_8)
        buffer_length += 1;
    else if (checksum_type == I106CH10_PFLAGS_CHKSUM_16)
        buffer_length += 2;
    else if (checksum_type == I106CH10_PFLAGS_CHKSUM_32)
        buffer_length += 4;

    // Now add filler for 4 byte alignment
    buffer_length += 3;
    buffer_length &= 0xfffffffc;
    buffer_size = buffer_length;

    header->PacketLength = HEADER_SIZE + buffer_size;
    if (header->PacketFlags & I106CH10_PFLAGS_SEC_HEADER)
        header->PacketLength += SEC_HEADER_SIZE;

    // Figure out the filler/checksum size and zero fill it
    fill_size = buffer_size - header->DataLength;
    memset(&data[header->DataLength], 0, fill_size);

    // If no checksum then we're done
    if (checksum_type == I106CH10_PFLAGS_CHKSUM_NONE)
        return I106_OK;

    // Calculate the checksum
    switch (checksum_type){
        case I106CH10_PFLAGS_CHKSUM_8:
            // Checksum the data and filler
            data_8 = (uint8_t *)data;
            sum_8  = (uint8_t *)&data[header->DataLength + fill_size - 1];
            for (i=0; i<buffer_size-1; i++){
                *sum_8 += *data_8;
                data_8++;
            }
            break;

        case I106CH10_PFLAGS_CHKSUM_16:
            data_16 = (uint16_t *)data;
            sum_16  = (uint16_t *)&data[header->DataLength + fill_size - 2];
            for (i=0; i<(buffer_size/2) - 1; i++){
                *sum_16 += *data_16;
                data_16++;
            }
            break;

        case I106CH10_PFLAGS_CHKSUM_32:
            data_32 = (uint32_t *)data;
            sum_32  = (uint32_t *)&data[header->DataLength + fill_size - 4];
            for (i=0; i<(buffer_size/4)-1; i++){
                *sum_32 += *data_32;
                data_32++;
            }
            break;
        default:
            break;
    }

    return I106_OK;
}


char * I106ErrorString(I106Status status){
    switch (status){
        case I106_OK                : return "No error";
        case I106_OPEN_ERROR        : return "File open failed";
        case I106_OPEN_WARNING      : return "File open warning";
        case I106_EOF               : return "End of file";
        case I106_BOF               : return "Beginning of file";
        case I106_READ_ERROR        : return "Read error";
        case I106_WRITE_ERROR       : return "Write error";
        case I106_MORE_DATA         : return "More data available";
        case I106_SEEK_ERROR        : return "Seek error";
        case I106_WRONG_FILE_MODE   : return "Wrong file mode";
        case I106_NOT_OPEN          : return "File not open";
        case I106_ALREADY_OPEN      : return "File already open";
        case I106_BUFFER_TOO_SMALL  : return "Buffer too small";
        case I106_NO_MORE_DATA      : return "No more data";
        case I106_NO_FREE_HANDLES   : return "No free file handles";
        case I106_INVALID_HANDLE    : return "Invalid file handle";
        case I106_TIME_NOT_FOUND    : return "Time not found";
        case I106_HEADER_CHKSUM_BAD : return "Bad header checksum";
        case I106_NO_INDEX          : return "No index";
        case I106_UNSUPPORTED       : return "Unsupported feature";
        case I106_BUFFER_OVERRUN    : return "Buffer overrun";
        case I106_INDEX_NODE        : return "Index node";
        case I106_INDEX_ROOT        : return "Index root";
        case I106_INDEX_ROOT_LINK   : return "Index root link";
        case I106_INVALID_DATA      : return "Invalid data";
        case I106_INVALID_PARAMETER : return "Invalid parameter";
        default                     : return "Unknown error";
    }
}
