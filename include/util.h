

#ifndef _I106_UTIL_H
#define _I106_UTIL_H

#include "irig106ch10.h"
#include "int.h"

I106Status SwapBytes(uint8_t *buffer, long bytes);
I106Status SwapShortWords(uint16_t *buffer, long bytes);

#endif
