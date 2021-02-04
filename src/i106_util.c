
#include <string.h>

#include "i106_util.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>

#if !defined(__GNUC__)
#include <io.h>
#else
#include <unistd.h>
#endif
#include "i106_decode_time.h"
#include "i106_index.h"


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

/* Set packet header to some sane values. Be sure to fill in proper values for:
    PacketLength
    DataLength
    HeaderVersion
    RTC
    Checksum
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


/* From old i106_index */
typedef struct {
    uint32_t          NodesUsed;       // Number of index nodes actually used
    uint32_t          NodesAvailable;  // Number of index nodes available in the table
    uint32_t          NodeIncrement;   // Amount to increase the number of nodes
    I106C10Header     Header;          // Header and buffer for an IRIG Format 1 time packet. 
    void            * TimePacket;      // This is necessary for date format and leap year flags
    I106Time          Time;            // as well as relative time to absolute time mapping
                                       // if absolute time isn't provided.
    PacketIndexInfo * IndexTable;      // The main table of indexes
} C10Index;


/* Module data */

static int indexes_inited = 0;
static C10Index indices[MAX_HANDLES];


/* Function Declarations */

void InitIndexes(void);
I106Status ProcessNodeIndexPacket(int handle, int64_t offset);
I106Status ProcessRootIndexPacket(int handle, int64_t root_offset, int64_t *next_offset);
void AddIndexNodeToIndex(int handle, IndexMsg *node_msg, uint16_t channel_id, uint8_t data_type);
void AddNodeToIndex(int handle, PacketIndexInfo *index_info);
I106Status FindTimePacket(int handle);
void RelInt2IrigTime(int handle, int64_t rtc, I106Time *time);
void SortIndexes(int handle);


/* Check an open Ch 10 file to see if it has a valid index */
I106Status IndexPresent(const int handle, int *found_index){
    I106Status       status;
    int64_t          offset;
    I106C10Header    header;
    unsigned long    buffer_size = 0L;
    void           * buffer = NULL;
    TMATS_Info       tmats;

    *found_index = 0;

    // If data file not open in read mode then return
    if (handles[handle].FileMode != READ)
        return I106_NOT_OPEN;

    // Save current position
    I106C10GetPos(handle, &offset);

    // One time loop to make it easy to break out
    do {

        // Check for index support in TMATS
        status = I106C10SetPos(handle, 0L);
        if (status != I106_OK)
            break;

        status = I106C10ReadNextHeader(handle, &header);
        if (status != I106_OK)
            break;

        // Make sure TMATS exists
        if (header.DataType != I106CH10_DTYPE_TMATS){
            status = I106_READ_ERROR;
            break;
        }

        // Read TMATS and parse it
        // Make sure the buffer is big enough and read the data
        if (buffer_size < header.PacketLength){
            buffer      = realloc(buffer, header.PacketLength);
            buffer_size = header.PacketLength;
        }
        status = I106C10ReadData(handle, buffer_size, buffer);
        if (status != I106_OK)
            break;

        // Process the TMATS info
        memset(&tmats, 0, sizeof(tmats));
        status = I106_Decode_TMATS(&header, buffer, &tmats);
        if (status != I106_OK)
            break;
            
        // Check if index enabled
        if (tmats.FirstG_Record->FirstG_DataSource->R_Record->IndexEnabled == 0){
            status = I106_OK;
            break;
        }

        // Check for index as last packet
        status = I106C10LastMsg(handle);
        if (status != I106_OK)
            break;

        off_t pos = lseek(handles[handle].File, 0, SEEK_END);
        status = I106C10ReadPrevHeader(handle, &header);
        if (status != I106_OK)
            break;

        if (header.DataType == I106CH10_DTYPE_RECORDING_INDEX)
            *found_index = 1;

    } while (0);

    // Restore the file position
    I106C10SetPos(handle, offset);

    return status;
}


