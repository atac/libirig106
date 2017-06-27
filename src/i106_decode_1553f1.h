/****************************************************************************

 i106_decode_1553f1.h

 ****************************************************************************/

#ifndef _I106_DECODE_1553F1_H
#define _I106_DECODE_1553F1_H

#include "config.h"
#include "irig106ch10.h"


/* Data structures */

#if defined(_MSC_VER)
#pragma pack(push)
#pragma pack(1)
#endif


// 1553 Command Word bit fields
typedef struct {
    uint16_t    WordCount   : 5;    // Data Word Count or Mode Code
    uint16_t    SubAddress  : 5;    // Subaddress Specifier
    uint16_t    TR          : 1;    // Transmit/Receive Flag
    uint16_t    RT          : 5;    // RT Address
} PACKED CommandWord;

// A union to make manipulating the command word easier
typedef union {
    CommandWord  CommandWord;
    uint16_t     Raw;
} CommandWordUnion;


/* 1553 Format 1 */

// Channel specific header
typedef struct {
    uint32_t    MessageCount  : 24;
    uint32_t    Reserved      :  6;
    uint32_t    TTB           :  2;  // Time tag bits
} PACKED MS1553F1_CSDW;

// Intra-message header
typedef struct {
    uint8_t     Time[8];              // Reference time
    uint16_t    Reserved1       : 3;
    uint16_t    WordError       : 1;
    uint16_t    SyncError       : 1;
    uint16_t    WordCountError  : 1;
    uint16_t    Reserved2       : 3;
    uint16_t    Timeout         : 1;
    uint16_t    FormatError     : 1;
    uint16_t    RT2RT           : 1;
    uint16_t    MessageError    : 1;
    uint16_t    BusID           : 1;
    uint16_t    Reserved3       : 2;
    uint8_t     GapTime1;
    uint8_t     GapTime2;
    uint16_t    Length;
} PACKED MS1553F1_IPH;

// 1553 message
typedef struct {
    unsigned int       MessageNumber;
    uint32_t            Offset;   // Offset into data buffer
    uint32_t            DataLength;
    MS1553F1_CSDW     * CSDW;
    MS1553F1_IPH      * IPH;
    CommandWordUnion  * CommandWord1;
    CommandWordUnion  * CommandWord2;
    uint16_t          * StatusWord1;
    uint16_t          * StatusWord2;
    uint16_t            WordCount;
    uint16_t          * Data;
} PACKED MS1553F1_Message;


/* 1553 Format 2 */

// Channel specific header
typedef struct {
    uint32_t  MessageCount;
} PACKED MS1553F2_CSDW;

// 16PP194 Intra-message header
typedef struct {
    uint8_t     Time[8];
    uint16_t    Reserved1      : 3;
    uint16_t    EchoError      : 1;
    uint16_t    Reserved2      : 2;
    uint16_t    StatusError    : 1;
    uint16_t    Reserved3      : 6;
    uint16_t    Timeout        : 1;
    uint16_t    Reset          : 1;
    uint16_t    TransmitError  : 1;
    uint16_t    Length;
} PACKED MS1553F2_IPH;

// 16PP194 word
typedef struct {
    uint32_t    DataWord       : 16;   // Data word contents
    uint32_t    SubAddress     : 4;    // Parity error flag
    uint32_t    Address        : 4;    // Parity error flag
    uint32_t    ParityError    : 1;    // Parity error flag
    uint32_t    WordError      : 1;    // Manchester error flag
    uint32_t    Gap            : 3;    // Gap time indicator
    uint32_t    BusID          : 3;    // Bus ID indicator
} PACKED _16PP194_Word;

// 16PP194 transaction
typedef struct {
    _16PP194_Word  Command;
    _16PP194_Word  Response;
    _16PP194_Word  CommandEcho;
    _16PP194_Word  NoGo;
    _16PP194_Word  NoGoEcho;
    _16PP194_Word  Status;
} PACKED _16PP194_Transaction;

#if defined(_MSC_VER)
#pragma pack(pop)
#endif


// Function Declaration
I106Status I106_Decode_First1553F1(I106C10Header *header, void *buffer, MS1553F1_Message *msg);
I106Status I106_Decode_Next1553F1(MS1553F1_Message *msg);
int MS1553WordCount(const CommandWordUnion *command_word);
char * GetCommandWord(unsigned int raw);

#endif
