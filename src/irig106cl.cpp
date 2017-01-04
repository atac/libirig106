/****************************************************************************

  irig106cl.cpp - A class that implements the IRIG 106 library

 ****************************************************************************/

#include <stdlib.h>   // For _MAX_PATH definition
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <time.h>

#include "stdint.h"

#include "irig106cl.h"
//#pragma make_public(SuI106Ch10Header)

//using namespace Irig106;

// Drag this stuff in if compiled in .NET environment
#if defined(_M_CEE)
//using namespace System;
//  using namespace System::ComponentModel;
//  using namespace System::Collections;
//  using namespace System::IO;
using namespace System::Text;
using namespace System::Runtime::InteropServices;
#endif

namespace Irig106
{

//=========================================================================

// Constructor / destructor
Irig106Lib::Irig106Lib(void)
    {
    this->pHeader               = new SuI106Ch10Header;
    this->pDataBuff             = NULL;
    this->ulBuffSize            = 0;
    this->bManageDataBuffMalloc = false;

    // Initialize the TMATS info data structure
    memset(&suTmatsInfo, 0, sizeof(SuTmatsInfo));

    // Someday we may get "smart" about these message buffer, and only
    // allocate memory for them if and when they are needed.  For now
    // go ahead and allocate memory for them once at the beginning.
    this->psu1553CurrMsg        = new Su1553F1_CurrMsg;
    this->psuDiscreteCurrMsg    = new SuDiscreteF1_CurrMsg;
    this->psuUartCurrMsg        = new SuUartF0_CurrMsg;
    this->psuTimeRef            = new SuTimeRef;

    }

Irig106Lib::~Irig106Lib(void)
    {
//    Close();
    delete this->pHeader;

    // Only free the data buffer pointer if we are managing it
    if (this->bManageDataBuffMalloc)
        free(this->pDataBuff);

    this->ulBuffSize = 0;

    enI106_Free_TmatsInfo(&suTmatsInfo);

    delete this->psu1553CurrMsg;
    delete this->psuDiscreteCurrMsg;
    delete this->psuUartCurrMsg;
    delete this->psuTimeRef;
    }


//=========================================================================
// irig106ch10

// Open() with C string name
EnI106Status Irig106Lib::Open(char * szFilename, EnI106Ch10Mode enMode)
    {
    EnI106Status    enStatus;

    // Open the data file
    enStatus = enI106Ch10Open(&iHandle, (char *)szFilename, enMode);

    return enStatus;
    }



//  Open() with .NET string name
#if defined(_M_CEE)
EnI106Status Irig106Lib::Open(String ^ sFilename, EnI106Ch10Mode enMode)
    {
    EnI106Status    enStatus;
    const char    * szFilename;

    // Convert the filename into a good ol' C string, getting rid of that Unicode junk
    szFilename = (const char *)
        (Marshal::StringToHGlobalAnsi(sFilename)).ToPointer();

    // HMMM... SINCE THIS CLASS IS NOW UNMANAGED, PIN_PTR PROBABLY ISN'T
    // NECESSARY, BUT I'LL PLAY WITH IT LATER. FOR NOW IT WORKS SO I'LL LEAVE
    // IT ALONE.  DOTNET IS SUCH A JOY.
    // Make a pointer to the handle
    pin_ptr<int>piHandle = &iHandle;

    // Open the data file
    enStatus = enI106Ch10Open(piHandle, (char *)szFilename, enMode);

    // Free up the filename storage space
    Marshal::FreeHGlobal(IntPtr((void*)szFilename));

    return enStatus;
    }
#endif

//-------------------------------------------------------------------------

//  Close
EnI106Status Irig106Lib::Close(void)
    {
    EnI106Status    enStatus;
    enStatus = enI106Ch10Close(iHandle);
    return enStatus;
    }

//-------------------------------------------------------------------------

// Read / Write
EnI106Status Irig106Lib::ReadNextHeader()
    {
    EnI106Status    enStatus;
    enStatus = enI106Ch10ReadNextHeader(iHandle, this->pHeader);
    return enStatus;
    }


//-------------------------------------------------------------------------

EnI106Status Irig106Lib::ReadPrevHeader()
    {
    EnI106Status    enStatus;
    enStatus = enI106Ch10ReadPrevHeader(iHandle, this->pHeader);
    return enStatus;
    }


//-------------------------------------------------------------------------

EnI106Status Irig106Lib::ReadData()
    {
    EnI106Status    enStatus;

    // Make sure the buffer is big enough
    if (this->ulBuffSize < this->pHeader->ulPacketLen)
        {
        this->pDataBuff = realloc(this->pDataBuff, this->pHeader->ulPacketLen);
        this->ulBuffSize = this->pHeader->ulPacketLen;
        this->bManageDataBuffMalloc = false;
        }
    enStatus = enI106Ch10ReadData(iHandle, this->ulBuffSize, this->pDataBuff);

    return enStatus;
    }



//-------------------------------------------------------------------------

#if defined(_M_CEE)
EnI106Status Irig106Lib::GetPos(int64_t % mpllOffset)
    {
    EnI106Status    enStatus;
    int64_t         llOffset;

    enStatus = enI106Ch10GetPos(this->iHandle, &llOffset);
    mpllOffset = llOffset;
    return enStatus;

    }

#endif
//-------------------------------------------------------------------------

/*
    //// Utilities
    //EnI106Status iHeaderInit(SuI106Ch10Header * psuHeader,
    //        unsigned int       uChanID,
    //        unsigned int       uDataType,
    //        unsigned int       uFlags,
    //        unsigned int       uSeqNum);
    //int      iGetHeaderLen(SuI106Ch10Header * psuHeader);
    //int      iGetDataLen(SuI106Ch10Header * psuHeader);
    //uint16_t uCalcHeaderChecksum(SuI106Ch10Header * psuHeader);
    //uint16_t uCalcSecHeaderChecksum(SuI106Ch10Header * psuHeader);
    //uint32_t uCalcDataBuffReqSize(uint32_t uDataLen, int iChecksumType);
    //EnI106Status uAddDataFillerChecksum(SuI106Ch10Header * psuI106Hdr, unsigned char achData[]);
*/




//=========================================================================
// i106_time

#if defined(_M_CEE)
String ^ Irig106Lib::strTime2String(SuIrig106Time * psuTime)
    {
    char *      szTime;

    szTime = IrigTime2String(psuTime);
    return Marshal::PtrToStringAnsi(System::IntPtr(szTime));
    }



System::Void Irig106Lib::TimeArray2LLInt(int64_t % mpllRelTime)
    {
    int64_t         llRelTime;

    vTimeArray2LLInt(this->pHeader->aubyRefTime, &llRelTime);
    mpllRelTime = llRelTime;

    return;

    }
#endif



//=========================================================================
// i106_decode_tmats

EnI106Status Irig106Lib::Decode_Tmats()
    {
    EnI106Status    enStatus;

    // Check to make sure we have a TMATS record in the buffer
    if (this->pHeader->ubyDataType != I106CH10_DTYPE_TMATS)
        return I106_READ_ERROR;

    // We've got a TMATS so decode it
    enStatus = enI106_Decode_Tmats(this->pHeader, this->pDataBuff, &this->suTmatsInfo);
    return enStatus;
    }

} // end namespace Irig106
