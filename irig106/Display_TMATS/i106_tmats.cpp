/****************************************************************************

 i106_tmats.cpp - A .NET 2005 TMATS reader program

 Copyright (c) 2008 Irig106.org

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

#include <malloc.h>
#include <string.h>

#include "about.h"
#include "input_form.h"

#include "config.h"
#include "stdint.h"
#include "irig106ch10.h"
#include "i106_decode_tmats.h"

using namespace System;
using namespace System::Collections;
using namespace System::Text;
using namespace System::Runtime::InteropServices;

using namespace I106Input;


// ========================================================================

[STAThreadAttribute]
int main(array<System::String ^> ^args)
    {
	// Enabling Windows XP visual effects before any controls are created
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);

	// Create the main window and run it
	Application::Run(gcnew InputForm());
	return 0;
    }



// --------------------------------------------------------------------------
// Utilities
// --------------------------------------------------------------------------

System::Void InputForm::ProcessIrigFile()
    {
    EnI106Status        enStatus;
//    pin_ptr<int>        piI106In = &iI106In;
    char              * szTime;

    enStatus = IrigIn->Open(txtFilename->Text);
    if (enStatus != I106_OK)
        {
        MessageBox::Show( "Error opening data file.", "Error",
            MessageBoxButtons::OK, MessageBoxIcon::Exclamation );
        return;
        }

    enStatus = IrigIn->ReadNextHeader();
    if (enStatus != I106_OK)
        {
        MessageBox::Show( "Error reading header.", "Error",
            MessageBoxButtons::OK, MessageBoxIcon::Exclamation );
        }

    else
        {
        // See if it's TMATS
        if (IrigIn->pHeader->ubyDataType == I106CH10_DTYPE_TMATS)
            {
            // Read the data into the buffer
            enStatus = IrigIn->ReadData();
            enStatus = IrigIn->Decode_Tmats();

            DecodeDisplayTMATS();
            } // end if TMATS

        // TMATS not found            
        else
            MessageBox::Show( "TMATS data not found", "Error", 
                MessageBoxButtons::OK, MessageBoxIcon::Exclamation );

        } // end read header ok

    // Figure out and display start and stop times
    IrigIn->SyncTime();

    enStatus = IrigIn->ReadNextHeader();
    if (enStatus == I106_OK)
        {
        SuIrig106Time       suStartTime;
        IrigIn->Rel2IrigTime(&suStartTime);
        szTime = IrigTime2String(&suStartTime);
        String ^ sTime;
        sTime = String::Format("Start - {0}", Marshal::PtrToStringAnsi(System::IntPtr(szTime)));
        this->statuslblStartTime->Text = sTime;
        }

    enStatus = IrigIn->LastMsg();
    enStatus = IrigIn->ReadNextHeader();
    if (enStatus == I106_OK)
        {
        SuIrig106Time       suStopTime;
        IrigIn->Rel2IrigTime(&suStopTime);
        szTime = IrigTime2String(&suStopTime);
        String ^ sTime;
        sTime = String::Format("Stop - {0}", Marshal::PtrToStringAnsi(System::IntPtr(szTime)));
        this->statuslblStopTime->Text = sTime;
        }


    // Close the data file
    IrigIn->Close();

    }



// --------------------------------------------------------------------------

System::Void InputForm::DecodeDisplayTMATS()
    {
    // Display the views
    DisplayRaw();
    DisplayChannels();
    DisplayTree();

    } // end DecodeDisplayTMATS()



/* ------------------------------------------------------------------------ */

// Output the raw, unformated TMATS record

System::Void InputForm::DisplayRaw()
    {

    // Convert ASCII TMATS to a String
    System::IntPtr  pBuffPtr((void *)&((char *)IrigIn->pDataBuff)[4]);

    String ^ sTMATS;
    sTMATS = Marshal::PtrToStringAnsi(pBuffPtr, IrigIn->pHeader->ulDataLen-4);

    // Display it in the text box

    this->textRaw->Text = sTMATS;
    Update();

    return;
    }



/* ------------------------------------------------------------------------ */