// Read an open Ch 10 file, read the various index packets, and build an 
// in-memory table of time and offsets.
I106Status ReadIndexes(const int handle){
    I106Status  status = I106_OK;
    int         found_index;
    int64_t     start;
    int64_t     pos;
    int64_t     next;

    // First, see if indexes have been init'ed
    if (indexes_inited == 0)
        InitIndexes();

    // Make sure indexes are in the file
    if ((status = IndexPresent(handle, &found_index)))
        return status;

    if (found_index == 0)
        return I106_NO_INDEX;

    // Save current position
    I106C10GetPos(handle, &start);

    // The reading mode must be I106_READ
    // TODO : get rid of this global
    if (handles[handle].FileMode != READ){
        return I106_WRONG_FILE_MODE;
    }

    // The optional intrapacket data header provides absolute time in IRIG Time
    // Format 1 format.  Unfortunately there are two important pieces of information
    // that are only in the CSDW, the date format flag and the leap year flag (need 
    // for DoY format).  The time CSDW isn't provided in the index.  So the plan
    // is to go read a time packet and hope that date format and leap year are
    // the same.

    FindTimePacket(handle);

    // Place the reading pointer at the last packet which is the Root Index Packet
    I106C10LastMsg(handle);

    // Save this file offset
    I106C10GetPos(handle, &pos);

	// Root packet found so start processing root index packets
    while (1){
        /* printf("Root index"); */

        // Check for exit conditions
        if ((status = ProcessRootIndexPacket(handle, pos, &next))){
            break;
        }

        if (pos == next)
            break;

        // Not done so setup for the next root index packet
        pos = next;

    }

    // Sort the resultant index
    SortIndexes(handle);

    // Restore the file position
    I106C10SetPos(handle, start);

	return status;
}


I106Status ProcessRootIndexPacket(int handle, int64_t offset, int64_t *next){
    I106Status         status = I106_OK;
    I106C10Header      header;
    void             * buffer = NULL;
    IndexMsg           msg;

    // Go to what should be a root index packet
    status = I106C10SetPos(handle, offset);
    if (status != I106_OK)
        return status;

    // Read what should be a root index packet
    if ((status = I106C10ReadNextHeader(handle, &header))){
        return status;
    }

    if (header.DataType != I106CH10_DTYPE_RECORDING_INDEX)
        return I106_INVALID_DATA;

    // Read the index packet
    buffer = malloc(header.PacketLength);

    // Read the data buffer
    if ((status = I106C10ReadData(handle, header.PacketLength, buffer)))
        return status;

    // Decode the first root index message
    status = I106_Decode_FirstIndex(&header, buffer, &msg);

    // Loop on all root index messages
    while (1){
        // Root message, go to node packet and decode
        if (status == I106_INDEX_ROOT){
            // Go process the node packet
            status = ProcessNodeIndexPacket(handle, *(msg.FileOffset));
            if (status != I106_OK)
                break;                
        }

        // Last root message links to the next root packet
        else if (status == I106_INDEX_ROOT_LINK)
            *next = *(msg.FileOffset);

        // If it comes back as a node message then there was a problem
        else if (status == I106_INDEX_NODE){
            status = I106_INVALID_DATA;
            break;
        }

        // Done reading messages from the index buffer
        else if (status == I106_NO_MORE_DATA){
            status = I106_OK;
            break;
        }

        // Any other return status is an error of some sort
        else
            break;

        // Get the next root index message
        status = I106_Decode_NextIndex(&msg);

    }

    free(buffer);

    return status;
}


// Go to the given offset, read what should be a node index packet, read
// the index values, and add the info to the index memory array.
I106Status ProcessNodeIndexPacket(int handle, int64_t offset){
    I106Status         status = I106_OK;
    I106C10Header      header;
    void             * buffer = NULL;
    IndexMsg           msg;

    // Go to what should be a node index packet
    status = I106C10SetPos(handle, offset);
    if (status != I106_OK)
        return status;

    // Read the packet header
    status = I106C10ReadNextHeader(handle, &header);

    if (status != I106_OK)
        return status;

    if (header.DataType != I106CH10_DTYPE_RECORDING_INDEX)
        return I106_INVALID_DATA;

    // Make sure our buffer is big enough, size *does* matter
    buffer = malloc(header.PacketLength);

    // Read the data buffer
    status = I106C10ReadData(handle, header.PacketLength, buffer);

    // Check for data read errors
    if (status != I106_OK)
        return status;

    // Decode the first node index message
    status = I106_Decode_FirstIndex(&header, buffer, &msg);

    // Loop on all node index messages
    while (1){
        // Node message, go to node packet and decode
        if (status == I106_INDEX_NODE)
            // Add this node to the index
            AddIndexNodeToIndex(handle, &msg, header.ChannelID, header.DataType);

        else if ((status == I106_INDEX_ROOT) || (status == I106_INDEX_ROOT_LINK)){
            status = I106_INVALID_DATA;
            break;
        }

        // Done reading messages from the index buffer
        else if (status == I106_NO_MORE_DATA){
            status = I106_OK;
            break;
        }

        // Any other return status is an error of some sort
        else
            break;

        // Get the next node index message
        status = I106_Decode_NextIndex(&msg);

    }

    free(buffer);

    return status;
}


