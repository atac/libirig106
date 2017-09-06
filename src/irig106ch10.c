/****************************************************************************

 irig106ch10.c

 ****************************************************************************/

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(__GNUC__)
#include <unistd.h>
#endif

/* #ifndef __APPLE__ */
/* #if defined(__GNUC__) */
/* #include <sys/io.h> */
/* #else */
/* #include <io.h> */
/* #endif */
/* #endif */

#include "config.h"
#include "int.h"

#include "irig106ch10.h"
#include "i106_time.h"


/* Macros and definitions */

#define BACKUP_SIZE     256


/*  Module data */

I106C10Handle  handles[MAX_HANDLES];
static int     handles_inited = 0;


/* Function Declaration */

void InitHandles();
int GetHandle();
I106Status ValidHandle(int handle);


I106Status I106C10Open(int *handle, const char filename[], I106C10Mode mode){
    int            read_count;
    uint16_t       signature;
    I106Status     status;
    I106C10Header  header;

    // Get the next available handle and initialize it.
    InitHandles();
    if ((*handle = GetHandle()) == -1)
        return I106_NO_FREE_HANDLES;

    handles[*handle].File_State = I106_CLOSED;
    handles[*handle].Index.SortStatus = UNSORTED;
    strncpy (handles[*handle].FileName, filename, sizeof(handles[*handle].FileName));
    handles[*handle].FileName[sizeof(handles[*handle].FileName) - 1] = '\0';
    handles[*handle].BytesWritten = 0L;

    // Open file in correct mode
    if (mode == READ || mode == READ_IN_ORDER)
        handles[*handle].File = open(filename, READ_FLAGS, 0);
    else if (mode == OVERWRITE)
        handles[*handle].File = open(filename, OVERWRITE_FLAGS, OVERWRITE_MODE);

    // Any other mode is an error
    else {
        handles[*handle].File_State = I106_CLOSED;
        handles[*handle].FileMode  = CLOSED;
        handles[*handle].InUse = 0;
        *handle = -1;
        return I106_OPEN_ERROR;
    }

    if (handles[*handle].File == -1){
        handles[*handle].InUse = 0;
        *handle = -1;
        return I106_OPEN_ERROR;
    }

    if (mode == READ || mode == READ_IN_ORDER){

        // Check for valid sync pattern
        read_count = read(handles[*handle].File, &signature, 2);
        if (read_count != 2 || signature != IRIG106_SYNC){
            close(handles[*handle].File);
            handles[*handle].InUse = 0;
            *handle = -1;
            return I106_OPEN_ERROR;
        }

        // Open OK and sync character OK so set read state to reflect this
        handles[*handle].FileMode   = mode;
        handles[*handle].File_State = I106_READ_HEADER;

        // Make sure first packet is a config packet
        I106C10SetPos(*handle, 0L);
        if ((status = I106C10ReadNextHeaderFile(*handle, &header)))
            return I106_OPEN_WARNING;
        if (header.DataType != I106CH10_DTYPE_COMPUTER_1)
            return I106_OPEN_WARNING;

        // Everything OK so get time and reset back to the beginning
        I106C10SetPos(*handle, 0L);
        handles[*handle].File_State = I106_READ_HEADER;
        handles[*handle].FileMode = mode;
    }

    else if (mode == OVERWRITE){
        handles[*handle].File_State = I106_WRITE;
        handles[*handle].FileMode = mode;
    }

    return I106_OK;
}


I106Status I106C10Close(int handle){
    I106Status status = I106_OK;

    // If handles have not been init'ed then bail
    if (handles_inited == 0)
        return I106_NOT_OPEN;

    // Check for a valid handle
    if ((status = ValidHandle(handle)))
        return status;

    // Close file if open
    if ((handles[handle].File != -1) && (handles[handle].InUse == 1))
        close(handles[handle].File);

    // Free index buffer and mark unsorted
    free(handles[handle].Index.Index);
    handles[handle].Index.Index          = NULL;
    handles[handle].Index.ArraySize      = 0;
    handles[handle].Index.ArrayUsed      = 0;
    handles[handle].Index.NumSearchSteps = 0;
    handles[handle].Index.SortStatus     = UNSORTED;

    // Reset some status variables
    handles[handle].File       = -1;
    handles[handle].InUse      = 0;
    handles[handle].FileMode   = CLOSED;
    handles[handle].File_State = I106_CLOSED;

    return status;
}


