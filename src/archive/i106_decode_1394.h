/****************************************************************************

 i106_decode_1394.h

 ****************************************************************************/

#ifndef _I106_DECODE_1394_H
#define _I106_DECODE_1394_H

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


/* IEEE 1394 Format 0 */

// Channel specific header
typedef struct {
    uint32_t    uTransCnt    : 16;      // Transaction count
    uint32_t    Reserved     :  9;
    uint32_t    uSyncCode    :  4;      // Synchronization code
    uint32_t    uPacketType  :  3;      // Packet body type
} PACKED Su1394F0_ChanSpec;


/* IEEE 1394 Format 1 */

// Channel specific header
typedef struct {
    uint32_t    uPacketCnt   : 16;      // Number of messages
    uint32_t    Reserved     : 16;
} PACKED Su1394F1_ChanSpec;

// Intra-message header
typedef struct {
    uint8_t     aubyIntPktTime[8];      // Reference time
    uint32_t    uDataLength  : 16;      // 
    uint32_t    Reserved     :  1;      // 
    uint32_t    bLBO         :  1;      // Local buffer overflow
    uint32_t    uTrfOvf      :  2;      // Transfer overflow
    uint32_t    uSpeed       :  4;      // Transmission speed
    uint32_t    uStatus      :  8;      // Status byte
} PACKED Su1394F1_Header;

#if defined(_MSC_VER)
#pragma pack(pop)
#endif

#ifdef __cplusplus
}
}
#endif

#endif
