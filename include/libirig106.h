
#ifndef _libirig106_h_
#define _libirig106_h_

#include <stdio.h>
#include <stdint.h>


/* Macros and definitions */

// TODO: check if this is still needed and/or could be moved to individual
// headers.
#if defined(__APPLE__)
#include <sys/uio.h>
#endif


// File open flags

// Microsoft
#if defined(_MSC_VER)
#define READ_FLAGS O_RDONLY | O_BINARY
#define OVERWRITE_FLAGS O_WRONLY | O_CREAT | _O_TRUNC | O_BINARY
#define OVERWRITE_MODE _S_IREAD | _S_IWRITE

// GCC
#elif defined(__GNUC__)
#define OVERWRITE_MODE S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#if !defined(__APPLE__)
#define READ_FLAGS O_RDONLY
#define OVERWRITE_FLAGS O_WRONLY | O_CREAT

// OSX
#else
#define READ_FLAGS O_RDONLY
#define OVERWRITE_FLAGS O_WRONLY | O_CREAT
#endif

// Everyone else
#else
#define OVERWRITE_FLAGS O_WRONLY | O_CREAT
#define OVERWRITE_MODE 0
#endif

#define MAX_HANDLES         100

#define IRIG106_SYNC        0xEB25
#define HEADER_SIZE         24
#define SEC_HEADER_SIZE     12

// Define the longest file path string size
#undef  MAX_PATH
#define MAX_PATH                       260

// Header packet flags
// TODO: reimplement with struct?
#define I106CH10_PFLAGS_CHKSUM_NONE       (uint8_t)0x00
#define I106CH10_PFLAGS_CHKSUM_8          (uint8_t)0x01
#define I106CH10_PFLAGS_CHKSUM_16         (uint8_t)0x02
#define I106CH10_PFLAGS_CHKSUM_32         (uint8_t)0x03
#define I106CH10_PFLAGS_CHKSUM_MASK       (uint8_t)0x03

#define I106CH10_PFLAGS_TIMEFMT_IRIG106   (uint8_t)0x00
#define I106CH10_PFLAGS_TIMEFMT_IEEE1588  (uint8_t)0x04
#define I106CH10_PFLAGS_TIMEFMT_Reserved1 (uint8_t)0x08
#define I106CH10_PFLAGS_TIMEFMT_Reserved2 (uint8_t)0x0C
#define I106CH10_PFLAGS_TIMEFMT_MASK      (uint8_t)0x0C

#define I106CH10_PFLAGS_OVERFLOW          (uint8_t)0x10
#define I106CH10_PFLAGS_TIMESYNCERR       (uint8_t)0x20
#define I106CH10_PFLAGS_IPTIMESRC         (uint8_t)0x40
#define I106CH10_PFLAGS_SEC_HEADER        (uint8_t)0x80

