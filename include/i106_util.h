

#ifndef _I106_UTIL_H
#define _I106_UTIL_H

#include "libirig106.h"

#include "i106_decode_tmats.h"
#include "i106_decode_time.h"
#include "i106_decode_index.h"

I106Status SwapBytes(uint8_t *buffer, long bytes);
I106Status SwapShortWords(uint16_t *buffer, long bytes);

// Utilities
int HeaderInit(I106C10Header *header, unsigned int channel_id,
    unsigned int data_type, unsigned int flags, unsigned int sequence_number);
int GetHeaderLength(I106C10Header *header);
uint32_t GetDataLength(I106C10Header *header);
uint16_t HeaderChecksum(I106C10Header *header);
uint16_t SecondaryHeaderChecksum(I106C10Header *header);
char * I106ErrorString(I106Status status);
/* int DataChecksum(void *buffer); */
I106Status AddFillerAndChecksum(I106C10Header *header, unsigned char data[]);


/* From old i106_index */

/* Data structures */
typedef struct{
    uint16_t  ChannelID;
    uint8_t   DataType;
    int64_t   RTC;
    I106Time  IrigTime;
    int64_t   Offset;
} PacketIndexInfo;

/* Function Declarations */

void InitIndex(int handle);
I106Status IndexPresent(const int handle, int * found_index);
I106Status ReadIndexes(const int handle);
I106Status MakeIndex(const int handle, uint16_t channel_id);

#endif
