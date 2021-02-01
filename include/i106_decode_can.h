/****************************************************************************

 i106_decode_can.h - CAN bus decoding, as per IRIG106 ch10, v. 2013
 Created by: Tommaso Falchi Delitala <volalto86@gmail.com>

 ****************************************************************************/

#ifndef _I106_DECODE_CAN_H
#define _I106_DECODE_CAN_H

#include "libirig106.h"
#include "i106_util.h"


/* Data structures */

#pragma pack(push,1)


// Channel specific data word
typedef struct {
    uint32_t    Count          : 16;
    uint32_t    Reserved       : 16;
} CAN_CSDW;


// Intra-packed message header
typedef struct {
    uint64_t    IPTS;                   // Reference time
    uint32_t    Length       :  4;      // Message length
    uint32_t    Reserved     : 12;
    uint32_t    SubChannel   : 14;      // Subchannel number
    uint32_t    FormatError  :  1;      // Format error flag
    uint32_t    DataError    :  1;      // Data error flag
} CAN_IPH;


// CAN ID Word
typedef struct {
    uint32_t    CanID          : 29;
    uint32_t    Reserved       :  1;    // 0 = 11-bit CAN id; 1 = 29-bit CAN Id
    uint32_t    RTR            :  1;    // Remote transfer request bit
    uint32_t    IDE            :  1;
} CAN_ID;

#pragma pack(pop)

typedef struct {
    unsigned int     MessageNumber;
    uint32_t         BytesRead;      // Offset into data buffer
    I106C10Header  * Header;
    CAN_CSDW       * CSDW;
    CAN_IPH        * IPH;
    CAN_ID         * ID;
    IntraPacketTS  * IPTS;
    uint8_t        * Data;
    TimeRef          Time;
} CAN_Message;


/* Function Declaration */
I106Status I106_Decode_FirstCAN(I106C10Header *header, void *buffer, CAN_Message *msg);
I106Status I106_Decode_NextCAN(CAN_Message *msg);

#endif
