/****************************************************************************

 irig106_dotnet_index.h - 

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


namespace Irig106DotNet
    {

    public ref class PacketIndex
        {

        // Data structures
        // ===============

        // Node index data
        value struct SuNodeEntry
            {
            unsigned __int64        llTime;     // Generic 8 byte time
            unsigned __int16        ChannelID;
            unsigned __int8         DataType;
            unsigned __int64        Offset;
            };


        // Class data
        // ==========

        Irig106DotNet::Irig106Lib ^ IrigFile;

        bool                        FileSizePresent;
        bool                        IntraPacketHeader;

        array<SuNodeEntry>        ^ IndexEntry;

        // Constructor / Destructor
        // ========================
        public:
        PacketIndex(void);
        PacketIndex(Irig106DotNet::Irig106Lib ^ IrigFileIn);

        // Methods
        // =======

        bool    IndexPresent();


        }; // end class PacketIndex

    } // end namespace Irig106DotNet