// Get the next header.  Depending on how the file was opened for reading,
// call the appropriate routine.
I106Status I106C10ReadNextHeader(int handle, I106C10Header *header){
    I106C10Mode mode = handles[handle].FileMode;

    if (mode == READ_NET_STREAM || mode == READ || mode == READ_IN_ORDER){
        if (mode == READ_IN_ORDER && handles[handle].Index.SortStatus == SORTED)
            return I106C10ReadNextHeaderInOrder(handle, header);
        return I106C10ReadNextHeaderFile(handle, header);
    }

    return I106_WRONG_FILE_MODE;
}


// Get the next header in the file from the current position
I106Status I106C10ReadNextHeaderFile(int handle, I106C10Header * header){
    int         read_count;
    int         header_ok;
    int64_t     offset;
    I106Status  status;

    // Check for a valid handle
    if ((status = ValidHandle(handle)))
        return status;

    // Check for invalid file modes
    I106C10Mode mode = handles[handle].FileMode;
    if (mode == CLOSED)
        return I106_NOT_OPEN;
    if (mode != READ && mode != READ_NET_STREAM && mode != READ_IN_ORDER)
        return I106_WRONG_FILE_MODE;

    // Check file state, and ensure we're at the start of a packet header.
    switch (handles[handle].File_State){
        case I106_CLOSED:
            return I106_NOT_OPEN;

        case I106_READ_DATA:
            if (mode != READ_NET_STREAM){
                if ((status = I106C10GetPos(handle, &offset)))
                    return I106_SEEK_ERROR;

                offset += handles[handle].PacketLength - 
                          handles[handle].HeaderBufferLength -
                          handles[handle].DataBufferPos;

                if ((status = I106C10SetPos(handle, offset)))
                    return I106_SEEK_ERROR;
            }

        case I106_READ_NET_STREAM:
        case I106_READ_HEADER:
        case I106_READ_UNSYNCED :
            break;

        default:
            return I106_WRONG_FILE_MODE;
    }

    // Now we might be at the beginning of a header. Read what we think
    // is a header, check it, and keep reading if things don't look correct.
    while (1){
        header_ok = 1;

        // Read the header
        if (handles[handle].FileMode != READ_NET_STREAM)
            read_count = read(handles[handle].File, header, HEADER_SIZE);

        // Keep track of how much header we've read
        handles[handle].HeaderBufferLength = HEADER_SIZE;

        // If there was an error reading, figure out why
        if (read_count != HEADER_SIZE){
            handles[handle].File_State = I106_READ_UNSYNCED;
            if (read_count == -1)
                return I106_READ_ERROR;
            return I106_EOF;
        }

        // Read OK, check the sync field
        if (header->SyncPattern != IRIG106_SYNC){
            handles[handle].File_State = I106_READ_UNSYNCED;
            header_ok = 0;
        }

        // Check the header checksum
        else if (header->Checksum != HeaderChecksum(header)){
            // If the header checksum was bad then set to unsynced state
            // and return the error. Next time we're called we'll go
            // through lots of heroics to find the next header.
            if (handles[handle].File_State != I106_READ_UNSYNCED){
                handles[handle].File_State = I106_READ_UNSYNCED;
                return I106_HEADER_CHKSUM_BAD;
            }
            header_ok = 0;
        }

        // @TODO: header version check?

        // Check secondary header if present 
        else if (header->PacketFlags & I106CH10_PFLAGS_SEC_HEADER){
            // Read the secondary header
            if (handles[handle].FileMode != READ_NET_STREAM)
                read_count = read(handles[handle].File, &header->Time[0], SEC_HEADER_SIZE);

            // Keep track of how much header we've read
            handles[handle].HeaderBufferLength += SEC_HEADER_SIZE;

            // If there was an error reading, figure out why
            if (read_count != SEC_HEADER_SIZE){
                handles[handle].File_State = I106_READ_UNSYNCED;
                if (read_count == -1)
                    return I106_READ_ERROR;
                return I106_EOF;
            }

            // Secondary header checksum
            else if (header->SecondaryChecksum != SecondaryHeaderChecksum(header)){
                // If the header checksum was bad then set to unsynced state
                // and return the error. Next time we're called we'll go
                // through lots of heroics to find the next header.
                if (handles[handle].File_State != I106_READ_UNSYNCED){
                    handles[handle].File_State = I106_READ_UNSYNCED;
                    return I106_HEADER_CHKSUM_BAD;
                }
                header_ok = 0;
            }
        }

        // If read header was OK then break out
        if (header_ok)
            break;

        // Read header was not OK so try again beyond previous read point
        if (handles[handle].FileMode != READ_NET_STREAM){
            if ((status = I106C10GetPos(handle, &offset)))
                return I106_SEEK_ERROR;

            offset -= handles[handle].HeaderBufferLength + 1;

            if ((status = I106C10SetPos(handle, offset)))
                return I106_SEEK_ERROR;
        }

    }

    // Save some data for later use
    handles[handle].PacketLength      = header->PacketLength;
    handles[handle].DataBufferLength  = GetDataLength(header);
    handles[handle].DataBufferPos     = 0;
    handles[handle].File_State        = I106_READ_DATA;

    return I106_OK;
}


