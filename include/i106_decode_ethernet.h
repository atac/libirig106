/****************************************************************************

 i106_decode_ethernet.h
 Created by Bob Baggerman

 ****************************************************************************/

#ifndef _I106_DECODE_ETHERNET_H
#define _I106_DECODE_ETHERNET_H

#include "libirig106.h"


/* Macros and definitions */

typedef enum{
    I106_ENET_FMT_PHYSICAL    =  0x00,
} I106EthernetFormat;

typedef enum {
    I106_ENET_CONTENT_FULLMAC =  0x00,
} I106EthernetContent;

typedef enum {
    I106_ENET_SPEED_AUTO      =  0x00,
    I106_ENET_SPEED_10MBPS    =  0x01,
    I106_ENET_SPEED_100MBPS   =  0x02,
    I106_ENET_SPEED_1GBPS     =  0x03,
    I106_ENET_SPEED_10GBPS    =  0x04,
} I106EthernetSpeed;


/* Data structures */

#pragma pack(push,1)

// Channel specific data word
typedef struct EthernetF0_CSDW EthernetF0_CSDW;
struct EthernetF0_CSDW {
    uint32_t    Frames     : 16;      // Number of frames
    uint32_t    Reserved1  : 12;
    uint32_t    Format     :  4;      // Format of frames
};

// Intra-message header
typedef struct {
    uint8_t     IPTS[8];            // Reference time
    uint32_t    Length       : 14;  // Data length
    uint32_t    LengthError  :  1;
    uint32_t    DataCRCError :  1;
    uint32_t    NetID        :  8;
    uint32_t    Speed        :  4;
    uint32_t    Content      :  2;
    uint32_t    FrameError   :  1;
    uint32_t    FrameCRCError:  1; 
} EthernetF0_IPH;

#pragma pack(pop)


// Ethernet physical frame
typedef struct {
    uint8_t   Destination[6];  // Destination address
    uint8_t   Source[6];       // Source address
    uint16_t  TypeLen;         // Ethernet type / 802.3 length, byte swapped!
    uint8_t   Data[1];         // Start of the data
} EthernetF0_Physical_FullMAC;


// Current Ethernet message
typedef struct {
    unsigned int       FrameNumber;
    uint32_t           Length;      // Overall data packet length
    EthernetF0_CSDW  * CSDW;
    EthernetF0_IPH   * IPH;
    uint8_t          * Data;
} EthernetF0_Message;


/* Function Declaration */
I106Status I106_Decode_FirstEthernetF0(I106C10Header *header, void *buffer, EthernetF0_Message *msg);
I106Status I106_Decode_NextEthernetF0(EthernetF0_Message *msg);

#endif