System::Void InputForm::DisplayTree()
    {
    int                     iGIndex;
    int                     iRIndex;
    int                     iMIndex;
    int                     iRDsiIndex;
    SuGDataSource         * psuGDataSource;
    SuRRecord             * psuRRecord;
    SuRDataSource         * psuRDataSource;
    SuMRecord             * psuMRecord;

    // Clear out the tree
    this->treeTree->Nodes->Clear();

    // G record
    this->treeTree->Nodes->Add("(G) Program Name  - " + Marshal::PtrToStringAnsi((System::IntPtr)IrigIn->suTmatsInfo.psuFirstGRecord->szProgramName));
    this->treeTree->Nodes->Add("(G) TMATS Version - " + Marshal::PtrToStringAnsi((System::IntPtr)IrigIn->suTmatsInfo.psuFirstGRecord->szIrig106Rev));

    // Walk the G record data sources
    psuGDataSource = IrigIn->suTmatsInfo.psuFirstGRecord->psuFirstGDataSource;
    do  {
        if (psuGDataSource == NULL) break;

        // G record data source info
        iGIndex = psuGDataSource->iDataSourceNum;

        TreeNode ^ GDataSourceNode = gcnew TreeNode(String::Format("(G\\DSI-{0}) Data Source ID - {1}", 
            psuGDataSource->iDataSourceNum, 
            Marshal::PtrToStringAnsi((System::IntPtr)psuGDataSource->szDataSourceID)));
        this->treeTree->Nodes->Add(GDataSourceNode);
        GDataSourceNode->Expand();

        GDataSourceNode->Nodes->Add(String::Format("(G\\DST-{0}) Data Source Type - {1}",
            psuGDataSource->iDataSourceNum,
            Marshal::PtrToStringAnsi((System::IntPtr)IrigIn->suTmatsInfo.psuFirstGRecord->psuFirstGDataSource->szDataSourceType)));

        // R record info
        psuRRecord = psuGDataSource->psuRRecord;
        if (psuRRecord != NULL)
            {
            iRIndex = psuRRecord->iRecordNum;

            // Make the R record 
            TreeNode ^ RRecordNode = gcnew TreeNode(String::Format("(R-{0}\\ID) ID - {1}",
                iRIndex, Marshal::PtrToStringAnsi((System::IntPtr)psuRRecord->szDataSourceID)));
            GDataSourceNode->Nodes->Add(RRecordNode);
            RRecordNode->Expand();

            // Walk the R record data sources
            psuRDataSource = psuRRecord->psuFirstDataSource;
            do  {
                if (psuRDataSource == NULL) break;

                iRDsiIndex = psuRDataSource->iDataSourceNum;
                TreeNode ^ RDataSourceNode = gcnew TreeNode(String::Format("(R-{0}\\DSI-{1}) Data Source ID - {2}",
                    iRIndex, iRDsiIndex,
                    Marshal::PtrToStringAnsi((System::IntPtr)psuRDataSource->szDataSourceID)));

                // Set the color based on enabled/disabled
                if (!psuRDataSource->bEnabled)
                    RDataSourceNode->BackColor = Color::LightSalmon;
                else
                    RDataSourceNode->BackColor = Color::LightGreen;

                RRecordNode->Nodes->Add(RDataSourceNode);
                iRDsiIndex = psuRDataSource->iDataSourceNum;
                RDataSourceNode->Nodes->Add(String::Format("(R-{0}\\CDT-{1}) Channel Type - {2}",
                    iRIndex, iRDsiIndex, 
                    Marshal::PtrToStringAnsi((System::IntPtr)psuRDataSource->szChannelDataType)));
                RDataSourceNode->Nodes->Add(String::Format("(R-{0}\\TK1-{1}) Track Number - {2}",
                    iRIndex, iRDsiIndex, psuRDataSource->iTrackNumber));
                RDataSourceNode->Nodes->Add(String::Format("(R-{0}\\CHE-{1}) {2}",
                    iRIndex, iRDsiIndex,
                    psuRDataSource->bEnabled ? "ENABLED" : "DISABLED"));

                // M record
                psuMRecord = psuRDataSource->psuMRecord;
                if (psuMRecord != NULL)
                    {
                    iMIndex = psuMRecord->iRecordNum;
                    //TreeNode ^ MRecordNode = gcnew TreeNode(String::Format("(M-{0}\\BB\\DLN) DLN - {1}",
                    //    iMIndex, Marshal::PtrToStringAnsi((System::IntPtr)psuMRecord->szDataLinkName)));
                    TreeNode ^ MRecordNode = gcnew TreeNode(String::Format("(M-{0}\\ID) - {1}",
                        iMIndex, Marshal::PtrToStringAnsi((System::IntPtr)psuMRecord->szDataSourceID)));

                    MRecordNode->Nodes->Add(String::Format("(M-{0}\\BSG1) {1}",
                        iMIndex, Marshal::PtrToStringAnsi((System::IntPtr)psuMRecord->szBasebandSignalType)));

                    //MRecordNode->Nodes->Add(String::Format("(M-{0}\\ID) {1}",
                    //    iMIndex, Marshal::PtrToStringAnsi((System::IntPtr)psuMRecord->szDataSourceID)));
                    MRecordNode->Nodes->Add(String::Format("(M-{0}\\BB\\DLN) {1}",
                        iMIndex, Marshal::PtrToStringAnsi((System::IntPtr)psuMRecord->szDataLinkName)));

                    RDataSourceNode->Nodes->Add(MRecordNode);
                    } // end if M record exists

                psuRDataSource = psuRDataSource->psuNextRDataSource;
                } while (bTRUE); // end for all R data sources

            psuRRecord = psuRRecord->psuNextRRecord;
            } // end if R record exists

        psuGDataSource = IrigIn->suTmatsInfo.psuFirstGRecord->psuFirstGDataSource->psuNextGDataSource;
        } while (bTRUE);

    Update();

    return;
    }