/* Add an index packet node to the in memory index array */
void AddIndexNodeToIndex(int handle, IndexMsg *msg, uint16_t channel_id, uint8_t data_type){
    PacketIndexInfo      index_info;
    I106C10Header        header;
    void               * buffer;
    TimeF1_CSDW        * csdw;

    // Store the info
    index_info.ChannelID  = channel_id;
    index_info.DataType   = data_type;
    index_info.Offset     = *(msg->FileOffset);
    index_info.RTC        = msg->Time->time;

    // If the optional intrapacket data header exists then get absolute time from it
    if (msg->CSDW->IPH == 1){
        csdw = (TimeF1_CSDW *)indices[handle].TimePacket;
        I106_Decode_TimeF1_Buffer(csdw->DateFormat, csdw->LeapYear,
            msg->Time, &index_info.IrigTime);
    }

    // Else if the indexed packet is a time packet then get the time from it
    else if (msg->NodeData->DataType == I106CH10_DTYPE_IRIG_TIME){
        // Go to what should be a time packet
        I106C10SetPos(handle, *(msg->FileOffset));

        // Read the packet header
        I106C10ReadNextHeader(handle, &header);

        // Make sure our buffer is big enough
        buffer = malloc(header.PacketLength);

        // Read the data buffer
        I106C10ReadData(handle, header.PacketLength, buffer);

        // Decode the time packet
        I106_Decode_TimeF1(&header, buffer, &index_info.IrigTime);

        free(buffer);

    }

    // Else no absolute time available, so make it from relative time
    else
        RelInt2IrigTime(handle, msg->Time->time, &index_info.IrigTime);

    AddNodeToIndex(handle, &index_info);

    return;
}


// Add decoded index information to the in memory index array
void AddNodeToIndex(int handle, PacketIndexInfo *index_info){

    // See if we need to make the node table bigger
    if (indices[handle].NodesAvailable <= indices[handle].NodesUsed){
        // Reallocate memory
        indices[handle].IndexTable = 
            (PacketIndexInfo *)realloc(indices[handle].IndexTable, 
            (indices[handle].NodesAvailable + indices[handle].NodeIncrement) * sizeof(PacketIndexInfo));

        indices[handle].NodesAvailable += indices[handle].NodeIncrement;

        // Make increment bigger next time
        indices[handle].NodeIncrement = (uint32_t)(indices[handle].NodeIncrement * 1.5);
    }

    memcpy(&indices[handle].IndexTable[indices[handle].NodesUsed], index_info, sizeof(PacketIndexInfo));
    indices[handle].NodesUsed++;

    return;
}


// Make an index of a channel by reading through the data file.
I106Status MakeIndex(const int handle, uint16_t channel_id){
    I106Status         status;
    I106C10Header      header;
    unsigned long      buffer_size = 0L;
    unsigned char    * buffer = NULL;
    PacketIndexInfo    index_info;
    int64_t            offset;

    // First establish time
    FindTimePacket(handle);

    // Loop through the file
    while (1) {

        // Get the current file offset
        I106C10GetPos(handle, &offset);

        // Read the next header
        status = I106C10ReadNextHeader(handle, &header);

        // If selected channel then put info into the index
        if (status == I106_OK && header.ChannelID == channel_id){

            // Make sure our buffer is big enough, size *does* matter
            if (buffer_size < header.PacketLength){
                buffer = (unsigned char *)realloc(buffer, header.PacketLength);
                buffer_size = header.PacketLength;
            }

            // Read the data buffer
            status = I106C10ReadData(handle, buffer_size, buffer);

            // Populate index info
            index_info.Offset     = offset;
            index_info.DataType   = header.DataType;
            index_info.ChannelID  = channel_id;
            TimeArray2LLInt(header.RTC, &index_info.RTC);
            AddNodeToIndex(handle, &index_info);
        }

        // If EOF break out of main read loop
        if (status == I106_EOF)
            return I106_OK;
    }

    return status;
}


/* Find a valid time packet for the index. 
 * Read the data file from the middle of the file to try to determine a 
 * valid relative time to clock time from a time packet. Store the result in
 * the time field in the index. */
