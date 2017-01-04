/****************************************************************************

 i106_index.h - 

 ****************************************************************************/

#ifndef _I106_INDEX_H
#define _I106_INDEX_H

#ifdef __cplusplus
namespace Irig106 {
extern "C" {
#endif

/*
 * Macros and definitions
 * ----------------------
 */


/*
 * Data structures
 * ---------------
 */

typedef struct
    {
    uint16_t        uChID;              ///< Channel ID
    uint8_t         ubyDataType;        ///< Data type
    int64_t         lRelTime;           ///< 48 bit relative time
    SuIrig106Time   suIrigTime;         ///< Absolute time
    int64_t         lFileOffset;        ///< File offset to packet
    } SuPacketIndexInfo;


/*
 * Global data
 * -----------
 */


/*
 * Function Declaration
 * --------------------
 */

void InitIndex(int iHandle);

EnI106Status I106_CALL_DECL enIndexPresent(const int iHandle, int * bFoundIndex);

EnI106Status I106_CALL_DECL enReadIndexes(const int iHandle);

EnI106Status I106_CALL_DECL enMakeIndex(const int iHandle, uint16_t uChID);

//EnI106Status I106_CALL_DECL SaveIndexTable(char* strFileName);

#ifdef __cplusplus
}
}
#endif

#endif

