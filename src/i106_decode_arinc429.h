/****************************************************************************

 i106_decode_arinc429.h
 Created by Bob Baggerman

 ****************************************************************************/

#ifndef _I106_DECODE_ARINC429_H
#define _I106_DECODE_ARINC429_H

#include "irig106ch10.h"


/* Data structures */

#if defined(_MSC_VER)
#pragma pack(push,1)
#endif

// Channel specific data word
typedef struct Arinc429F0_CSDW Arinc429F0_CSDW;
struct Arinc429F0_CSDW {
    uint32_t    Count     : 16;      // Message count
    uint32_t    Reserved  : 16;
} PACKED;

// Intra-message header
typedef struct Arinc429F0_IPH Arinc429F0_IPH;
struct Arinc429F0_IPH {
    uint32_t    GapTime        : 20;
    uint32_t    Reserved       :  1; 
    uint32_t    BusSpeed       :  1;
    uint32_t    ParityError    :  1;
    uint32_t    FormatError    :  1;
    uint32_t    BusNumber      :  8;
} PACKED;


// ARINC 429 data format
typedef struct Arinc429F0_Data Arinc429F0_Data;
struct Arinc429F0_Data {
    uint32_t    Label          :  8;
    uint32_t    SDI            :  2;      // Source/Destination Identifiers
    uint32_t    Data           : 19;
    uint32_t    SSM            :  2;      // Sign/Status Matrix
    uint32_t    Parity         :  1;
} PACKED;


// Current ARINC 429 message
typedef struct {
    unsigned int       MessageNumber;
    uint32_t           Offset;
    Arinc429F0_CSDW  * CSDW;
    int64_t            IPTS;
    Arinc429F0_IPH   * IPH;
    Arinc429F0_Data  * Data;
} PACKED Arinc429F0_Message;


#if defined(_MSC_VER)
#pragma pack(pop)
#endif


/* Function Declaration */
I106Status I106_Decode_FirstArinc429F0(I106C10Header *header, void *buffer, Arinc429F0_Message *msg);
I106Status I106_Decode_NextArinc429F0(Arinc429F0_Message *msg);

#endif