I106Status FindTimePacket(int handle){
    int                  require_sync = 0;    // Require external time source sync
    int                  max_seconds   = 10;  // Max time to look in seconds

    int64_t              offset;
    int64_t              last;
    int64_t              time_limit;
    int64_t              current_time;
    I106Status           status;
    I106Status           return_status;
    I106C10Header        header;
    unsigned long        buffer_size = 0;
    void               * buffer = NULL;
    TimeF1_CSDW        * time = NULL;

    // Get and save the current file position
    status = I106C10GetPos(handle, &offset);
    if (status != I106_OK)
        return status;

    // Get time from the middle of the file
    status = I106C10LastMsg(handle);
    status = I106C10GetPos(handle, &last);
    status = I106C10SetPos(handle, last/2);

    // Read the next header
    status = I106C10ReadNextHeader(handle, &header);
    if (status == I106_EOF)
        return I106_TIME_NOT_FOUND;

    if (status != I106_OK)
        return status;

    // Calculate the time limit if there is one
    if (max_seconds > 0){
        TimeArray2LLInt(header.RTC, &time_limit);
        time_limit += (int64_t)time_limit * (int64_t)10000000;
    }
    else
        time_limit = 0;

    // Loop, looking for appropriate time message
    while (1){

        // See if we've passed our time limit
        if (time_limit > 0){
            TimeArray2LLInt(header.RTC, &current_time);
            if (time_limit < current_time){
                return_status = I106_TIME_NOT_FOUND;
                break;
            }
        }

        // If IRIG time type then process it
        if (header.DataType == I106CH10_DTYPE_IRIG_TIME){

            // Read header OK, make buffer for time message
            if (buffer_size < header.PacketLength){
                buffer       = realloc(buffer, header.PacketLength);
                time         = (TimeF1_CSDW *)buffer;
                buffer_size  = header.PacketLength;
            }

            // Read the data buffer
            status = I106C10ReadData(handle, buffer_size, buffer);
            if (status != I106_OK){
                return_status = I106_TIME_NOT_FOUND;
                break;
            }

            // If external sync OK then decode it and set relative time
            if ((require_sync == 0) || (time->TimeSource == 1)){
                memcpy(&indices[handle].Header, &header, sizeof(I106C10Header));
                indices[handle].TimePacket = buffer;
                I106_Decode_TimeF1(&header, buffer, &indices[handle].Time);
                return_status = I106_OK;
                break;
            }
        }

        // Read the next header and try again
        status = I106C10ReadNextHeader(handle, &header);
        if (status == I106_EOF){
            return_status = I106_TIME_NOT_FOUND;
            break;
        }

        if (status != I106_OK){
            return_status = status;
            break;
        }

    }

    // Restore file position
    status = I106C10SetPos(handle, offset);
    if (status != I106_OK)
        return_status = status;

    return return_status;
}


/* Take a 64 bit relative time value and turn it into a real time based on 
 * the reference IRIG time from the index.
 * This routines was lifted from the enI106_RelInt2IrigTime() from i106_time.
 * The difference is that that routine uses the index relative time reference
 * rather than the globally maintained reference. */
void RelInt2IrigTime(int handle, int64_t rtc, I106Time *time){
    int64_t         ref_rtc;
    int64_t         time_diff;
    int64_t         frac_diff;
    int64_t         sec_diff;

    int64_t         seconds;
    int64_t         fraction;

    // Figure out the relative time difference
    TimeArray2LLInt(indices[handle].Header.RTC, &ref_rtc);
    time_diff = rtc - ref_rtc;
    sec_diff  = time_diff / 10000000;
    frac_diff = time_diff % 10000000;

    seconds   = indices[handle].Time.Seconds + sec_diff;
    fraction  = indices[handle].Time.Fraction + frac_diff;

    // This seems a bit extreme but it's defensive programming
    while (fraction < 0){
        fraction += 10000000;
        seconds  -= 1;
    }
        
    while (fraction >= 10000000){
        fraction -= 10000000;
        seconds  += 1;
    }

    // Now add the time difference to the last IRIG time reference
    time->Fraction = (unsigned long)fraction;
    time->Seconds = (unsigned long)seconds;
    time->Format  = indices[handle].Time.Format;

    return;
}


/* Initialize an index table. */
void InitIndex(int handle){
    indices[handle].NodesAvailable = 0;
    indices[handle].NodesUsed      = 0;
    indices[handle].NodeIncrement  = 1000;

    if (indexes_inited && (indices[handle].TimePacket != NULL))
        free(indices[handle].TimePacket);
    indices[handle].TimePacket = NULL;

    if (indexes_inited && (indices[handle].IndexTable != NULL))
        free(indices[handle].IndexTable);
    indices[handle].IndexTable = NULL;
  
    return;
}