// Get the next header in time order from the file
I106Status I106C10ReadNextHeaderInOrder(int handle, I106C10Header *header){
    InOrderIndex   *index = &handles[handle].Index;
    I106Status      status;
    int64_t         offset;
    I106FileState   saved_file_state;

    // If we're at the end of the list then we are at the end of the file
    if (index->ArrayPos == index->ArrayUsed)
        return I106_EOF;

    // Save the read state going in
    saved_file_state = handles[handle].File_State;

    // Move file pointer to the proper, er, point
    offset = index->Index[index->ArrayPos].Offset;
    status = I106C10SetPos(handle, offset);

    // Go ahead and get the next header
    status = I106C10ReadNextHeaderFile(handle, header);

    // If the state was unsynced before but is synced now, figure out where in the
    // index we are
    if ((saved_file_state == I106_READ_UNSYNCED) && (handles[handle].File_State != I106_READ_UNSYNCED)){
        I106C10GetPos(handle, &offset);
        offset -= GetHeaderLength(header);
        index->ArrayPos = 0;
        while (index->ArrayPos < index->ArrayUsed){
            if (offset == index->Index[index->ArrayPos].Offset)
                break;
            index->ArrayPos++;
        }
        // if psuIndex->iArrayCurr == psuIndex->iArrayUsed then bad things happened
    }

    // Move array index to the next element
    index->ArrayPos++;

    return status;
}


