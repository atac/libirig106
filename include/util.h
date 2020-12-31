

#ifndef _I106_UTIL_H
#define _I106_UTIL_H

#include "libirig106.h"

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

#endif