/* ------------------------------------------------------------------------ */

void InputForm::DisplayChannels()
    {
    int                     iGIndex;
    int                     iRIndex;
    int                     iRDsiIndex;
    SuGDataSource         * psuGDataSource;
    SuRRecord             * psuRRecord;
    SuRDataSource         * psuRDataSource;

    // Clear out the text box
    this->textChannels->Text = "";

    // Print out the TMATS info
    // ------------------------

    // G record
    this->textChannels->Text += L"Program Name - " + Marshal::PtrToStringAnsi((System::IntPtr)IrigIn->suTmatsInfo.psuFirstGRecord->szProgramName) + "\r\n";
    this->textChannels->Text += L"TMATS Version - " + Marshal::PtrToStringAnsi((System::IntPtr)IrigIn->suTmatsInfo.psuFirstGRecord->szIrig106Rev) + "\r\n";
    this->textChannels->Text += L"Channel  Type          Enabled   Data Source         \r\n";
    this->textChannels->Text += L"-------  ------------  --------  --------------------\r\n";

    // Data sources
    psuGDataSource = IrigIn->suTmatsInfo.psuFirstGRecord->psuFirstGDataSource;
    do  {
        if (psuGDataSource == NULL) break;

        // G record data source info
        iGIndex = psuGDataSource->iDataSourceNum;

        // R record info
        psuRRecord = psuGDataSource->psuRRecord;
        do  {
            if (psuRRecord == NULL) break;
            iRIndex = psuRRecord->iRecordNum;

            // R record data sources
            psuRDataSource = psuRRecord->psuFirstDataSource;
            do  {
                if (psuRDataSource == NULL) break;
                iRDsiIndex = psuRDataSource->iDataSourceNum;
                this->textChannels->Text += String::Format("{0,6}   {1,-12} {2}  {3}\r\n",
                    psuRDataSource->iTrackNumber,
                    Marshal::PtrToStringAnsi((System::IntPtr)psuRDataSource->szChannelDataType),
                    psuRDataSource->bEnabled ? " Enabled " : " Disabled",
                    Marshal::PtrToStringAnsi((System::IntPtr)psuRDataSource->szDataSourceID));
                psuRDataSource = psuRDataSource->psuNextRDataSource;
                } while (bTRUE);

            psuRRecord = psuRRecord->psuNextRRecord;
            } while (bTRUE);


        psuGDataSource = IrigIn->suTmatsInfo.psuFirstGRecord->psuFirstGDataSource->psuNextGDataSource;
        } while (bTRUE);

    // Update the dialog box display"
    Update();

    return;
    }