I106Status I106C10ReadPrevHeader(int handle, I106C10Header *header){
    int         buffer_size, buffer_pos;
    int64_t     pos, initial_backup = 0;
    I106Status  status;
    uint8_t     buffer[BACKUP_SIZE + HEADER_SIZE];
    uint16_t    checksum;

    // Check for a valid handle
    if ((status = ValidHandle(handle)))
        return status;

    // Check for invalid file modes
    I106C10Mode mode = handles[handle].FileMode;
    if (mode == CLOSED)
        return I106_NOT_OPEN;
    // @TODO: handle read_in_order mode
    if (mode != READ && mode != READ_IN_ORDER && mode != READ_NET_STREAM)
        return I106_WRONG_FILE_MODE;

    // Check file state
    I106FileState state = handles[handle].File_State;
    if (state == I106_CLOSED)
        return I106_NOT_OPEN;
    // Backup to a point just before the most recently read header.
    // The amount to backup is the size of the previous header and the amount
    // of data already read.
    if (state == I106_READ_DATA)
        initial_backup = handles[handle].HeaderBufferLength + handles[handle].DataBufferPos;
    else if (state != I106_READ_HEADER && state != I106_READ_UNSYNCED)
        return I106_WRONG_FILE_MODE;

    // This puts us at the beginning of the most recently read header (or BOF)
    I106C10GetPos(handle, &pos);
    pos -= initial_backup;

    // If at the beginning of the file then done, return BOF
    if (pos <= 0){
        I106C10SetPos(handle, 0);
        return I106_BOF;
    }

    // Loop until previous packet found
    while (1){

        // Figure out how much to backup
        if (pos >= BACKUP_SIZE)
            buffer_size = BACKUP_SIZE;
        else
            buffer_size = (int)pos;

        // Backup that amount
        I106C10SetPos(handle, pos - buffer_size);

        // Read a buffer of data to scan backwards through
        read(handles[handle].File, buffer, buffer_size + HEADER_SIZE);

        // Go to the end of the buffer and start scanning backwards
        for (buffer_pos = buffer_size - 1; buffer_pos >= 0; buffer_pos--){

            // Keep track of where we are in the file
            pos--;

            // Check for sync pattern
            if ((buffer[buffer_pos] != 0x25) || (buffer[buffer_pos + 1] != 0xEB))
                continue;

            // Compute checksum
            checksum = HeaderChecksum((I106C10Header *)(&buffer[buffer_pos]));
            if (checksum != ((I106C10Header *)(&buffer[buffer_pos]))->Checksum)
                continue;

            // Let ReadNextHeader() have a crack
            if ((status = I106C10SetPos(handle, pos)))
                continue;
            if ((status = I106C10ReadNextHeaderFile(handle, header)))
                continue;

            // If everything checks out, return OK.
            return status;
        }

        // Return a seek error if we've reached BOF without a valid header.
        if (pos == 0)
            return I106_SEEK_ERROR;
    }
}


// Read chapter 10 data based on file mode.
I106Status I106C10ReadData(int handle, unsigned long buffer_size, void *buffer){
    I106C10Mode mode = handles[handle].FileMode;

    if (mode == READ_NET_STREAM || mode == READ || mode == READ_IN_ORDER)
        return I106C10ReadDataFile(handle, buffer_size, buffer);

    return I106_WRONG_FILE_MODE;
}


I106Status I106C10ReadDataFile(int handle, unsigned long buffer_size, void *buffer){
    int             read_count;
    unsigned long   read_amount;
    I106Status      status = I106_OK;

    // Check for a valid handle
    if ((status = ValidHandle(handle)))
        return I106_INVALID_HANDLE;

    // Check for invalid file modes
    I106C10Mode mode = handles[handle].FileMode;
    if (mode == CLOSED)
        return I106_NOT_OPEN;
    else if (mode != READ)
        return I106_WRONG_FILE_MODE;

    // Check file state
    I106FileState state = handles[handle].File_State;
    if (state == I106_CLOSED)
        return I106_NOT_OPEN;
    else if (state == I106_WRITE)
        return I106_WRONG_FILE_MODE;
    else if (state != I106_READ_DATA){
        // @TODO: MIGHT WANT TO SUPPORT THE "MORE DATA" METHOD INSTEAD
        handles[handle].File_State = I106_READ_UNSYNCED;
        return I106_READ_ERROR;
    }

    // Make sure there is enough room in the user buffer
    // @TODO: MIGHT WANT TO SUPPORT THE "MORE DATA" METHOD INSTEAD
    read_amount = handles[handle].DataBufferLength - handles[handle].DataBufferPos;
    if (buffer_size < read_amount)
        return I106_BUFFER_TOO_SMALL;

    // Read the data, filler, and data checksum
    if (handles[handle].FileMode != READ_NET_STREAM)
        read_count = read(handles[handle].File, buffer, read_amount);

    // If there was an error reading, figure out why
    if ((unsigned long)read_count != read_amount){
        handles[handle].File_State = I106_READ_UNSYNCED;
        if (read_count == -1)
            return I106_READ_ERROR;
        return I106_EOF;
    }

    // Keep track of our read position in the current data buffer
    handles[handle].DataBufferPos = read_amount;

    // @TODO: MAY WANT TO DO CHECKSUM CHECKING SOMEDAY

    // Expect a header next read
    handles[handle].File_State = I106_READ_HEADER;

    return I106_OK;
}    


