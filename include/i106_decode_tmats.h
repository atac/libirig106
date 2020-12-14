/****************************************************************************

 i106_decode_tmats.h

 ****************************************************************************/

#ifndef _I106_DECODE_TMATS_H
#define _I106_DECODE_TMATS_H

#include "config.h"
#include "irig106ch10.h"

#if defined(_MSC_VER)
#define strcasecmp(s1, s2)          _stricmp(s1, s2)
#define strncasecmp(s1, s2, n)      _strnicmp(s1, s2, n)
#pragma warning(disable : 4996)
#endif

/* Macros and definitions */

// TMATS signature generating options
#define TMATS_SIGFLAG_NONE          0x0000
#define TMATS_SIGFLAG_INC_ALL       0x0001  // Include all fields
#define TMATS_SIGFLAG_INC_COMMENT   0x0002  // Include comment fields
#define TMATS_SIGFLAG_INC_VENDOR    0x0004  // Include vendor fields
#define TMATS_SIGFLAG_INC_G         0x0008  // Include vendor fields

// TMATS signature version
#define TMATS_SIGVER_1              1
#define TMATS_SIGVER_DEFAULT        TMATS_SIGVER_1
#define TMATS_SIGVER_MAX            TMATS_SIGVER_1


/* Data structures */

// Channel specific data word

#pragma pack(push, 1)

typedef struct {
    uint32_t    C10Version     :  8;      // Recorder Ch 10 Version
    uint32_t    ConfigChange   :  1;      // Recorder configuration changed
    uint32_t    Format         :  1;      // TMATS / XML Format
    uint32_t    Reserved       : 22;      // Reserved
} TMATS_CSDW;

#pragma pack(pop)

// NEED TO ADD STORAGE FOR REQUIRED DATA FIELDS
// NEED TO ADD SUPPORT OF "OTHER" DATA FIELDS TO PERMIT TMATS WRITE

// P Records
// ---------

// Asynchronous Embedded Streams definitions

typedef struct P_AsyncEmbedded P_AsyncEmbedded;
struct P_AsyncEmbedded {
    int                       EmbeddedStreamNumber;  // P-x\AEF\XXX-n
    char                    * DataLinkName;          // P-x\AEF\DLN-n
    struct P_Record         * P_Record;              // Corresponding P record
    struct P_AsyncEmbedded  * NextEmbedded;
};


// Subframe ID Counter definitions
// Note: Subframe definitions go away starting in 106-11

typedef struct P_SubframeLocation P_SubframeLocation;
struct P_SubframeLocation {
    int                          SubframeLocationNumber;  // P-x\SFx-x-x-n
    char                       * SubframeLocation;        // P-x\SF4-x-x-n
    struct P_SubframeLocation  * NextSubframeLocation;
};
    

typedef struct P_SubframeDefinition P_SubframeDefinition;
struct P_SubframeDefinition {
    int                            SubframeDefinitionNumber;  // P-x\SFx-x-n
    char                         * SubframeName;              // P-x\SF1-x-n
    char                         * SuperComPosition;          // P-x\SF2-x-n
    char                         * SuperComDefined;           // P-x\SF3-x-n
    P_SubframeLocation           * FirstSubframeLocation;     // P-x\SF4-x-x-x
    char                         * LocationInterval;          // P-x\SF5-x-n
    char                         * SubframeDepth;             // P-x\SF6-x-n

    struct P_SubframeDefinition  * NextSubframeDefinition;
};
    
    
typedef struct P_SubframeID P_SubframeID;
struct P_SubframeID {
    int                     CounterNumber;              // P-x\ISFx-n
    char                  * CounterName;                // P-x\ISF1-n
    char                  * CounterType;                // P-x\ISF2-n
    char                  * WordPosition;               // P-x\IDC1-n
    char                  * WordLength;                 // P-x\IDC2-n
    char                  * BitLocation;                // P-x\IDC3-n
    char                  * CounterLength;              // P-x\IDC4-n
    char                  * Endian;                     // P-x\IDC5-n
    char                  * InitValue;                  // P-x\IDC6-n
    char                  * MFForInitValue;             // P-x\IDC7-n
    char                  * EndValue;                   // P-x\IDC8-n
    char                  * MFForEndValue;              // P-x\IDC9-n
    char                  * CountDirection;             // P-x\IDC10-n

