/****************************************************************************

 irig106_dotnet.cpp - A .NET assembly wrapper for the IRIG 106 DLL

 Copyright (c) 2010 Irig106.org

 All rights reserved.

 Redistribution and use in source and binary forms, with or without 
 modification, are permitted provided that the following conditions are 
 met:

   * Redistributions of source code must retain the above copyright 
     notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above copyright 
     notice, this list of conditions and the following disclaimer in the 
     documentation and/or other materials provided with the distribution.

   * Neither the name Irig106.org nor the names of its contributors may 
     be used to endorse or promote products derived from this software 
     without specific prior written permission.

 This software is provided by the copyright holders and contributors 
 "as is" and any express or implied warranties, including, but not 
 limited to, the implied warranties of merchantability and fitness for 
 a particular purpose are disclaimed. In no event shall the copyright 
 owner or contributors be liable for any direct, indirect, incidental, 
 special, exemplary, or consequential damages (including, but not 
 limited to, procurement of substitute goods or services; loss of use, 
 data, or profits; or business interruption) however caused and on any 
 theory of liability, whether in contract, strict liability, or tort 
 (including negligence or otherwise) arising in any way out of the use 
 of this software, even if advised of the possibility of such damage.

 ****************************************************************************/

#include "stdafx.h"

#include "irig106_dotnet.h"
#include "irig106_dotnet_tmats.h"

//#include "stdint.h"
#include "irig106ch10.h"

using namespace System;
using namespace System::Runtime::InteropServices;

//=========================================================================

namespace Irig106DotNet
    {

    namespace DLL
        {
        // Main 
        [DllImport("irig106.dll", CharSet=CharSet::Ansi, EntryPoint="enI106Ch10Open")]
            extern "C" ReturnStatus I106Ch10Open(int ^, String ^, Ch10FileMode);

        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" ReturnStatus enI106Ch10Close(int);

        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" ReturnStatus enI106Ch10ReadNextHeader(int, Irig106DotNet::SuI106Ch10Header ^);

        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" ReturnStatus enI106Ch10ReadPrevHeader(int, Irig106DotNet::SuI106Ch10Header ^);

        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" ReturnStatus enI106Ch10ReadData(int, unsigned long, void *);

        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" ReturnStatus enI106Ch10WriteMsg(int, Irig106DotNet::SuI106Ch10Header ^, void *);

        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" ReturnStatus enI106Ch10FirstMsg(int); 

        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" ReturnStatus enI106Ch10LastMsg(int);

        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" ReturnStatus enI106Ch10SetPos(int, __int64);

        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" ReturnStatus enI106Ch10GetPos(int, __int64 %);

        // Time
        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" ReturnStatus enI106_SetRelTime(
                            int                      iI106Ch10Handle,
                            IrigTime               ^ psuTime,
                            uint8_t                * abyRelTime);

        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" ReturnStatus enI106_Rel2IrigTime(
                            int                      iI106Ch10Handle,
                            uint8_t                * abyRelTime,
                            IrigTime               ^ psuTime);

        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" ReturnStatus enI106_RelInt2IrigTime(
                            int                      iI106Ch10Handle,
                            __int64                  llRelTime,
                            IrigTime               ^ psuTime);

        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" ReturnStatus enI106_Irig2RelTime(
                            int                      iI106Ch10Handle,
                            IrigTime               ^ psuTime,
//                          array<unsigned __int8> ^ abyRelTime);
                            uint8_t                * abyRelTime);


#if 0
        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" ReturnStatus enI106_Ch4Binary2IrigTime(
                            SuI106Ch4_Binary_Time * psuCh4BinaryTime,
                            SuIrig106Time         * psuIrig106Time);

        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" ReturnStatus enI106_IEEE15882IrigTime(
                            SuIEEE1588_Time       * psuCh4BinaryTime,
                            SuIrig106Time         * psuIrig106Time);

        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" ReturnStatus vFillInTimeStruct(
                            SuI106Ch10Header      * psuHeader,
                            SuIntraPacketTS       * psuIntraPacketTS, 
                            SuTimeRef             * psuTimeRef);
