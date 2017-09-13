/****************************************************************************

 i106_decode_tmats.c - 

 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <strings.h>
#endif
#ifndef __APPLE__
#include <malloc.h>
#endif
#include <ctype.h>
#include <assert.h>

#include "config.h"
#include "int.h"

#include "irig106ch10.h"
#include "i106_decode_tmats.h"

/******************************************************************************

Here's how this module decodes and stores TMATS data. 

Any field that is to be decoded and stored must have a corresponding entry 
in one of the defined data structures.  In other words, there is no support 
for storing and later retrieving arbitrary TMATS lines.  Maybe that would have 
been better, but for now only TMATS lines that are understood by this module
will be stored.

This module makes no assumptions about the ordering, numbering, or
numbers of TMATS lines. Information is stored in linked lists that are
created and grow as needed.  For now there is the assumption that there
is only one G record, but there may be multiples of other records.

There is a linked list for each type of record (except the one and only
G record).  As the TMATS lines are read one by one, the info is decoded
and stored in the appropriate existing record, or a new record is create
if necessary.

After all TMATS lines are read and decoded, the linked lists are scanned
and connected into a tree.  That is, R records are connected to the
corresponding G, M records are connected to the corresponding R, etc.
When done, the TMATS info is organized into a tree similar to that depicted
in IRIG 106 Chapter 9 (TMATS) Figure 9-1 "Group relationships".

There are at least two ways to use this information.  One is to start with
the top level G record and walk the tree.  This is a good way to provide
access to all the decoded data, for example, to print out everything in 
the tree.  The other way to access data is to start at the beginning of
one of the linked lists of records, and walk the linked list.  This might
be a good way to get just some specific data, like a list of Channel ID's
for 1553IN type data sources.

******************************************************************************/


// NEED TO ADD STORAGE FOR REQUIRED DATA FIELDS
// NEED TO ADD SUPPORT OF "OTHER" DATA FIELDS TO PERMIT TMATS WRITE

/* Macros and definitions */

#define CR      (13)
#define LF      (10)

/* Module data */

// This is an empty string that text fields can point to before
// they get a value. This ensures that if fields don't get set while
// reading the TMATS record they will point to something benign.
char                 empty_string[] = "";

static TMATS_Info  * module_tmats_info;
static int           tmats_version = 0;


/* Function Declaration */

int DecodeGLine(char *code_name, char *data_item, G_Record **first_g_record);
int DecodeRLine(char *code_name, char *data_item, R_Record **first_r_record);
int DecodeMLine(char *code_name, char *data_item, M_Record **first_m_record);
int DecodeBLine(char *code_name, char *data_item, B_Record **first_b_record);
int DecodePLine(char *code_name, char *data_item, P_Record **first_p_record);

R_Record * GetR_Record(R_Record **first_r_record, int index, int make_new);
M_Record * GetM_Record(M_Record **first_m_record, int index, int make_new);
B_Record * GetB_Record(B_Record **first_b_record, int index, int make_new);
P_Record * GetP_Record(P_Record **first_p_record, int index, int make_new);

G_DataSource * GetG_DataSource(G_Record *g_record, int index, int make_new);
R_DataSource * GetR_DataSource(R_Record *r_record, int index, int make_new);
P_AsyncEmbedded * GetP_AsyncEmbedded(P_Record *p_record, int index, int make_new);
P_SubframeID * GetP_SubframeID(P_Record *p_record, int index, int make_new);
P_SubframeDefinition * GetP_SubframeDefinition(P_SubframeID *subframe_id, int index, int make_new);
P_SubframeLocation * GetP_SubframeLocation(P_SubframeDefinition *subframe_definition, int index, int make_new);

void ConnectG(TMATS_Info *tmats_info);
void ConnectR(TMATS_Info *tmats_info);
void ConnectM(TMATS_Info *tmats_info);
void ConnectP(TMATS_Info *tmats_info);

void * TMATSMalloc(size_t size);

char * FirstNonWhitespace(char *string);

uint32_t Fletcher32(uint8_t *data, int count);


/* The idea behind this routine is to read the TMATS record, parse it, and 
 * put the various data fields into a tree structure that can be used later
 * to find various settings.  This routine assumes the buffer is a complete
 * TMATS record from a file, including the Channel Specific Data Word.  After
 * pulling out the CSDW stuff, it punts to the text decoder which does the
 * actual heaving lifting.
 */
I106Status I106_Decode_TMATS(I106C10Header *header, void *buffer, TMATS_Info *tmats_info){
    TMATS_CSDW  * csdw;

    // Decode any available info from channel specific data word
    switch (header->HeaderVersion){
        case 0x03 :  // 106-07, 106-09
            csdw = (TMATS_CSDW *)buffer;
            tmats_info->C10Version   = csdw->C10Version;
            tmats_info->ConfigChange = csdw->ConfigChange;
            break;
        default :
            tmats_info->C10Version   = 0x00;
            tmats_info->ConfigChange = 0x00;
            break;
    }

    return I106_Decode_TMATS_Text((char *)buffer + sizeof(TMATS_CSDW), header->DataLength,
            tmats_info);
}


// This routine parses just the text portion of TMATS.
I106Status I106_Decode_TMATS_Text(char *text, uint32_t data_length, TMATS_Info *tmats_info){
    unsigned long       buffer_pos = 0;
    char              * buffer;
    char                line[2048];
    int                 line_pos;
    char              * code_name;
    char              * data_item;
    int                 parse_error;

    // Store a copy for module wide use
    module_tmats_info = tmats_info;

    // Initialize the TMATS info data structure
    I106_Free_TMATS_Info(tmats_info);

    // Initialize the first (and only, for now) G record
    tmats_info->FirstG_Record = (G_Record *)TMATSMalloc(sizeof(G_Record));
    tmats_info->FirstG_Record->ProgramName       = NULL;
    tmats_info->FirstG_Record->Irig106Revision   = NULL;
    tmats_info->FirstG_Record->NumberDataSources = NULL;
    tmats_info->FirstG_Record->FirstG_DataSource = NULL;

    // Init buffer pointers
    buffer = (char *)text;

    // Loop until we get to the end of the buffer
    while (buffer_pos < data_length){

        // Take a single line from the buffer.
        line[0] = '\0';
        line_pos  = 0;
        while (1){
            // If at the end of the buffer then break out
            if (buffer_pos >= data_length)
                break;

            // Copy anything other than CR and LF into "line".
            if ((buffer[buffer_pos] != CR) && (buffer[buffer_pos] != LF)){
                line[line_pos] = buffer[buffer_pos];
                if (line_pos < 2048)
                    line_pos++;
                line[line_pos] = '\0';
            }

            // Advance the buffer
            buffer_pos++;

            // If we find a semicolon and the line isn't empty, break out.
            if (buffer[buffer_pos-1] == ';' && strlen(line) != 0)
                break;
        }


        // Decode the TMATS line

        // Split the line into left hand and right hand sides
        code_name = strtok(line, ":");
        data_item = strtok(NULL, ";");

        // If errors tokenizing the line then skip 
        if ((code_name == NULL) || (data_item == NULL))
            continue;

        // Determine and decode different TMATS types
        switch (code_name[0]){
            case 'G' : // General Information
                parse_error = DecodeGLine(code_name, data_item,
                        &tmats_info->FirstG_Record);
                break;

            case 'B' : // Bus Data Attributes
                parse_error = DecodeBLine(code_name, data_item,
                        &tmats_info->FirstB_Record);
                break;

            case 'R' : // Tape/Storage Source Attributes
                parse_error = DecodeRLine(code_name, data_item,
                        &tmats_info->FirstR_Record);
                break;

            case 'T' : // Transmission Attributes
                break;

            case 'M' : // Multiplexing/Modulation Attributes
                parse_error = DecodeMLine(code_name, data_item,
                        &tmats_info->FirstM_Record);
                break;

            case 'P' : // PCM Format Attributes
                parse_error = DecodePLine(code_name, data_item,
                        &tmats_info->FirstP_Record);
                break;

            // TODO: implement these and any other missing groups/attributes
            case 'D' : // PCM Measurement Description
                break;

            case 'S' : // Packet Format Attributes
                break;

            case 'A' : // PAM Attributes
                break;

            case 'C' : // Data Conversion Attributes
                break;

            case 'H' : // Airborne Hardware Attributes
                break;

            case 'V' : // Vendor Specific Attributes
                break;

            default :
                break;

        }

    }


    /* Now link the various records together into a tree.  This is a bit involved.
    
    G/DSI-n <-+-> T-x\ID
              +-> M-x\ID
    
    G/DSI-n <---> R-x\ID
                  R-x\DSI-n <---> M-x\ID
                                  M-x\BB\DLN   <-+-> P-x\DLN  \
                                                 +-> B-x\DLN   - With M-x Baseband
                                                 +-> S-x\DLN  /

                                  M-x\SI\DLN-n <-+-> P-x\DLN  \
                                                 +-> B-x\DLN   - With M-x Subchannels
                                                 +-> S-x\DLN  /

                  R-x\CDLN-n <-------------------+-> P-x\DLN  \
                                                 +-> B-x\DLN   - Without M-x
                                                 +-> S-x\DLN  /

    -> D-x\DLN
    -> D-x\DLN

    -> D-x\DLN
    */

    ConnectG(tmats_info);
    ConnectR(tmats_info);
    ConnectM(tmats_info);
    ConnectP(tmats_info);

    module_tmats_info = NULL;

    return I106_OK;
}



