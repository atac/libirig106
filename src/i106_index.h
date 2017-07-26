/****************************************************************************

 i106_index.h

 ****************************************************************************/

#ifndef _I106_INDEX_H
#define _I106_INDEX_H

#include "irig106ch10.h"
#include "i106_time.h"


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
