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
#include <ctype.h>

#include "AboutDisplayTmats.h"
#include "InputForm.h"

#include "config.h"
#include "stdint.h"
#include "irig106ch10.h"
#include "i106_decode_tmats.h"

using namespace System;
using namespace System::Collections;
using namespace System::Text;
using namespace System::Runtime::InteropServices;

using namespace I106Input;


TreeNode ^ MakePRecordNode(SuPRecord * psuPRecord);

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

            //SuTmatsInfo   suTmatsInfo;
//            enStatus = enI106_Decode_Tmats(psuI106Hdr, pvBuff, psuTmatsInfo);

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
//    size_t              iDataBuffLen;
    EnI106Status        enStatus;

    // Free any previous TMATS info
//    enI106_Free_TmatsInfo(this->psuTmatsInfo);

    // Allocate memory and store the info
//    memset((void *)psuTmatsInfo, 0, sizeof(SuTmatsInfo));

    // Decode the new info
    enStatus = IrigIn->Decode_Tmats();
//    psuTmatsInfo = &IrigIn->suTmatsInfo;

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
//  SuPRecord             * psuPRecord;

    // Clear out the tree
    this->treeTree->Nodes->Clear();

    // G record
    switch (IrigIn->suTmatsInfo.iCh10Ver)
        {
        case 0 :
            this->treeTree->Nodes->Add("        Recorder Ch 10 Version  - 05 or earlier");
            break;
        case 7 :
            this->treeTree->Nodes->Add("        Recorder Ch 10 Version  - 07");
            break;
        case 8 :
            this->treeTree->Nodes->Add("        Recorder Ch 10 Version  - 09");
            break;
        default :
            this->treeTree->Nodes->Add("        Recorder Ch 10 Version  - Unknown");
            break;
        } // end switch on Ch 10 version

    if (IrigIn->suTmatsInfo.psuFirstGRecord->szIrig106Rev != NULL)
        this->treeTree->Nodes->Add("(G\\106) TMATS Ch 9 Version      - " + Marshal::PtrToStringAnsi((System::IntPtr)IrigIn->suTmatsInfo.psuFirstGRecord->szIrig106Rev));

    if (IrigIn->suTmatsInfo.psuFirstGRecord->szProgramName != NULL)
        this->treeTree->Nodes->Add("(G\\PN)  Program Name            - " + Marshal::PtrToStringAnsi((System::IntPtr)IrigIn->suTmatsInfo.psuFirstGRecord->szProgramName));

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

        if (IrigIn->suTmatsInfo.psuFirstGRecord->psuFirstGDataSource->szDataSourceType != NULL)
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

            // Indexes enabled/disabled
            if (psuRRecord->szIndexEnabled != NULL)
                if(toupper(psuRRecord->szIndexEnabled[0]) == 'T')
                    RRecordNode->Nodes->Add(String::Format("(R-{0}\\IDX\\E) Indexes ENABLED", iRIndex));
                else
                    RRecordNode->Nodes->Add(String::Format("(R-{0}\\IDX\\E) Indexes DISABLED", iRIndex));

            // Events enabled/disabled
            if (psuRRecord->szEventsEnabled != NULL)
                if (toupper(psuRRecord->szEventsEnabled[0]) == 'T')
                    RRecordNode->Nodes->Add(String::Format("(R-{0}\\EV\\E) Events ENABLED", iRIndex));
                else
                    RRecordNode->Nodes->Add(String::Format("(R-{0}\\EV\\E) Events DISABLED", iRIndex));

            // Walk the R record data sources
            psuRDataSource = psuRRecord->psuFirstDataSource;
            do  {
                // Break out when there are no more
                if (psuRDataSource == NULL) break;

                iRDsiIndex = psuRDataSource->iDataSourceNum;
                TreeNode ^ RDataSourceNode = gcnew TreeNode(String::Format("(R-{0}\\DSI-{1}) Data Source ID - {2}",
                    iRIndex, iRDsiIndex,
                    Marshal::PtrToStringAnsi((System::IntPtr)psuRDataSource->szDataSourceID)));

                // Set the color based on enabled/disabled
                if ((psuRDataSource->szEnabled != NULL) &&
                    (toupper(psuRDataSource->szEnabled[0]) != 'T'))
                    RDataSourceNode->BackColor = Color::LightSalmon;
                else
                    RDataSourceNode->BackColor = Color::LightGreen;

                RRecordNode->Nodes->Add(RDataSourceNode);
                iRDsiIndex = psuRDataSource->iDataSourceNum;

                // Put up channel type and channel specific parameters
                if (psuRDataSource->szChannelDataType != NULL)
                    {
                    TreeNode ^ ChanTypeNode = gcnew TreeNode(String::Format("(R-{0}\\DST-{1}) Channel Type - {2}",
                        iRIndex, iRDsiIndex, 
                        Marshal::PtrToStringAnsi((System::IntPtr)psuRDataSource->szChannelDataType)));
                    RDataSourceNode->Nodes->Add(ChanTypeNode);

                    // PCM Attributes
                    if (strcasecmp(psuRDataSource->szChannelDataType, "PCMIN") == 0)
                        {
                        // (R-x\PDTF-n)
                        if (psuRDataSource->szPcmDataTypeFormat != NULL)
                            {
                            String  ^ PcmDataTypeFormat;
                            switch (psuRDataSource->szPcmDataTypeFormat[0])
                                {
                                case '0' : PcmDataTypeFormat = "FORMAT 0 (RESERVED)";        break;
                                case '1' : PcmDataTypeFormat = "FORMAT 1 (IRIG 106 CH 4/8)"; break;
                                default  : PcmDataTypeFormat = "UNKNOWN";                    break;
                                }
                            ChanTypeNode->Nodes->Add(String::Format("(R-{0}\\PDTF-{1}) Data Type Format - {2}",
                                iRIndex, iRDsiIndex, PcmDataTypeFormat));
                            } // end if PDTF

                        // (R-x\PDP-n)
                        if (psuRDataSource->szPcmDataPacking != NULL)
                            {
                            String  ^ PcmDataPacking;
                            if      (strcasecmp(psuRDataSource->szPcmDataPacking, "UN")  == 0) PcmDataPacking = "UNPACKED";
                            else if (strcasecmp(psuRDataSource->szPcmDataPacking, "PFS") == 0) PcmDataPacking = "PACKED WITH FRAME SYNC";
                            else if (strcasecmp(psuRDataSource->szPcmDataPacking, "TM")  == 0) PcmDataPacking = "THROUGHPUT MODE";
                            else                                                               PcmDataPacking = "UNKNOWN";
                            ChanTypeNode->Nodes->Add(String::Format("(R-{0}\\PDP-{1}) Data Packing - {2}",
                                iRIndex, iRDsiIndex, PcmDataPacking));
                            } // end if PDP

                        // (R-x\ICE-n)
                        if (psuRDataSource->szPcmInputClockEdge != NULL)
                            {
                            ChanTypeNode->Nodes->Add(String::Format("(R-{0}\\ICE-{1}) Input Clock Edge - {2} Degrees",
                                iRIndex, iRDsiIndex, 
                                Marshal::PtrToStringAnsi((System::IntPtr)psuRDataSource->szPcmInputClockEdge)));
                            } // end if ICE

                        // (R-x\IST-n)
                        if (psuRDataSource->szPcmInputSignalType != NULL)
                            {
                            String  ^ PcmInputSignalType;
                            if      (strcasecmp(psuRDataSource->szPcmInputSignalType, "SE")    == 0) PcmInputSignalType = "SINGLE ENDED";
                            else if (strcasecmp(psuRDataSource->szPcmInputSignalType, "DIFF")  == 0) PcmInputSignalType = "DIFFERENTIAL";
                            else if (strcasecmp(psuRDataSource->szPcmInputSignalType, "RS422") == 0) PcmInputSignalType = "RS-422 STANDARD DIFFERENTIAL";
                            else if (strcasecmp(psuRDataSource->szPcmInputSignalType, "TTL")   == 0) PcmInputSignalType = "SINGLE ENDED WITH TTL";
                            else                                                                     PcmInputSignalType = "UNKNOWN";
                            ChanTypeNode->Nodes->Add(String::Format("(R-{0}\\IST-{1}) Input Signal Type - {2}",
                                iRIndex, iRDsiIndex, PcmInputSignalType));
                            } // end if IST

                        // (R-x\ITH-n)
                        if (psuRDataSource->szPcmInputThreshold != NULL)
                            {
                            ChanTypeNode->Nodes->Add(String::Format("(R-{0}\\ITH-{1}) Input Threshold - {2} Volts",
                                iRIndex, iRDsiIndex, 
                                Marshal::PtrToStringAnsi((System::IntPtr)psuRDataSource->szPcmInputThreshold)));
                            } // end if ITH

                        // (R-x\ITM-n)
                        if (psuRDataSource->szPcmInputTermination != NULL)
                            {
                            ChanTypeNode->Nodes->Add(String::Format("(R-{0}\\ITM-{1}) Input Termination - {2}",
                                iRIndex, iRDsiIndex, 
                                Marshal::PtrToStringAnsi((System::IntPtr)psuRDataSource->szPcmInputTermination)));
                            } // end if ITM

                        // (R-x\PTF-n)
                        if (psuRDataSource->szPcmVideoTypeFormat != NULL)
                            {
                            ChanTypeNode->Nodes->Add(String::Format("(R-{0}\\PTF-{1}) PCM Video Type - {2}",
                                iRIndex, iRDsiIndex, 
                                Marshal::PtrToStringAnsi((System::IntPtr)psuRDataSource->szPcmVideoTypeFormat)));
                            } // end if PTF

                        } // end if PCM channel type

                    // Video Attributes
                    if (strcasecmp(psuRDataSource->szChannelDataType, "VIDIN") == 0)
                        {
                        // (R-x\VTF-n)
                        if (psuRDataSource->szVideoDataType != NULL)
                            {
                            String  ^ VideoDataType;
                            switch (psuRDataSource->szVideoDataType[0])
                                {
                                case '0' : VideoDataType = "FORMAT 0 (MPEG-2/H.264)";     break;
                                case '1' : VideoDataType = "FORMAT 1 (MPEG-2 ISO 13818)"; break;
                                case '2' : VideoDataType = "FORMAT 2 (MPEG-4 ISO 14496)"; break;
                                default  : VideoDataType = "UNKNOWN";                     break;
                                }
                            ChanTypeNode->Nodes->Add(String::Format("(R-{0}\\VTF-{1}) Video Data Format Type - {2}",
                                iRIndex, iRDsiIndex, VideoDataType));
                            } // end if VTF

                        // (R-x\VXF-n)
                        if (psuRDataSource->szVideoEncodeType != NULL)
                            {
                            String  ^ VideoEncodeType;
                            switch (psuRDataSource->szVideoEncodeType[0])
                                {
                                case '0' : VideoEncodeType = "2ON2 (MPEG-2)";  break;
                                case '1' : VideoEncodeType = "264ON2 (H.264)"; break;
                                default  : VideoEncodeType = "UNKNOWN";        break;
                                }
                            ChanTypeNode->Nodes->Add(String::Format("(R-{0}\\VXF-{1}) MPEG-2 Format - {2}",
                                iRIndex, iRDsiIndex, VideoEncodeType));
                            } // end if VXF

                        // (R-x\VST-n)
                        if (psuRDataSource->szVideoSignalType != NULL)
                            {
                            String  ^ VideoSignalType;
                            switch (psuRDataSource->szVideoSignalType[0])
                                {
                                case '0' : VideoSignalType = "AUTO DETECT"; break;
                                case '1' : VideoSignalType = "COMPOSITE";   break;
                                case '2' : VideoSignalType = "YUV";         break;
                                case '3' : VideoSignalType = "S-VIDEO";     break;
                                case '4' : VideoSignalType = "DVI";         break;
                                case '5' : VideoSignalType = "RGB";         break;
                                case '6' : VideoSignalType = "SDI";         break;
                                case '7' : VideoSignalType = "VGA";         break;
                                default  : VideoSignalType = "UNKNOWN";     break;
                                }
                            ChanTypeNode->Nodes->Add(String::Format("(R-{0}\\VST-{1}) Video Signal Input Type - {2}",
                                iRIndex, iRDsiIndex, VideoSignalType));
                            } // end if VST

                        // (R-x\VSF-n)
                        if (psuRDataSource->szVideoSignalFormat != NULL)
                            {
                            String  ^ VideoSignalFormat;
                            switch (psuRDataSource->szVideoSignalType[0])
                                {
                                case '0' : VideoSignalFormat = "AUTO DETECT"; break;
                                case '1' : VideoSignalFormat = "NTSC";        break;
                                case '2' : VideoSignalFormat = "PAL";         break;
                                case '3' : VideoSignalFormat = "ATSC";        break;
                                case '4' : VideoSignalFormat = "DVB";         break;
                                case '5' : VideoSignalFormat = "ISDB";        break;
                                case '6' : VideoSignalFormat = "SECAM";       break;
                                default  : VideoSignalFormat = "UNKNOWN";     break;
                                }
                            ChanTypeNode->Nodes->Add(String::Format("(R-{0}\\VSF-{1}) Video Signal Format - {2}",
                                iRIndex, iRDsiIndex, VideoSignalFormat));
                            } // end if VSF

/*
szVideoConstBitRate;    // (R-x\CBR-n)
szVideoVarPeakBitRate;  // (R-x\VBR-n)
szVideoEncodingDelay;   // (R-x\VED-n)
*/

                        } // end if video channel type

                    } // end if szChannelDataType not null

                if (psuRDataSource->szTrackNumber != NULL)
                    RDataSourceNode->Nodes->Add(String::Format("(R-{0}\\TK1-{1}) Track Number - {2}",
                        iRIndex, iRDsiIndex, Marshal::PtrToStringAnsi((System::IntPtr)psuRDataSource->szTrackNumber)));
                if (psuRDataSource->szEnabled != NULL)
                    RDataSourceNode->Nodes->Add(String::Format("(R-{0}\\CHE-{1}) {2}",
                        iRIndex, iRDsiIndex,
                        toupper(psuRDataSource->szEnabled[0])=='T' ? "ENABLED" : "DISABLED"));
                if (psuRDataSource->szChanDataLinkName != NULL)
                    RDataSourceNode->Nodes->Add(String::Format("(R-{0}\\CDLN-{1}) Channel Data Link Name - {2}",
                        iRIndex, iRDsiIndex, 
                        Marshal::PtrToStringAnsi((System::IntPtr)psuRDataSource->szChanDataLinkName)));

                if (psuRDataSource->psuPRecord != NULL)
                    {
                    RDataSourceNode->Nodes->Add(MakePRecordNode(psuRDataSource->psuPRecord));
                    }

                // M record
                psuMRecord = psuRDataSource->psuMRecord;
                if (psuMRecord != NULL)
                    {
                    iMIndex = psuMRecord->iRecordNum;
                    TreeNode ^ MRecordNode = gcnew TreeNode(String::Format("(M-{0}\\ID) - {1}",
                        iMIndex, Marshal::PtrToStringAnsi((System::IntPtr)psuMRecord->szDataSourceID)));

                    if (psuMRecord->szBasebandSignalType != NULL)
                        MRecordNode->Nodes->Add(String::Format("(M-{0}\\BSG1) {1}",
                            iMIndex, Marshal::PtrToStringAnsi((System::IntPtr)psuMRecord->szBasebandSignalType)));

                    if (psuMRecord->szBBDataLinkName != NULL)
                        MRecordNode->Nodes->Add(String::Format("(M-{0}\\BB\\DLN) {1}",
                            iMIndex, Marshal::PtrToStringAnsi((System::IntPtr)psuMRecord->szBBDataLinkName)));

                    RDataSourceNode->Nodes->Add(MRecordNode);

                    // P record
                    if (psuMRecord->psuPRecord != NULL)
                        {
                        MRecordNode->Nodes->Add(MakePRecordNode(psuMRecord->psuPRecord));
                        } // end if P record exists

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

TreeNode ^ MakePRecordNode(SuPRecord * psuPRecord)
    {
    int                     iPIndex;

    iPIndex = psuPRecord->iRecordNum;

    TreeNode ^ PRecordNode = gcnew TreeNode(String::Format("(P-{0}\\DLN) Data Link Name - {1}",
        iPIndex, Marshal::PtrToStringAnsi((System::IntPtr)psuPRecord->szDataLinkName)));

    if (psuPRecord->szPcmCode != NULL)
        PRecordNode->Nodes->Add(String::Format("(P-{0}\\D1) PCM Code - {1}",
            iPIndex, Marshal::PtrToStringAnsi((System::IntPtr)psuPRecord->szPcmCode)));
    if (psuPRecord->szBitsPerSec != NULL)
        PRecordNode->Nodes->Add(String::Format("(P-{0}\\D2) Bits per Second - {1}",
            iPIndex, Marshal::PtrToStringAnsi((System::IntPtr)psuPRecord->szBitsPerSec)));
    if (psuPRecord->szPolarity != NULL)
        PRecordNode->Nodes->Add(String::Format("(P-{0}\\D4) Polarity - {1}",
            iPIndex, Marshal::PtrToStringAnsi((System::IntPtr)psuPRecord->szPolarity)));
    if (psuPRecord->szTypeFormat != NULL)
        PRecordNode->Nodes->Add(String::Format("(P-{0}\\TF) Type Format - {1}",
            iPIndex, Marshal::PtrToStringAnsi((System::IntPtr)psuPRecord->szTypeFormat)));
    if (psuPRecord->szCommonWordLen != NULL)
        PRecordNode->Nodes->Add(String::Format("(P-{0}\\F1) Common Word Length - {1}",
            iPIndex, Marshal::PtrToStringAnsi((System::IntPtr)psuPRecord->szCommonWordLen)));
    if (psuPRecord->szNumMinorFrames != NULL)
        PRecordNode->Nodes->Add(String::Format("(P-{0}\\MF\\N) Number of Minor Frames - {1}",
            iPIndex, Marshal::PtrToStringAnsi((System::IntPtr)psuPRecord->szNumMinorFrames)));
    if (psuPRecord->szWordsInMinorFrame != NULL)
        PRecordNode->Nodes->Add(String::Format("(P-{0}\\MF1) Words per Minor Frame - {1}",
            iPIndex, Marshal::PtrToStringAnsi((System::IntPtr)psuPRecord->szWordsInMinorFrame)));
    if (psuPRecord->szBitsInMinorFrame != NULL)
        PRecordNode->Nodes->Add(String::Format("(P-{0}\\MF2) Bits per Minor Frame - {1}",
            iPIndex, Marshal::PtrToStringAnsi((System::IntPtr)psuPRecord->szBitsInMinorFrame)));

    return PRecordNode;
    }



/* ------------------------------------------------------------------------ */

void InputForm::DisplayChannels()
    {
    int                     iGIndex;
    int                     iRIndex;
    int                     iRDsiIndex;
    int                     bEnabled;
    SuGDataSource         * psuGDataSource;
    SuRRecord             * psuRRecord;
    SuRDataSource         * psuRDataSource;

    // Clear out the text box
    this->textChannels->Text = "";

    // Print out the TMATS info
    // ------------------------

    // G record
    this->textChannels->Text += L"Program Name - " + Marshal::PtrToStringAnsi((System::IntPtr)IrigIn->suTmatsInfo.psuFirstGRecord->szProgramName) + "\r\n";

    switch (IrigIn->suTmatsInfo.iCh10Ver)
        {
        case 0 :
            this->textChannels->Text += L"Recorder Ch 10 Version  - 05 or earlier\r\n";
            break;
        case 7 :
            this->textChannels->Text += L"Recorder Ch 10 Version  - 07\r\n";
            break;
        case 8 :
            this->textChannels->Text += L"Recorder Ch 10 Version  - 09\r\n";
            break;
        default :
            this->textChannels->Text += L"Recorder Ch 10 Version  - Unknown\r\n";
            break;
        } // end switch on Ch 10 version

    this->textChannels->Text += L"TMATS Ch 9 Version - " + Marshal::PtrToStringAnsi((System::IntPtr)IrigIn->suTmatsInfo.psuFirstGRecord->szIrig106Rev) + "\r\n";

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

                if ((psuRDataSource->szEnabled != NULL) &&
                    (toupper(psuRDataSource->szEnabled[0])=='T'))
                    bEnabled = true;
                else
                    bEnabled = false;

                iRDsiIndex = psuRDataSource->iDataSourceNum;
                this->textChannels->Text += String::Format("{0,6}   {1,-12} {2}  {3}\r\n",
                    Marshal::PtrToStringAnsi((System::IntPtr)psuRDataSource->szTrackNumber),
                    Marshal::PtrToStringAnsi((System::IntPtr)psuRDataSource->szChannelDataType),
                    bEnabled ? " Enabled " : " Disabled",
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


