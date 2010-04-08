/****************************************************************************

 irig106_dotnet_defs.h - .NET versions of the various IRIG library data structs

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

// Integer size typedef's to make it easier to copy structures over from
// the IRIG 106 C library.

typedef __int8              int8_t;
typedef __int16             int16_t;
typedef __int32             int32_t;
typedef __int64             int64_t;

typedef unsigned __int8     uint8_t;
typedef unsigned __int16    uint16_t;
typedef unsigned __int32    uint32_t;
typedef unsigned __int64    uint64_t;



namespace Irig106DotNet 
    {

// ------------------------------------------------------------------------
// General data and structures
// ------------------------------------------------------------------------

    /// Error return codes
    //  ------------------
    public enum class ReturnStatus
        {
        OK                 =  0,   ///< Everything okey dokey
        OPEN_ERROR         =  1,   ///< Fatal problem opening for read or write
        OPEN_WARNING       =  2,   ///< Non-fatal problem opening for read or write
        EOF                =  3,   ///< End of file encountered
        BOF                =  4,   //
        READ_ERROR         =  5,   ///< Error reading data from file
        WRITE_ERROR        =  6,   ///< Error writing data to file
        MORE_DATA          =  7,   //
        SEEK_ERROR         =  8,
        WRONG_FILE_MODE    =  9,
        NOT_OPEN           = 10,
        ALREADY_OPEN       = 11,
        BUFFER_TOO_SMALL   = 12,
        NO_MORE_DATA       = 13,
        NO_FREE_HANDLES    = 14,
        INVALID_HANDLE     = 15,
        TIME_NOT_FOUND     = 16,
        HEADER_CHKSUM_BAD  = 17,
        NO_INDEX           = 18,
        UNSUPPORTED        = 19,
        BUFFER_OVERRUN     = 20
        };


    // IRIG 106 data types
    // -------------------
    public enum class DataType : unsigned __int8
        {
        COMPUTER_0        = 0x00,
        USER_DEFINED      = 0x00,
        COMPUTER_1        = 0x01,
        TMATS             = 0x01,
        COMPUTER_2        = 0x02,
        RECORDING_EVENT   = 0x02,
        COMPUTER_3        = 0x03,
        RECORDING_INDEX   = 0x03,
        COMPUTER_4        = 0x04,
        COMPUTER_5        = 0x05,
        COMPUTER_6        = 0x06,
        COMPUTER_7        = 0x07,
        PCM_FMT_0         = 0x08,
        PCM_FMT_1         = 0x09,
        PCM               = 0x09,   // Depricated
        IRIG_TIME         = 0x11,
        MIL_1553_FMT_1    = 0x19,
        MIL_1553_FMT_2    = 0x1A,   // 16PP194 Bus
        ANALOG            = 0x21,
        DISCRETE          = 0x29,
        MESSAGE           = 0x30,
        ARINC_429         = 0x38,
        VIDEO_FMT_0       = 0x40,
        VIDEO_FMT_1       = 0x41,
        VIDEO_FMT_2       = 0x42,
        IMAGE_FMT_0       = 0x48,
        IMAGE_FMT_1       = 0x49,
        UART_FMT_0        = 0x50,
        IEEE_1394_FMT_0   = 0x58,
        IEEE_1394_FMT_1   = 0x59,
        PARALLEL_FMT_0    = 0x60,
        ETHERNET_FMT_0    = 0x68
        }; // end enum DataType


    // Header packet flags
    // -------------------
    public enum class HeaderFlag : unsigned __int8
        {
        CHKSUM_NONE         = 0x00,
        CHKSUM_8            = 0x01,
        CHKSUM_16           = 0x02,
        CHKSUM_32           = 0x03,
        CHKSUM_MASK         = 0x03,

        TIMEFMT_IRIG106     = 0x00,
        TIMEFMT_IEEE1588    = 0x04,
        TIMEFMT_Reserved1   = 0x08,
        TIMEFMT_Reserved2   = 0x0C,
        TIMEFMT_MASK        = 0x0C,

        OVERFLOW            = 0x10,
        TIMESYNCERR         = 0x20,
        IPTIMESRC           = 0x40,
        SEC_HEADER          = 0x80
        }; // end enum HeaderFlag


    /// Data file open modes
    //  --------------------
    public enum class Ch10FileMode
        {
        READ               = 1,    // Open an existing file for reading
        OVERWRITE          = 2,    // Create a new file or overwrite an exising file
        APPEND             = 3,    // Append data to the end of an existing file
        READ_IN_ORDER      = 4     // Open an existing file for reading in time order
        };

    // Time representation
    // -------------------
    // Time has a number of representations in the IRIG 106 spec.
    // The structure below is used as a convenient standard way of
    // representing time.  The nice thing about standards is that there 
    // are so many to choose from, and time is no exception. But none of 
    // the various C time representations really fill the bill. So I made 
    // a new time representation.  So there.
    public enum class DateFmt
        {
        I106_DATEFMT_DAY         =  0,
        I106_DATEFMT_DMY         =  1,
        };

    [StructLayout(LayoutKind::Sequential, Pack=1)]
    public ref struct IrigTime
        {
        unsigned __int32        ulSecs;     // This is a time_t
        unsigned __int32        ulFrac;     // LSB = 100ns
        DateFmt                 enFmt;      // Day or DMY format
        };


    // IRIG 106 header and optional secondary header data structure
    // ------------------------------------------------------------
// THIS WORKS IN THE HEADER STRUCT BELOW AS A VALUE STRUCT BUT NOT AS A REF STRUCT
// AND I REALLY DON'T KNOW WHY.  THAT'S DANGEROUS.
    [StructLayout(LayoutKind::Sequential, Pack=1)]
    public value struct    SuRelTime
        {
        uint32_t      uLo;              ///< Low 4 bytes of relative time
        uint16_t      uHi;              ///< High 2 bytes of relative time
        };

    [StructLayout(LayoutKind::Sequential, Pack=1)]
    public ref struct SuI106Ch10Header
        {
        uint16_t      uSync;                // Packet Sync Pattern
        uint16_t      uChID;                // Channel ID
        uint32_t      ulPacketLen;          // Total packet length
        uint32_t      ulDataLen;            // Data length
        uint8_t       ubyHdrVer;            // Header Version
        uint8_t       ubySeqNum;            // Sequence Number
        HeaderFlag    ubyPacketFlags;       // PacketFlags
        DataType      ubyDataType;          // Data type
        SuRelTime     suRelTime;            // Relative time counter
//        uint32_t      uLo;              ///< Low 4 bytes of relative time
//        uint16_t      uHi;              ///< High 2 bytes of relative time
        //ref struct    suRelTime
        //    {
        //    uint32_t      uLo;              ///< Low 4 bytes of relative time
        //    uint16_t      uHi;              ///< High 2 bytes of relative time
        //    };

        uint16_t      uChecksum;            // Header Checksum
        uint64_t      ullTime;              // Time (start secondary header)
        uint16_t      uReserved;            //
        uint16_t      uSecChecksum;         // Secondary Header Checksum
        };

    } // end namespace Irig106DotNet

