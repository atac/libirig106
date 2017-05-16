/****************************************************************************

 i106_decode_index.h - Computer generated data format 3 recording index
 Created by Bob Baggerman

 ****************************************************************************/

#ifndef _I106_DECODE_INDEX_H
#define _I106_DECODE_INDEX_H

#ifdef __cplusplus
namespace Irig106 {
extern "C" {
#endif


// Macros and definitions

#if defined(_MSC_VER)
#pragma pack(push)
#pragma pack(1)
#endif

#if defined(__GNUC__)
#define PACKED __attribute__ ((packed))
#else
#define PACKED
#endif


// Index time
typedef union Index_Time_S {
    SuIntraPacketRtc        suRtcTime;  // RTC format time stamp
    SuI106Ch4_Binary_Time   suCh4Time;  // Ch 4 format time stamp
    SuIEEE1588_Time         su1588Time; // IEEE-1588 format time stamp
    uint64_t                llTime;     // Generic 8 byte time
} PACKED SuIndex_Time;


// Channel specific data word
typedef struct Index_ChanSpec_S{
    uint32_t    uIdxEntCount    : 16;   // Total number of indexes
    uint32_t    uReserved       : 13;
    uint32_t    bIntraPckHdr    :  1;   // Intra-packet header present
    uint32_t    bFileSize       :  1;   // File size present
    uint32_t    uIndexType      :  1;   // Index type
} PACKED SuIndex_ChanSpec;


// Node Index

// Node index data
typedef struct Index_NodeData_S{
    uint32_t    uChannelID      : 16;
    uint32_t    uDataType       :  8;
    uint32_t    uReserved       :  8;
} PACKED SuIndex_NodeData;

// Node index message without optional secondary data header
typedef struct Index_NodeMsg_S {
    SuIndex_Time                suTime;
    SuIndex_NodeData            suData;     // Node index data
    int64_t                     lOffset;
} PACKED SuIndex_NodeMsg;

// Node index message with optional secondary data header
typedef struct Index_NodeMsgOptTime_S {
    SuIndex_Time                suTime;
    SuIndex_Time                suSecondaryTime;
    SuIndex_NodeData            suData;     // Node index data
    int64_t                     lOffset;
} PACKED SuIndex_NodeMsgOptTime;


// Root Index

// Root index message without optional secondary data header
typedef struct Index_RootMsg_S {
    SuIndex_Time                suTime;
    int64_t                     lOffset;   // Offset to node packet
} PACKED SuIndex_RootMsg;


// Root index message with optional secondary data header
typedef struct Index_RootMsgOptTime_S{
    SuIndex_Time                suTime;
    SuIndex_Time                suSecondaryTime;
    int64_t                     lOffset;   // Offset to node packet
} PACKED SuIndex_RootMsgOptTime;


// Data structure to hold state for First / Next
typedef struct SuIndex_CurrMsg_S{
    unsigned int            uMsgNum;
    uint32_t                ulDataLen;
    SuIndex_ChanSpec      * psuChanSpec;
    int64_t               * piFileSize;
    void                  * pvIndexArray;       // Pointer to the beginning of the index array

    // The following pointer point to data within the packet buffer
    // Pointers are NULL for fields that don't exist within the
    // current index message.

    // Common index message fields
    SuIndex_Time          * psuTime;
    SuIndex_Time          * psuOptionalTime;    // Also known as the optional intra-packet data header
    int64_t               * plFileOffset;

    // Node index message specific fields
    SuIndex_NodeData      * psuNodeData;         // Node index data. Duh!

} SuIndex_CurrMsg;


#if defined(_MSC_VER)
#pragma pack(pop)
#endif


// Function Declaration
EnI106Status I106_CALL_DECL enI106_Decode_FirstIndex(SuI106Ch10Header * psuHeader, void * pvBuff, SuIndex_CurrMsg * psuMsg);
EnI106Status I106_CALL_DECL enI106_Decode_NextIndex(SuIndex_CurrMsg * psuMsg);


#ifdef __cplusplus
}
}
#endif

#endif
