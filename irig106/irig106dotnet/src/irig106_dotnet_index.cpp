/****************************************************************************

 irig106_dotnet_index.cpp - 

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

using namespace System;
using namespace System::Runtime::InteropServices;

#include "irig106_dotnet.h"
#include "irig106_dotnet_index.h"


namespace Irig106DotNet
    {
    namespace DLL
        {
        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" Irig106DotNet::ReturnStatus bIndexPresent(int iHandle, bool * bFoundIndex);
        } // end namespace DLL

    } // end Irig106DotNet namespace



namespace Irig106DotNet
    {

    // ------------------------
    // Constructor / Destructor
    // ------------------------

    PacketIndex::PacketIndex(void)
        {
        this->IrigFile = nullptr;
        } // end PacketIndex(void)


    PacketIndex::PacketIndex(Irig106DotNet::Irig106Lib ^ IrigFileIn)
        {
        this->IrigFile = IrigFileIn;

        // If file input mode is read then read the indexes
        if (IrigFile->OpenMode == Irig106DotNet::Ch10FileMode::READ)
            {
            // If indexes are present then read them into memory
            if (IndexPresent())
                {
                } // end if indexes present
            } // end if read mode

        } // end PacketIndex(Irig106DotNet::Irig106Lib ^ IrigFile)


    // -------
    // Methods
    // -------

    bool PacketIndex::IndexPresent(void)
        {
        bool    FoundIndex;

        DLL::bIndexPresent(this->IrigFile->Handle, &FoundIndex);

        return FoundIndex;
        } // end IndexPresent()

    } // end namespace PacketIndex

