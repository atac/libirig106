/****************************************************************************

 i106_decode_pcmf1.h
 
 Created by Bob Baggerman
 Expanded by Hans-Gerhard Flohr, Hasotec GmbH, www.hasotec.de
 2014/04/07 Initial Version 1.0 
 2014/04/23 Version 1.1 
 Changes: Meaning of swap data words

 ****************************************************************************/

#ifndef _I106_DECODE_PCMF1_H
#define _I106_DECODE_PCMF1_H

#include "irig106ch10.h"
#include "i106_decode_tmats.h"
#include "i106_time.h"


/* Macros and definitions */

typedef enum {
    PCM_PARITY_NONE     = 0,
    PCM_PARITY_ODD      = 1,
    PCM_PARITY_EVEN     = 2,
} PCM_PARITY;

typedef enum {
    PCM_MSB_FIRST       = 0,
    PCM_LSB_FIRST       = 1,
} PCM_BIT_TRANSFER_ORDER;

#ifndef d100NANOSECONDS
    #define d100NANOSECONDS     10000000.
#endif

// Bit 0: Starts at the most left position of the (byte) array (2exp7)
// Caution: If you use the macro, don't use operators like '<<=', '++' etc for the BitPosition
#ifndef IsBitSetL2R
    #define IsBitSetL2R(Array, BitPosition)   ((Array)[ ((BitPosition) >> 3) ] & 0x80 >> ((BitPosition) & 7) )
#endif

#ifndef IsBitSetR2L
    // Bit 0: Starts at the most right position of the (byte) array (2exp0)
    // Caution: Don't use operators like '<<=', '++' etc for the BitPosition
    #define IsBitSetR2L(Array, BitPosition)   ((Array)[ ((BitPosition) >> 3) ] & 1 << ((BitPosition) & 7) )
#endif


/* Data structures */

#if defined(_MSC_VER)
#pragma pack(push,1)
#endif

// Channel specific data word
typedef struct PCMF1_CSDW PCMF1_CSDW;
struct PCMF1_CSDW {
    uint32_t    SyncOffset           : 18;
    uint32_t    UnpackedMode         :  1;      // Unpacked mode flag
    uint32_t    PackedMode           :  1;      // Packed mode flag
    uint32_t    Throughput           :  1;      // Throughput mode flag
    uint32_t    Alignment            :  1;      // 16/32 bit alignment flag
    uint32_t    Reserved1            :  2;
    uint32_t    MajorFrameStatus     :  2;      // Major frame lock status
    uint32_t    MinorFrameStatus     :  2;      // Minor frame lock status
    uint32_t    MinorFrameIndicator  :  1;
    uint32_t    MajorFrameIndicator  :  1;
    uint32_t    IPH                  :  1;
    uint32_t    Reserved2            :  1;
} PACKED;

// Intra-message header
typedef struct PCMF1_IPH PCMF1_IPH;
struct PCMF1_IPH {
    uint64_t    IPTS;
    uint32_t    Reserved1         : 12;
    uint32_t    MajorFrameStatus  :  2;      // Major frame lock status
    uint32_t    MinorFrameStatus  :  2;      // Minor frame lock status
    uint32_t    Reserved2         : 16;
} PACKED;


#if defined(_MSC_VER)
#pragma pack(pop)
#endif


// Channel attributes
// Note:
// The PCMF1_Attributes structure covers most of the information needed to decode raw PCM data. 
// Only a part of the relevant data is supplied in the message PCMF1_CSDW.
// Most of the attributes must be imported from TMATS or supplied by another source.
typedef struct PCMF1_Attributes PCMF1_Attributes;
struct PCMF1_Attributes {
    R_DataSource  * R_DataSource;                 // Pointer to the corresponding RDataSource
    int             RecordNumber;                 // P-x
    uint32_t        BitsPerSecond;                // P-x\D2 number of bits per seconds

    // Fx
    uint32_t        CommonWordLength;             // in bits P-x\F1
    uint32_t        WordTransferOrder;            // Msb (0)/ LSB (1, unsupported) P-x\F2
    uint32_t        ParityType;                   // Parity (0=none, 1= odd, 2= even) P-x\F3
    uint32_t        ParityTransferOrder;          // Trailing (0) Leading (1) P-x\F4

    // MFx
    uint32_t        MinorFrames;                  // P-x\MF\N Number of MinorFrames
    uint32_t        WordsInMinorFrame;            // P-x\MF1 Words in Minor Frame
    uint32_t        BitsInMinorFrame;             // P-x\MF2 Bits in Minor Frame including syncword
    uint32_t        MinorFrameSyncType;           // P-x\MF3
    uint32_t        MinorFrameSyncPatternLength;  // P-x\MF4
    uint64_t        MinorFrameSyncPattern;        // P-x\MF5 Wordlen can be up to 64 bits, so let the sync word also 64 bits long
    // SYNCx
    uint32_t        MinSyncs;                     // P-x\SYNC1 Minimal number of syncs 0: first sync, 1: second sync etc.;
    // SYNC2 - SYNC4 not implemented