/* G Records */

int DecodeGLine(char *code_name, char *data_item, G_Record **first_g){
    char         * code_field;
    int            tokens;
    int            index;
    G_Record     * g_record;
    G_DataSource * data_source;

    // See which G field it is
    code_field = strtok(code_name, "\\");
    assert(code_field[0] == 'G');

    // Get the G record
    g_record = *first_g;

    code_field = strtok(NULL, "\\");

    // PN - Program Name
    if (strcasecmp(code_field, "PN") == 0){
        g_record->ProgramName = (char *)TMATSMalloc(strlen(data_item) + 1);
        strcpy(g_record->ProgramName, data_item);
    }

    // 106 - IRIG 106 Ch 9 rev level
    else if (strcasecmp(code_field, "106") == 0){
        g_record->Irig106Revision = (char *)TMATSMalloc(strlen(data_item) + 1);
        strcpy(g_record->Irig106Revision, data_item);
        tmats_version = atoi(data_item);
    }

    // OD - Date
    else if (strcasecmp(code_field, "OD") == 0){
        g_record->OriginationDate = (char *)TMATSMalloc(strlen(data_item) + 1);
        strcpy(g_record->OriginationDate, data_item);
    }

    // DSI - Data source identifier info
    else if (strcasecmp(code_field, "DSI") == 0){
        code_field = strtok(NULL, "\\");
        // N - Number of data sources
        if (strcasecmp(code_field, "N") == 0){
            g_record->NumberDataSources = (char *)TMATSMalloc(strlen(data_item) + 1);
            strcpy(g_record->NumberDataSources, data_item);
        }
    }

    // DSI-n - Data source identifiers
    else if (strncasecmp(code_field, "DSI-",4) == 0){
        tokens = sscanf(code_field, "%*3c-%i", &index);
        if (tokens == 1){
            data_source = GetG_DataSource(g_record, index, 1);
            assert(data_source != NULL);
            data_source->DataSourceID = (char *)TMATSMalloc(strlen(data_item) + 1);
            strcpy(data_source->DataSourceID, data_item);
        }
    }

    // DST-n - Data source type
    else if (strncasecmp(code_field, "DST-", 4) == 0){
        tokens = sscanf(code_field, "%*3c-%i", &index);
        if (tokens == 1){
            data_source = GetG_DataSource(g_record, index, 1);
            assert(data_source != NULL);
            data_source->DataSourceType = (char *)TMATSMalloc(strlen(data_item) + 1);
            strcpy(data_source->DataSourceType, data_item);
        }
    }

    return 0;
}


// Return the G record Data Source record with the given index or
// make a new one if necessary.
G_DataSource * GetG_DataSource(G_Record *g_record, int index, int make_new){
    G_DataSource **data_source = &(g_record->FirstG_DataSource);

    // Walk the linked list of data sources, looking for a match or
    // the end of the list
    while (1){
        // If record pointer in linked list is null then exit
        if (*data_source == NULL)
            break;

        // If the data source number matched then record found, exit
        if ((*data_source)->DataSourceNumber == index)
            break;

        // Not found but next record exists so make it our current pointer
        data_source = &((*data_source)->NextG_DataSource);
    }

    // If no record found then put a new one on the end of the list
    if ((*data_source == NULL) && make_new){
        // Allocate memory for the new record
        *data_source = (G_DataSource *)TMATSMalloc(sizeof(G_DataSource));
        memset(*data_source, 0, sizeof(G_DataSource));
        (*data_source)->DataSourceNumber = index;

        // Now initialize some fields
        (*data_source)->DataSourceNumber = index;
        (*data_source)->DataSourceID     = NULL;
        (*data_source)->DataSourceType   = NULL;
        (*data_source)->R_Record         = NULL;
        (*data_source)->NextG_DataSource = NULL;
    }

    return *data_source;
}


/*
    Tie the G record data sources to their underlying R and T
    records.

    For recorder case...
    G/DSI-n <---> R-x\ID

    For telemetry case...
    G/DSI-n <-+-> T-x\ID
              +-> R-x\ID

*/

void ConnectG(TMATS_Info * tmats_info){
    R_Record       * r_record;
    G_DataSource   * g_datasource;

    // Step through the G data source records
    g_datasource = tmats_info->FirstG_Record->FirstG_DataSource;
    while (g_datasource != NULL){
        // Walk through the R records linked list looking for a match
        r_record = tmats_info->FirstR_Record;
        while (r_record != NULL){
            // See if G/DSI-n = R-x\ID
            if ((g_datasource->DataSourceID != NULL) &&
                    (r_record->DataSourceID != NULL) &&
                    (strcasecmp(g_datasource->DataSourceID, r_record->DataSourceID) == 0)){

                // Note, if g_datasource->r_record != NULL then that 
                // is probably an error in the TMATS file
                assert(g_datasource->R_Record == NULL);
                g_datasource->R_Record = r_record;
            }

            // Get the next R record
            r_record = r_record->NextR_Record;
        }

        // Get the next G data source record
        g_datasource = g_datasource->NextG_DataSource;
    }
}


/* R Records */

// Macros to make decoding R record logic more compact

