/****************************************************************************

 i106_index.c - A higher level interface to the indexing system

 This module provides higher level routines for reading and writing Ch 10
 index information.  It uses the structures and routines in i106_decode_index
 module to read and write data file root and node index packets.

 ****************************************************************************/

#include <string.h>
#include <stdio.h>

#if !defined(__GNUC__)
#include <io.h>
#endif

#include <stdlib.h>
#include <time.h>

#include "config.h"
#include "stdint.h"

#include "irig106ch10.h"
#include "i106_time.h"

#include "i106_decode_tmats.h"
#include "i106_decode_time.h"
#include "i106_decode_index.h"
#include "i106_index.h"


/* Data structures */

typedef struct {
    uint32_t          NodesUsed;        // Number of index nodes actually used
    uint32_t          NodesAvailable;   // Number of index nodes available in the table
    uint32_t          NodeIncrement;    // Amount to increase the number of nodes
    I106C10Header     Header;           // Header and buffer for an IRIG Format 1 time packet. 
    void            * TimePacket;       // This is necessary for date format and leap year flags
    I106Time          Time;             // as well as relative time to absolute time mapping
                                      // if absolute time isn't provided.
    PacketIndexInfo * IndexTable;  // The main table of indexes
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
        status = I106C10FirstMsg(handle);
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

        status = I106C10ReadNextHeader(handle, &header);
        if (status != I106_OK)
            break;

        if (header.DataType == I106CH10_DTYPE_RECORDING_INDEX)
            *found_index = 0;

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
    status = IndexPresent(handle, &found_index);
    if (status != I106_OK)
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
    status = I106C10GetPos(handle, &pos);

	// Root packet found so start processing root index packets
    while (1){
        // Process the root packet at the given offset
        status = ProcessRootIndexPacket(handle, pos, &next);

        // Check for exit conditions
        if (status != I106_OK)
            break;

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
    IndexMsg    msg;

    // Go to what should be a root index packet
    status = I106C10SetPos(handle, offset);
    if (status != I106_OK)
        return status;

    // Read what should be a root index packet
    status = I106C10ReadNextHeader(handle, &header);

    if (status != I106_OK)
        return status;

    if (header.DataType != I106CH10_DTYPE_RECORDING_INDEX)
        return I106_INVALID_DATA;

    // Read the index packet
    buffer = malloc(header.PacketLength);

    // Read the data buffer
    status = I106C10ReadData(handle, header.PacketLength, buffer);

    // Check for data read errors
    if (status != I106_OK)
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
    I106Status           status;
    I106C10Header        header;
    void               * buffer;
    SuTimeF1_ChanSpec  * csdw;

    // Store the info
    index_info.ChannelID  = channel_id;
    index_info.DataType   = data_type;
    index_info.Offset     = *(msg->FileOffset);
    index_info.RTC        = msg->Time->time;

    // If the optional intrapacket data header exists then get absolute time from it
    if (msg->CSDW->IPH == 1){
        csdw = (SuTimeF1_ChanSpec *)indices[handle].TimePacket;
        enI106_Decode_TimeF1_Buff(csdw->uDateFmt, csdw->bLeapYear,
            msg->Time, &index_info.IrigTime);
    }

    // Else if the indexed packet is a time packet then get the time from it
    else if (msg->NodeData->DataType == I106CH10_DTYPE_IRIG_TIME){
        // Go to what should be a time packet
        status = I106C10SetPos(handle, *(msg->FileOffset));

        // Read the packet header
        status = I106C10ReadNextHeader(handle, &header);

        // Make sure our buffer is big enough
        buffer = malloc(header.PacketLength);

        // Read the data buffer
        status = I106C10ReadData(handle, header.PacketLength, buffer);

        // Decode the time packet
        enI106_Decode_TimeF1(&header, buffer, &index_info.IrigTime);

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

        // Setup a one time loop to make it easy to break out on error
        do {
            if (status == I106_EOF)
                break;

            // Check for header read errors
            if (status != I106_OK)
                break;

            // If selected channel then put info into the index
            if (header.ChannelID == channel_id){

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

        } while (0);

        // If EOF break out of main read loop
        if (status == I106_EOF)
            break;
    }

    return I106_OK;
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
    SuTimeF1_ChanSpec  * time = NULL;

    // Get and save the current file position
    status = I106C10GetPos(handle, &offset);
    if (status != I106_OK)
        return status;

    // Get time from the middle of the file
    status = I106C10LastMsg(handle);
    status = I106C10GetPos(handle, &last);
    status = I106C10SetPos(handle, last/2);

    // Read the next header
    status = I106C10ReadNextHeaderFile(handle, &header);
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
                time         = (SuTimeF1_ChanSpec *)buffer;
                buffer_size  = header.PacketLength;
            }

            // Read the data buffer
            status = I106C10ReadData(handle, buffer_size, buffer);
            if (status != I106_OK){
                return_status = I106_TIME_NOT_FOUND;
                break;
            }

            // If external sync OK then decode it and set relative time
            if ((require_sync == 0) || (time->uTimeSrc == 1)){
                memcpy(&indices[handle].Header, &header, sizeof(I106C10Header));
                indices[handle].TimePacket = buffer;
                enI106_Decode_TimeF1(&header, buffer, &indices[handle].Time);
                return_status = I106_OK;
                break;
            }
        }

        // Read the next header and try again
        status = I106C10ReadNextHeaderFile(handle, &header);
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
