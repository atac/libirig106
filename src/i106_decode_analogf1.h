/****************************************************************************

 i106_decode_analogf1.h
 Created by Bob Baggerman
 Brought to life by Spencer Hatch in Andøya, Norge, NOV 2014

 ****************************************************************************/

/*
TODO:
  
*For each data packet, write relevant samples to subchan buff
----measure location using SubChBytesRead, i.e., you are currently at
    (uint8_t *)pauSubData + SubChBytesRead

*At end of data packet, write "SubChBytesRead" to SubChOutFile, flush SubChBuffer,
set BytesRead to zero, continue along
 */


#ifndef _I106_DECODE_ANALOGF1_H
#define _I106_DECODE_ANALOGF1_H

#include "irig106ch10.h"
#include "i106_decode_tmats.h"
#include "i106_time.h"


/* Macros and definitions */

#define ANALOG_MAX_SUBCHANS 256

typedef enum ANALOG_MODE {
    ANALOG_PACKED,
    ANALOG_UNPACKED_LSB_PADDED,
    ANALOG_RESERVED,
    ANALOG_UNPACKED_MSB_PADDED,
} ANALOG_MODE;

typedef enum {
    ANALOG_MSB_FIRST,
    ANALOG_LSB_FIRST,
} ANALOG_BIT_TRANSFER_ORDER;

// R-x\AF-n-m
typedef enum{
    ANALOG_FMT_ONES,
    ANALOG_FMT_TWOS,
    ANALOG_FMT_SIGNMAG_0,
    ANALOG_FMT_SIGNMAG_1,
    ANALOG_FMT_OFFSET_BIN,
    ANALOG_FMT_UNSIGNED_BIN,
    ANALOG_FMT_SINGLE_FLOAT,
} ANALOG_FORMAT;   


/* Data structures */

#if defined(_MSC_VER)
#pragma pack(push,1)
#endif


// Channel specific data word
typedef struct {
    uint32_t    Mode           :  2;
    uint32_t    Length         :  6;      // Bits in A/D value
    uint32_t    Subchannel     :  8;      // Subchannel number
    uint32_t    Subchannels    :  8;      // Total number of subchannels
    uint32_t    Factor         :  4;      // Sample rate exponent
    uint32_t    Same           :  1;      // One/multiple Channel Specific
    uint32_t    Reserved       :  3;
} PACKED AnalogF1_CSDW;


// Subchannel information structure
typedef struct AnalogF1_Subchannel AnalogF1_Subchannel;
struct AnalogF1_Subchannel {
    uint32_t         ChannelID;            // Overall channel ID
    AnalogF1_CSDW  * CSDW;                 // CSDW corresponding to subchan
    unsigned int     BytesRead;            // Number of bytes read for subchan
    uint8_t        * Data;                 // Pointer to the start of the data
    char             OutputFilename[256];  // Subchan output filename
    FILE           * OutputFile;           // Subchan output file handle
} PACKED;


// Channel attributes
typedef struct AnalogF1_Attributes AnalogF1_Attributes;
struct AnalogF1_Attributes {
    R_DataSource        * R_Datasource;              // Pointer to the corresponding RDataSource
    int                   DataSourceNumber;          // R-x

    char                * DataSourceID;

    int                   TrackNumber;               // Only valid if szTrackNumber != NULL
    int                   PhysicalChannelNumber;
    int                   Enabled;                   // Only valid if szEnabled != NULL
    char                * BusDataLinkName;           // R-x\BDLN-n (-04, -05)
    char                * ChannelDataLinkName;       // R-x\CDLN-n (-07, -09)
    int                   DataTypeFormat;            // (R-x\ATF-n)
    int                   ChannelsPerPacket;         // (R-1\ACH\N-n) //ORIG
    uint64_t              SampleRate;                // (R-1\ASR-n)   //ORIG