    char                  * NumberSubframeDefinitions;  // P-x\SF\N-n
    P_SubframeDefinition  * FirstSubframeDefinition;
    
    struct P_SubframeID   * NextSubframeID;
};

    
// P Record definition

typedef struct P_Record P_Record;
struct P_Record {
    int                RecordNumber;                 // P-x
    char             * DataLinkName;                 // P-x\DLN
    char             * PCMCode;                      // P-x\D1
    char             * BitsPerSecond;                // P-x\D2
    char             * Polarity;                     // P-x\D4
    char             * TypeFormat;                   // P-x\TF
    char             * CommonWordLength;             // P-x\F1
    char             * WordTransferOrder;            // P-x\F2 most significant bit "M", least significant bit "L". default: M
    char             * ParityType;                   // P-x\F3 even "EV", odd "OD", or none "NO", default: none
    char             * ParityTransferOrder;          // P-x\F4 leading "L", default: trailing
    char             * NumberMinorFrames;            // P-x\MF\N
    char             * WordsInMinorFrame;            // P-x\MF1
    char             * BitsInMinorFrame;             // P-x\MF2
    char             * MinorFrameSyncType;           // P-x\MF3
    char             * MinorFrameSyncPatternLength;  // P-x\MF4
    char             * MinorFrameSyncPattern;        // P-x\MF5
    char             * InSyncCritical;               // P-x\SYNC1
    char             * InSyncErrors;                 // P-x\SYNC2
    char             * OutSyncCritical;              // P-x\SYNC3
    char             * OutSyncErrors;                // P-x\SYNC4

    char             * NumberAsyncEmbedded;          // P-x\AEF\N  <-- ADD THIS ONE
    P_AsyncEmbedded  * FirstAsyncEmbedded;           // Link to embedded stream defs

    char             * NumberSubframeCounters;       // P-x\ISF\N
    P_SubframeID     * FirstSubframeID;              // Link to Subframe ID Counter defs
    
    struct P_Record  * NextP_Record;
};


// B Records

typedef struct B_Record B_Record;
struct B_Record {
    int                      RecordNumber;  // B-x
    char                   * DataLinkName;  // B-x\DLN
    char                   * NumberBuses;   // B-x\NBS\N
    struct B_Record        * NextB_Record;
};


// M Records

typedef struct M_Record M_Record;
struct M_Record {
    int                      RecordNumber;         // M-x
    char                   * DataSourceID;         // M-x\ID
    char                   * BB_DataLinkName;      // M-x\BB\DLN
    char                   * BasebandSignalType;   // M-x\BSG1
    struct P_Record        * P_Record;             // Corresponding P record
    struct B_Record        * B_Record;             // Corresponding B record
    struct M_Record        * NextM_Record;         // Used to keep track of M records
};


// R Records

// R record data source
typedef struct R_DataSource R_DataSource;
struct R_DataSource {
    int                    DataSourceNumber;          // R-x\XXX-n
    char                 * DataSourceID;              // R-x\DSI-n
    char                 * ChannelDataType;           // R-x\CDT-n
    char                 * RawTrackNumber;            // R-x\TK1-n
    int                    TrackNumber;               // Only valid if RawTrackNumber != NULL
    char                 * RawEnabled;                // R-x\CHE-n
    int                    Enabled;                   // Only valid if RawEnabled != NULL
    char                 * PCMDataLinkName;           // R-x\PDLN-n (-04, -05)
    char                 * BusDataLinkName;           // R-x\BDLN-n (-04, -05)
    char                 * ChannelDataLinkName;       // R-x\CDLN-n (-07, -09)

    // Video channel attributes
    char                 * VideoDataType;             // (R-x\VTF-n)
    char                 * VideoEncodeType;           // (R-x\VXF-n)
    char                 * VideoSignalType;           // (R-x\VST-n)
    char                 * VideoSignalFormat;         // (R-x\VSF-n)
    char                 * VideoConstBitRate;         // (R-x\CBR-n)
    char                 * VideoVarPeakBitRate;       // (R-x\VBR-n)
    char                 * VideoEncodingDelay;        // (R-x\VED-n)

