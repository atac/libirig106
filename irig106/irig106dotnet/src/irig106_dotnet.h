/****************************************************************************

 irig106_dotnet.h - A .NET assembly wrapper for the IRIG 106 DLL

 Copyright (c) 2010 Irig106.org

 All rights reserved.

 Redistribution and use in source and binary forms without prior
 written consent from irig106.org is prohibited.

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

#pragma once


using namespace System;
using namespace System::Runtime::InteropServices;

#include "irig106_dotnet_defs.h"
#include "irig106_dotnet_tmats.h"


namespace Irig106DotNet 
    {

	public ref class Irig106Lib
	    {

        public:

        // Class data
        // ----------

        public:

            int                         Handle;
            SuI106Ch10Header          ^ Header;
            array<SByte>              ^ DataBuff;
            Ch10FileMode                OpenMode;

        // Constructor / Destructor
        // ------------------------

        Irig106Lib(void);

        ~Irig106Lib(void);

        // Irig library version info
        static System::Void    DotNetLibVersion(int ^ iMajor, int ^ iMinor, int ^ iBuild);
        static String        ^ DotNetLibDateTime();

        // irig106ch10
        // -----------

        // Open / close
        ReturnStatus        Open(String ^ sFilename, Ch10FileMode enMode);
        ReturnStatus        Close(void);

        // Read / Write
        // ------------

        ReturnStatus        ReadNextHeader(void);
        ReturnStatus        ReadPrevHeader(void);

        ReturnStatus        ReadData(void);

        ReturnStatus        WritePacket(void);
        ReturnStatus        WritePacket(SuI106Ch10Header  ^ Header,
                                        array<SByte>      ^ DataBuff);

        ReturnStatus        FirstMsg(void);
        ReturnStatus        LastMsg(void);

        ReturnStatus        SetPos(__int64   llOffset);
        ReturnStatus        GetPos(__int64 % pllOffset);

        // Utilities
        int                 GetHeaderLen(void);
        unsigned int        GetDataLen(void);

        unsigned __int16    CalcHeaderChecksum();
        System::Void        SetHeaderChecksum();

        //uint16_t uCalcSecHeaderChecksum(SuI106Ch10Header * psuHeader);

        //unsigned __int32 CalcDataBuffReqSize(unsigned __int32 uDataLen, int iChecksumType);
        //unsigned __int32 CalcDataBuffReqSize(uint32_t uDataLen);
        //unsigned __int32 CalcDataBuffReqSize();
        //EnI106Status AddDataFillerChecksum(SuI106Ch10Header * psuI106Hdr, unsigned char achData[]);
        //EnI106Status AddDataFillerChecksum();

        // irig106_time
        // ------------

        ReturnStatus SetRelTime(IrigTime ^ psuTime, SuRelTime ^ suRelTime);

        ReturnStatus Rel2IrigTime(SuRelTime ^ suRelTime, IrigTime ^ psuTime);
        ReturnStatus Rel2IrigTime(IrigTime  ^ psuTime);

        ReturnStatus RelInt2IrigTime(__int64 llRelTime, IrigTime ^ psuTime);
        ReturnStatus Irig2RelTime(IrigTime ^ psuTime, SuRelTime ^ suRelTime);

        void         LLInt2RelTime(__int64 ^ pllRelTime, SuRelTime ^ suRelTime);
        void         RelTime2LLInt(SuRelTime ^ suRelTime, __int64 ^ pllRelTime);
        void         RelTime2LLInt(__int64 ^ pllRelTime);

        ReturnStatus SyncTime(bool bRequireSync, int iTimeLimit);
        ReturnStatus SyncTime();

        String     ^ IrigTime2String(IrigTime ^ psuTime);

    };
}