// Header data types
// TODO: review for completeness
#define I106CH10_DTYPE_COMPUTER_0      (uint8_t)0x00
#define I106CH10_DTYPE_USER_DEFINED    (uint8_t)0x00
#define I106CH10_DTYPE_COMPUTER_1      (uint8_t)0x01
#define I106CH10_DTYPE_TMATS           (uint8_t)0x01
#define I106CH10_DTYPE_COMPUTER_2      (uint8_t)0x02
#define I106CH10_DTYPE_RECORDING_EVENT (uint8_t)0x02
#define I106CH10_DTYPE_COMPUTER_3      (uint8_t)0x03
#define I106CH10_DTYPE_RECORDING_INDEX (uint8_t)0x03
#define I106CH10_DTYPE_COMPUTER_4      (uint8_t)0x04
#define I106CH10_DTYPE_COMPUTER_5      (uint8_t)0x05
#define I106CH10_DTYPE_COMPUTER_6      (uint8_t)0x06
#define I106CH10_DTYPE_COMPUTER_7      (uint8_t)0x07
#define I106CH10_DTYPE_PCM_FMT_0       (uint8_t)0x08
#define I106CH10_DTYPE_PCM_FMT_1       (uint8_t)0x09
#define I106CH10_DTYPE_PCM             (uint8_t)0x09    // Deprecated
#define I106CH10_DTYPE_IRIG_TIME       (uint8_t)0x11
#define I106CH10_DTYPE_1553_FMT_1      (uint8_t)0x19
#define I106CH10_DTYPE_1553_FMT_2      (uint8_t)0x1A    // 16PP194 Bus
#define I106CH10_DTYPE_ANALOG          (uint8_t)0x21
#define I106CH10_DTYPE_DISCRETE        (uint8_t)0x29
#define I106CH10_DTYPE_MESSAGE         (uint8_t)0x30
#define I106CH10_DTYPE_ARINC_429_FMT_0 (uint8_t)0x38
#define I106CH10_DTYPE_VIDEO_FMT_0     (uint8_t)0x40
#define I106CH10_DTYPE_VIDEO_FMT_1     (uint8_t)0x41
#define I106CH10_DTYPE_VIDEO_FMT_2     (uint8_t)0x42
#define I106CH10_DTYPE_IMAGE_FMT_0     (uint8_t)0x48
#define I106CH10_DTYPE_IMAGE_FMT_1     (uint8_t)0x49
#define I106CH10_DTYPE_UART_FMT_0      (uint8_t)0x50
#define I106CH10_DTYPE_1394_FMT_0      (uint8_t)0x58
#define I106CH10_DTYPE_1394_FMT_1      (uint8_t)0x59
#define I106CH10_DTYPE_PARALLEL_FMT_0  (uint8_t)0x60
#define I106CH10_DTYPE_ETHERNET_FMT_0  (uint8_t)0x68
#define I106CH10_DTYPE_CAN             (uint8_t)0X78

// Error return codes
typedef enum {
    I106_OK,                 // Everything okey dokey
    I106_OPEN_ERROR,         // Fatal problem opening for read or write
    I106_OPEN_WARNING,       // Non-fatal problem opening for read or write
    I106_EOF,                // End of file encountered
    I106_BOF,
    I106_READ_ERROR,         // Error reading data from file
    I106_WRITE_ERROR,        // Error writing data to file
    I106_MORE_DATA,          // More read data available
    I106_SEEK_ERROR,         // Unable to seek to positino
    I106_WRONG_FILE_MODE,    // Operation compatible with file open mode
    I106_NOT_OPEN,           // File not open for reading or writing
    I106_ALREADY_OPEN,       // File already open
    I106_BUFFER_TOO_SMALL,   // User buffer too small to hold data
    I106_NO_MORE_DATA,       // No more data to read
    I106_NO_FREE_HANDLES,    // Too many files open
    I106_INVALID_HANDLE,     // Passed file handle doesn't point to an open file
    I106_TIME_NOT_FOUND,     // No valid time packet found
    I106_HEADER_CHKSUM_BAD,  // Invalid header checksum
    I106_NO_INDEX,           // No index found
    I106_UNSUPPORTED,        // Unsupported operation
    I106_BUFFER_OVERRUN,     // Data exceeds buffer size
    I106_INDEX_NODE,         // Returned decoded node message
    I106_INDEX_ROOT,         // Returned decoded root message
    I106_INDEX_ROOT_LINK,    // Returned decoded link to next root (i.e. last root)
    I106_INVALID_DATA,       // Packet data is invalid for some reason
    I106_INVALID_PARAMETER   // Passed parameter is invalid
} I106Status;

// Data file open mode
typedef enum {
    CLOSED,
    READ,              // Open an existing file for reading
    OVERWRITE,         // Create a new file or overwrite an exising file
    APPEND,            // Append data to the end of an existing file
    READ_IN_ORDER,     // Open an existing file for reading in time order
    READ_NET_STREAM,   // Open network data stream for reading
    WRITE_NET_STREAM,  // Open network data stream for writing
} I106C10Mode;

// Used to keep track of the next expected data file structure
typedef enum {
	I106_CLOSED,
	I106_WRITE,
	I106_READ_UNSYNCED,
    I106_READ_HEADER,
	I106_READ_DATA,
	I106_READ_NET_STREAM,
} I106FileState;

