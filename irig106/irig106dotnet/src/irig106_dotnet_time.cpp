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