I106Status I106C10WriteMsg(int handle, I106C10Header *header, void *buffer){

    // Check for a valid handle
    if (ValidHandle(handle))
        return I106_INVALID_HANDLE;

    // Check for invalid file modes
    I106C10Mode mode = handles[handle].FileMode;
    if (mode == CLOSED)
        return I106_NOT_OPEN;
    else if (mode != OVERWRITE)
        return I106_WRONG_FILE_MODE;

    // Figure out header length
    int header_length = GetHeaderLength(header);

    // Write the header
    int write_count = write(handles[handle].File, header, header_length);

    // If there was an error reading, figure out why
    if (write_count != header_length)
        return I106_WRITE_ERROR;
    
    // Write the data
    write_count = write(
        handles[handle].File, buffer, header->PacketLength - header_length);

    // If there was an error reading, figure out why
    if ((unsigned long)write_count != (header->PacketLength - header_length))
        return I106_WRITE_ERROR;

    // Update the number of bytes written
    handles[handle].BytesWritten += header->PacketLength;

    return I106_OK;
}


// Move file pointer
I106Status I106C10FirstMsg(int handle){

    if (ValidHandle(handle))
        return I106_INVALID_HANDLE;

    // Check file modes
    I106C10Mode mode = handles[handle].FileMode;
    if (mode == CLOSED)
        return I106_NOT_OPEN;
    else if (mode == READ || mode == READ_IN_ORDER){
        if (mode == READ_IN_ORDER)
            handles[handle].Index.ArrayPos = 0;
        return I106C10SetPos(handle, 0L);
    }

    return I106_WRONG_FILE_MODE;
}


I106Status I106C10LastMsg(int handle){
    I106Status     return_status, status;
    int64_t        pos;
    I106C10Header  header;
    int            read_count;  
    struct stat    stat_buffer, file_stat;

    // Check for a valid handle
    if (ValidHandle(handle))
        return I106_INVALID_HANDLE;

    // Check file modes
    if (handles[handle].FileMode == CLOSED)
        return I106_NOT_OPEN;
    else if (handles[handle].FileMode == READ_IN_ORDER){
        handles[handle].Index.ArrayPos = handles[handle].Index.ArrayUsed - 1;
        return I106_OK;
    }
    else if (handles[handle].FileMode != READ)
        return I106_WRONG_FILE_MODE;

    // Figure out how big the file is and go to the end
    fstat(handles[handle].File, &stat_buffer);
    pos = stat_buffer.st_size - HEADER_SIZE;

    // Seek to the end of the file
    if ((status = I106C10SetPos(handle, pos)))
        return status;

    // Find last header
    if ((status = I106C10ReadPrevHeader(handle, &header)))
        return status;

    // Seek back to the beginning of the packet
    pos -= handles[handle].HeaderBufferLength;
    if ((status = I106C10SetPos(handle, pos)))
        return status;

    return I106_OK;
}


I106Status I106C10SetPos(int handle, int64_t offset){

    // Check for a valid handle
    if ((handle <  0) || (handle >= MAX_HANDLES) || (handles[handle].InUse == 0))
        return I106_INVALID_HANDLE;

    // Check file modes
    switch (handles[handle].FileMode){
        case CLOSED:
            return I106_NOT_OPEN;
            break;

        case OVERWRITE:
        case APPEND:
        case READ_NET_STREAM: 
        default:
            return I106_WRONG_FILE_MODE;
            break;

        case READ_IN_ORDER:
        case READ:
            // Seek
#if defined(_WIN32)
            {
                __int64  status;
                status = _lseeki64(handles[handle].File, offset, SEEK_SET);
            }
#else
            {
                off64_t  status;
                status = lseek64(handles[handle].File, (off64_t)offset, SEEK_SET);
                assert(status >= 0);
            }
#endif

            // Can't be sure we're on a message boundary so set unsync'ed
            handles[handle].File_State = I106_READ_UNSYNCED;
            break;
    }

    return I106_OK;
}