// Initialize all index tables for the first time
void InitIndexes(void){
    for (int i=0; i<MAX_HANDLES; i++)
        InitIndex(i);

    indexes_inited = 1;
}


/* Sort an existing index table in memory by relative time */
int CompareIndexes(const void *index1, const void *index2){
    if (((PacketIndexInfo *)index1)->RTC > ((PacketIndexInfo *)index2)->RTC)
        return 1;

    if (((PacketIndexInfo *)index1)->RTC < ((PacketIndexInfo *)index2)->RTC)
        return -1;

    return 0;
}


void SortIndexes(int handle){
    qsort(
        indices[handle].IndexTable, 
        indices[handle].NodesUsed, 
        sizeof(PacketIndexInfo), 
        &CompareIndexes);
}



/* From old i106_time */

/* Macros and definitions */

// Number of leap years from 1970 to `y' (not including `y' itself).
#define nleap(y) (((y) - 1969) / 4 - ((y) - 1901) / 100 + ((y) - 1601) / 400)

// Nonzero if `y' is a leap year, else zero.
#define leap(y) (((y) % 4 == 0 && (y) % 100 != 0) || (y) % 400 == 0)

// Additional leapday in February of leap years.
#define leapday(m, y) ((m) == 1 && leap (y))

#define ADJUST_TM(tm_member, tm_carry, modulus) \
    if ((tm_member) < 0) { \
        tm_carry -= (1 - ((tm_member)+1) / (modulus)); \
        tm_member = (modulus-1) + (((tm_member)+1) % (modulus)); \
    } else if ((tm_member) >= (modulus)) { \
        tm_carry += (tm_member) / (modulus); \
        tm_member = (tm_member) % (modulus); \
    }

// Length of month `m' (0 .. 11)
#define monthlen(m, y) (julian_day[(m)+1] - julian_day[m] + leapday (m, y))


/* Module data */
// TODO: evil global, delete ASAP
static TimeRef  time_ref[MAX_HANDLES];  // Relative / absolute time reference


/* Function Declaration */

// Update the current reference time value
I106Status I106_SetRelTime(int handle, I106Time *time, uint8_t rtc[]){

    // Save the absolute time value
    time_ref[handle].IrigTime.Seconds = time->Seconds;
    time_ref[handle].IrigTime.Fraction = time->Fraction;
    time_ref[handle].IrigTime.Format  = time->Format;

    // Save the relative (i.e. the 10MHz counter) value
    time_ref[handle].RTC = 0;
    memcpy((char *)&(time_ref[handle].RTC), (char *)&rtc[0], 6);

    return I106_OK;
}


// Take a 6 byte relative time value (like the one in the IRIG header) and
// turn it into a real time based on the current reference IRIG time.
I106Status I106_Rel2IrigTime(int handle, uint8_t rtc[], I106Time *time){
    int64_t     rel_time;

    // Convert 6 byte time array to 16 bit int.  This only works for 
    // positive time, but that shouldn't be a problem
    rel_time = 0L;
    memcpy(&rel_time, &rtc[0], 6);

    return I106_RelInt2IrigTime(handle, rel_time, time);
}


// Take a 64 bit relative time value and turn it into a real time based on 
// the current reference IRIG time.
I106Status I106_RelInt2IrigTime(int handle, int64_t rel_time, I106Time *time){
    int64_t         time_diff;
    int64_t         frac_diff;
    int64_t         sec_diff;

    int64_t         seconds;
    int64_t         fraction;


    // Figure out the relative time difference
    time_diff = rel_time - time_ref[handle].RTC;
    sec_diff  = time_diff / 10000000;
    frac_diff = time_diff % 10000000;

    seconds   = time_ref[handle].IrigTime.Seconds + sec_diff;
    fraction  = time_ref[handle].IrigTime.Fraction + frac_diff;

    // This seems a bit extreme but it's defensive programming
    while (fraction < 0){
        fraction += 10000000;
        seconds -= 1;
    }
        
    while (fraction >= 10000000){
        fraction -= 10000000;
        seconds += 1;
    }

    // Now add the time difference to the last IRIG time reference
    time->Fraction = (unsigned long)fraction;
    time->Seconds  = (unsigned long)seconds;
    time->Format   = time_ref[handle].IrigTime.Format;

    return I106_OK;
}