#endif

        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" void vLLInt2TimeArray(
                            __int64                ^ pllRelTime,
                            array<unsigned __int8> ^ abyRelTime);

        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" void vTimeArray2LLInt(
                            array<unsigned __int8> ^ abyRelTime,
                            __int64                ^ pllRelTime);

        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" ReturnStatus enI106_SyncTime(
                            int                     iI106Ch10Handle,
                            bool                    bRequireSync,
                            int                     iTimeLimit);

#if 0
        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            enI106Ch10SetPosToIrigTime(
                            int                     iI106Ch10Handle, 
                            SuIrig106Time         * psuSeekTime);
#endif

// General purpose time utilities
// ------------------------------

// Convert IRIG time into an appropriate string
        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" char * IrigTime2String(IrigTime ^ psuTime);

// IT WOULD BE NICE TO HAVE SOME FUNCTIONS TO COMPARE 6 BYTE
// TIME ARRAY VALUES FOR EQUALITY AND INEQUALITY

// This is handy enough that we'll go ahead and export it to the world
// HMMM... MAYBE A BETTER WAY TO DO THIS IS TO MAKE THE TIME VARIABLES
// AND STRUCTURES THOSE DEFINED IN THIS PACKAGE.
// time_t I106_CALL_DECL mkgmtime(struct tm * psuTmTime);


        } // end namespace DLL

    } // end namespace Irig106DLL

//=========================================================================