// Decode an R record
#define DECODE_R(pattern, field)                                       \
    else if (strcasecmp(code_field, #pattern) == 0){                   \
        r_record->field = (char *)TMATSMalloc(strlen(data_item) + 1);  \
        strcpy(r_record->field, data_item);                            \
    }

// Decode an R record and convert the result to a boolean
#define DECODE_R_BOOL(pattern, field, bfield, truechar)                          \
    else if (strcasecmp(code_field, #pattern) == 0){                             \
        r_record->field = (char *)TMATSMalloc(strlen(data_item) + 1);            \
        strcpy(r_record->field, data_item);                                      \
        r_record->bfield = toupper(*FirstNonWhitespace(data_item)) == truechar;  \
    }

// Decode an R Data Source record
#define DECODE_R_DS(pattern, field)                                             \
    else if (strncasecmp(code_field, #pattern"-", strlen(#pattern) + 1) == 0){  \
        int                tokens;                                              \
        int                dsi_index;                                           \
        char               format[20];                                          \
        R_DataSource     * datasource;                                          \
        sprintf(format, "%%*%dc-%%i", (int)strlen(#pattern));                   \
        tokens = sscanf(code_field, format, &dsi_index);                        \
        if (tokens == 1){                                                       \
            datasource = GetR_DataSource(r_record, dsi_index, 1);               \
            assert(datasource != NULL);                                         \
            datasource->field = (char *)TMATSMalloc(strlen(data_item) + 1);     \
            strcpy(datasource->field, data_item);                               \
        }                                                                       \
        else                                                                    \
            assert(0);                                                          \
    }

// Decode an R Data Source record and convert to a boolean
#define DECODE_R_DS_BOOL(pattern, raw_field, field, truechar)                    \
    else if (strncasecmp(code_field, #pattern"-", strlen(#pattern) + 1) == 0){   \
        int                 tokens;                                              \
        int                 dsi_index;                                           \
        char                format[20];                                          \
        R_DataSource     * datasource;                                           \
        sprintf(format, "%%*%dc-%%i", (int)strlen(#pattern));                    \
        tokens = sscanf(code_field, format, &dsi_index);                         \
        if (tokens == 1){                                                        \
            datasource = GetR_DataSource(r_record, dsi_index, 1);                \
            assert(datasource != NULL);                                          \
            datasource->raw_field = (char *)TMATSMalloc(strlen(data_item) + 1);  \
            strcpy(datasource->raw_field, data_item);                            \
            datasource->field =                                                  \
                toupper(*FirstNonWhitespace(data_item)) == truechar;             \
        }                                                                        \
        else                                                                     \
            assert(0);                                                           \
    }

// Decode an R Data Source record and convert to an int
#define DECODE_R_DS_INT(pattern, raw_field, field)                              \
    else if (strncasecmp(code_field, #pattern"-", strlen(#pattern) + 1) == 0){  \
        int                tokens;                                              \
        int                dsi_index;                                           \
        char               format[20];                                          \
        R_DataSource     * datasource;                                          \
        sprintf(format, "%%*%dc-%%i", (int)strlen(#pattern));                   \
        tokens = sscanf(code_field, format, &dsi_index);                        \
        if (tokens == 1){                                                       \
            datasource = GetR_DataSource(r_record, dsi_index, 1);               \
            assert(datasource != NULL);                                         \
            datasource->raw_field = (char *)TMATSMalloc(strlen(data_item) + 1); \
            strcpy(datasource->raw_field, data_item);                           \
            datasource->field = atoi(data_item);                                \
        }                                                                       \
        else                                                                    \
            assert(0);                                                          \
    }


int DecodeRLine(char *code_name, char *data_item, R_Record **first_r_record){
    int             found;
    char          * code_field;
    int             tokens;
    int             r_index;
    int             dsi_index;
    R_Record      * r_record;
    R_DataSource  * datasource;

    found = 0;

    // See which R field it is
    code_field = strtok(code_name, "\\");
    assert(code_field[0] == 'R');

    // Get the R record index number
    tokens = sscanf(code_field, "%*1c-%i", &r_index);
    if (tokens == 1){
        r_record = GetR_Record(first_r_record, r_index, 1);
        assert(r_record != NULL);
    }
    else
        return 1;
    
    code_field = strtok(NULL, "\\");

    if (0) {}                       // Keep macro logic happy
    
    DECODE_R(ID, DataSourceID)      // ID - Data source ID
    DECODE_R(N, NumberDataSources)  // N - Number of data sources

    // IDX - Indexes
    else if (strcasecmp(code_field, "IDX") == 0){
        code_field = strtok(NULL, "\\");
        if (0) {}                                              // Keep macro logic happy
        DECODE_R_BOOL(E, RawIndexEnabled, IndexEnabled, 'T');  // IDX\E - Index enabled
    }

    // EV - Events
    else if (strcasecmp(code_field, "EV") == 0){
        code_field = strtok(NULL, "\\");
        if (0) {}                                                // Keep macro logic happy
        DECODE_R_BOOL(E, RawEventsEnabled, EventsEnabled, 'T');  // EV\E - Events enabled
    }
    
    DECODE_R_DS(DSI, DataSourceID)            // DSI-n - Data source identifier

    // CDT-n/DST-n - Channel data type
    // A certain vendor who will remain nameless (mainly because I don't
    // know which one) encodes the channel data type as a Data Source
    // Type.  This appears to be incorrect according to the Chapter 9
    // spec but can be readily found in Chapter 10 data files.
    else if ((strncasecmp(code_field, "CDT-", 4) == 0) ||   // -07
             (strncasecmp(code_field, "DST-", 4) == 0)){
        tokens = sscanf(code_field, "%*3c-%i", &dsi_index);
        if (tokens == 1){
            datasource = GetR_DataSource(r_record, dsi_index, 1);
            assert(datasource != NULL);
            datasource->ChannelDataType = (char *)TMATSMalloc(strlen(data_item) + 1);
            strcpy(datasource->ChannelDataType, data_item);
        }
        else
            return 1;
    }

    DECODE_R_DS_INT(TK1, RawTrackNumber, TrackNumber)   // TK1-n - Track number / Channel number
    DECODE_R_DS_BOOL(CHE, RawEnabled, Enabled, 'T')     // CHE-n - Channel Enabled

    DECODE_R_DS(BDLN, BusDataLinkName)        // BDLN-n - Data Link Name (-04, -05)
    DECODE_R_DS(PDLN, PCMDataLinkName)        // PDLN-n - PCM Data Link Name (-04, -05)
    DECODE_R_DS(CDLN, ChannelDataLinkName)    // CDLN-n - Channel Data Link Name (-07, -09)

    // Video Attributes
    DECODE_R_DS(VTF,  VideoDataType)          // VTF-n - Video Data Type Format
    DECODE_R_DS(VXF,  VideoEncodeType)        // VXF-n - Video Encoder Type
    DECODE_R_DS(VST,  VideoSignalType)        // VST-n - Video Signal Type
    DECODE_R_DS(VSF,  VideoSignalFormat)      // VSF-n - Video Signal Format
    DECODE_R_DS(CBR,  VideoConstBitRate)      // CBR-n - Video Constant Bit Rate
    DECODE_R_DS(VBR,  VideoVarPeakBitRate)    // VBR-n - Video Variable Peak Bit Rate
    DECODE_R_DS(VED,  VideoEncodingDelay)     // VED-n - Video Encoding Delay

    // PCM Attributes
    DECODE_R_DS(PDTF, PCMDataTypeFormat)      // PDTF-n - PCM Data Type Format
    DECODE_R_DS(PDP,  PCMDataPacking)         // PDP-n - PCM Data Packing Option
    DECODE_R_DS(ICE,  PCMInputClockEdge)      // ICE-n - PCM Input Clock Edge
    DECODE_R_DS(IST,  PCMInputSignalType)     // IST-n - PCM Input Signal Type
    DECODE_R_DS(ITH,  PCMInputThreshold)      // ITH-n - PCM Input Threshold
    DECODE_R_DS(ITM,  PCMInputTermination)    // ITM-n - PCM Input Termination
    DECODE_R_DS(PTF,  PCMVideoTypeFormat)     // PTF-n - PCM Video Type Format

    // Analog Attributes
    else if (strncasecmp(code_field, "ACH", 3) == 0){
        code_field = strtok(NULL, "\\");
        if (0) {}                                // Keep macro logic happy
        DECODE_R_DS(N, AnalogChannelsPerPacket)  // ACH\N - Analog channels per packet
    }

    DECODE_R_DS(ASR, AnalogSampleRate)        // ASR-n - Analog sample rate
    DECODE_R_DS(ADP, AnalogDataPacking)       // ADP-n - Analog data packing

    return 0;
}


R_Record * GetR_Record(R_Record **first_r_record, int index, int make_new){
    R_Record ** r_record = first_r_record;

    // Loop looking for matching index number or end of list
    while (1){
        // Check for end of list
        if (*r_record == NULL)
            break;

        // Check for matching index number
        if ((*r_record)->RecordNumber == index)
            break;

        // Move on to the next record in the list
        r_record = &((*r_record)->NextR_Record);
    }

    // If no record found then put a new one on the end of the list
    if ((*r_record == NULL) && make_new){
        // Allocate memory for the new record
        *r_record = (R_Record *)TMATSMalloc(sizeof(R_Record));
        memset(*r_record, 0, sizeof(R_Record));
        (*r_record)->RecordNumber = index;

    }

    return *r_record;
}


// Return the R record Data Source record with the given index or
// make a new one if necessary.
R_DataSource * GetR_DataSource(R_Record * r_record, int index, int make_new){
    R_DataSource ** datasource = &(r_record->FirstDataSource);

    // Walk the linked list of data sources, looking for a match or
    // the end of the list
    while (1){
        // If record pointer in linked list is null then exit
        if (*datasource == NULL)
            break;

        // If the data source number matched then record found, exit
        if ((*datasource)->DataSourceNumber == index)
            break;

        // Not found but next record exists so make it our current pointer
        datasource = &((*datasource)->NextR_DataSource);
    }

    // If no record found then put a new one on the end of the list
    if ((*datasource == NULL) && make_new){
        // Allocate memory for the new record
        *datasource = (R_DataSource *)TMATSMalloc(sizeof(R_DataSource));
        memset(*datasource, 0, sizeof(R_DataSource));
        (*datasource)->DataSourceNumber = index;

    }

    return *datasource;
}


/*
    Tie the R record data sources to their underlying M records. In some
    cases the M record is skipped and the P, B, and S records tie directly.
    The fields used for these ties varies based on the Ch 9 release version.

    R-x\DSI-n <---> M-x\ID

    106-04 and 106-05 version...
    R-x\PDLN-n <---------------------> P-x\DLN   - Without M-x
    R-x\BDLN-n <---------------------> B-x\DLN   - Without M-x

    106-07 and 106-09 version...
    R-x\CDLN-n <-------------------+-> P-x\DLN  \
                                   +-> B-x\DLN   - Without M-x
                                   +-> S-x\DLN  /
*/
void ConnectR(TMATS_Info * tmats_info){
    R_Record       * r_record;
    R_DataSource   * datasource;
    M_Record       * m_record;
    P_Record       * p_record;

    // Walk the linked list of R records
    r_record = tmats_info->FirstR_Record;
    while (r_record != NULL){

        // Walk the linked list of R data sources
        datasource = r_record->FirstDataSource;
        while (datasource != NULL){

            // Walk through the M records linked list
            m_record = tmats_info->FirstM_Record;
            while (m_record != NULL){
                // See if R-x\DSI-n = M-x\ID
                if ((datasource->DataSourceID != NULL) &&
                        (m_record->DataSourceID != NULL) &&
                        (strcasecmp(datasource->DataSourceID,
                                    m_record->DataSourceID) == 0)){
                    // Note, if psuCurrRDataSrc->psuMRecord != NULL then that 
                    // is probably an error in the TMATS file
                    assert(datasource->M_Record == NULL);
                    datasource->M_Record = m_record;
                }

                // Get the next M record
                m_record = m_record->NextM_Record;
            }

            // Walk through the P records linked list
            p_record = tmats_info->FirstP_Record;
            while (p_record != NULL){
                // R to P tieing changed with the -07 release.  Try to do it the
                // "right" way first, but accept the "wrong" way if that doesn't work.
                // TMATS 04 and 05
                if ((tmats_version == 4) || (tmats_version == 5)){
                    // See if R-x\PDLN-n = P-x\DLN, aka the "right" way
                    if ((datasource->PCMDataLinkName != NULL) &&
                            (p_record->DataLinkName != NULL) &&
                            (strcasecmp(datasource->PCMDataLinkName,
                                        p_record->DataLinkName) == 0)){
                        // Note, if psuCurrRDataSrc->psuPRecord != NULL then that 
                        // is probably an error in the TMATS file
                        assert(datasource->P_Record == NULL);
                        datasource->P_Record = p_record;
                    }

                }

                // TMATS 07, 09, and beyond (I hope)
                else {
                    // See if R-x\CDLN-n = P-x\DLN, aka the "right" way
                    if ((datasource->ChannelDataLinkName != NULL) &&
                            (p_record->DataLinkName != NULL) &&
                            (strcasecmp(datasource->ChannelDataLinkName,
                                        p_record->DataLinkName) == 0)){
                        // Note, if psuCurrRDataSrc->psuPRecord != NULL then that 
                        // is probably an error in the TMATS file
                        assert(datasource->P_Record == NULL);
                        datasource->P_Record = p_record;
                    }

                }

                // Get the next P record
                p_record = p_record->NextP_Record;
            }

            // Walk the P, B, and S record link lists

            // Get the next R data source record
            datasource = datasource->NextR_DataSource;
        }

        // Get the next R record
        r_record = r_record->NextR_Record;
    }
}


/* M Records */

int DecodeMLine(char *code_name, char *data_item, M_Record **first_m_record){
    char         * code_field;
    int            tokens;
    int            index;
    M_Record     * m_record;

    // See which M field it is
    code_field = strtok(code_name, "\\");
    assert(code_field[0] == 'M');

    // Get the M record index number
    tokens = sscanf(code_field, "%*1c-%i", &index);
    if (tokens == 1){
        m_record = GetM_Record(first_m_record, index, 1);
        assert(m_record != NULL);
    }
    else
        return 1;
    
    code_field = strtok(NULL, "\\");

    // ID - Data source ID
    if (strcasecmp(code_field, "ID") == 0){
        m_record->DataSourceID = (char *)TMATSMalloc(strlen(data_item) + 1);
        strcpy(m_record->DataSourceID, data_item);
    }

    // BSG1 - Baseband signal type
    else if (strcasecmp(code_field, "BSG1") == 0){
        m_record->BasebandSignalType = (char *)TMATSMalloc(strlen(data_item) + 1);
        strcpy(m_record->BasebandSignalType, data_item);
    }

    // BB\DLN - Data link name
    else if (strcasecmp(code_field, "BB") == 0){
        code_field = strtok(NULL, "\\");
        // DLN - Data link name
        if (strcasecmp(code_field, "DLN") == 0){
            m_record->BB_DataLinkName = (char *)TMATSMalloc(strlen(data_item) + 1);
            strcpy(m_record->BB_DataLinkName, data_item);
        }
    }

    return 0;
}


M_Record * GetM_Record(M_Record **first_m_record, int index, int make_new){
    M_Record ** m_record = first_m_record;

    // Loop looking for matching index number or end of list
    while (1){
        // Check for end of list
        if (*m_record == NULL)
            break;

        // Check for matching index number
        if ((*m_record)->RecordNumber == index)
            break;

        // Move on to the next record in the list
        m_record = &((*m_record)->NextM_Record);
    }

    // If no record found then put a new one on the end of the list
    if ((*m_record == NULL) && make_new){
        // Allocate memory for the new record
        *m_record = (M_Record *)TMATSMalloc(sizeof(M_Record));
        memset(*m_record, 0, sizeof(M_Record));
        (*m_record)->RecordNumber  = index;
    }

    return *m_record;
}


/*
    Tie the M record baseband and subchannel sources to their underlying P,
    B, and S records.

    Baseband case...
    M-x\BB\DLN   <-+-> P-x\DLN
                   +-> B-x\DLN
                   +-> S-x\DLN

    Subchannel case...
    M-x\SI\DLN-n <-+-> P-x\DLN
                   +-> B-x\DLN
                   +-> S-x\DLN

*/

void ConnectM(TMATS_Info *tmats_info){
    M_Record       * m_record;
    B_Record       * b_record;
    P_Record       * p_record;

    // Walk the linked list of M records
    m_record = tmats_info->FirstM_Record;
    while (m_record != NULL){

        // Walk through the P record linked list
        p_record = tmats_info->FirstP_Record;
        while (p_record != NULL){

            // Note, if psuCurrRRecord->psuPRecord != NULL then that 
            // is probably an error in the TMATS file
            // HMMM... CHECK THESE TIE FIELDS
            if ((tmats_version == 4) || (tmats_version == 5)){
                if ((m_record->BB_DataLinkName != NULL) &&
                        (p_record->DataLinkName != NULL) &&
                        (strcasecmp(m_record->BB_DataLinkName,
                                    p_record->DataLinkName) == 0)){
                    assert(m_record->P_Record == NULL);
                    m_record->P_Record = p_record;
                }
            }

            else if ((tmats_version == 7) || (tmats_version == 9)){
                if ((m_record->BB_DataLinkName != NULL) &&
                        (p_record->DataLinkName != NULL) &&
                        (strcasecmp(m_record->BB_DataLinkName,
                                    p_record->DataLinkName) == 0)){
                    assert(m_record->P_Record == NULL);
                    m_record->P_Record = p_record;
                }
            }

            // Get the next P record
            p_record = p_record->NextP_Record;
        }

        // Walk through the B record linked list
        b_record = tmats_info->FirstB_Record;
        while (b_record != NULL){

                // See if M-x\BB\DLN = B-x\DLN
                if ((m_record->BB_DataLinkName != NULL) &&
                        (b_record->DataLinkName   != NULL) &&
                        (strcasecmp(m_record->BB_DataLinkName,
                                    b_record->DataLinkName) == 0)){
                    // Note, if psuCurrMRecord->psuBRecord != NULL then that 
                    // is probably an error in the TMATS file
                    assert(m_record->B_Record == NULL);
                    m_record->B_Record = b_record;
                }

            // Get the next B record
            b_record = b_record->NextB_Record;
        }

        // Walk through the S record linked list another day


        // Get the next M record
        m_record = m_record->NextM_Record;
    }

    // Do subchannels some other day!
}



/* B Records */

// Macros to make decoding B record logic more compact

#define DECODE_B(pattern, field)                                       \
    else if (strcasecmp(code_field, #pattern) == 0){                   \
        b_record->field = (char *)TMATSMalloc(strlen(data_item) + 1);  \
        strcpy(b_record->field, data_item);                            \
    }


int DecodeBLine(char *code_name, char *data_item, B_Record **first_b_record){
    char         * code_field;
    int            tokens;
    int            index;
    B_Record     * b_record;

    // See which B field it is
    code_field = strtok(code_name, "\\");
    assert(code_field[0] == 'B');

    // Get the B record index number
    tokens = sscanf(code_field, "%*1c-%i", &index);
    if (tokens == 1){
        b_record = GetB_Record(first_b_record, index, 1);
        assert(b_record != NULL);
    }
    else
        return 1;
    
    code_field = strtok(NULL, "\\");

    if (0) {}                              // Keep macro logic happy
    DECODE_B(DLN, DataLinkName)            // DLN - Data link name

    // NBS\N - Number of buses
    else if (strncasecmp(code_field, "NBS", 3) == 0){
        code_field = strtok(NULL, "\\");

        if (0) {}                          // Keep macro logic happy
        DECODE_B(N, NumberBuses)                 // NBS\N - Number of buses
    }

    return 0;
}


B_Record * GetB_Record(B_Record **first_b_record, int index, int make_new){
    B_Record ** b_record = first_b_record;

    // Loop looking for matching index number or end of list
    while (1){
        // Check for end of list
        if (*b_record == NULL)
            break;

        // Check for matching index number
        if ((*b_record)->RecordNumber == index)
            break;

        // Move on to the next record in the list
        b_record = &((*b_record)->NextB_Record);
    }

    // If no record found then put a new one on the end of the list
    if ((*b_record == NULL) && make_new){
        // Allocate memory for the new record
        *b_record = (B_Record *)TMATSMalloc(sizeof(B_Record));
        memset(*b_record, 0, sizeof(B_Record));
        (*b_record)->RecordNumber = index;
    }

    return *b_record;
}


/* P Records */

// Macros to make decoding P record logic more compact

#define DECODE_P(pattern, field)                                       \
    else if (strcasecmp(code_field, #pattern) == 0){                   \
        p_record->field = (char *)TMATSMalloc(strlen(data_item) + 1);  \
        strcpy(p_record->field, data_item);                            \
    }


#define DECODE_P_SF(pattern, field)                                           \
    else if (strncasecmp(code_field, #pattern"-", strlen(#pattern)+1) == 0){  \
        int                tokens;                                            \
        int                index;                                             \
        char               format[20];                                        \
        P_SubframeID     * subframe;                                          \
        sprintf(format, "%%*%dc-%%i", (int)strlen(#pattern));                      \
        tokens = sscanf(code_field, format, &index);                          \
        if (tokens == 1){                                                     \
            subframe = GetP_SubframeID(p_record, index, 1);                   \
            assert(subframe != NULL);                                         \
            subframe->field = (char *)TMATSMalloc(strlen(data_item) + 1);     \
            strcpy(subframe->field, data_item);                               \
        }                                                                     \
    }


#define DECODE_P_SFDEF(pattern, field)                                                   \
    else if (strncasecmp(code_field, #pattern"-", strlen(#pattern) + 1) == 0){           \
        int                     tokens;                                                  \
        int                     sf_index;                                                \
        int                     count_index;                                             \
        char                    format[20];                                              \
        P_SubframeID          * subframe_id;                                             \
        P_SubframeDefinition  * subframe_definition;                                     \
        sprintf(format, "%%*%dc-%%i-%%i", (int)strlen(#pattern));                             \
        tokens = sscanf(code_field, format, &sf_index, &count_index);                    \
        if (tokens == 2){                                                                \
            subframe_id = GetP_SubframeID(p_record, sf_index, 1);                        \
            assert(subframe_id != NULL);                                                 \
            subframe_definition = GetP_SubframeDefinition(subframe_id, count_index, 1);  \
            assert(subframe_definition != NULL);                                         \
            subframe_definition->field = (char *)TMATSMalloc(strlen(data_item) + 1);     \
            strcpy(subframe_definition->field, data_item);                               \
        }                                                                                \
    }


#define DECODE_P_SFDEFLOC(pattern, field)                                               \
    else if (strncasecmp(code_field, #pattern"-", strlen(#pattern) + 1) == 0){          \
        int                     tokens;                                                 \
        int                     sf_index;                                               \
        int                     count_index;                                            \
        int                     location;                                               \
        char                    format[20];                                             \
        P_SubframeID          * subframe_id;                                            \
        P_SubframeDefinition  * subframe_definition;                                    \
        P_SubframeLocation    * subframe_location;                                      \
        sprintf(format, "%%*%dc-%%i-%%i-%%i", (int)strlen(#pattern));                   \
        tokens = sscanf(code_field, format, &sf_index, &count_index, &location);        \
        if (tokens == 3){                                                               \
            subframe_id = GetP_SubframeID(p_record, sf_index, 1);                       \
            assert(subframe_id != NULL);                                                \
            subframe_definition = GetP_SubframeDefinition(subframe_id, count_index, 1); \
            assert(subframe_definition != NULL);                                        \
            subframe_location = GetP_SubframeLocation(subframe_definition, location, 1);  \
            assert(subframe_location != NULL);                                          \
            subframe_location->field = (char *)TMATSMalloc(strlen(data_item) + 1);      \
            strcpy(subframe_location->field, data_item);                                \
        }                                                                               \
    }


int DecodePLine(char *code_name, char *data_item, P_Record **first_p_record){
    char             * code_field;
    int                tokens;
    int                p_index;
    P_Record         * p_record;
    P_AsyncEmbedded  * aef;
    int                aef_index;

    // See which P field it is
    code_field = strtok(code_name, "\\");
    assert(code_field[0] == 'P');

    // Get the P record index number
    tokens = sscanf(code_field, "%*1c-%i", &p_index);
    if (tokens == 1){
        p_record = GetP_Record(first_p_record, p_index, 1);
        assert(p_record != NULL);
    }
    else
        return 1;
    
    code_field = strtok(NULL, "\\");

    if (0) {}                          // Keep macro logic happy
    
    DECODE_P(DLN, DataLinkName)        // DLN - Data link name
    DECODE_P(D1, PCMCode)              // D1 - PCM Code
    DECODE_P(D2, BitsPerSecond)        // D2 - Bit Rate
    DECODE_P(D4, Polarity)             // D4 - Polarity
    DECODE_P(TF, TypeFormat)           // TF - Type Format
    DECODE_P(F1, CommonWordLength)     // F1 - Common World Length
    DECODE_P(F2, WordTransferOrder)    // F2 - MSB / LSB first
    DECODE_P(F3, ParityType)           // F3 - Even, odd, none
    DECODE_P(F4, ParityTransferOrder)  // F4 - Leading / Trailing

    // MF
    else if (strcasecmp(code_field, "MF") == 0){
        code_field = strtok(NULL, "\\");
        if (0) {}                          // Keep macro logic happy
        DECODE_P(N, NumberMinorFrames)     // MF\N - Number of minor frames
    }

    DECODE_P(MF1, WordsInMinorFrame)            // MF1 - Number of word in minor frame
    DECODE_P(MF2, BitsInMinorFrame)             // MF2 - Number of bits in minor frame
    DECODE_P(MF3, MinorFrameSyncType)           // MF3 - Minor Frame Sync Type
    DECODE_P(MF4, MinorFrameSyncPatternLength)  // MF4 - Minor frame sync pattern length
    DECODE_P(MF5, MinorFrameSyncPattern)        // MF5 - Minor frame sync pattern
    DECODE_P(SYNC1, InSyncCritical)             // SYNC1 - In-sync criteria
    DECODE_P(SYNC2, InSyncErrors)               // SYNC2 - In-sync errors allowed
    DECODE_P(SYNC3, OutSyncCritical)            // SYNC3 - Out-of-sync criteria
    DECODE_P(SYNC4, OutSyncErrors)              // SYNC4 - Out-of-sync errors allowed

    // ISF - Subframe sync
    else if (strcasecmp(code_field, "ISF") == 0){
        code_field = strtok(NULL, "\\");
        if (0) {}                            // Keep macro logic happy
        DECODE_P(N, NumberSubframeCounters)  // ISF\N - Number of subframe ID counters
    }

    DECODE_P_SF(ISF1, CounterName)            // ISF1-n - Subframe ID counter name
    DECODE_P_SF(ISF2, CounterType)            // ISF2-n - Subframe sync type
    DECODE_P_SF(IDC1, WordPosition)           // IDC1-n - Minor frame word position
    DECODE_P_SF(IDC2, WordLength)             // IDC2-n - Minor frame word length
    DECODE_P_SF(IDC3, BitLocation)            // IDC3-n - Bit location
    DECODE_P_SF(IDC4, CounterLength)          // IDC4-n - Counter bit length
    DECODE_P_SF(IDC5, Endian)                 // IDC5-n - Counter endian
    DECODE_P_SF(IDC6, InitValue)              // IDC6-n - Initial value
    DECODE_P_SF(IDC7, MFForInitValue)         // IDC7-n - Initial count minor frame
    DECODE_P_SF(IDC8, EndValue)               // IDC8-n - End value
    DECODE_P_SF(IDC9, MFForEndValue)          // IDC9-n - End value minor frame
    DECODE_P_SF(IDC10, CountDirection)        // IDC10-n - Count direction

    else if (strcasecmp(code_field, "SF") == 0){
        code_field = strtok(NULL, "\\");
        if (0) {}                                  // Keep macro logic happy
        DECODE_P_SF(N, NumberSubframeDefinitions)  // SF\N-n - Number of subframes
    }

    DECODE_P_SFDEF(SF1, SubframeName)         // SF1-n-x - Subframe name
    DECODE_P_SFDEF(SF2, SuperComPosition)     // SF2-n-x - Number of supercom word positions (or NO)
    DECODE_P_SFDEF(SF3, SuperComDefined)      // SF3-n-x - How supercom word position is defined
    DECODE_P_SFDEFLOC(SF4, SubframeLocation)  // SF4-n-x-y - Subframe location
    DECODE_P_SFDEF(SF5, LocationInterval)     // SF5-n-x - Word location interval
    DECODE_P_SFDEF(SF6, SubframeDepth)        // SF6-n-x - Subframe depth (whatever that is)


    // AEF - Asynchronous embedded format
    else if (strcasecmp(code_field, "AEF") == 0){
        code_field = strtok(NULL, "\\");
        
        if (0) {}                         // Keep macro logic happy
        DECODE_P(N, NumberAsyncEmbedded)  // AEF\N - Number of async embedded frames

        // AEF\DLN-n - Data link name
        else if (strncasecmp(code_field, "DLN-", 4) == 0){
            tokens = sscanf(code_field, "%*3c-%i", &aef_index);
            if (tokens == 1){
                aef = GetP_AsyncEmbedded(p_record, aef_index, 1);
                assert(aef != NULL);
                aef->DataLinkName = (char *)TMATSMalloc(strlen(data_item) + 1);
                strcpy(aef->DataLinkName, data_item);
            }
        }
    }

    return 0;
}


P_Record * GetP_Record(P_Record **first_p_record, int index, int make_new){
    P_Record ** p_record = first_p_record;

    // Loop looking for matching index number or end of list
    while (1){
        // Check for end of list
        if (*p_record == NULL)
            break;

        // Check for matching index number
        if ((*p_record)->RecordNumber == index)
            break;

        // Move on to the next record in the list
        p_record = &((*p_record)->NextP_Record);
    }

    // If no record found then put a new one on the end of the list
    if ((*p_record == NULL) && make_new){
        // Allocate memory for the new record
        *p_record = (P_Record *)TMATSMalloc(sizeof(P_Record));
        memset(*p_record, 0, sizeof(P_Record));
        (*p_record)->RecordNumber = index;
    }

    return *p_record;
}


// Return the P record Asynchronous Embedded Format record with the given index or
// make a new one if necessary.
P_AsyncEmbedded * GetP_AsyncEmbedded(P_Record *p_record, int index, int make_new){
    P_AsyncEmbedded ** aef = &(p_record->FirstAsyncEmbedded);

    // Walk the linked list of embedded streams, looking for a match or
    // the end of the list
    while (1){
        // If record pointer in linked list is null then exit
        if (*aef == NULL)
            break;

        // If the data source number matched then record found, exit
        if ((*aef)->EmbeddedStreamNumber == index)
            break;

        // Not found but next record exists so make it our current pointer
        aef = &((*aef)->NextEmbedded);
    }

    // If no record found then put a new one on the end of the list
    if ((*aef == NULL) && make_new){
        // Allocate memory for the new record
        *aef = (P_AsyncEmbedded *)TMATSMalloc(sizeof(P_AsyncEmbedded));
        memset(*aef, 0, sizeof(P_AsyncEmbedded));
        (*aef)->EmbeddedStreamNumber = index;
    }

    return *aef;
}


// Return the P record Subframe ID record with the given index or
// make a new one if necessary.
P_SubframeID * GetP_SubframeID(P_Record *p_record, int index, int make_new){
    P_SubframeID ** subframe = &(p_record->FirstSubframeID);

    // Walk the linked list of subframe ids, looking for a match or
    // the end of the list
    while (1){
        // If record pointer in linked list is null then exit
        if (*subframe == NULL)
            break;

        // If the data source number matched then record found, exit
        if ((*subframe)->CounterNumber == index)
            break;

        // Not found but next record exists so make it our current pointer
        subframe = &((*subframe)->NextSubframeID);
    }

    // If no record found then put a new one on the end of the list
    if ((*subframe == NULL) && make_new){
        // Allocate memory for the new record
        *subframe = (P_SubframeID *)TMATSMalloc(sizeof(P_SubframeID));
        memset(*subframe, 0, sizeof(P_SubframeID));
        (*subframe)->CounterNumber = index;
    }

    return *subframe;
}


// Return the P record Subframe ID record with the given index or
// make a new one if necessary.
P_SubframeDefinition * GetP_SubframeDefinition(P_SubframeID * subframe_id, int index, int make_new){
    P_SubframeDefinition ** subframe_definition = &(subframe_id->FirstSubframeDefinition);

    // Walk the linked list of subframe defs, looking for a match or
    // the end of the list
    while (1){
        // If record pointer in linked list is null then exit
        if (*subframe_definition == NULL)
            break;

        // If the data source number matched then record found, exit
        if ((*subframe_definition)->SubframeDefinitionNumber == index)
            break;

        // Not found but next record exists so make it our current pointer
        subframe_definition = &((*subframe_definition)->NextSubframeDefinition);
    }

    // If no record found then put a new one on the end of the list
    if ((*subframe_definition == NULL) && make_new){
        // Allocate memory for the new record
        *subframe_definition = (P_SubframeDefinition *)TMATSMalloc(sizeof(P_SubframeDefinition));
        memset(*subframe_definition, 0, sizeof(P_SubframeDefinition));
        (*subframe_definition)->SubframeDefinitionNumber = index;
    }

    return *subframe_definition;
}


P_SubframeLocation * GetP_SubframeLocation(P_SubframeDefinition *subframe_definition,
        int index, int make_new){
    
    P_SubframeLocation ** subframe_location = &(subframe_definition->FirstSubframeLocation);

    // Walk the linked list of subframe locations, looking for a match or
    // the end of the list
    while (1){
        // If record pointer in linked list is null then exit
        if (*subframe_location == NULL)
            break;

        // If the data source number matched then record found, exit
        if ((*subframe_location)->SubframeLocationNumber == index)
            break;

        // Not found but next record exists so make it our current pointer
        subframe_location = &((*subframe_location)->NextSubframeLocation);
    }

    // If no record found then put a new one on the end of the list
    if ((*subframe_location == NULL) && make_new){
        // Allocate memory for the new record
        *subframe_location = (P_SubframeLocation *)TMATSMalloc(sizeof(P_SubframeLocation));
        memset(*subframe_location, 0, sizeof(P_SubframeLocation));
        (*subframe_location)->SubframeLocationNumber = index;
    }

    return *subframe_location;
}
    
    
/*
    Tie the P record asynchronous embedded format field to the definition
    of the embedded stream P record.

    P-x\AEF\DLN-n <---> P-x\DLN
*/
void ConnectP(TMATS_Info * tmats_info){
    P_Record           * p_record;
    P_Record           * p_embedded;
    P_AsyncEmbedded    * aef;

    // Walk the linked list of P records
    p_record = tmats_info->FirstP_Record;
    while (p_record != NULL){

        // Walk the list of P embedded stream records
        aef = p_record->FirstAsyncEmbedded;
        while (aef != NULL){

            // Walk the linked list of P records to find matching async embedded stream
            p_embedded = tmats_info->FirstP_Record;
            while (p_embedded != NULL){

                // See if P-x\AEF\DLN-n = P-x\DLN
                if ((p_embedded->DataLinkName != NULL) &&
                        (aef->DataLinkName != NULL) &&
                        (strcasecmp(p_embedded->DataLinkName,
                                    aef->DataLinkName) == 0)){
                    aef->P_Record = p_embedded;
                }

                // Get the next embedded P record
                p_embedded = p_embedded->NextP_Record;
            }

            // Get the next P AEF record
            aef = aef->NextEmbedded;
        }

        // Get the next P record
        p_record = p_record->NextP_Record;
    }
}


// The I106_Decode_Tmats() procedure malloc()'s a lot of memory.  This
// procedure walks the SuMemBlock list, freeing memory as it goes.
void I106_Free_TMATS_Info(TMATS_Info *tmats_info){
    MemoryBlock     * block;
    MemoryBlock     * next;

    if (tmats_info == NULL)
        return;

    // Walk the linked memory list, freely freeing as we head down the freeway
    block = tmats_info->FirstMemoryBlock;
    while (block != NULL){
        // Free the memory
        free(block->MemoryBlock);

        // Free the memory block and move to the next one
        next = block->NextMemoryBlock;
        free(block);
        block = next;
    }

    // Initialize the TMATS info data structure
    memset(tmats_info, 0, sizeof(TMATS_Info));
}


// Allocate memory but keep track of it for I106_Free_TMATSInfo() later.
void * TMATSMalloc(size_t size){
    void          * buffer;
    MemoryBlock  ** block;

    // Malloc the new memory
    buffer = malloc(size);
    assert(buffer != NULL);

    // Walk to (and point to) the last linked memory block
    block = &module_tmats_info->FirstMemoryBlock;
    while (*block != NULL)
        block = &(*block)->NextMemoryBlock;
        
    // Populate the memory block struct
    *block = (MemoryBlock *)malloc(sizeof(MemoryBlock));
    assert(*block != NULL);
    (*block)->MemoryBlock     = buffer;
    (*block)->NextMemoryBlock = NULL;

    return buffer;
}


/* Write procedures */

I106Status I106_Encode_TMATS(I106C10Header *header, void *buffer, char *tmats){

    // Channel specific data word
    *(uint32_t *)buffer = 0;

    // Figure out the total TMATS message length
    header->DataLength = strlen(tmats) + 4;

    // Copy TMATS setup info to buffer.  This assumes there is enough
    // space in the buffer to hold the TMATS string.
    strcpy((char *)buffer + 4, tmats);

    // Make the data buffer checksum and update the header
    AddFillerAndChecksum(header, (unsigned char *)buffer);

    return I106_OK;
}


// A little routine to return the first non-white space character in string
char * FirstNonWhitespace(char * string){
    char * first_char = string;
    while (isspace(*first_char) && (*first_char != '\0'))
        first_char++;
    return first_char;
}


// Calculate a "fingerprint" checksum code from TMATS info
// Do not include CSDW!!!
I106Status I106_TMATS_Signature(void      *raw_buffer,         // TMATS text without CSDW
                                uint32_t   data_length,        // Length of TMATS in raw_buffer
                                int        signature_version,  // Request signature version (0 = default)
                                int        signature_flags,    // Additional flags
                                uint16_t  *opcode,             // Version and flag op code
                                uint32_t  *signature){         // TMATS signature

    unsigned long       buffer_index;
    char              * buffer;
    char                line[2048];
    char                line_upper[2048];
    int                 line_index;
    int                 copy_index;
    char              * code_name;
    char              * data_item;
    char              * code;
    char              * section;

    // Check the requested signature version
    if (signature_version == 0)
        signature_version = TMATS_SIGVER_DEFAULT;

    if (signature_version > 1)
        return I106_INVALID_PARAMETER;

    // Init buffer pointers
    buffer = (char *)raw_buffer;
    buffer_index = 0;

    *signature = 0;

    // Loop until we get to the end of the buffer
    while (1){

        // If at the end of the buffer then break out of the big loop
        if (buffer_index >= data_length)
            break;

        // Initialize input line buffer
        line[0] = '\0';
        line_index  = 0;

        // Read from buffer until complete line
        while (1){
            // If at the end of the buffer then break out
            if (buffer_index >= data_length)
                break;

            // If nonprintable then swallow them, they mean nothing to TMATS
            // Else copy next character to line buffer
            if (isprint((unsigned char)buffer[buffer_index])){
                line[line_index] = buffer[buffer_index];
                if (line_index < 2048)
                  line_index++;
                line[line_index] = '\0';
            }

            // Next character from buffer
            buffer_index++;

            // If line terminator and line buffer not empty then break out
            if (buffer[buffer_index - 1] == ';'){
                if (strlen(line) != 0)
                    break;
            }
        }


        // If not code name then break out
        if (line[0] == '\0')
            continue;

        // Make an upper case copy
        copy_index = 0;
        while (1){
            if (islower(line[copy_index]))
                line_upper[copy_index] = toupper(line[copy_index]);
            else
                line_upper[copy_index] = line[copy_index];
            if (line[copy_index] == '\0')
                break;
            copy_index++;
        }
        
        code_name = strtok(line_upper, ":");
        data_item = strtok(NULL, ";");

        // If not code name then break out
        if ((code_name == NULL) || (data_item == NULL))
            continue;

        // Test for COMMENT field
        if (((signature_flags & TMATS_SIGFLAG_INC_COMMENT) != TMATS_SIGFLAG_INC_COMMENT) &&
                ((signature_flags & TMATS_SIGFLAG_INC_ALL    ) != TMATS_SIGFLAG_INC_ALL    ) && 
                (strcmp(code_name, "COMMENT") == 0))
            continue;

        section = strtok(code_name, "\\");
        code    = strtok(NULL, ":");

        // Comment fields
        if (((signature_flags & TMATS_SIGFLAG_INC_COMMENT) != TMATS_SIGFLAG_INC_COMMENT) &&
                ((signature_flags & TMATS_SIGFLAG_INC_ALL    ) != TMATS_SIGFLAG_INC_ALL    ) && 
                (strcmp(code, "COM") == 0))
            continue;

        // Ignored R sections
        if (((signature_flags & TMATS_SIGFLAG_INC_ALL) != TMATS_SIGFLAG_INC_ALL) && 
                (section[0] == 'R')){
            if ((strcmp (code, "RI1"      ) == 0) ||
                (strcmp (code, "RI2"      ) == 0) ||
                (strcmp (code, "RI3"      ) == 0) ||
                (strcmp (code, "RI4"      ) == 0) ||
                (strcmp (code, "RI5"      ) == 0) ||
                (strcmp (code, "RI6"      ) == 0) ||
                (strcmp (code, "RI7"      ) == 0) ||
                (strcmp (code, "RI8"      ) == 0) ||
                (strcmp (code, "RI9"      ) == 0) ||
                (strcmp (code, "RI10"     ) == 0) ||
                (strcmp (code, "DPOC1"    ) == 0) ||
                (strcmp (code, "DPOC2"    ) == 0) ||
                (strcmp (code, "DPOC3"    ) == 0) ||
                (strcmp (code, "MPOC4"    ) == 0) ||
                (strcmp (code, "MPOC1"    ) == 0) ||
                (strcmp (code, "MPOC2"    ) == 0) ||
                (strcmp (code, "MPOC3"    ) == 0) ||
                (strcmp (code, "MPOC4"    ) == 0) ||
                (strcmp (code, "RIM\\N"   ) == 0) ||
                (strncmp(code, "RIMI-",  5) == 0) ||
                (strncmp(code, "RIMS-",  5) == 0) ||
                (strncmp(code, "RIMF-",  5) == 0) ||
                (strcmp (code, "RMM\\N"   ) == 0) ||
                (strncmp(code, "RMMID-", 6) == 0) ||
                (strncmp(code, "RMMS-",  5) == 0) ||
                (strncmp(code, "RMMF-",  5) == 0))
                continue;
            }

        // Vendor fields
        if (((signature_flags & TMATS_SIGFLAG_INC_VENDOR) != TMATS_SIGFLAG_INC_VENDOR) && 
                ((signature_flags & TMATS_SIGFLAG_INC_ALL   ) != TMATS_SIGFLAG_INC_ALL   ) && 
                (section[0] == 'V'))
            continue;
                
        // G fields
        if (((signature_flags & TMATS_SIGFLAG_INC_G     ) != TMATS_SIGFLAG_INC_G     ) && 
                ((signature_flags & TMATS_SIGFLAG_INC_ALL   ) != TMATS_SIGFLAG_INC_ALL   ) && 
                (section[0] == 'G'))
            continue;

        // Make another upper case copy
        copy_index = 0;
        while (1){
            if (islower(line[copy_index]))
                line_upper[copy_index] = toupper(line[copy_index]);
            else
                line_upper[copy_index] = line[copy_index];
            if (line[copy_index] == '\0')
                break;
            copy_index++;
        }

        *signature += Fletcher32((uint8_t *)line_upper, strlen(line_upper));
    }
    
    // Everything seems OK so make the op code
    *opcode = ((signature_flags & 0x000F) << 4) | (signature_version & 0x000F);

    return I106_OK;
}


// http://en.wikipedia.org/wiki/Fletcher%27s_checksum
uint32_t Fletcher32(uint8_t *data, int count){
    uint32_t    addend;
    uint32_t    sum1 = 0;
    uint32_t    sum2 = 0;
    int         index;
 
    index = 0;
    while (index < count){
        addend  = data[index++];
        if (index < count)
            addend |= data[index++] << 8;
        sum1 = (sum1 + addend) % 0xffff;
        sum2 = (sum2 + sum1) % 0xffff;
    }
 
    return (sum2 << 16) | sum1;
}