// Take a real clock time and turn it into a 6 byte relative time.
I106Status I106_Irig2RelTime(int handle, I106Time *time, uint8_t rtc[]){
    int64_t  diff;
    int64_t  new_rtc;

    // Calculate time difference (LSB = 100 nSec) between the passed time 
    // and the time reference
    diff = (int64_t)(+ time->Seconds - time_ref[handle].IrigTime.Seconds) * 10000000 +
        (int64_t)(+ time->Fraction - time_ref[handle].IrigTime.Fraction);

    // Add this amount to the reference 
    new_rtc = time_ref[handle].RTC + diff;

    // Now convert this to a 6 byte relative time
    memcpy(&rtc, &new_rtc, 6);

    return I106_OK;
}


// Take a Irig Ch4 time value (like the one in a secondary IRIG header) and
// turn it into an Irig106 time
I106Status I106_Ch4Binary2IrigTime(I106Ch4_Binary_Time *ch4_time, I106Time *irig_time){
    irig_time->Seconds = (unsigned long)
        ( (double)ch4_time->HighBinTime * CH4BINARYTIME_HIGH_LSB_SEC
        + (unsigned long)ch4_time->LowBinTime * CH4BINARYTIME_LOW_LSB_SEC );
    irig_time->Fraction = (unsigned long)ch4_time->Micro * _100_NANO_SEC_IN_MICRO_SEC;

    return I106_OK;
}


// Take a IEEE-1588 time value (like the one in a secondary IRIG header) and
// turn it into an Irig106 time
I106Status I106_IEEE15882IrigTime(IEEE1588_Time *i1588_time, I106Time  *irig_time){
    irig_time->Seconds = (unsigned long)i1588_time->Seconds;

    //Convert 'nanoseconds' to '100 nanoseconds'
    irig_time->Fraction = (unsigned long)i1588_time->NanoSeconds / 100;     

    return I106_OK;
}


// Warning - array to int / int to array functions are little endian only!

// Create a 6 byte array value from a 64 bit int relative time
void LLInt2TimeArray(int64_t *rel_time, uint8_t rtc[]){
    memcpy((char *)&rtc, (char *)rel_time, 6);
}


// Create a 64 bit int relative time from 6 byte array value
void TimeArray2LLInt(uint8_t rtc[], int64_t *rel_time){
    *rel_time = 0L;
    memcpy((char *)rel_time, (char *)rtc, 6);
}


// Read the data file from the current position to try to determine a valid 
// relative time to clock time from a time packet.
I106Status I106_SyncTime(int handle, int sync, int max_seconds){
    int64_t             offset;
    int64_t             time_limit;
    int64_t             current_time;
    I106Status          status;
    I106Status          return_status;
    I106C10Header       header;
    I106Time            time;
    unsigned long       buffer_size = 0;
    void              * buffer = NULL;
    TimeF1_CSDW       * csdw = NULL;

    // Get and save the current file position
    status = I106C10GetPos(handle, &offset);
    if (status != I106_OK)
        return status;

    // Read the next header
    status = I106C10ReadNextHeader(handle, &header);
    if (status == I106_EOF)
        return I106_TIME_NOT_FOUND;

    if (status != I106_OK)
        return status;

    // Calculate the time limit if there is one
    if (max_seconds > 0){
        TimeArray2LLInt(header.RTC, &time_limit);
        time_limit = time_limit + (int64_t)max_seconds * (int64_t)10000000;
    }
    else
        time_limit = 0;

    // Loop, looking for appropriate time message
    while (1){

        // See if we've passed our time limit
        if (time_limit > 0){
            TimeArray2LLInt(header.RTC, &current_time);
            if (time_limit < current_time){
                return_status = I106_TIME_NOT_FOUND;
                break;
            }
        }

        // If IRIG time type then process it
        if (header.DataType == I106CH10_DTYPE_IRIG_TIME){

            // Read header OK, make buffer for time message
            if (buffer_size < header.PacketLength){
                buffer       = realloc(buffer, header.PacketLength);
                csdw         = (TimeF1_CSDW *)buffer;
                buffer_size  = header.PacketLength;
            }

            // Read the data buffer
            status = I106C10ReadData(handle, buffer_size, buffer);
            if (status != I106_OK){
                return_status = I106_TIME_NOT_FOUND;
                break;
            }

            // If external sync OK then decode it and set relative time
            if ((sync == 0) || (csdw->TimeSource == 1)){
                I106_Decode_TimeF1(&header, buffer, &time);
                I106_SetRelTime(handle, &time, header.RTC);
                return_status = I106_OK;
                break;
            }
        }

        // Read the next header and try again
        status = I106C10ReadNextHeader(handle, &header);
        if (status == I106_EOF){
            return_status = I106_TIME_NOT_FOUND;
            break;
        }

        if (status != I106_OK){
            return_status = status;
            break;
        }

    }

    // Restore file position
    status = I106C10SetPos(handle, offset);
    if (status != I106_OK)
        return_status = status;

    // Return the malloc'ed memory
    free(buffer);

    return return_status;
}


