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


I106Status I106C10Open(int *handle, const char filename[], I106C10Mode mode){
    int            read_count;
    int            flags;
    int            file_mode;
    uint16_t       signature;
    I106Status     status;
    I106C10Header  header;

    // Initialize handle data if necessary
    InitHandles();

    // Get the next available handle
    *handle = GetHandle();
    if (*handle == -1)
        return I106_NO_FREE_HANDLES;

    // Initialize some data
    handles[*handle].File_State = I106_CLOSED;
    handles[*handle].Index.SortStatus = UNSORTED;

    // Get a copy of the file name
    strncpy (handles[*handle].FileName, filename, sizeof(handles[*handle].FileName));
    handles[*handle].FileName[sizeof(handles[*handle].FileName) - 1] = '\0';

    // Reset total bytes written
    handles[*handle].BytesWritten = 0L;


    /* Read Mode */

    // Open for read
    if ((mode == READ) || (mode == READ_IN_ORDER)){

        // Try to open file
        flags = O_RDONLY;
#if defined(_MSC_VER)
        flags |= O_BINARY;
#endif
#ifndef __APPLE__
        flags |= O_LARGEFILE;
#endif
        handles[*handle].File = open(filename, flags, 0);
        if (handles[*handle].File == -1){
            handles[*handle].InUse = 0;
            *handle = -1;
            return I106_OPEN_ERROR;
        }
    

        /* Check to make sure it is a valid IRIG 106 Ch 10 data file */

        // Check for valid signature (sync pattern)

        // If we couldn't even read the first 2 bytes then return error
        read_count = read(handles[*handle].File, &signature, 2);
        if (read_count != 2){
            close(handles[*handle].File);
            handles[*handle].InUse = 0;
            *handle = -1;
            return I106_OPEN_ERROR;
        }

        // If the first word isn't the sync value then return error
        if (signature != IRIG106_SYNC){
            close(handles[*handle].File);
            handles[*handle].InUse = 0;
            *handle = -1;
            return I106_OPEN_ERROR;
        }

        // Open OK and sync character OK so set read state to reflect this
        handles[*handle].FileMode  = mode;
        handles[*handle].File_State = I106_READ_HEADER;

        // Make sure first packet is a config packet
        I106C10SetPos(*handle, 0L);
        status = I106C10ReadNextHeaderFile(*handle, &header);
        if (status != I106_OK)
            return I106_OPEN_WARNING;
        if (header.DataType != I106CH10_DTYPE_COMPUTER_1)
            return I106_OPEN_WARNING;

        // Everything OK so get time and reset back to the beginning
        I106C10SetPos(*handle, 0L);
        handles[*handle].File_State = I106_READ_HEADER;
        handles[*handle].FileMode = mode;
    }


    /* Overwrite Mode */

    // Open for overwrite
    else if (mode == OVERWRITE){

        // Try to open file
#if defined(_MSC_VER)
        flags = O_WRONLY | O_CREAT | _O_TRUNC | O_BINARY;
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
        handles[*handle].File = open(filename, flags, file_mode);
        if (handles[*handle].File == -1){
            handles[*handle].InUse = 0;
            *handle = -1;
            return I106_OPEN_ERROR;
        }

        // Open OK and write state to reflect this
        handles[*handle].File_State = I106_WRITE;
        handles[*handle].FileMode = mode;
    }


    /* Any other mode is an error */
    else {
        handles[*handle].File_State = I106_CLOSED;
        handles[*handle].FileMode  = CLOSED;
        handles[*handle].InUse = 0;
        *handle = -1;
        return I106_OPEN_ERROR;
    }

    return I106_OK;
}


I106Status I106C10Close(int handle){

    // If handles have not been init'ed then bail
    if (handles_inited == 0)
        return I106_NOT_OPEN;

    // Check for a valid handle
    if ((handle <  0) || (handle >= MAX_HANDLES) || (handles[handle].InUse == 0))
        return I106_INVALID_HANDLE;

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
    handles[handle].File      = -1;
    handles[handle].InUse     = 0;
    handles[handle].FileMode  = CLOSED;
    handles[handle].File_State = I106_CLOSED;

    return I106_OK;
}


// Get the next header.  Depending on how the file was opened for reading,
// call the appropriate routine.
I106Status I106C10ReadNextHeader(int handle, I106C10Header * header){
    I106Status status;

    switch (handles[handle].FileMode){
        case READ_NET_STREAM : 
        case READ : 
            status = I106C10ReadNextHeaderFile(handle, header);
            break;

        case READ_IN_ORDER : 
            if (handles[handle].Index.SortStatus == SORTED)
                status = I106C10ReadNextHeaderInOrder(handle, header);
            else
                status = I106C10ReadNextHeaderFile(handle, header);
            break;

        default :
            status = I106_WRONG_FILE_MODE;
            break;
    }
    
    return status;
}


