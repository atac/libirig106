/****************************************************************************

 i106_decode_ethernet.h - 
 Created by Bob Baggerman

 ****************************************************************************/

#ifndef _I106_DECODE_ETHERNET_H
#define _I106_DECODE_ETHERNET_H

#ifdef __cplusplus
namespace Irig106 {
extern "C" {
#endif


/*
 * Macros and definitions
 * ----------------------
 */

typedef enum
    {
    I106_ENET_FMT_PHYSICAL    =  0x00,
    } EnI106EthernetFmt;

typedef enum
    {
    I106_ENET_CONTENT_FULLMAC =  0x00,
    } EnI106EthernetContent;

typedef enum
    {
    I106_ENET_SPEED_AUTO      =  0x00,
    I106_ENET_SPEED_10MBPS    =  0x01,
    I106_ENET_SPEED_100MBPS   =  0x02,
    I106_ENET_SPEED_1GBPS     =  0x03,
    I106_ENET_SPEED_10GBPS    =  0x04,
    } EnI106EthernetSpeed;


/*
 * Data structures
 * ---------------
 */

#if defined(_MSC_VER)
#pragma pack(push,1)
#endif

// Channel specific data word
// --------------------------

typedef struct EthernetF0_ChanSpec_S
    {
    uint32_t    uNumFrames      : 16;      // Number of frames
    uint32_t    Reserved1       : 12;
    uint32_t    uFormat         :  4;      // Format of frames
#if !defined(__GNUC__)
    } SuEthernetF0_ChanSpec;
#else
    } __attribute__ ((packed)) SuEthernetF0_ChanSpec;
#endif

// Intra-message header
typedef struct EthernetF0_Header_S
    {
    uint8_t     aubyIntPktTime[8];         // Reference time
    uint32_t    uDataLen        : 14;      // Data length
    uint32_t    Reserved1       :  2;      // 
    uint32_t    uNetID          :  8;      // Network identifier
    uint32_t    uSpeed          :  4;      // Ethernet speed
    uint32_t    uContent        :  2;      // Captured data content
    uint32_t    bFrameError     :  1;      // Frame error
    uint32_t    Reserved2       :  1;      // 
#if !defined(__GNUC__)
    } SuEthernetF0_Header;
#else
    } __attribute__ ((packed)) SuEthernetF0_Header;
#endif

#if defined(_MSC_VER)
#pragma pack(pop)
#endif


// Ethernet physical frame
typedef struct
    {
    uint8_t                 abyDestAddr[6]; // Destination address
    uint8_t                 abySrcAddr[6];  // Source address
    uint16_t                uTypeLen;       // Ethernet type / 802.3 length, byte swapped!
    uint8_t                 abyData[1];     // Start of the data
#if !defined(__GNUC__)
    } SuEthernetF0_Physical_FullMAC;
#else
    } __attribute__ ((packed)) SuEthernetF0_Physical_FullMAC;
#endif



// Current Ethernet message
typedef struct
    {
    unsigned int            uFrameNum;
    uint32_t                ulDataLen;      // Overall data packet length
    SuEthernetF0_ChanSpec * psuChanSpec;
    SuEthernetF0_Header   * psuEthernetF0Hdr;
    uint8_t               * pauData;
#if !defined(__GNUC__)
    } SuEthernetF0_CurrMsg;
#else
    } __attribute__ ((packed)) SuEthernetF0_CurrMsg;
#endif





/*
 * Function Declaration
 * --------------------
 */

EnI106Status I106_CALL_DECL 
    enI106_Decode_FirstEthernetF0(SuI106Ch10Header     * psuHeader,
                                  void                 * pvBuff,
                                  SuEthernetF0_CurrMsg * psuMsg);

EnI106Status I106_CALL_DECL 
    enI106_Decode_NextEthernetF0(SuEthernetF0_CurrMsg * psuMsg);

#ifdef __cplusplus
}
}
#endif

#endif
