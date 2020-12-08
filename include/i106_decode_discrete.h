/****************************************************************************

 i106_decode_discrete.h

 ****************************************************************************/

#ifndef _I106_DECODE_DISCRETE_H
#define _I106_DECODE_DISCRETE_H

#include "config.h"
#include "irig106ch10.h"
#include "i106_time.h"


/* Macros and definitions */
#define I106CH10_NUM_DISCRETE_INPUTS_PER_STATE  (uint16_t)32


/* Data structures */

/* Discrete Format 1 */

#pragma pack(push, 1)

// Channel specific header
typedef struct {
    uint32_t    Reserved1  :  24;
    uint32_t    Length     :  5;      // Number of bits in the event
    uint32_t    Reserved2  :  1;
    uint32_t    Alignment  :  1;      // 0 = lsb, 1 = msb
    uint32_t    State      :  1;      // 0 = date recorded on change, 1 = recorded at time interval
} DiscreteF1_CSDW;

// Current discrete message
typedef struct {
    unsigned int       BytesRead;
    DiscreteF1_CSDW  * CSDW;
    IntraPacketTS    * IPTS;
    uint32_t           Data;
} DiscreteF1_Message;

#pragma pack(pop)


/* Function Declaration */
I106Status I106_Decode_FirstDiscreteF1(I106C10Header *header, void *buffer, DiscreteF1_Message *msg, TimeRef *time);
I106Status I106_Decode_NextDiscreteF1(I106C10Header *header, DiscreteF1_Message *msg, TimeRef *time);

#endif