// Get the next header in the file from the current position
I106Status I106C10ReadNextHeaderFile(int handle, I106C10Header * header){
    int         read_count;
    int         header_ok;
    int64_t     skip_size;
    int64_t     offset;
    I106Status  status;

    // Check for a valid handle
    if ((handle <  0) || (handle >= MAX_HANDLES) || (handles[handle].InUse == 0))
        return I106_INVALID_HANDLE;

    // Check for invalid file modes
    switch (handles[handle].FileMode){
        case CLOSED:
            return I106_NOT_OPEN;
            break;
        case OVERWRITE:
        case APPEND:
        case READ_IN_ORDER: 
        default:
            return I106_WRONG_FILE_MODE;
            break;
        case READ_NET_STREAM:
        case READ:
            break;
    }

    // Check file state
    switch (handles[handle].File_State){
        case I106_READ_NET_STREAM:
        case I106_CLOSED:
            return I106_NOT_OPEN;
            break;

        case I106_WRITE:
            return I106_WRONG_FILE_MODE;
            break;

        case I106_READ_HEADER:
            break;

        case I106_READ_DATA:
            skip_size = handles[handle].PacketLength - 
                handles[handle].HeaderBufferLength -
                handles[handle].DataBufferPos;

            if (handles[handle].FileMode != READ_NET_STREAM){
                status = I106C10GetPos(handle, &offset);
                if (status != I106_OK)
                    return I106_SEEK_ERROR;

                offset += skip_size;

                status = I106C10SetPos(handle, offset);
                if (status != I106_OK)
                    return I106_SEEK_ERROR;
            }

        case I106_READ_UNSYNCED :
            break;
    }

    // Now we might be at the beginning of a header. Read what we think
    // is a header, check it, and keep reading if things don't look correct.
    while (1){
        // Assume header is OK, only set false if not
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
            else
                return I106_EOF;
        }

        // Setup a one time loop to make it easy to break out if
        // there is an error encountered
        do {
            // Read OK, check the sync field
            if (header->SyncPattern != IRIG106_SYNC){
                handles[handle].File_State = I106_READ_UNSYNCED;
                header_ok = 0;
                break;
            }

            // Always check the header checksum
            if (header->Checksum != HeaderChecksum(header)){
                // If the header checksum was bad then set to unsynced state
                // and return the error. Next time we're called we'll go
                // through lots of heroics to find the next header.
                if (handles[handle].File_State != I106_READ_UNSYNCED){
                    handles[handle].File_State = I106_READ_UNSYNCED;
                    return I106_HEADER_CHKSUM_BAD;
                }
                header_ok = 0;
                break;
            }

            // MIGHT NEED TO CHECK HEADER VERSION HERE

            // Header seems OK at this point
            // Figure out if there is a secondary header
            if (header->PacketFlags & I106CH10_PFLAGS_SEC_HEADER){
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
                    else
                        return I106_EOF;
                }

                // Always check the secondary header checksum now
                if (header->SecondaryChecksum != SecondaryHeaderChecksum(header)){
                    // If the header checksum was bad then set to unsynced state
                    // and return the error. Next time we're called we'll go
                    // through lots of heroics to find the next header.
                    if (handles[handle].File_State != I106_READ_UNSYNCED){
                        handles[handle].File_State = I106_READ_UNSYNCED;
                        return I106_HEADER_CHKSUM_BAD;
                    }
                    header_ok = 0;
                    break;
                }

            }

        } while (0); // end one time loop

        // If read header was OK then break out
        if (header_ok)
            break;

        // Read header was not OK so try again beyond previous read point
        if (handles[handle].FileMode != READ_NET_STREAM){
            status = I106C10GetPos(handle, &offset);
            if (status != I106_OK)
                return I106_SEEK_ERROR;

            offset = offset - handles[handle].HeaderBufferLength + 1;

            status = I106C10SetPos(handle, offset);
            if (status != I106_OK)
                return I106_SEEK_ERROR;
        }

    }

    // Save some data for later use
    handles[handle].PacketLength      = header->PacketLength;
    handles[handle].DataBufferLength  = GetDataLength(header);
    handles[handle].DataBufferPos     = 0;
    handles[handle].File_State         = I106_READ_DATA;

    return I106_OK;
}


