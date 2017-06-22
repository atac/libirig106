/****************************************************************************

 i106_decode_video.h - Video message decoding routines

 ****************************************************************************/

#ifndef _I106_DECODE_VIDEO_H
#define _I106_DECODE_VIDEO_H


/* Data structures */

#if defined(_MSC_VER)
#pragma pack(push,1)
#endif


/* Video Format 0 */

// Video Format 0 channel specific header
typedef struct {
    uint32_t    Reserved    : 24;
    uint32_t    Type        :  4;  // Payload type
    uint32_t    KLV         :  1;  // KLV present
    uint32_t    SRS         :  1;  // SCR/RTC Sync
    uint32_t    IPH         :  1;  // Intra-Packet Header
    uint32_t    ET          :  1;  // Embedded Time
} PACKED VideoF0_CSDW;


// Video Format 0 intra-packet header
typedef struct {
    uint8_t  IPTS[8];  // Reference time
} PACKED VideoF0_IPH;


// Current video format 0 data message
typedef struct {
    VideoF0_CSDW  * CSDW;  // Pointer to channel specific header
    VideoF0_IPH   * IPH;   // Pointer to intra-packet header
    uint8_t       * Data;  // Pointer to transport stream data
} PACKED VideoF0_Message;


/* Video Format 1 */

// Video Format 1 channel specific header
typedef struct {
    uint32_t    PacketCount  : 12;  // Number of packets
    uint32_t    Type         :  1;  // TS/PS type
    uint32_t    Mode         :  1;  // Const/Var mode
    uint32_t    ET           :  1;  // Embedded Time
    uint32_t    EPL          :  4;  // Encoding Profile and Level
    uint32_t    IPH          :  1;  // Intra-Packet Header
    uint32_t    SRS          :  1;  // SCR/RTC Sync
    uint32_t    KLV          :  1;  // KLV present
    uint32_t    Reserved     : 10;
} PACKED VideoF1_CSDW;


// Video Format 2 channel specific header
typedef struct {
    uint32_t    PacketCount  : 12;  // Number of packets
    uint32_t    Type         :  1;  // TS/PS type
    uint32_t    Mode         :  1;  // Const/Var mode
    uint32_t    ET           :  1;  // Embedded Time
    uint32_t    EP           :  4;  // Encoding Profile
    uint32_t    IPH          :  1;  // Intra-Packet Header
    uint32_t    SRS          :  1;  // SCR/RTC Sync
    uint32_t    KLV          :  1;  // KLV present
    uint32_t    EL           :  4;  // Encoding Level
    uint32_t    AET          :  1;  // Audio Encoding Type
    uint32_t    Reserved     :  5;
} PACKED VideoF2_CSDW;


#if defined(_MSC_VER)
#pragma pack(pop)
#endif


/* Function Declaration */
I106Status I106_Decode_FirstVideoF0(I106C10Header *header, void * buffer, VideoF0_Message * msg);
I106Status I106_Decode_NextVideoF0(I106C10Header *header, VideoF0_Message *msg);

#endif