I106Status I106C10GetPos(int handle, int64_t *offset){

    // Check for a valid handle
    if ((handle <  0) || (handle >= MAX_HANDLES) || (handles[handle].InUse == 0))
        return I106_INVALID_HANDLE;

    // Check file modes
    switch (handles[handle].FileMode){
        case CLOSED:
            return I106_NOT_OPEN;
            break;

        case OVERWRITE:
        case APPEND:
        case READ_NET_STREAM: 
        default:
            return I106_WRONG_FILE_MODE;
            break;

        case READ_IN_ORDER:
        case READ:
            // Get position
#if defined(_WIN32)
            *offset = _telli64(handles[handle].File);
#else
            {
                *offset = (int64_t)lseek64(handles[handle].File, (off64_t)0, SEEK_CUR);
                assert(*offset >= 0);
            }
#endif
            break;
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
    int  header_length;

    if ((header->PacketFlags & I106CH10_PFLAGS_SEC_HEADER) == 0)
        header_length = HEADER_SIZE;
    else
        header_length = HEADER_SIZE + SEC_HEADER_SIZE;

    return header_length;
}


// Figure out data length including padding and any data checksum
uint32_t GetDataLength(I106C10Header *header){
    int data_length;

    data_length  = header->PacketLength - GetHeaderLength(header);

    return data_length;
}


uint16_t HeaderChecksum(I106C10Header *header){
    uint16_t        header_sum;
    uint16_t      * header_array = (uint16_t *)header;

    header_sum = 0;
    for (int i=0; i<(HEADER_SIZE-2)/2; i++)
        header_sum += header_array[i];

    return header_sum;
}


uint16_t SecondaryHeaderChecksum(I106C10Header *header){
    uint16_t        sum;
    // TODO: MAKE THIS 16 BIT UNSIGNEDS LIKE ABOVE
    unsigned char * byte_array = (unsigned char *)header;

    sum = 0;
    for (int i=0; i<SEC_HEADER_SIZE-2; i++)
        sum += byte_array[i + HEADER_SIZE];

    return sum;
}


uint32_t BufferSize(uint32_t data_length, int checksum_type){
    
    // Start with the length of the data
    uint32_t  buffer_length = data_length;

    // Add in enough for the selected checksum
    switch (checksum_type){
        case I106CH10_PFLAGS_CHKSUM_NONE :
            break;
        case I106CH10_PFLAGS_CHKSUM_8    :
            buffer_length += 1;
            break;
        case I106CH10_PFLAGS_CHKSUM_16   :
            buffer_length += 2;
            break;
        case I106CH10_PFLAGS_CHKSUM_32   :
            buffer_length += 4;
            break;
        default :
            buffer_length = 0;
        }

    // Now add filler for 4 byte alignment
    buffer_length += 3;
    buffer_length &= 0xfffffffc;

    return buffer_length;
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
    buffer_size = BufferSize(header->DataLength, checksum_type);

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
    char  *error_msg;

    switch (status){
        case I106_OK                : error_msg = "No error";              break;
        case I106_OPEN_ERROR        : error_msg = "File open failed";      break;
        case I106_OPEN_WARNING      : error_msg = "File open warning";     break;
        case I106_EOF               : error_msg = "End of file";           break;
        case I106_BOF               : error_msg = "Beginning of file";     break;
        case I106_READ_ERROR        : error_msg = "Read error";            break;
        case I106_WRITE_ERROR       : error_msg = "Write error";           break;
        case I106_MORE_DATA         : error_msg = "More data available";   break;
        case I106_SEEK_ERROR        : error_msg = "Seek error";            break;
        case I106_WRONG_FILE_MODE   : error_msg = "Wrong file mode";       break;
        case I106_NOT_OPEN          : error_msg = "File not open";         break;
        case I106_ALREADY_OPEN      : error_msg = "File already open";     break;
        case I106_BUFFER_TOO_SMALL  : error_msg = "Buffer too small";      break;
        case I106_NO_MORE_DATA      : error_msg = "No more data";          break;
        case I106_NO_FREE_HANDLES   : error_msg = "No free file handles";  break;
        case I106_INVALID_HANDLE    : error_msg = "Invalid file handle";   break;
        case I106_TIME_NOT_FOUND    : error_msg = "Time not found";        break;
        case I106_HEADER_CHKSUM_BAD : error_msg = "Bad header checksum";   break;
        case I106_NO_INDEX          : error_msg = "No index";              break;
        case I106_UNSUPPORTED       : error_msg = "Unsupported feature";   break;
        case I106_BUFFER_OVERRUN    : error_msg = "Buffer overrun";        break;
        case I106_INDEX_NODE        : error_msg = "Index node";            break;
        case I106_INDEX_ROOT        : error_msg = "Index root";            break;
        case I106_INDEX_ROOT_LINK   : error_msg = "Index root link";       break;
        case I106_INVALID_DATA      : error_msg = "Invalid data";          break;
        case I106_INVALID_PARAMETER : error_msg = "Invalid parameter";     break;
        default                     : error_msg = "Unknown error";         break;
    }

    return error_msg;
}


// Initialize handle data if necessary
void InitHandles(){
    if (handles_inited == 0){
        for (int i=0; i<MAX_HANDLES; i++){
            handles[i].InUse     = 0;
            handles[i].FileMode  = CLOSED;
            handles[i].File_State = I106_CLOSED;
        }
        handles_inited = 1;
    }
}


// Get the next available handle
int GetHandle(){
    int  handle;

    handle = -1;
    for (int i=0; i<MAX_HANDLES; i++){
        if (handles[i].InUse == 0){
            handles[i].InUse = 1;
            return i;
        }
    }

    return handle;
}


I106Status ValidHandle(int handle){
    if ((handle <  0) || (handle >= MAX_HANDLES) || (handles[handle].InUse == 0))
        return I106_INVALID_HANDLE;
    return I106_OK;
}


// TODO : Move this functionality to i106_index.*

// -----------------------------------------------------------------------
// Generate an index from the data file
// -----------------------------------------------------------------------

/*  
Support for read back in time order is experimental.  Some 106-04 recorders 
record data *way* out of time order.  But most others don't.  And starting
with 106-05 the most out of order is 1 second.

The best way to support read back in order is to do it on the fly as the file
is being read.  But that's more than I'm willing to do right now.  This indexing
scheme does get the job done for now.
*/

// Read the index from a previously generated index file.
int ReadInOrderIndex(int handle, char *filename){
    int               file;
    int               flags;
    int               read_start;
    int               read_count;
    int               read_ok = 0;
    InOrderIndex    * index = &handles[handle].Index;

    // Setup a one time loop to make it easy to break out on errors
    do {

        // Try opening and reading the index file
#if defined(_MSC_VER)
        flags = O_RDONLY | O_BINARY;
#else
        flags = O_RDONLY;
#endif
        file = open(filename, flags, 0);
        if (file == -1)
            break;

        // Read the index data from the file
        while (1){
            read_start = index->ArraySize;
            index->ArraySize += 100;
            index->Index = (InOrderPacketInfo *)realloc(index->Index,
                sizeof(InOrderPacketInfo)*index->ArraySize);
            read_count = read(file, &(index->Index[read_start]),
                100*sizeof(InOrderPacketInfo));
            index->ArrayUsed += read_count / sizeof(InOrderPacketInfo);
            if (read_count != 100*sizeof(InOrderPacketInfo))
                break;
        }

        close(file);

        // MIGHT WANT TO DO SOME SANITY CHECKS IN HERE

        index->SortStatus = SORTED;
        read_ok = 1;
    } while (0);

    return read_ok;
}


int WriteInOrderIndex(int handle, char *filename){
    int               flags;
    int               file_mode;
    int               file;
    InOrderIndex    * index = &handles[handle].Index;

    // Write out an index file for use next time
#if defined(_MSC_VER)
    flags     = O_WRONLY | O_CREAT | O_BINARY;
    file_mode = _S_IREAD | _S_IWRITE;
#elif defined(__GNUC__)
#if __APPLE__
    flags = O_WRONLY | O_CREAT;
#else
    flags = O_WRONLY | O_CREAT | O_LARGEFILE;
#endif
    file_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
#else
    flags = O_WRONLY | O_CREAT;
    file_mode = 0;
#endif
    file = open(filename, flags, file_mode);
    if (file != -1){

        // Read the index data from the file
        for (int i=0; i<index->ArrayUsed; i++)
            write(file, &(index->Index[i]), sizeof(InOrderPacketInfo));

        close(file);
    }

    return 0;
}


// This is used in qsort in vMakeInOrderIndex() below
int FileTimeCompare(const void *index1, const void *index2){
    if (((InOrderPacketInfo *)index1)->Time < ((InOrderPacketInfo *)index2)->Time)
        return -1;
    if (((InOrderPacketInfo *)index1)->Time > ((InOrderPacketInfo *)index2)->Time)
        return  1;
    return 0;
}


// Read all headers and make an index based on time
void MakeInOrderIndex(int handle){
    I106Status        status;
    int64_t           start;    // File position coming in
    int64_t           pos;      // Current file position
    I106C10Header     header;   // Data packet header
    int64_t           time;     // Current header time
    InOrderIndex    * index = &handles[handle].Index;

    // Remember the current file position
    status = I106C10GetPos(handle, &start);

    status = I106C10SetPos(handle, 0L);

    // Read headers, put time and file offset into index array
    while (1){
        status = I106C10ReadNextHeaderFile(handle, &header);

        // If EOF break out
        if (status == I106_EOF)
            break;

        // If an error then clean up and get out
        if (status != I106_OK){
            free(index->Index);
            index->Index          = NULL;
            index->ArraySize      = 0;
            index->ArrayUsed      = 0;
            index->NumSearchSteps = 0;
            index->SortStatus     = SORT_ERROR;
            break;
        }

        // Get the time and position
        status = I106C10GetPos(handle, &pos);
        pos -= GetHeaderLength(&header);
        TimeArray2LLInt(header.RTC, &time);

        // Check the array size, make it bigger if necessary
        if (index->ArrayUsed >= index->ArraySize){
            index->ArraySize += 100;
            index->Index = (InOrderPacketInfo *)realloc(index->Index,
                sizeof(InOrderPacketInfo)*index->ArraySize);
        }

        // Copy the info into the next array element
        index->Index[index->ArrayUsed].Offset = pos;
        index->Index[index->ArrayUsed].Time   = time;
        index->ArrayUsed++;
    }

    // Sort the index array
    // It is required that TMATS is the first record and IRIG time is the
    // second record so don't include those in the sort
    qsort(&(index->Index[2]), index->ArrayUsed - 2, sizeof(InOrderPacketInfo),
        FileTimeCompare);

    // Put the file point back where we started and find the current index
    // THIS SHOULD REALLY BE DONE FOR THE FILE-READ-OK LOGIC PATH ALSO
    status = I106C10SetPos(handle, start);
    index->ArrayPos = 0;
    while (index->ArrayPos < index->ArrayUsed){
        if (start == index->Index[index->ArrayPos].Offset)
            break;
        index->ArrayPos++;
    }

    // If we didn't find it then it's an error
    if (index->ArrayPos == index->ArrayUsed){
        free(index->Index);
        index->Index          = NULL;
        index->ArraySize      = 0;
        index->ArrayUsed      = 0;
        index->NumSearchSteps = 0;
        index->SortStatus     = SORT_ERROR;
    }
    else
        index->SortStatus = SORTED;
}
