/****************************************************************************

 about.cpp - A .NET 2005 class that implements the "about" dialog

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
#include "AboutDub.h"
#include "SelectForm.h"
#include "CopyStatus.h"

using namespace System;
using namespace System::Collections;
using namespace System::Text;
using namespace System::Runtime::InteropServices;
using namespace System::Windows::Forms;
using namespace System::Diagnostics;

using namespace i106Dub;
using namespace Irig106;


// --------------------------------------------------------------------------
// Utilities
// --------------------------------------------------------------------------

System::Void SelectForm::InitFormFromIrig()
    {
    EnI106Status        enStatus;
    uint8_t             uLastPacketType;

    // Open the data file
    IrigIn->Close();
    enStatus = IrigIn->Open(txtInFile->Text);
    if (enStatus != Irig106::I106_OK)
        {
        MessageBox::Show( "Error opening input data file.", "Error",
            MessageBoxButtons::OK, MessageBoxIcon::Exclamation );
        return;
        }

    // Get the file size and start/stop offsets for later
    System::IO::FileInfo ^ pFileInfo;
    pFileInfo = gcnew System::IO::FileInfo(txtInFile->Text);
    llFileSize            = pFileInfo->Length;
    psuDubBegin->llOffset = 0;
    psuDubEnd->llOffset   = llFileSize;

    // Read the TMATS and populate form from TMATS info
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

            //SuTmatsInfo   suTmatsInfo;
            IrigIn->Decode_Tmats();
            PopulateChannelTable();

            } // end if TMATS

        // TMATS not found            
        else
            {
            MessageBox::Show( "TMATS data not found", "Error", 
                MessageBoxButtons::OK, MessageBoxIcon::Exclamation );
            return;
            }
        } // end read header ok

    // Figure out and display start and stop times
    IrigIn->SyncTime();

    // Get the first packet info
    enStatus = IrigIn->ReadNextHeader();
    if (enStatus == I106_OK)
        {
        IrigIn->Rel2IrigTime(psuFileBeginTime);
        psuDubBegin->suITime = *psuFileBeginTime;
        IrigIn->TimeArray2LLInt(&(psuDubBegin->llRtcTime));

        this->statuslblStartTime->Text = 
            String::Format("Begin - {0}", IrigIn->strTime2String(psuFileBeginTime));
        this->txtStartTime->Text = IrigIn->strTime2String(psuFileBeginTime);
        this->traStartTime->Value = 0;
        this->traStartTime->Enabled = true;
        }

    // Get the last packet info
    enStatus = IrigIn->LastMsg();
    enStatus = IrigIn->ReadNextHeader();
    if (enStatus == I106_OK)
        {
        IrigIn->Rel2IrigTime(psuFileEndTime);
        psuDubEnd->suITime = *psuFileEndTime;
        IrigIn->TimeArray2LLInt(&(psuDubEnd->llRtcTime));

        this->statuslblStopTime->Text = 
            String::Format("End - {0}", IrigIn->strTime2String(psuFileEndTime));
        this->txtStopTime->Text = IrigIn->strTime2String(psuFileEndTime);
        this->traStopTime->Value = this->traStopTime->Maximum;
        this->traStopTime->Enabled = true;

        uLastPacketType = IrigIn->pHeader->ubyDataType;
        }

    return;
    }



// --------------------------------------------------------------------------

System::Void SelectForm::PopulateChannelTable()
    {
    int                     iGIndex;
    int                     iRIndex;
    int                     iRDsiIndex;
    SuGDataSource         * psuGDataSource;
    SuRRecord             * psuRRecord;
    SuRDataSource         * psuRDataSource;
    int                     iCurrRow = 0;

    System::Windows::Forms::Label    ^ pTextBox;
    System::Windows::Forms::Label    ^ pTypeTextBox;
    System::Windows::Forms::Label    ^ pNameTextBox;
    System::Windows::Forms::CheckBox ^ pCopyCheckBox;
    System::Windows::Forms::CheckBox ^ pIndexCheckBox;

    // Convert the columns to autosize
    tlpChannels->SuspendLayout();
    TableLayoutColumnStyleCollection ^ ColumnStyles = tlpChannels->ColumnStyles;
    for (int iCurrCol = 0; iCurrCol < ColumnStyles->Count; iCurrCol++)
        {
        ColumnStyles[iCurrCol]->SizeType = SizeType::AutoSize;
        ColumnStyles[iCurrCol]->Width = 0;
        }
    tlpChannels->ResumeLayout();

    // Remove any existing controls and rows
    tlpChannels->SuspendLayout();
    tlpChannels->Controls->Clear();
    tlpChannels->RowStyles->Clear();
    tlpChannels->RowCount = 0;
    tlpChannels->ResumeLayout();

    // Clear out the channel info list
    ChannelInfo->Clear();

    // Put up common check boxes
    tlpChannels->SuspendLayout();

    // Top row labels
    tlpChannels->RowStyles->Add(gcnew RowStyle(SizeType::Absolute, 30));
    pTextBox = gcnew System::Windows::Forms::Label;
    pTextBox->AutoSize = true;
    pTextBox->Text = "Channel";
    pTextBox->Anchor = AnchorStyles::Left;
    tlpChannels->Controls->Add(pTextBox, 0, iCurrRow);
    pTextBox = gcnew System::Windows::Forms::Label;
    pTextBox->AutoSize = true;
    pTextBox->Text = "Index";
    pTextBox->Anchor = AnchorStyles::Left;
    tlpChannels->Controls->Add(pTextBox, 1, iCurrRow);
    pTextBox = gcnew System::Windows::Forms::Label;
    pTextBox->AutoSize = true;
    pTextBox->Text = "Type";
    pTextBox->Anchor = AnchorStyles::Left;
    tlpChannels->Controls->Add(pTextBox, 2, iCurrRow);
    pTextBox = gcnew System::Windows::Forms::Label;
    pTextBox->AutoSize = true;
    pTextBox->Text = "Description";
    pTextBox->Anchor = AnchorStyles::Left;
    tlpChannels->Controls->Add(pTextBox, 3, iCurrRow);
    iCurrRow++;

    // Ch 0 TMATS
    tlpChannels->RowStyles->Add(gcnew RowStyle(SizeType::Absolute, 30));

    pCopyCheckBox = gcnew System::Windows::Forms::CheckBox;
    pCopyCheckBox->Text     = "0 - TMATS";
    pCopyCheckBox->Anchor   = AnchorStyles::Left;
    pCopyCheckBox->AutoSize = true;
    pCopyCheckBox->Checked  = true;
    tlpChannels->Controls->Add(pCopyCheckBox, 0, iCurrRow);

    pIndexCheckBox = gcnew System::Windows::Forms::CheckBox;
    pIndexCheckBox->Anchor   = AnchorStyles::None;
    pIndexCheckBox->AutoSize = true;
    pIndexCheckBox->Checked  = false;
    if ((IrigIn->suTmatsInfo.psuFirstRRecord->szIndexEnabled != NULL) &&
        (IrigIn->suTmatsInfo.psuFirstRRecord->bIndexEnabled))
         pIndexCheckBox->Enabled  = true;
    else
         pIndexCheckBox->Enabled  = false;
    tlpChannels->Controls->Add(pIndexCheckBox, 1, iCurrRow);

    pTextBox = gcnew System::Windows::Forms::Label;
    pTextBox->AutoSize = true;
    pTextBox->Text = "Channel 0 - TMATS Packets";
    pTextBox->Anchor = AnchorStyles::Left;
    tlpChannels->Controls->Add(pTextBox, 3, iCurrRow);

    Channel0Info->pCopyTMATSCheckBox  = pCopyCheckBox;
    Channel0Info->pIndexTMATSCheckBox = pIndexCheckBox;

    iCurrRow++;

    // Ch 0 Index
    tlpChannels->RowStyles->Add(gcnew RowStyle(SizeType::Absolute, 30));

    pCopyCheckBox = gcnew System::Windows::Forms::CheckBox;
    pCopyCheckBox->Text     = "0 - Index";
    pCopyCheckBox->Anchor   = AnchorStyles::Left;
    pCopyCheckBox->AutoSize = true;
    pCopyCheckBox->Checked  = false;
    pCopyCheckBox->Enabled  = false;
    tlpChannels->Controls->Add(pCopyCheckBox, 0, iCurrRow);

    pTextBox = gcnew System::Windows::Forms::Label;
    pTextBox->AutoSize = true;
    pTextBox->Text = "Channel 0 - Index Packets";
    pTextBox->Anchor = AnchorStyles::Left;
    tlpChannels->Controls->Add(pTextBox, 3, iCurrRow);

    iCurrRow++;

    // Ch 0 Events
    tlpChannels->RowStyles->Add(gcnew RowStyle(SizeType::Absolute, 30));

    pCopyCheckBox = gcnew System::Windows::Forms::CheckBox;
    pCopyCheckBox->Text     = "0 - Events";
    pCopyCheckBox->Anchor   = AnchorStyles::Left;
    pCopyCheckBox->AutoSize = true;
    pCopyCheckBox->Checked  = true;
    tlpChannels->Controls->Add(pCopyCheckBox, 0, iCurrRow);

    pIndexCheckBox = gcnew System::Windows::Forms::CheckBox;
    pIndexCheckBox->Anchor   = AnchorStyles::None;
    pIndexCheckBox->AutoSize = true;
//  pIndexCheckBox->Checked  = true;
    if ((IrigIn->suTmatsInfo.psuFirstRRecord->szIndexEnabled != NULL) &&
        (IrigIn->suTmatsInfo.psuFirstRRecord->bIndexEnabled))
        {
         pIndexCheckBox->Enabled  = true;
         pIndexCheckBox->Checked  = true;
        }
    else
        {
         pIndexCheckBox->Enabled  = false;
         pIndexCheckBox->Checked  = false;
        }
    tlpChannels->Controls->Add(pIndexCheckBox, 1, iCurrRow);

    pTextBox = gcnew System::Windows::Forms::Label;
    pTextBox->AutoSize = true;
    pTextBox->Text = "Channel 0 - Event Packets";
    pTextBox->Anchor = AnchorStyles::Left;
    tlpChannels->Controls->Add(pTextBox, 3, iCurrRow);

    Channel0Info->pCopyEventsCheckBox  = pCopyCheckBox;
    Channel0Info->pIndexEventsCheckBox = pIndexCheckBox;

    iCurrRow++;

    // Ch 0 Other
    tlpChannels->RowStyles->Add(gcnew RowStyle(SizeType::Absolute, 30));

    pCopyCheckBox = gcnew System::Windows::Forms::CheckBox;
    pCopyCheckBox->Text     = "0 - Other";
    pCopyCheckBox->Anchor   = AnchorStyles::Left;
    pCopyCheckBox->AutoSize = true;
    pCopyCheckBox->Checked  = true;
    tlpChannels->Controls->Add(pCopyCheckBox, 0, iCurrRow);

    pIndexCheckBox = gcnew System::Windows::Forms::CheckBox;
    pIndexCheckBox->Anchor   = AnchorStyles::None;
    pIndexCheckBox->AutoSize = true;
    pIndexCheckBox->Checked  = false;
    if ((IrigIn->suTmatsInfo.psuFirstRRecord->szIndexEnabled != NULL) &&
        (IrigIn->suTmatsInfo.psuFirstRRecord->bIndexEnabled))
         pIndexCheckBox->Enabled  = true;
    else
         pIndexCheckBox->Enabled  = false;
    tlpChannels->Controls->Add(pIndexCheckBox, 1, iCurrRow);

    pTextBox = gcnew System::Windows::Forms::Label;
    pTextBox->AutoSize = true;
    pTextBox->Text = "Channel 0 - Other Packet Types";
    pTextBox->Anchor = AnchorStyles::Left;
    tlpChannels->Controls->Add(pTextBox, 3, iCurrRow);

    Channel0Info->pCopyOtherCheckBox  = pCopyCheckBox;
    Channel0Info->pIndexOtherCheckBox = pIndexCheckBox;

    iCurrRow++;

    // Data sources
    psuGDataSource =  IrigIn->suTmatsInfo.psuFirstGRecord->psuFirstGDataSource;
    while (true)  
        {
        if (psuGDataSource == NULL) break;

        // G record data source info
        iGIndex = psuGDataSource->iDataSourceNum;

        // R record info
        psuRRecord = psuGDataSource->psuRRecord;
        while (true)
            {
            if (psuRRecord == NULL) 
                break;
            iRIndex = psuRRecord->iRecordNum;

            // Check for time index
            if ((psuRRecord->szIndexEnabled != NULL) &&
                (psuRRecord->bIndexEnabled == bTRUE))
                {
//                tlpChannels->Controls->
                }

            // Get the first R record data sources
            psuRDataSource = psuRRecord->psuFirstDataSource;

            // Loop through all the R record data sources
            while (true)
                {
                // If record pointer is null then we're done
                if (psuRDataSource == NULL) 
                    break;

                iRDsiIndex = psuRDataSource->iDataSourceNum;

                // Make a copy enable checkbox
                tlpChannels->RowStyles->Add(gcnew RowStyle(SizeType::Absolute, 30));
                pCopyCheckBox = gcnew System::Windows::Forms::CheckBox;
                pCopyCheckBox->AutoSize = true;
                pCopyCheckBox->Anchor = AnchorStyles::Left;
                pCopyCheckBox->Text =
                    String::Format(" {0,2}",
                    psuRDataSource->iTrackNumber);
                if ((psuRDataSource->szEnabled == NULL) || (psuRDataSource->bEnabled == false))
                    pCopyCheckBox->Enabled = false;
                tlpChannels->Controls->Add(pCopyCheckBox, 0, iCurrRow);

                // Make channel type label
                pTypeTextBox = gcnew System::Windows::Forms::Label;
                pTypeTextBox->AutoSize = true;
                pTypeTextBox->Text =
                    String::Format("{0}",
                    Marshal::PtrToStringAnsi((System::IntPtr)psuRDataSource->szChannelDataType));
                pTypeTextBox->Anchor = AnchorStyles::Left;
                pTypeTextBox->AutoSize = true;
                tlpChannels->Controls->Add(pTypeTextBox, 2, iCurrRow);

                // Make an index enable checkbox
                pIndexCheckBox = gcnew System::Windows::Forms::CheckBox;
                pIndexCheckBox->AutoSize = true;
                pIndexCheckBox->Anchor = AnchorStyles::None;
                //pIndexCheckBox->Text =
                //    String::Format("",
                //    psuRDataSource->iTrackNumber);

                // If it's a time packet then copy by default
                if (pTypeTextBox->Text == "TIMEIN")
                    pCopyCheckBox->Checked  = true;
                else
                    pCopyCheckBox->Checked  = false;

                // If no index then don't enable the check box
                if ((IrigIn->suTmatsInfo.psuFirstRRecord->szIndexEnabled == NULL ) ||
                    (IrigIn->suTmatsInfo.psuFirstRRecord->bIndexEnabled  == false) ||
                    (psuRDataSource->szEnabled                           == NULL ) ||
                    (psuRDataSource->bEnabled                            == false))
                    pIndexCheckBox->Enabled = false;

                // Index so enable index check box
                else 
                    {
                    pIndexCheckBox->Enabled = true;
                    // If time packet the enable index by default
                    if (pTypeTextBox->Text == "TIMEIN")
                        pIndexCheckBox->Checked = true;
                    else
                        pIndexCheckBox->Checked = false;
                    }
                tlpChannels->Controls->Add(pIndexCheckBox, 1, iCurrRow);

                // Make channel name label
                pNameTextBox = gcnew System::Windows::Forms::Label;
                pNameTextBox->AutoSize = true;
                pNameTextBox->Text =
                    String::Format("{0}",
                    Marshal::PtrToStringAnsi((System::IntPtr)psuRDataSource->szDataSourceID));
                pNameTextBox->Anchor = AnchorStyles::Left;
                pNameTextBox->AutoSize = true;
                tlpChannels->Controls->Add(pNameTextBox, 3, iCurrRow);

                // Put channel info into list for later
                SuChannelInfo ^ NewChanInfo = gcnew SuChannelInfo;
                NewChanInfo->iChannelNum     = psuRDataSource->iTrackNumber;
                NewChanInfo->pCopyCheckBox   = pCopyCheckBox;
                NewChanInfo->pIndexCheckBox  = pIndexCheckBox;
                NewChanInfo->ChannelType = gcnew String(psuRDataSource->szChannelDataType,0,strlen(psuRDataSource->szChannelDataType));
                ChannelInfo->Add(NewChanInfo);

                iCurrRow++;
                psuRDataSource = psuRDataSource->psuNextRDataSource;
                } // end looping on all R data source records

            psuRRecord = psuRRecord->psuNextRRecord;
            } // end looping on all R records

        psuGDataSource = IrigIn->suTmatsInfo.psuFirstGRecord->psuFirstGDataSource->psuNextGDataSource;
        } // end looping  on all G records

    // Jink with the size of the table to make it fit right on the form
    SetTableHeight();

    tlpChannels->ResumeLayout();
    tlpChannels->PerformLayout();

    return;
    }



// --------------------------------------------------------------------------

// The TablePanelLayout control is so goofy.  Getting the height just right
// for the number of rows is a PITA.  Hopefully this will do it.

System::Void SelectForm::SetTableHeight()
    {
    int     iCurrRow;
    int     iControlHeight;

    // The height is the number of rows times the row height plus a fudge
    iControlHeight = 0;
    TableLayoutRowStyleCollection ^ RowStyles = tlpChannels->RowStyles;
    for (iCurrRow = 0; iCurrRow < RowStyles->Count; iCurrRow++)
        {
        iControlHeight += int((RowStyles[iCurrRow]->Height) + 3.0);
//      iControlHeight += 33;
        }
    iControlHeight += 10;

    // If the calculated height is greater than the space we have then just
    // make it the max space between the controls it falls between
    if (iControlHeight > (traStartTime->Top - cmdOutput->Bottom - 10))
        iControlHeight = traStartTime->Top - cmdOutput->Bottom - 10;

    // Set the height
    tlpChannels->Height = iControlHeight;

    return;
    }


// --------------------------------------------------------------------------

System::Void SelectForm::cmdDub_Click(System::Object^  sender, System::EventArgs^  e) 
        {
        CopyStatus::SuCopyParams      ^ mParams = gcnew CopyStatus::SuCopyParams;
        bool                            bTimeChanFound;

        // Make a set of copy parameters
        // -------------------------------------

        // Setup the Channel 0 parameters
//      mParams->ChanEnabled[0] = true;
        mParams->bCh0TMATSCopy   = Channel0Info->pCopyTMATSCheckBox->Checked;
        mParams->bCh0TMATSIndex  = Channel0Info->pIndexTMATSCheckBox->Checked;
        mParams->bCh0EventsCopy  = Channel0Info->pCopyEventsCheckBox->Checked;
        mParams->bCh0EventsIndex = Channel0Info->pIndexEventsCheckBox->Checked;
        mParams->bCh0OtherCopy   = Channel0Info->pCopyOtherCheckBox->Checked;
        mParams->bCh0OtherIndex  = Channel0Info->pIndexOtherCheckBox->Checked;

        // Setup other channel parameters
        bTimeChanFound = false;
        for each (SuChannelInfo ^ Channel in ChannelInfo)
            {
            if (Channel->pCopyCheckBox->Checked == true)
                {
                // If channel enable then we'll dub it
                mParams->ChanEnabled[Channel->iChannelNum] = 1;

                // See if we've found a time channel
                if (Channel->ChannelType == "TIMEIN")
                    bTimeChanFound = true;

                // If channel index enabled then we'll make it
                if (Channel->pIndexCheckBox->Checked == true)
                    mParams->ChanIndex[Channel->iChannelNum] = 1;

                } // end if channel enabled

            } // end for each channel defined

        // Warn user if TMATS not enabled
        if (mParams->bCh0TMATSCopy == false)
            {
            String             ^ sErrMsg  = "Copy TMATS not enabled.  IRIG 106 requires a TMATS record. Copy anyway?";
            String             ^ sCaption = "Copy TMATS not enabled";
            MessageBoxButtons    oButtons = MessageBoxButtons::YesNo;
            System::Windows::Forms::DialogResult oResult;

            // Displays the MessageBox.
            oResult = MessageBox::Show(this, sErrMsg, sCaption, oButtons);
            if (oResult == ::DialogResult::No)
                {
                return;
                }
            } // end if copy TMATS not enabled

        // Warn user if time channel not found
        if (bTimeChanFound == false)
            {
            String             ^ sErrMsg  = "No IRIG time channel enabled.  IRIG 106 requires a time channel. Copy anyway?";
            String             ^ sCaption = "No IRIG Time Channel";
            MessageBoxButtons    oButtons = MessageBoxButtons::YesNo;
            System::Windows::Forms::DialogResult oResult;

            // Displays the MessageBox.
            oResult = MessageBox::Show(this, sErrMsg, sCaption, oButtons);
            if (oResult == ::DialogResult::No)
                {
                return;
                }
            } // end if time channel not found

        // Fill in the rest of the copy parameters
        mParams->sInFile       = txtInFile->Text;
        mParams->sOutFile      = txtOutFile->Text;
        mParams->llBeginOffset = psuDubBegin->llOffset;
        mParams->llEndOffset   = psuDubEnd->llOffset;
        mParams->bSkipTime     = !bTimeChanFound;

        // Make the form and start the asynchronous operation.
        CopyStatus ^ mCopyStatusForm = gcnew CopyStatus();
        mCopyStatusForm->Show();
        mCopyStatusForm->StartCopy(mParams);
        }



