/****************************************************************************

 irig106_dotnet_time.cpp - 

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


#include "stdafx.h"

//#include "irig106_dotnet_defs.h"
//#include "irig106_dotnet.h"
//#include "irig106_dotnet_time.h"

using namespace System;
using namespace System::Runtime::InteropServices;

//=========================================================================

namespace Irig106DotNet
    {

    namespace DLL
        {

#if 0
        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" ReturnStatus enI106_SetRelTime(
                            int                 iI106Ch10Handle,
                            SuIrig106Time     * psuTime,
                            unsigned __int8     abyRelTime[]);

        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" ReturnStatus enI106_Rel2IrigTime(
                            int                 iI106Ch10Handle,
                            unsigned char       abyRelTime[],
                            SuIrig106Time     * psuTime);

        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" ReturnStatus enI106_RelInt2IrigTime(
                            int                 iI106Ch10Handle,
                            __int64             llRelTime,
                            SuIrig106Time     * psuTime);

        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" ReturnStatus enI106_Irig2RelTime(
                            int                 iI106Ch10Handle,
                            SuIrig106Time     * psuTime,
                            unsigned __int8     abyRelTime[]);

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

        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            void vLLInt2TimeArray(
                            __int64               * pllRelTime,
                            unsigned __int8         abyRelTime[]);

        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            void vTimeArray2LLInt(
                            unsigned __int8         abyRelTime[],
                            __int64               * pllRelTime);

        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            enI106_SyncTime(
                            int                     iI106Ch10Handle,
                            int                     bRequireSync,
                            int                     iTimeLimit);

        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            enI106Ch10SetPosToIrigTime(
                            int                     iI106Ch10Handle, 
                            SuIrig106Time         * psuSeekTime);


// General purpose time utilities
// ------------------------------

// Convert IRIG time into an appropriate string
char * IrigTime2String(SuIrig106Time * psuTime);

// IT WOULD BE NICE TO HAVE SOME FUNCTIONS TO COMPARE 6 BYTE
// TIME ARRAY VALUES FOR EQUALITY AND INEQUALITY

// This is handy enough that we'll go ahead and export it to the world
// HMMM... MAYBE A BETTER WAY TO DO THIS IS TO MAKE THE TIME VARIABLES
// AND STRUCTURES THOSE DEFINED IN THIS PACKAGE.
time_t I106_CALL_DECL mkgmtime(struct tm * psuTmTime);

#endif

        } // end namespace DLL

    } // end namespace Irig106DLL

//=========================================================================

namespace Irig106DotNet
{

// Constructor / destructor



} // end namespace Irig106DLL