// TODO: seek based on RTC? Excuse me??
I106Status I106C10SetPosToIrigTime(int handle, I106Time *irig_seek_time){
    uint8_t           rtc_seek_time[6];
    int64_t           seek_time;
    InOrderIndex    * index = &handles[handle].Index;
    int               upper_limit;
    int               lower_limit;

    // If there is no index in memory then barf
    if (index->SortStatus != SORTED)
        return I106_NO_INDEX;

    // We have an index so do a binary search for time

    // Convert clock time to 10 MHz count
    I106_Irig2RelTime(handle, irig_seek_time, rtc_seek_time);
    TimeArray2LLInt(rtc_seek_time, &seek_time);

    // Check time bounds
    if (seek_time < index->Index[0].Time){
        I106C10SetPos(handle, 0L);
        return I106_TIME_NOT_FOUND;
    };

    if (seek_time > index->Index[index->ArrayUsed - 1].Time){
        I106C10LastMsg(handle);
        return I106_TIME_NOT_FOUND;
    };

    // If we don't already have it, figure out how many search steps
    if (index->NumSearchSteps == 0){
        upper_limit = 1;
        while (upper_limit < index->ArrayUsed){
            upper_limit *= 2;
            index->NumSearchSteps++;
        }
    }

    // Loop prescribed number of times
    lower_limit = 0;
    upper_limit = index->ArrayUsed - 1;
    index->ArrayPos = (upper_limit - lower_limit) / 2;
    for (int i = 0; i < index->NumSearchSteps; i++){
        if (index->Index[index->ArrayPos].Time > seek_time)
            upper_limit = (upper_limit - lower_limit) / 2;
        else if (index->Index[index->ArrayPos].Time < seek_time)
            lower_limit = (upper_limit - lower_limit) / 2;
        else
            break;
    }

    return I106_OK;
}


/* General purpose time utilities */

// Convert IRIG time into an appropriate string
char * IrigTime2String(I106Time *time){
    static char    time_str[30];
    struct tm     *tm;

    // Convert IRIG time into it's components
    tm = gmtime((time_t *)&(time->Seconds));

    // Make the appropriate string
    switch (time->Format){

        // Year / Month / Day format ("2008/02/29 12:34:56.789")
        case I106_DATEFMT_DMY:
            sprintf(time_str, "%4.4i/%2.2i/%2.2i %2.2i:%2.2i:%2.2i.%3.3i",
                tm->tm_year + 1900,
                tm->tm_mon + 1,
                tm->tm_mday,
                tm->tm_hour,
                tm->tm_min,
                tm->tm_sec,
                time->Fraction / 10000);
            break;

        // Day of the Year format ("001:12:34:56.789")
        case I106_DATEFMT_DAY:
        default:
            sprintf(time_str, "%3.3i:%2.2i:%2.2i:%2.2i.%3.3i",
                tm->tm_yday+1,
                tm->tm_hour,
                tm->tm_min,
                tm->tm_sec,
                time->Fraction / 10000);
            break;
    }

    return time_str;
}