    // ISFx, IDCx, SFx not implemented 

    // Needed for some strange PCM sources
    uint32_t        DontSwapRawData;              // Inhibit byte or word swap on the raw input data

    // Computed values 
    uint64_t        MinorFrameSyncMask;           // Computed from P-x\MF4 (MinorFrameSyncPatternLength)
    uint64_t        CommonWordMask;               // Computed from P-x\F1
                                                
    double          Delta100NanoSeconds;          // Computed from P-x\D2, the bits per sec
    int32_t         PrepareNextDecodingRun;       // First bit flag for a complete decoding run: preload a minor frame sync word to the test word

    // The output buffer must be allocated if PrepareNextDecodingRun is notzero
    // The buffer consists of two parts: A data buffer and an error buffer
    int32_t         BufferSize;                   // Size of the output buffer in (64-bit) words
    uint64_t      * Buffer;                       // Contains the decoded data of a minor frame
    uint8_t       * BufferError;                  // Contains the error flags (parity error) for each data word in a minor frame

    // Variables for bit decoding
    // Must be kept for the whole decoding run because the data 
    // may overlap the CH10 packets (at least in troughput mode)

    uint64_t        SyncCount;                    // -1: Nothing found, 0: 1 sync found etc. analog to Min Syncs
    uint64_t        SyncErrors;                   // Counter for statistics 
    uint64_t        TestWord;                     // Currently collected word resp. syncword
    uint64_t        BitsLoaded;                   // Bits already loaded (and shifted through) the TestWord. 
    // The amount must be at least the sync word len to check for a sync word
    uint32_t        BitPosition;                  // Bit position in the current buffer
    uint32_t        MinorFrameBitCount;           // Counter for the number of bits in a minor frame (inclusive syncword)
    uint32_t        MinorFrameWordCount;          // Counter for the Minor frame words (inclusive syncword)
    uint32_t        DataWordBitCount;             // Counter for the bits of a data word
    int32_t         SaveData;                     // Save the data (0: do nothing, 1 save, 2: save terminated)
} PACKED;

// Current PCM message
typedef struct PCMF1_Message PCMF1_Message;
struct PCMF1_Message {
    I106C10Header     * Header;           // The overall packet header
    PCMF1_CSDW        * CSDW;             // Header in the data stream
    PCMF1_Attributes  * Attributes;       // Pointer to the Pcm Format structure, values must be imported from TMATS 
    // or another source
    PCMF1_IPH         * IPH;              // Optional intra packet header, consists of the time 
    // suIntraPckTime (like SuIntraPacketTS) and the header itself
    unsigned int        BytesRead;        // Number of bytes read in this message
    uint32_t            Length;           // Overall data packet length
    int64_t             IPTS;             // Intrapacket or header time ! Relative Time !
    int64_t             IPTS_Base;        // Intrapacket or header time ! Relative Time !
    uint32_t            SubPacketLength;  // MinorFrameLen in Bytes padded, see bAlignment. 
    // In throughput mode it's the length of the whole packet
    uint32_t            SubPacketBits;    // MinorFrameLen in Bits
    uint8_t           * Data;             // Pointer to the start of the data
    TimeRef             Time;
} PACKED;


/* Function Declaration */
I106Status I106_Decode_FirstPCMF1(I106C10Header *header, void *buffer, PCMF1_Message *msg);
I106Status I106_Decode_NextPCMF1(PCMF1_Message *msg);
I106Status DecodeMinorFrame_PCMF1(PCMF1_Message *msg);
I106Status Set_Attributes_PCMF1(R_DataSource *r_datasource, PCMF1_Attributes *attributes);
I106Status Set_Attributes_Ext_PCMF1(
        R_DataSource *r_datasource,
        PCMF1_Attributes *attributes,
        int32_t record_number,
        int32_t bits_per_second,
        int32_t common_word_length,
        int32_t word_transfer_order,
        int32_t parity_type,
        int32_t parity_transfer_order,
        int32_t minor_frames,
        int32_t words_in_minor_frame,
        int32_t bits_in_minor_frame,
        int32_t minor_frame_sync_type,
        int32_t minor_frame_sync_length,
        int64_t minor_frame_sync_pattern,
        int32_t min_syncs,
        int64_t minor_frame_sync_mask,
        int32_t no_byte_swap);
I106Status CreateOutputBuffers_PCMF1(PCMF1_Attributes *attributes);
I106Status FreeOutputBuffers_PCMF1(PCMF1_Attributes *attributes);

// Help functions
I106Status CheckParity_PCMF1(uint64_t test_word, int word_length, int parity_type, int parity_transfer_order);
I106Status SwapBytes_PCMF1(uint8_t *buffer, long bytes);
I106Status SwapShortWords_PCMF1(uint16_t *buffer, long bytes);

#endif
