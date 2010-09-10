/****************************************************************************

 irig106_dotnet_time.h - 

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



//using namespace System::Collections::Generic;

namespace Irig106DotNet 
    {

// ========================================================================

	public ref class Time
	    {

        // Data structures
        // ===============

        public:

#if 0
        enum DateFmt
            {
            I106_DATEFMT_DAY         =  0,
            I106_DATEFMT_DMY         =  1,
            }

        ref struct SuIrig106Time
            {
            unsigned __int32        ulSecs;     // This is a time_t
            unsigned __int32        ulFrac;     // LSB = 100ns
            DateFmt                 enFmt;      // Day or DMY format
            }
#endif

        }; // end class Time


// ========================================================================

	public ref class PacketTime
	    {

        // Data structures
        // ===============

        public :

        enum TimeFormat
            {
            IRIG_B          =  0x00,
            IRIG_A          =  0x01,
            IRIG_G          =  0x02,
            INT_RTC         =  0x03,
            GPS_UTC         =  0x04,
            GPS_NATIVE      =  0x05,
            }

        enum TimeSource
            {
            INTERNAL        =  0x00,
            EXTERNAL        =  0x01,
            INTERNAL_RMM    =  0x02,
            NONE            =  0x0F    
            }


#if 0
// Channel specific header
typedef struct 
    {
    uint32_t    uTimeSrc    :  4;      // Time source    
    uint32_t    uTimeFmt    :  4;      // Time format
    uint32_t    bLeapYear   :  1;      // Leap year
    uint32_t    uDateFmt    :  1;      // Date format
    uint32_t    uReserved2  :  2;
    uint32_t    uReserved3  : 16;
    } SuTimeF1_ChanSpec;
#endif

        public:

#if 0
// Time message - Day format
typedef struct
    {
    uint16_t    uTmn        :  4;      // Tens of milliseconds
    uint16_t    uHmn        :  4;      // Hundreds of milliseconds
    uint16_t    uSn         :  4;      // Units of seconds
    uint16_t    uTSn        :  3;      // Tens of seconds
    uint16_t    Reserved1   :  1;      // 0

    uint16_t    uMn         :  4;      // Units of minutes
    uint16_t    uTMn        :  3;      // Tens of minutes
    uint16_t    Reserved2   :  1;      // 0
    uint16_t    uHn         :  4;      // Units of hours
    uint16_t    uTHn        :  2;      // Tens of Hours
    uint16_t    Reserved3   :  2;      // 0

    uint16_t    uDn         :  4;      // Units of day number
    uint16_t    uTDn        :  4;      // Tens of day number
    uint16_t    uHDn        :  2;      // Hundreds of day number
    uint16_t    Reserved4   :  6;      // 0
    } SuTime_MsgDayFmt;

// Time message - DMY format
typedef struct
    {
    uint16_t    uTmn        :  4;      // Tens of milliseconds
    uint16_t    uHmn        :  4;      // Hundreds of milliseconds
    uint16_t    uSn         :  4;      // Units of seconds
    uint16_t    uTSn        :  3;      // Tens of seconds
    uint16_t    Reserved1   :  1;      // 0

    uint16_t    uMn         :  4;      // Units of minutes
    uint16_t    uTMn        :  3;      // Tens of minutes
    uint16_t    Reserved2   :  1;      // 0
    uint16_t    uHn         :  4;      // Units of hours
    uint16_t    uTHn        :  2;      // Tens of Hours
    uint16_t    Reserved3   :  2;      // 0

    uint16_t    uDn         :  4;      // Units of day number
    uint16_t    uTDn        :  4;      // Tens of day number
    uint16_t    uOn         :  4;      // Units of month number
    uint16_t    uTOn        :  1;      // Tens of month number
    uint16_t    Reserved4   :  3;      // 0

    uint16_t    uYn         :  4;      // Units of year number
    uint16_t    uTYn        :  4;      // Tens of year number
    uint16_t    uHYn        :  4;      // Hundreds of year number
    uint16_t    uOYn        :  2;      // Thousands of year number
    uint16_t    Reserved5   :  2;      // 0
    } SuTime_MsgDmyFmt;
#endif

        }; // end class PacketTime

    } // end namespace Irig106DotNet