// Get the next header in time order from the file
I106Status I106C10ReadNextHeaderInOrder(int handle, I106C10Header * header){
    InOrderIndex  * index = &handles[handle].Index;
    I106Status      status;
    int64_t         offset;
    I106FileState       saved_file_state;

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


I106Status I106C10ReadPrevHeader(int handle, I106C10Header * header){
    int         still_reading;
    int         read_count;
    int64_t     pos;
    I106Status  status;

    uint64_t    initial_backup;
    int         backup_amount;
    uint64_t    next_read_pos;
    uint8_t     buffer[BACKUP_SIZE+HEADER_SIZE];
    int         buffer_pos;
    uint16_t    checksum;

    // Check for a valid handle
    if ((handle <  0) || (handle >= MAX_HANDLES) || (handles[handle].InUse == 0))
        return I106_INVALID_HANDLE;

    // Check for invalid file modes
    switch (handles[handle].FileMode){
        case CLOSED:
            return I106_NOT_OPEN;
            break;

        case OVERWRITE:
        case APPEND:
        case READ_IN_ORDER: // HANDLE THE READ IN ORDER MODE!!!!
        case READ_NET_STREAM: 
        default:
            return I106_WRONG_FILE_MODE;
            break;

        case READ:
            break;
    }

    // Check file mode
    switch (handles[handle].File_State){
        case I106_READ_NET_STREAM:
        case I106_CLOSED:
            return I106_NOT_OPEN;
            break;

        case I106_WRITE:
            return I106_WRONG_FILE_MODE;
            break;

        case I106_READ_HEADER:
        case I106_READ_DATA:
            // Backup to a point just before the most recently read header.
            // The amount to backup is the size of the previous header and the amount
            // of data already read.
            initial_backup = handles[handle].HeaderBufferLength + handles[handle].DataBufferPos;
            break;

        case I106_READ_UNSYNCED:
            initial_backup = 0;
            break;
    }

    // This puts us at the beginning of the most recently read header (or BOF)
    I106C10GetPos(handle, &pos);
    pos = pos - initial_backup;

    // If at the beginning of the file then done, return BOF
    if (pos <= 0){
        I106C10SetPos(handle, 0);
        return I106_BOF;
    }

    // Loop until previous packet found
    still_reading = 1;
    while (still_reading){

        // Figure out how much to backup
        if (pos >= BACKUP_SIZE)
            backup_amount = BACKUP_SIZE;
        else
            backup_amount = (int)pos;

        // Backup that amount
        next_read_pos = pos - backup_amount;
        I106C10SetPos(handle, next_read_pos);

        // Read a buffer of data to scan backwards through
        read_count = read(handles[handle].File, buffer, backup_amount + HEADER_SIZE);

        // Go to the end of the buffer and start scanning backwards
        for (buffer_pos = backup_amount - 1; buffer_pos >= 0; buffer_pos--){

            // Keep track of where we are in the file
            pos--;

            // Check for sync chars
            if ((buffer[buffer_pos] != 0x25) || (buffer[buffer_pos + 1] != 0xEB))
                continue;

            // Sync chars found so check header checksum
            checksum = HeaderChecksum((I106C10Header *)(&buffer[buffer_pos]));
            if (checksum != ((I106C10Header *)(&buffer[buffer_pos]))->Checksum)
                continue;

            // Header checksum found so let ReadNextHeader() have a crack
            status = I106C10SetPos(handle, pos);
            if (status != I106_OK)
                continue;
            status = I106C10ReadNextHeaderFile(handle, header);
            if (status != I106_OK)
                continue;

            // Header OK so break out
            still_reading = 0;
            break;

            // At the beginning of the buffer go back and read some more

        }

        // Check to see if we're at the BOF.  BTW, if we're at the beginning of
        // a file and a valid header wasn't found then it IS a seek error.
        if (pos == 0){
            still_reading = 0;
            status = I106_SEEK_ERROR;
        }
            
    }

    return status;
}


// Get the next header.  Depending on how the file was opened for reading,
// call the appropriate routine.
I106Status I106C10ReadData(int handle, unsigned long buffer_size, void *buffer){
    I106Status  status;

    switch (handles[handle].FileMode){
        case READ_NET_STREAM: 
        case READ: 
            status = I106C10ReadDataFile(handle, buffer_size, buffer);
            break;
        case READ_IN_ORDER : 
            status = I106C10ReadDataFile(handle, buffer_size, buffer);
            break;

        default :
            status = I106_WRONG_FILE_MODE;
            break;

    }
    
    return status;
}


I106Status I106C10ReadDataFile(int handle, unsigned long buffer_size, void *buffer){
    int             read_count;
    unsigned long   read_amount;

    // Check for a valid handle
    if ((handle <  0) || (handle >= MAX_HANDLES) || (handles[handle].InUse == 0))
        return I106_INVALID_HANDLE;

    // Check for invalid file modes
    switch (handles[handle].FileMode){
        case READ_IN_ORDER:
        case READ_NET_STREAM:
        case WRITE_NET_STREAM:
        case OVERWRITE:
        case APPEND:
            return I106_WRONG_FILE_MODE;
            break;
        case CLOSED:
            return I106_NOT_OPEN;
            break;
        case READ:
        default:
            break;
    }

    // Check file state
    switch (handles[handle].File_State){
        case I106_CLOSED:
            return I106_NOT_OPEN;
            break;

        case I106_WRITE:
            return I106_WRONG_FILE_MODE;
            break;

        case I106_READ_DATA:
            break;

        default :
            // MIGHT WANT TO SUPPORT THE "MORE DATA" METHOD INSTEAD
            handles[handle].File_State = I106_READ_UNSYNCED;
            return I106_READ_ERROR;
            break;
    }

    // Make sure there is enough room in the user buffer
    // MIGHT WANT TO SUPPORT THE "MORE DATA" METHOD INSTEAD
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
        else
            return I106_EOF;
    }

    // Keep track of our read position in the current data buffer
    handles[handle].DataBufferPos = read_amount;

    // MAY WANT TO DO CHECKSUM CHECKING SOMEDAY

    // Expect a header next read
    handles[handle].File_State = I106_READ_HEADER;

    return I106_OK;
}    


