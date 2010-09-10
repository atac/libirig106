/****************************************************************************

 irig106_dotnet_version.cpp - Maintain DLL version and build time

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

#include "irig106_dotnet.h"

using namespace System;
using namespace System::Runtime::InteropServices;

//=========================================================================

//namespace Irig106DotNet
//    {
//
//    namespace DLL
//        {
//        // Main 
//        //[DllImport("irig106.dll", CharSet=CharSet::Ansi, EntryPoint="enI106Ch10Open")]
//        //    extern "C" ReturnStatus I106Ch10Open(int ^, String ^, Ch10FileMode);
//
//
//        } // end namespace DLL
//
//    } // end namespace Irig106DotNet

/* There may be some easy way to drag version information out of a DLL or EXE but it has 
 * eluded me in the past.  Maybe it's the Reflection junk.  I don't know know.  So I 
 * took the chicken's way out and just implemented these version routines.  So I need 
 * to maintain version in two places, here and in the version resource.  Sorry.
 */

/* BTW, someday I need to mod the development environment to always compile this version file.
 * It's trivial to do with gcc and a makefile, but this is a bit more complicated.
 */

namespace Irig106DotNet
{

#define VERSION_DOTNET_MAJOR    0
#define VERSION_DOTNET_MINOR    3
#define VERSION_DOTNET_BUILD    0

//-------------------------------------------------------------------------

System::Void Irig106Lib::DotNetLibVersion(int ^ iMajor, int ^ iMinor, int ^ iBuild)
    {
    *iMajor = VERSION_DOTNET_MAJOR;
    *iMinor = VERSION_DOTNET_MINOR;
    *iBuild = VERSION_DOTNET_BUILD;
    return;
    }



//-------------------------------------------------------------------------

String ^ Irig106Lib::DotNetLibDateTime()
    {
    return String::Format("{0} {1}", __DATE__, __TIME__);
    }



//-------------------------------------------------------------------------

} // end namespace Irig106DotNet