    // PCM channel attributes
    char                 * PCMDataTypeFormat;         // (R-x\PDTF-n)
    char                 * PCMDataPacking;            // (R-x\PDP-n)
    char                 * PCMInputClockEdge;         // (R-x\ICE-n)
    char                 * PCMInputSignalType;        // (R-x\IST-n)
    char                 * PCMInputThreshold;         // (R-x\ITH-n)
    char                 * PCMInputTermination;       // (R-x\ITM-n)
    char                 * PCMVideoTypeFormat;        // (R-x\PTF-n)

    // Analog channel attributes
    char                 * AnalogChannelsPerPacket;   // (R-1\ACH\N-n)
    char                 * AnalogSampleRate;          // (R-1\ASR-n)
    char                 * AnalogDataPacking;         // (R-1\ADP-n)

    struct M_Record      * M_Record;                  // Corresponding M record
    struct P_Record      * P_Record;                  // Corresponding P record
    struct R_DataSource  * NextR_DataSource;
}; 

// R record
typedef struct R_Record R_Record;
struct R_Record {
    int                RecordNumber;       // R-x
    char             * DataSourceID;       // R-x\ID
    char             * NumberDataSources;  // R-x\N
    char             * RawIndexEnabled;    // R-x\IDX\E
    int                IndexEnabled;       // Only valid if RawIndexEnabled != NULL
    char             * RawEventsEnabled;   // R-x\EVE\E
    int                EventsEnabled;      // Only valid if RawEventsEnabled != NULL
    R_DataSource     * FirstDataSource;    //
    struct R_Record  * NextR_Record;       // Used to keep track of R records
};


// G Records

// G record, data source
typedef struct G_DataSource G_DataSource;
struct G_DataSource {
    int                    DataSourceNumber;  // G\XXX-n
    char                 * DataSourceID;      // G\DSI-n
    char                 * DataSourceType;    // G\DST-n
    struct R_Record      * R_Record;          // Corresponding R record
    struct G_DataSource  * NextG_DataSource;
};

// G record
typedef struct {
    char                     * ProgramName;        // G\PN
    char                     * Irig106Revision;    // G\106
    char                     * OriginationDate;    // G\OD
    char                     * NumberDataSources;  // G\DSI\N
    G_DataSource             * FirstG_DataSource;
} G_Record;


// Memory linked list

// Linked list that keeps track of malloc'ed memory
typedef struct MemoryBlock MemoryBlock;
struct MemoryBlock {
    void                    * MemoryBlock;
    struct MemoryBlock      * NextMemoryBlock;
};

// Decoded TMATS info

typedef struct {
    int             C10Version;
    int             ConfigChange;
    G_Record      * FirstG_Record;
    R_Record      * FirstR_Record;
    M_Record      * FirstM_Record;
    B_Record      * FirstB_Record;
    P_Record      * FirstP_Record;
    void          * FirstT_Record;
    void          * FirstD_Record;
    void          * FirstS_Record;
    void          * FirstA_Record;
    void          * FirstC_Record;
    void          * FirstH_Record;
    void          * FirstV_Record;
    MemoryBlock   * FirstMemoryBlock;
} TMATS_Info;


/* Function Declaration */

I106Status I106_Decode_TMATS(I106C10Header * header, void *buffer, TMATS_Info *tmats_info);
I106Status I106_Decode_TMATS_Text(char *buffer, uint32_t data_length, TMATS_Info *tmats_info);
void I106_Free_TMATS_Info(TMATS_Info *tmats_info);
I106Status I106_Encode_TMATS(I106C10Header *header, void *buffer, char *tmats);
void I106GetRawTMATS(I106C10Header* header, void* buffer, char** tmats_raw, int* length);
I106Status I106_TMATS_Signature(
    void *buffer,           // TMATS text without CSDW
    uint32_t data_length,   // Length of TMATS in pvBuff
    int signature_version,  // Request signature version (0 = default)
    int signature_flags,    // Additional flags
    uint16_t *opcode,       // Version and flag op code
    uint32_t *signature);   // TMATS signature

#endif