I106Status I106C10WriteMsg(int handle, I106C10Header *header, void *buffer){
    int  header_length;
    int  write_count;

    // Check for a valid handle
    if ((handle <  0) || (handle >= MAX_HANDLES) || (handles[handle].InUse == 0))
        return I106_INVALID_HANDLE;

    // Check for invalid file modes
    switch (handles[handle].FileMode){
        case OVERWRITE:
        case APPEND:
        case WRITE_NET_STREAM:
        case CLOSED:
            return I106_NOT_OPEN;
            break;

        case READ:
        case READ_IN_ORDER: 
        case READ_NET_STREAM:
            return I106_WRONG_FILE_MODE;
            break;
    }

    // Figure out header length
    header_length = GetHeaderLength(header);

    // Write the header
    write_count = write(handles[handle].File, header, header_length);

    // If there was an error reading, figure out why
    if (write_count != header_length)
        return I106_WRITE_ERROR;
    
    // Write the data
    write_count = write(handles[handle].File, buffer, header->PacketLength - header_length);

    // If there was an error reading, figure out why
    if ((unsigned long)write_count != (header->PacketLength - header_length))
        return I106_WRITE_ERROR;

    // Update the number of bytes written
    handles[handle].BytesWritten += header->PacketLength;

    return I106_OK;
}


// Move file pointer
I106Status I106C10FirstMsg(int handle){

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
            handles[handle].Index.ArrayPos = 0;
            I106C10SetPos(handle, 0L);
            break;

        case READ:
            I106C10SetPos(handle, 0L);
            break;
    }

    return I106_OK;
}


I106Status I106C10LastMsg(int handle){
    I106Status     return_status;
    I106Status     status;
    int64_t        pos;
    I106C10Header  header;
    int            read_count;  
#if !defined(_MSC_VER)
    struct stat    stat_buffer;
#endif

    // Check for a valid handle
    if ((handle <  0) || (handle >= MAX_HANDLES) || (handles[handle].InUse == 0))
        return I106_INVALID_HANDLE;

    // Check file modes
    struct stat file_stat;
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

        // If it's opened for reading in order then just set the index pointer
        // to the last index.
        case READ_IN_ORDER   :
            handles[handle].Index.ArrayPos = handles[handle].Index.ArrayUsed - 1;
            return_status = I106_OK;
            break;

        // If there is no index then do it the hard way
        case READ :

            // Figure out how big the file is and go to the end
#if defined(_MSC_VER)       
            pos = _filelengthi64(handles[handle].File) - HEADER_SIZE;
#else   
            fstat(handles[handle].File, &stat_buffer);
            pos = stat_buffer.st_size - HEADER_SIZE;
#endif      

            // Now loop forever looking for a valid packet or die trying
            while (1){
                // Not at the beginning so go back 1 byte and try again
                pos -= 1;

                // Go to the new position and look for a legal header
                status = I106C10SetPos(handle, pos);
                if (status != I106_OK)
                    return I106_SEEK_ERROR;

                // Read and check the header
                read_count = read(handles[handle].File, &header, HEADER_SIZE);

                if (read_count != HEADER_SIZE){
                    continue;
                }

                if (header.SyncPattern != IRIG106_SYNC)
                    continue;
            
                // Sync pattern matched so check the header checksum
                if (header.Checksum == HeaderChecksum(&header)){
                    return_status = I106_OK;
                    break;
                }

                // No match, check for begining of file
                // ONLY NEED TO GO BACK THE MAX PACKET SIZE
                if (pos <= 0){
                    return_status = I106_SEEK_ERROR;
                    break;
                }

            }

            // Go back to the good position
            status = I106C10SetPos(handle, pos);

            break;
    }

    return return_status;
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