namespace Irig106DotNet
{

// Constructor / destructor
Irig106Lib::Irig106Lib(void)
    {

    this->Header                = gcnew Irig106DotNet::SuI106Ch10Header;
    this->DataBuff              = gcnew array<SByte>(100000);

/*
    // Initialize the TMATS info data structure
    memset(&suTmatsInfo, 0, sizeof(SuTmatsInfo));

    // Someday we may get "smart" about these message buffer, and only
    // allocate memory for them if and when they are needed.  For now
    // go ahead and allocate memory for them once at the beginning.
    this->psu1553CurrMsg        = new Su1553F1_CurrMsg;
    this->psuDiscreteCurrMsg    = new SuDiscreteF1_CurrMsg;
    this->psuUartCurrMsg        = new SuUartF0_CurrMsg;
    this->psuTimeRef            = new SuTimeRef;
*/
    } // end constructor



Irig106Lib::~Irig106Lib(void)
    {
/*
//    Close();

    enI106_Free_TmatsInfo(&suTmatsInfo);

    delete this->psu1553CurrMsg;
    delete this->psuDiscreteCurrMsg;
    delete this->psuUartCurrMsg;
    delete this->psuTimeRef;
*/
    } // end destructor

//=========================================================================
// irig106ch10

// Open a Ch 10 file for reading or writing
ReturnStatus Irig106Lib::Open(String ^ sFilename, Ch10FileMode enMode)
    {
    ReturnStatus    enStatus;

    enStatus = DLL::I106Ch10Open(this->Handle, sFilename, enMode);

    return enStatus;
    }



//-------------------------------------------------------------------------

// Close a Ch 10 file
ReturnStatus Irig106Lib::Close(void)
    {
    ReturnStatus    enStatus;
    enStatus = DLL::enI106Ch10Close(Handle);
    return enStatus;
    }



//-------------------------------------------------------------------------

// Read the next packet header without reading the packet data yet
ReturnStatus Irig106Lib::ReadNextHeader(void)
    { 
    ReturnStatus    enStatus;
    enStatus = DLL::enI106Ch10ReadNextHeader(this->Handle, Header);
    return enStatus;
    }



//-------------------------------------------------------------------------

// Back up and read the previous packet header
ReturnStatus Irig106Lib::ReadPrevHeader(void)
    {
    ReturnStatus    enStatus;
    enStatus = DLL::enI106Ch10ReadPrevHeader(this->Handle, this->Header); 
    return enStatus;
    }



//-------------------------------------------------------------------------

// Read packet data after having read the packet header
ReturnStatus Irig106Lib::ReadData(void)
    {
    ReturnStatus    enStatus;
    pin_ptr<SByte>   pDataBuff   = &this->DataBuff[0];

    // Make sure the buffer is big enough
    if (this->DataBuff->Length < (int)this->Header->ulPacketLen)
        {
        Array::Resize(this->DataBuff, this->Header->ulPacketLen+1000);
        }

    enStatus = DLL::enI106Ch10ReadData(this->Handle, this->DataBuff->Length, pDataBuff);

    return enStatus;
    }



//-------------------------------------------------------------------------

// Write a packet with header and data buffer from this class data
ReturnStatus Irig106Lib::WritePacket(void)
    { 
    ReturnStatus    enStatus;
    pin_ptr<SByte>   pDataBuff   = &this->DataBuff[0];

    enStatus = DLL::enI106Ch10WriteMsg(this->Handle, this->Header, pDataBuff); 

    return enStatus;
    }



//-------------------------------------------------------------------------

// Write a packet with passed header and data buffer
ReturnStatus Irig106Lib::WriteMsg(SuI106Ch10Header       ^ Header,
                                  array<unsigned __int8> ^ DataBuff)
    {
    ReturnStatus    enStatus;
    pin_ptr<unsigned __int8>   pDataBuff   = &DataBuff[0];

    enStatus = DLL::enI106Ch10WriteMsg(this->Handle, Header, pDataBuff); 

    return enStatus;
    }



//-------------------------------------------------------------------------

// Move the read file pointer to the first packet in the data file
ReturnStatus Irig106Lib::FirstMsg(void)
    { 
    return DLL::enI106Ch10FirstMsg(this->Handle); 
    }



//-------------------------------------------------------------------------

// Move the read file pointer to the last packet in the data file
ReturnStatus Irig106Lib::LastMsg(void)
    { 
    return DLL::enI106Ch10LastMsg(this->Handle); 
    }



//-------------------------------------------------------------------------

// Set the read file pointer to the llOffset into the file.  The offset
// doesn't need to fall on a packet boundary.
ReturnStatus Irig106Lib::SetPos(int64_t   llOffset)
    {
    return DLL::enI106Ch10SetPos(this->Handle, llOffset);
    }



//-------------------------------------------------------------------------

// Get the current read offset into the data file
ReturnStatus Irig106Lib::GetPos(int64_t % pllOffset)
    {
    return DLL::enI106Ch10GetPos(this->Handle, pllOffset);
    }


// ========================================================================
// irig106_time

// I never really got the hang of trying to pass .NET arrays into the non-.NET
// DLL routines.  So the time routines that take a 6 element relative time array
// get the array made on the fly.

// Set the association between relative time and IRIG clock time
ReturnStatus Irig106Lib::SetRelTime(IrigTime ^ psuTime, SuRelTime ^ suRelTime)
    {
    uint8_t     abyRelTime[6];

    abyRelTime[0] = suRelTime->uLo       & 0x000000ff;
    abyRelTime[1] = suRelTime->uLo >>  8 & 0x000000ff;
    abyRelTime[2] = suRelTime->uLo >> 16 & 0x000000ff;
    abyRelTime[3] = suRelTime->uLo >> 24 & 0x000000ff;
    abyRelTime[4] = suRelTime->uHi       & 0x000000ff;
    abyRelTime[5] = suRelTime->uHi >>  8 & 0x000000ff;

    return DLL::enI106_SetRelTime(this->Handle, psuTime, abyRelTime);
    }



//-------------------------------------------------------------------------

// Convert from RTC relative time to IRIG clock time
ReturnStatus Irig106Lib::Rel2IrigTime(SuRelTime ^ suRelTime, IrigTime ^ psuTime)
    {
    uint8_t     abyRelTime[6];

    abyRelTime[0] = suRelTime->uLo       & 0x000000ff;
    abyRelTime[1] = suRelTime->uLo >>  8 & 0x000000ff;
    abyRelTime[2] = suRelTime->uLo >> 16 & 0x000000ff;
    abyRelTime[3] = suRelTime->uLo >> 24 & 0x000000ff;
    abyRelTime[4] = suRelTime->uHi       & 0x000000ff;
    abyRelTime[5] = suRelTime->uHi >>  8 & 0x000000ff;

    return DLL::enI106_Rel2IrigTime(this->Handle, &abyRelTime[0], psuTime);
    }



//-------------------------------------------------------------------------

// Convert from RTC relative time to IRIG clock time
ReturnStatus Irig106Lib::Rel2IrigTime(IrigTime ^ psuTime)
    {
    return Rel2IrigTime(Header->suRelTime, psuTime);
    }




//-------------------------------------------------------------------------

ReturnStatus Irig106Lib::RelInt2IrigTime(__int64 llRelTime, IrigTime ^ psuTime)
    {
    return DLL::enI106_RelInt2IrigTime(this->Handle, llRelTime, psuTime);
    }



//-------------------------------------------------------------------------

ReturnStatus Irig106Lib::Irig2RelTime(IrigTime ^ psuTime, SuRelTime ^ suRelTime)
    {
    ReturnStatus        Status;
    uint8_t             abyRelTime[6];

    Status = DLL::enI106_Irig2RelTime(this->Handle, psuTime, abyRelTime);

    if (Status == ReturnStatus::OK)
        {
        suRelTime->uLo = ((uint32_t)(abyRelTime[0]      ) & 0x000000ff) |
                         ((uint32_t)(abyRelTime[1] <<  8) & 0x0000ff00) |
                         ((uint32_t)(abyRelTime[2] << 16) & 0x00ff0000) |
                         ((uint32_t)(abyRelTime[3] << 24) & 0xff000000);
        suRelTime->uHi = ((uint16_t)(abyRelTime[4]      ) & 0x000000ff) |
                         ((uint16_t)(abyRelTime[5] <<  8) & 0x0000ff00);
        }

    return Status;
    }


//-------------------------------------------------------------------------

void Irig106Lib::LLInt2RelTime(__int64 ^ pllRelTime, SuRelTime ^ suRelTime)
    {
    suRelTime->uLo = (uint32_t)(*pllRelTime        & 0x00000000ffffffff);
    suRelTime->uHi = (uint32_t)(*pllRelTime >>  32 & 0x00000000ffffffff);
    }



//-------------------------------------------------------------------------

void Irig106Lib::RelTime2LLInt(SuRelTime ^ suRelTime, __int64 ^ pllRelTime)
    {
    pllRelTime = (__int64)((((unsigned __int64)suRelTime->uLo      ) & 0x00000000ffffffff) |
                           (((unsigned __int64)suRelTime->uHi << 32) & 0xffffffff00000000));
    }



//-------------------------------------------------------------------------

ReturnStatus Irig106Lib::SyncTime(bool bRequireSync, int iTimeLimit)
    {
    return DLL::enI106_SyncTime(this->Handle, bRequireSync, iTimeLimit);
    }



//-------------------------------------------------------------------------

ReturnStatus Irig106Lib::SyncTime()
    {
    return SyncTime(false, 10);
    }



//-------------------------------------------------------------------------

String ^ Irig106Lib::IrigTime2String(IrigTime ^ psuTime)
    {
    char    * szTime;
    String  ^ sTime;

    // IrigTime2String() returns a pointer to a C string
    szTime = DLL::IrigTime2String(psuTime);

    // Copy ASCII string to a .NET string
    sTime = Marshal::PtrToStringAnsi((System::IntPtr)szTime);

    return sTime;
    }



} // end namespace Irig106DLL


