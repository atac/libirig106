
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#if defined(__GNUC__)
#include <unistd.h>
#endif

#include "libirig106.h"
#include "i106_time.h"


/* Macros and definitions */

// Size of buffer when searching in reverse order.
#define BACKUP_SIZE     256


/*  Module data */

I106C10Handle  handles[MAX_HANDLES];
static int     handles_inited = 0;


/* Function Declaration */

void InitHandles();
int GetHandle();
I106Status ValidHandle(int handle);
I106Status I106C10CheckOpen(int *handle, I106C10Mode mode);


I106Status InitHandle(int *handle, const char filename[]){
    // Get the next available handle and initialize it.
    InitHandles();
    if ((*handle = GetHandle()) == -1)
        return I106_NO_FREE_HANDLES;

    handles[*handle].File_State = I106_CLOSED;
    handles[*handle].Index.SortStatus = UNSORTED;
    strncpy(handles[*handle].FileName, filename, sizeof(handles[*handle].FileName));
    handles[*handle].FileName[sizeof(handles[*handle].FileName) - 1] = '\0';
    handles[*handle].BytesWritten = 0L;

    return I106_OK;
}


I106Status I106C10OpenBuffer(int *handle, void *buffer, int size, I106C10Mode mode){
    I106Status status;
    const char filename[] = "<buffer>";

    if ((status = InitHandle(handle, filename)))
        return status;

    // Write buffer to tmpfile and attach to handle
    if (mode == READ || mode == READ_IN_ORDER){
        if ((handles[*handle].fp = tmpfile()) == NULL)
            return I106_OPEN_ERROR;

        if (0 > (handles[*handle].File = fileno(handles[*handle].fp)))
            return I106_OPEN_ERROR;

        if (0 > write(handles[*handle].File, buffer, size))
            return I106_OPEN_ERROR;

        lseek(handles[*handle].File, 0, SEEK_SET);
    }

    // Any other mode is an error
    else {
        handles[*handle].File_State = I106_CLOSED;
        handles[*handle].FileMode  = CLOSED;
        handles[*handle].InUse = 0;
        *handle = -1;
        return I106_OPEN_ERROR;
    }

    return I106C10CheckOpen(handle, mode);
}

I106Status I106C10Open(int *handle, const char filename[], I106C10Mode mode){
    I106Status status;

    if ((status = InitHandle(handle, filename)))
        return status;

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

    return I106C10CheckOpen(handle, mode);
}


