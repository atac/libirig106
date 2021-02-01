/****************************************************************************

 i106_decode_index.h - Computer generated data format 3 recording index
 Created by Bob Baggerman

 ****************************************************************************/

#ifndef _I106_DECODE_INDEX_H
#define _I106_DECODE_INDEX_H

#include "libirig106.h"
#include "i106_util.h"

// Macros and definitions

#pragma pack(push, 1)

// Index time
typedef union {
    IntraPacketRTC       rtc_time;
    I106Ch4_Binary_Time  ch4_time;
    IEEE1588_Time        i1588_time;
    uint64_t             time;
} IndexTime;


// Channel specific data word
typedef struct {
    uint32_t  Count      : 16;   // Total number of indexes
    uint32_t  Reserved   : 13;
    uint32_t  IPH        :  1;   // Intra-packet header present
    uint32_t  FileSize   :  1;   // File size present
    uint32_t  IndexType  :  1;   // Index type
} IndexCSDW;


/* Node Index */

// Node index data
typedef struct {
    uint32_t    ChannelID      : 16;
    uint32_t    DataType       :  8;
    uint32_t    Reserved       :  8;
} IndexNodeData;

// Node index message without optional secondary data header
typedef struct {
    IndexTime      Time;
    IndexNodeData  Data;
    int64_t        Offset;
} IndexNodeMsg;

// Node index message with optional secondary data header
typedef struct {
    IndexTime                Time;
    IndexTime                SecondaryTime;
    IndexNodeData            Data;
    int64_t                  Offset;
} IndexNodeMsgTime;


/* Root Index */

// Root index message without optional secondary data header
typedef struct {
    IndexTime                Time;
    int64_t                  Offset;
} IndexRootMsg;

// Root index message with optional secondary data header
typedef struct {
    IndexTime                Time;
    IndexTime                SecondaryTime;
    int64_t                  Offset;
} IndexRootMsgTime;


// Data structure to hold state for First / Next
typedef struct {
    unsigned int    MessageNumber;
    uint32_t        DataLength;
    IndexCSDW     * CSDW;
    int64_t       * FileSize;
    void          * IndexArray;  // Pointer to the beginning of the index array

    // The following pointers point to data within the packet buffer
    // Pointers are NULL for fields that don't exist within the
    // current index message.

    // Common index message fields
    IndexTime      * Time;
    IndexTime      * IPTS;
    int64_t        * FileOffset;

    // Node index message specific fields
    IndexNodeData  * NodeData;

} IndexMsg;


// Function Declaration
I106Status I106_Decode_FirstIndex(I106C10Header *header, void * buffer, IndexMsg *msg);
I106Status I106_Decode_NextIndex(IndexMsg *msg);

#pragma pack(pop)

#endif