// This function fills in the SuTimeRef structure with the "best" relative 
// and/or absolute time stamp available from the packet header and intra-packet 
// header (if available).
I106Status FillInTimeStruct(I106C10Header *header, IntraPacketTS * ipts, TimeRef * timeref){
    int secondary_time_format;

    // Get the secondary header time format
    secondary_time_format = header->PacketFlags & I106CH10_PFLAGS_TIMEFMT_MASK;
    timeref->RTCValid = 0;
    timeref->TimeValid = 0;
    
    // Set the relative time from the packet header
    TimeArray2LLInt(header->RTC, &(timeref->RTC));
    timeref->RTCValid = 1;

    // If secondary header is available, use that time for absolute
    if ((header->PacketFlags & I106CH10_PFLAGS_SEC_HEADER) != 0){
        switch(secondary_time_format){
            case I106CH10_PFLAGS_TIMEFMT_IRIG106:
                I106_Ch4Binary2IrigTime((I106Ch4_Binary_Time *)header->RTC, &(timeref->IrigTime));
                timeref->TimeValid = 1;
                break;
            case I106CH10_PFLAGS_TIMEFMT_IEEE1588:
                I106_IEEE15882IrigTime((IEEE1588_Time *)header->RTC, &(timeref->IrigTime));
                timeref->TimeValid = 1;
                break;
            default:
                //Currently reserved, should we have a default way to decode?
                break;
        }
    }

    // Now process values from the intra-packet headers if available
    if (ipts != NULL){
        
        // If relative time
        if ((header->PacketFlags & I106CH10_PFLAGS_IPTIMESRC) == 0){
            TimeArray2LLInt(ipts->IPTS, &(timeref->RTC));
            timeref->RTCValid = 1;
        }

        // else is absolute time
        else {
            switch(secondary_time_format){
                case I106CH10_PFLAGS_TIMEFMT_IRIG106:
                    I106_Ch4Binary2IrigTime((I106Ch4_Binary_Time *)ipts, &(timeref->IrigTime));
                    timeref->TimeValid = 1;
                    break;
                case I106CH10_PFLAGS_TIMEFMT_IEEE1588:                  
                    I106_IEEE15882IrigTime((IEEE1588_Time *)ipts, &(timeref->IrigTime));
                    timeref->TimeValid = 1;
                    break;
                default:
                    //Current reserved, should we have a default way to decode
                    break;
            }
        }
    }
    
    return I106_OK;
}


/* Return the equivalent in seconds past 12:00:00 a.m. Jan 1, 1970 GMT
   of the Greenwich Mean time and date in the exploded time structure `tm'.

   The standard mktime() has the annoying "feature" of assuming that the 
   time in the tm structure is local time, and that it has to be corrected 
   for local time zone.  In this library time is assumed to be UTC and UTC
   only.  To make sure no timezone correction is applied this time conversion
   routine was lifted from the standard C run time library source.  Interestingly
   enough, this routine was found in the source for mktime().

   This function does always put back normalized values into the `tm' struct,
   parameter, including the calculated numbers for `tm->tm_yday',
   `tm->tm_wday', and `tm->tm_isdst'.

   Returns -1 if the time in the `tm' parameter cannot be represented
   as valid `time_t' number. 
 */
uint32_t mkgmtime(struct tm *time){

    // Accumulated number of days from 01-Jan up to start of current month.
    static short julian_day[] = {
        0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
    };

    int years, months, days, hours, minutes, seconds;

    years   = time->tm_year + 1900;  // year - 1900 -> year
    months  = time->tm_mon;          // 0..11
    days    = time->tm_mday - 1;     // 1..31 -> 0..30
    hours   = time->tm_hour;         // 0..23
    minutes = time->tm_min;          // 0..59
    seconds = time->tm_sec;          // 0..61 in ANSI C.

    ADJUST_TM(seconds, minutes, 60)
    ADJUST_TM(minutes, hours,   60)
    ADJUST_TM(hours,   days,    24)
    ADJUST_TM(months,  years,   12)

    while (days < 0) {
        if (--months < 0) {
            --years;
            months = 11;
        }
        days += monthlen(months, years);
    } ;

    while (days >= monthlen(months, years)){
        days -= monthlen(months, years);
        if (++months >= 12) {
            ++years;
            months = 0;
        }
    }

    // Restore adjusted values in tm structure
    time->tm_year = years - 1900;
    time->tm_mon  = months;
    time->tm_mday = days + 1;
    time->tm_hour = hours;
    time->tm_min  = minutes;
    time->tm_sec  = seconds;

    // Set `days' to the number of days into the year.
    days += julian_day[months] + (months > 1 && leap (years));
    time->tm_yday = days;

    // Now calculate `days' to the number of days since Jan 1, 1970.
    days = (unsigned)days + 365 * (unsigned)(years - 1970) +
           (unsigned)(nleap (years));
    time->tm_wday = ((unsigned)days + 4) % 7; /* Jan 1, 1970 was Thursday. */
    time->tm_isdst = 0;

    if (years < 1970)
        return (uint32_t)-1;

    return (uint32_t)(86400L * days  + 3600L * hours + 60L * minutes + seconds);
}