// Index sort state
typedef enum {
    UNSORTED,
    SORTED,
    SORT_ERROR,
} I106SortStatus;


/* Data structures */

#pragma pack(push, 1)

// IRIG 106 header and optional secondary header data structure
typedef struct {
    uint16_t  SyncPattern;
    uint16_t  ChannelID;
    uint32_t  PacketLength;
    uint32_t  DataLength;
    uint8_t   HeaderVersion;
    uint8_t   SequenceNumber;
    uint8_t   PacketFlags;
    uint8_t   DataType;
    uint8_t   RTC[6];
    uint16_t  Checksum;

    // Secondary header (optional)
    uint32_t  Time[2];
    uint16_t  Reserved;
    uint16_t  SecondaryChecksum;
} I106C10Header;

// Structure for holding file index
// TODO: Move to i106_index
typedef struct {
    int64_t  Offset;  // File position byte offset
    int64_t  Time;    // Packet RTC at this offset
} InOrderPacketInfo;

// Various file index array indexes
typedef struct InOrderIndex InOrderIndex;
struct InOrderIndex {
    I106SortStatus           SortStatus;
    InOrderPacketInfo  * Index;
    int                  ArraySize;
    int                  ArrayUsed;
    int                  ArrayPos;
    int64_t              NextReadOffset;
    int                  NumSearchSteps;
};

// Data structure for IRIG 106 read/write handle
typedef struct I106C10Handle I106C10Handle;
struct I106C10Handle {
    int            InUse;
    int            File;
    FILE          *fp;
    char           FileName[MAX_PATH];
    I106C10Mode    FileMode;
    I106FileState  File_State;
    InOrderIndex   Index;
    unsigned long  PacketLength;
    unsigned long  HeaderBufferLength;
    unsigned long  DataBufferLength;
    unsigned long  DataBufferPos;
    unsigned long  BytesWritten;
    char           Reserved[128];
};

#pragma pack(pop)


/* Global data */

extern I106C10Handle  handles[MAX_HANDLES];


/* Function Declaration */

// Open / Close
I106Status I106C10Open(int *handle, const char filename[], I106C10Mode mode);
I106Status I106C10OpenBuffer(int *handle, void *buffer, int size, I106C10Mode mode);
I106Status I106C10Close(int handle);

// Read / Write
I106Status I106C10ReadNextHeader(int handle, I106C10Header *header);
I106Status I106C10ReadNextHeaderFile(int handle, I106C10Header *header);
I106Status I106C10ReadNextHeaderInOrder(int handle, I106C10Header *header);
I106Status I106C10ReadPrevHeader(int handle, I106C10Header *header);
I106Status I106C10ReadData(int handle, unsigned long buffer_size, void * buffer);
I106Status I106C10ReadDataFile(int handle, unsigned long buffer_size, void * buffer);
I106Status I106C10WriteMsg(int handle, I106C10Header *header, void *buffer);

// Move file pointer
I106Status I106C10FirstMsg(int handle);
I106Status I106C10LastMsg(int handle);
I106Status I106C10SetPos(int handle, int64_t offset);
I106Status I106C10GetPos(int handle, int64_t * offset);

// Utilities
int HeaderInit(I106C10Header *header, unsigned int channel_id,
    unsigned int data_type, unsigned int flags, unsigned int sequence_number);
int GetHeaderLength(I106C10Header *header);
uint32_t GetDataLength(I106C10Header *header);
uint16_t HeaderChecksum(I106C10Header *header);
uint16_t SecondaryHeaderChecksum(I106C10Header *header);
char * I106ErrorString(I106Status status);
/* int DataChecksum(void *buffer); */
uint32_t BufferSize(uint32_t data_length, int checksum_type);
I106Status AddFillerAndChecksum(I106C10Header *header, unsigned char data[]);

// In-order indexing
void MakeInOrderIndex(int handle);
int ReadInOrderIndex(int handle, char *filename);
int WriteInOrderIndex(int handle, char *filename);

#endif