    uint32_t              Packed;                    // (R-1\ADP-n)   //ORIG
    char                * MeasurementName;           // (R-x\AMN-n-m)
    uint32_t              DataLength;                // (R-x\ADL-n-m)
    char                * BitMask;                   // (R-x\AMSK-n-m)
    uint32_t              MeasurementTransferOrder;  // Msb (0)/ LSB (1, unsupported) R-x\AMTO-n-m
    uint32_t              SampleFactor;              // (R-x\ASF-n-m)
    uint64_t              SampleFilter;              // (R-x\ASBW-n-m)
    uint32_t              DCCoupled;                 // D (0) / A (1)  R-x\ACP-n-m
    uint32_t              RecorderInputImpedance;    // (R-x\AII-n-m)
    int32_t               ChannelGain;               // (R-x\AGI-n-m)
    uint32_t              FullScaleRange;            // (R-x\AFSI-n-m)
    int32_t               OffsetVoltage;             // (R-x\AOVI-n-m)
    int32_t               LSBValue;                  // (R-x\ALSV-n-m)
    uint32_t              AnalogFormat;              // (R-x\AF-n-m)
    uint32_t              DifferentialInput;         // (R-x\AIT-n-m)
    uint32_t              Audio;                     // (R-x\AV-n-m)
    uint32_t              AudioFormat;               // (R-x\AVF-n-m)      
      
    // Stuff from R_Record
    uint32_t              DataSources;               // R-x\N
    uint32_t              IndexEnabled;              // R-x\ID
    uint32_t              EventsEnabled;             // R-x\EV\E

    // Computed values 
    int32_t               PrepareNextDecodingRun;    // First bit flag for a complete decoding run: preload a minor frame sync word to the test word

    // The possibility exists for multiple CSDWs and we want to keep a running copy of them, which we do with a subchannel structure
    AnalogF1_Subchannel * Subchannels[256];          // 256 is max number of subchannels

    // The buffer consists of two parts: A data buffer and an error buffer
    int32_t               BufferSize;                // Size of the output buffer in bytes
    uint8_t             * Buffer;                    // Contains the data
    uint8_t             * BufferError;               // Contains aberrant data

    // Variables for bit decoding
    int32_t               SaveData;                  // Save the data (0: do nothing, 1 save, 2: save terminated)
} PACKED;


// Current Analog message 
typedef struct AnalogF1_Message AnalogF1_Message;
struct AnalogF1_Message {
    I106C10Header        * Header;      // The overall packet header
    AnalogF1_CSDW        * CSDW;        // Header(s) in the data stream
    AnalogF1_Attributes  * Attributes;  // Pointer to analog-channel attributes structure, with most (all?) values imported from TMATS
    uint32_t               BytesRead;   // Number of bytes read in this message
    uint32_t               Length;      // Overall data packet length (in bytes)
    uint8_t              * Data;        // Pointer to the start of the data
    TimeRef                Time;
} PACKED;

#if defined(_MSC_VER)
#pragma pack(pop)
#endif


/* Function Declaration */
I106Status I106_Setup_AnalogF1(I106C10Header *header, void *buffer, AnalogF1_Message *msg);
I106Status I106_Decode_FirstAnalogF1(I106C10Header *header, void *buffer, AnalogF1_Message *msg);
I106Status I106_Decode_NextAnalogF1(AnalogF1_Message *msg);
I106Status Set_Attributes_AnalogF1(R_DataSource *r_datasource, AnalogF1_Attributes *attributes);
I106Status CreateOutputBuffers_AnalogF1(AnalogF1_Attributes *attributes, uint32_t data_length);
I106Status FreeOutputBuffers_AnalogF1(AnalogF1_Attributes *attributes);

// Help functions
I106Status PrintCSDW_AnalogF1(AnalogF1_CSDW *csdw);
I106Status PrintAttributesfromTMATS_AnalogF1(R_DataSource *r_datasource, AnalogF1_Attributes *attributes, FILE *output_file);

#endif
