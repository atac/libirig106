
#include "util.h"


// Swaps "bytes" bytes in place
I106Status SwapBytes(uint8_t *buffer, long bytes){
    uint32_t data = 0x03020100;
    uint8_t tmp;

    if (bytes & 1)
        return I106_BUFFER_OVERRUN; // May be also an underrun ...

    while ((bytes -= 2) >= 0){
        tmp = *buffer;
        *buffer = *(buffer + 1);
        *++buffer = tmp;
        buffer++;
    }

    SwapShortWords((uint16_t *)&data, 4);

    return I106_OK;
}


// Swaps "bytes" bytes of 16 bit words in place
I106Status SwapShortWords(uint16_t *buffer, long bytes){
    uint16_t tmp;

    if (bytes & 3)
        return I106_BUFFER_OVERRUN; // May be also an underrun ...

    bytes >>= 1;
    while ((bytes -= 2) >= 0){
        tmp = *buffer;
        *buffer = *(buffer + 1);
        *++buffer = tmp;
        buffer++;
    }

    return I106_OK;
}
