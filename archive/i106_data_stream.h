/****************************************************************************

 i106_data_stream.h - A module to decode Chapter 10 UDP data streaming

 ****************************************************************************/

#ifndef _I106_DATA_STREAMING_H
#define _I106_DATA_STREAMING_H

#include "config.h"

#ifdef __cplusplus
namespace Irig106 {
extern "C" {
#endif

// Data structures

#if defined(_MSC_VER)
#pragma pack(push)
#pragma pack(1)
#endif

// UDP Transfer Header - Non-segmented
typedef struct {
    uint32_t    uVersion        : 4;
    uint32_t    uMsgType        : 4;
    uint32_t    uSeqNum         : 24;
    uint8_t     achData[1];             // Start of Ch 10 data packet
} PACKED SuUDP_Transfer_Header_NonSeg;

enum {UDP_Transfer_Header_NonSeg_Len = sizeof(SuUDP_Transfer_Header_NonSeg) - 1};

// UDP Transfer Header - Segmented
typedef struct {
    uint32_t    uVersion        : 4;
    uint32_t    uMsgType        : 4;
    uint32_t    uSeqNum         :24;
    uint32_t    uChID           :16;
    uint32_t    uChanSeqNum     : 8;
    uint32_t    uReserved       : 8;
    uint32_t    uSegmentOffset;
    uint8_t     achData[1];             // Start of Ch 10 data packet
} PACKED SuUDP_Transfer_Header_Seg;

enum {UDP_Transfer_Header_Seg_Len = sizeof(SuUDP_Transfer_Header_Seg) - 1};

#if defined(_MSC_VER)
#pragma pack(pop)
#endif


// Function Declaration

// Open / Close
EnI106Status I106_CALL_DECL enI106_OpenNetStreamRead(int iHandle, uint16_t uPort);
EnI106Status I106_CALL_DECL enI106_OpenNetStreamWrite(int iHandle, uint32_t uIpAddress, uint16_t uUdpPort);
EnI106Status I106_CALL_DECL enI106_CloseNetStream(int iHandle);

// Read
int I106_CALL_DECL enI106_ReadNetStream(int iHandle, void * pvBuffer, uint32_t uBuffSize);

// Manipulate receive buffer
EnI106Status I106_CALL_DECL enI106_DumpNetStream(int iHandle);
EnI106Status I106_CALL_DECL enI106_MoveReadPointer(int iHandle, long iRelOffset);

// Write
EnI106Status I106_CALL_DECL enI106_WriteNetStream(int iHandle, void * pvBuffer, uint32_t uBuffSize);
EnI106Status I106_CALL_DECL enI106_WriteNetNonSegmented(int iHandle, void * pvBuffer, uint32_t uBuffSize);
EnI106Status I106_CALL_DECL enI106_WriteNetSegmented(int iHandle, void * pvBuffer, uint32_t uBuffSize);

#ifdef __cplusplus
}
}
#endif

#endif