I106Status I106C10CheckOpen(int *handle, I106C10Mode mode){
    int            read_count;
    uint16_t       signature;

    if (handles[*handle].File == -1){
        handles[*handle].InUse = 0;
        *handle = -1;
        return I106_OPEN_ERROR;
    }

    if (mode == READ || mode == READ_IN_ORDER){

        // Check for valid sync pattern
        // TODO: re-enable this later. Not appropriate for OpenBuffer
        /* read_count = read(handles[*handle].File, &signature, 2); */
        /* if (read_count != 2 || signature != IRIG106_SYNC){ */
        /*     close(handles[*handle].File); */
        /*     handles[*handle].InUse = 0; */
        /*     *handle = -1; */
        /*     return I106_OPEN_ERROR; */
        /* } */

        // Open OK and sync character OK so set read state to reflect this
        handles[*handle].FileMode   = mode;
        handles[*handle].File_State = I106_READ_HEADER;

        // @TODO: move this to a new "validate" function
        // Make sure first packet is a config packet
        /* I106C10SetPos(*handle, 0L); */
        /* if ((status = I106C10ReadNextHeaderFile(*handle, &header))) */
        /*     return I106_OPEN_WARNING; */
        /* if (header.DataType != I106CH10_DTYPE_COMPUTER_1) */
        /*     return I106_OPEN_WARNING; */

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

    if ((handles[handle].fp != NULL) && (handles[handle].InUse == 1))
        fclose(handles[handle].fp);

    // Free index buffer and mark unsorted
    free(handles[handle].Index.Index);
    handles[handle].Index.Index          = NULL;
    handles[handle].Index.ArraySize      = 0;
    handles[handle].Index.ArrayUsed      = 0;
    handles[handle].Index.NumSearchSteps = 0;
    handles[handle].Index.SortStatus     = UNSORTED;

    // Reset some status variables
    handles[handle].File       = -1;
    handles[handle].fp         = NULL;
    handles[handle].InUse      = 0;
    handles[handle].FileMode   = CLOSED;
    handles[handle].File_State = I106_CLOSED;

    return status;
}


// Simple header validation. Returns 1 = valid, 0 = invalid
int ValidHeader(I106C10Header *header){
    if (header->SyncPattern != IRIG106_SYNC)
        return 0;

    else if (header->Checksum != HeaderChecksum(header))
        return 0;

    if (header->PacketFlags & I106CH10_PFLAGS_SEC_HEADER)
        if (header->SecondaryChecksum != SecondaryHeaderChecksum(header))
            return 0;

    return 1;
}


// Reads a header from a file. If ftell() is not at a sync pattern will seek
// forward to find one.
I106Status I106NextHeader(int fd, I106C10Header *header){
    int read_count, valid = 1;
    int64_t offset;

    // Read what we think is a header, and keep reading if things don't look correct.
    while (1){

        // Find the offset where we start parsing
        offset = lseek(fd, 0, SEEK_CUR);

        // Read the header and secondary header if present
        read_count = read(fd, header, HEADER_SIZE);
        if (read_count != HEADER_SIZE){
            if (read_count == -1)
                return I106_READ_ERROR;
            return I106_EOF;
        }

        if (header->PacketFlags & I106CH10_PFLAGS_SEC_HEADER){
            read_count = read(fd, &header->Time[0], SEC_HEADER_SIZE);
            if (read_count != SEC_HEADER_SIZE){
                if (read_count == -1)
                    return I106_READ_ERROR;
                return I106_EOF;
            }
        }

        // If not valid, seek forward and try again
        if (!ValidHeader(header)){
            if ((lseek(fd, offset + 1, SEEK_SET)) == -1)
                return I106_SEEK_ERROR;
            if (lseek(fd, 0, SEEK_CUR) <= offset)
                return I106_EOF;
            continue;
        }

        // Otherwise return
        return I106_OK;
    }
}


// Reads a header from a buffer. If not at a sync pattern will seek
// forward to find one.
I106Status I106NextHeaderBuffer(char *buffer, int64_t buffer_size, int64_t offset, I106C10Header *header){

    // Read what we think is a header, and keep reading if things don't look correct.
    while (1){

        if (offset > buffer_size)
            return I106_EOF;

        // Read the header and check for errors
        memcpy(header, buffer+offset, 32);

        // If not valid, seek forward and try again
        if (!ValidHeader(header)){
            offset += 1;
            continue;
        }

        // Otherwise return
        return I106_OK;
    }
}


// Search backwards for a header
I106Status I106PrevHeader(int fd, I106C10Header *header){
    I106Status status;
    int64_t offset = lseek(fd, 0, SEEK_CUR);

    while (1){
        offset -= 30;
        if (lseek(fd, offset, SEEK_SET) == -1)
            if (lseek(fd, 0, SEEK_CUR) == 0)
                return I106_BOF;
            else
                return I106_SEEK_ERROR;
        if ((status = I106NextHeader(fd, header)))
            continue;

        return status;
    }
}


// Search backwards for a header
I106Status I106PrevHeaderBuffer(char *buffer, int64_t buffer_size, int64_t offset, I106C10Header *header){
    I106Status status;

    while (1){
        offset -= 30;
        memcpy(header, buffer+offset, 32);
        if ((status = I106NextHeaderBuffer(buffer, buffer_size, offset, header)))
            continue;

        return status;
    }
}


// Get the next header.  Depending on how the file was opened for reading,
// call the appropriate routine.
I106Status I106C10ReadNextHeader(int handle, I106C10Header *header){
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

                if ((status = I106C10SetPos(handle, offset))){
                    if (status == I106_EOF)
                        return I106_EOF;
                    return I106_SEEK_ERROR;
                }
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

        // Find the offset where we start parsing
        if ((status = I106C10GetPos(handle, &offset)))
            return I106_SEEK_ERROR;

        // Read the header
        if (handles[handle].FileMode != READ_NET_STREAM)
            read_count = read(handles[handle].File, header, HEADER_SIZE);

        // Keep track of how much header we've read
        handles[handle].HeaderBufferLength = HEADER_SIZE;

        // If there was an error reading, figure out why
        if (read_count != HEADER_SIZE){
            printf("Read count %i != header size: %i at offset %lu", read_count, HEADER_SIZE, offset);
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

            if ((status = I106C10SetPos(handle, offset + 1))){
                if (status == I106_EOF)
                    return status;
                return I106_SEEK_ERROR;
            }
        }

    }

    // Save some data for later use
    handles[handle].PacketLength      = header->PacketLength;
    handles[handle].DataBufferLength  = GetDataLength(header);
    handles[handle].DataBufferPos     = 0;
    handles[handle].File_State        = I106_READ_DATA;

    return I106_OK;
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
        off_t read_count = read(handles[handle].File, buffer, buffer_size + HEADER_SIZE);
        assert(read_count != -1);
        if (read_count == 0){
            pos-= BACKUP_SIZE;
            continue;
        }

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
            if ((status = I106C10ReadNextHeader(handle, header)))
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
    int             read_count = 0;
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


I106Status I106C10LastMsg(int handle){
    I106Status     status;
    int64_t        pos;
    I106C10Header  header;

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
    pos = (int64_t)lseek(handles[handle].File, 0, SEEK_END) - (int64_t)HEADER_SIZE;

    // Seek to the end of the file
    if ((status = I106C10SetPos(handle, pos)))
        return status;

    // Find last header
    if ((status = I106C10ReadPrevHeader(handle, &header)))
        return status;

    // Seek back to the beginning of the packet
    I106C10GetPos(handle, &pos);
    pos -= handles[handle].HeaderBufferLength;
    if ((status = I106C10SetPos(handle, pos)))
        return status;

    return I106_OK;
}


I106Status I106C10SetPos(int handle, int64_t offset){
    off_t pos = lseek(handles[handle].File, (off_t)offset, SEEK_SET);
    if (pos != offset)
        return I106_EOF;
    return I106_OK;
}


I106Status I106C10GetPos(int handle, int64_t *offset){
    *offset = (int64_t)lseek(handles[handle].File, (off_t)0, SEEK_CUR);
    assert(*offset >= 0);
    return I106_OK;
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
