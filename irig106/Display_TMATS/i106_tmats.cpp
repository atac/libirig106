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

//#include "irig106_dotnet.h"

#include "AboutDisplayTmats.h"
#include "InputForm.h"

using namespace System;
using namespace System::Collections;
using namespace System::Text;
using namespace System::Runtime::InteropServices;
using namespace System::IO;

using namespace I106Input;
using namespace Irig106DotNet;

TreeNode ^ MakePRecordNode(Irig106DotNet::Tmats::SuPRecord ^ PRecord);


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
    bool    bStatus;
    String ^ sExt;

    // Figure out if it seems to be an IRIG file or just a TMATS text file
    sExt = Path::GetExtension(txtFilename->Text);
    sExt = sExt->ToLower();
    
    // Seems to be an IRIG file so try opening it that way
    if ((sExt == ".ch10") || (sExt == ".c10"))
        bStatus = OpenAsIrigFile(txtFilename->Text);

    // Seems to be a TMATS file so try opening it that way
    else if ((sExt == ".tmt") || (sExt == ".tma") || (sExt == ".txt"))
        bStatus = OpenAsTmatsFile(txtFilename->Text);

    // Unknown file extension so try both ways
    else
        {
        bStatus = OpenAsIrigFile(txtFilename->Text);
        if (bStatus != true)
            bStatus = OpenAsTmatsFile(txtFilename->Text);
        }

    // If no TMATS found then warn the poor user
    if (bStatus == false)
        MessageBox::Show( "No valid TMATS found!", "Error",
            MessageBoxButtons::OK, MessageBoxIcon::Exclamation );

    return;
    }



// ------------------------------------------------------------------------

bool InputForm::OpenAsIrigFile(String ^ Filename)
    {
    Irig106DotNet::ReturnStatus        Status;

    Status = IrigIn->Open(Filename, Irig106DotNet::Ch10FileMode::READ);
    if (Status != Irig106DotNet::ReturnStatus::OK)
        return false;

    Status = IrigIn->ReadNextHeader();
    if (Status != Irig106DotNet::ReturnStatus::OK)
        return false;

    else
        {
        // See if it's TMATS
        if (IrigIn->Header->ubyDataType == Irig106DotNet::DataType::TMATS)
            {
            // Read the data into the buffer
            Status = IrigIn->ReadData();

            // Decode the new info
            Status = Tmats->DecodeTmats(this->IrigIn->Header, this->IrigIn->DataBuff);
            if (Status != Irig106DotNet::ReturnStatus::OK)
                return false;

            DecodeDisplayTMATS();
            } // end if TMATS

        // TMATS not found            
        else
            return false;

        } // end read header ok

    // Figure out and display start and stop times
    IrigIn->SyncTime();

    Status = IrigIn->ReadNextHeader();
    if (Status == Irig106DotNet::ReturnStatus::OK)
        {
        Irig106DotNet::IrigTime       suStartTime;
        IrigIn->Rel2IrigTime(%suStartTime);
        //szTime = IrigTime2String(%suStartTime);
        String ^ sTime;
        //sTime = String::Format("Start - {0}", Marshal::PtrToStringAnsi(System::IntPtr(szTime)));
        sTime = String::Format("Start - {0}", IrigIn->IrigTime2String(%suStartTime));
        this->statuslblStartTime->Text = sTime;
        }

    Status = IrigIn->LastMsg();
    Status = IrigIn->ReadNextHeader();
    if (Status == Irig106DotNet::ReturnStatus::OK)
        {
        Irig106DotNet::IrigTime       suStopTime;
        IrigIn->Rel2IrigTime(%suStopTime);
        String ^ sTime;
        sTime = String::Format("Stop - {0}", IrigIn->IrigTime2String(%suStopTime));
        this->statuslblStopTime->Text = sTime;
        }

    // Close the data file
    IrigIn->Close();

    return true;
    }



// ------------------------------------------------------------------------

// Open a TMATS text file, decode it, and display it
bool InputForm::OpenAsTmatsFile(String ^ Filename)
    {
    StreamReader                  ^ TmatsFile;
    Irig106DotNet::ReturnStatus     Status;

    // Open the TMATS text file for reading
    TmatsFile = gcnew StreamReader(Filename);
    Tmats->sDataBuff = TmatsFile->ReadToEnd();

    // Decode it
    Status = Tmats->DecodeTmats(Tmats->sDataBuff);
    if (Status != Irig106DotNet::ReturnStatus::OK)
        {
        return false;
        }

    // Display it
    DecodeDisplayTMATS();

    this->statuslblStartTime->Text = "";
    this->statuslblStopTime->Text  = "";

    return true;
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

    // Unformatted display
    if (this->radUnformatted->Checked)
        {
        this->textRaw->Text = Tmats->sDataBuff;
        }

    // Formatted display
    else if (this->radFormatted->Checked)
        {
        String ^ sRawText = "";
        for each(Tmats::SuTmatsLine ^ TmatsLine in Tmats->Lines)
            sRawText = sRawText + String::Format("{0}:{1};\n\r\n\r", TmatsLine->CodeName, TmatsLine->DataItem);

        this->textRaw->Text = sRawText;
        }

    // Don't know why we're here but go unformatted
    else
        {
        this->textRaw->Text = Tmats->sDataBuff;
        }

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
    Irig106DotNet::Tmats::SuRRecord       ^ RRecord;
    Irig106DotNet::Tmats::SuMRecord       ^ MRecord;

    // Clear out the tree
    this->treeTree->Nodes->Clear();

    // G record
    if (Tmats->bCsdwValid)
        switch (Tmats->iCh10Ver)
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

    if (Tmats->GRecord->sIrig106Rev != nullptr)
        this->treeTree->Nodes->Add("(G\\106) TMATS Ch 9 Version      - " + Tmats->GRecord->sIrig106Rev);

    if (Tmats->GRecord->sProgramName != nullptr)
        this->treeTree->Nodes->Add("(G\\PN)  Program Name            - " + Tmats->GRecord->sProgramName);

    // Walk the G record data sources
    for each (Irig106DotNet::Tmats::SuGDataSource ^ GDataSource in Tmats->GRecord->GDataSources)
        {
        // G record data source info
        iGIndex = GDataSource->iDataSourceNum;

        TreeNode ^ GDataSourceNode = gcnew TreeNode(String::Format("(G\\DSI-{0}) Data Source ID - {1}", 
            GDataSource->iDataSourceNum, GDataSource->sDataSourceID));
        this->treeTree->Nodes->Add(GDataSourceNode);
        GDataSourceNode->Expand();

        if (GDataSource->sDataSourceType != nullptr)
            GDataSourceNode->Nodes->Add(String::Format("(G\\DST-{0}) Data Source Type - {1}",
                GDataSource->iDataSourceNum, GDataSource->sDataSourceType));

        // R record info
        RRecord = GDataSource->RRecord;
        if (RRecord != nullptr)
            {
            iRIndex = RRecord->iRecordNum;

            // Make the R record 
            TreeNode ^ RRecordNode = gcnew TreeNode(String::Format("(R-{0}\\ID) ID - {1}",
                iRIndex, RRecord->sDataSourceID));
            GDataSourceNode->Nodes->Add(RRecordNode);
            RRecordNode->Expand();

            // Indexes enabled/disabled
            if (RRecord->sIndexEnabled != nullptr)
                if(RRecord->bIndexEnabled)
                    RRecordNode->Nodes->Add(String::Format("(R-{0}\\IDX\\E) Indexes ENABLED", iRIndex));
                else
                    RRecordNode->Nodes->Add(String::Format("(R-{0}\\IDX\\E) Indexes DISABLED", iRIndex));

            // Events enabled/disabled
            if (RRecord->sEventsEnabled != nullptr)
                if (RRecord->bEventsEnabled)
                    RRecordNode->Nodes->Add(String::Format("(R-{0}\\EV\\E) Events ENABLED", iRIndex));
                else
                    RRecordNode->Nodes->Add(String::Format("(R-{0}\\EV\\E) Events DISABLED", iRIndex));

            // Walk the R record data sources
            for each (Irig106DotNet::Tmats::SuRDataSource ^ RDataSource in RRecord->RDataSources)
                {
                iRDsiIndex = RDataSource->iDataSourceNum;
                TreeNode ^ RDataSourceNode = gcnew TreeNode(String::Format("(R-{0}\\DSI-{1}) Data Source ID - {2}",
                    iRIndex, iRDsiIndex, RDataSource->sDataSourceID));

                // Set the color based on enabled/disabled
                if ((RDataSource->sEnabled != nullptr) &&
                    (RDataSource->bEnabled == false))
                    RDataSourceNode->BackColor = Color::LightSalmon;
                else
                    RDataSourceNode->BackColor = Color::LightGreen;

                RRecordNode->Nodes->Add(RDataSourceNode);
                iRDsiIndex = RDataSource->iDataSourceNum;

                // Put up channel type and channel specific parameters
                if (RDataSource->sChannelDataType != nullptr)
                    {
                    TreeNode ^ ChanTypeNode = gcnew TreeNode(String::Format("(R-{0}\\DST-{1}) Channel Type - {2}",
                        iRIndex, iRDsiIndex, RDataSource->sChannelDataType));
                    RDataSourceNode->Nodes->Add(ChanTypeNode);

                    // PCM Attributes
                    if (String::Compare(RDataSource->sChannelDataType, "PCMIN", true) == 0)
                        {
                        // (R-x\PDTF-n)
                        if (RDataSource->sPcmDataTypeFormat != nullptr)
                            {
                            String  ^ PcmDataTypeFormat;
                            switch (RDataSource->sPcmDataTypeFormat[0])
                                {
                                case '0' : PcmDataTypeFormat = "FORMAT 0 (RESERVED)";        break;
                                case '1' : PcmDataTypeFormat = "FORMAT 1 (IRIG 106 CH 4/8)"; break;
                                default  : PcmDataTypeFormat = "UNKNOWN";                    break;
                                }
                            ChanTypeNode->Nodes->Add(String::Format("(R-{0}\\PDTF-{1}) Data Type Format - {2}",
                                iRIndex, iRDsiIndex, PcmDataTypeFormat));
                            } // end if PDTF

                        // (R-x\PDP-n)
                        if (RDataSource->sPcmDataPacking != nullptr)
                            {
                            String  ^ PcmDataPacking;
                            if      (String::Compare(RDataSource->sPcmDataPacking, "UN",  true) == 0) PcmDataPacking = "UNPACKED";
                            else if (String::Compare(RDataSource->sPcmDataPacking, "PFS", true) == 0) PcmDataPacking = "PACKED WITH FRAME SYNC";
                            else if (String::Compare(RDataSource->sPcmDataPacking, "TM",  true) == 0) PcmDataPacking = "THROUGHPUT MODE";
                            else                                                                 PcmDataPacking = "UNKNOWN";
                            ChanTypeNode->Nodes->Add(String::Format("(R-{0}\\PDP-{1}) Data Packing - {2}",
                                iRIndex, iRDsiIndex, PcmDataPacking));
                            } // end if PDP

                        // (R-x\ICE-n)
                        if (RDataSource->sPcmInputClockEdge != nullptr)
                            {
                            ChanTypeNode->Nodes->Add(String::Format("(R-{0}\\ICE-{1}) Input Clock Edge - {2} Degrees",
                                iRIndex, iRDsiIndex, RDataSource->sPcmInputClockEdge));
                            } // end if ICE

                        // (R-x\IST-n)
                        if (RDataSource->sPcmInputSignalType != nullptr)
                            {
                            String  ^ PcmInputSignalType;
                            if      (String::Compare(RDataSource->sPcmInputSignalType, "SE")    == 0) PcmInputSignalType = "SINGLE ENDED";
                            else if (String::Compare(RDataSource->sPcmInputSignalType, "DIFF")  == 0) PcmInputSignalType = "DIFFERENTIAL";
                            else if (String::Compare(RDataSource->sPcmInputSignalType, "RS422") == 0) PcmInputSignalType = "RS-422 STANDARD DIFFERENTIAL";
                            else if (String::Compare(RDataSource->sPcmInputSignalType, "TTL")   == 0) PcmInputSignalType = "SINGLE ENDED WITH TTL";
                            else                                                                      PcmInputSignalType = "UNKNOWN";
                            ChanTypeNode->Nodes->Add(String::Format("(R-{0}\\IST-{1}) Input Signal Type - {2}",
                                iRIndex, iRDsiIndex, PcmInputSignalType));
                            } // end if IST

                        // (R-x\ITH-n)
                        if (RDataSource->sPcmInputThreshold != nullptr)
                            {
                            ChanTypeNode->Nodes->Add(String::Format("(R-{0}\\ITH-{1}) Input Threshold - {2} Volts",
                                iRIndex, iRDsiIndex, RDataSource->sPcmInputThreshold));
                            } // end if ITH

                        // (R-x\ITM-n)
                        if (RDataSource->sPcmInputTermination != nullptr)
                            {
                            ChanTypeNode->Nodes->Add(String::Format("(R-{0}\\ITM-{1}) Input Termination - {2}",
                                iRIndex, iRDsiIndex, RDataSource->sPcmInputTermination));
                            } // end if ITM

                        // (R-x\PTF-n)
                        if (RDataSource->sPcmVideoTypeFormat != nullptr)
                            {
                            ChanTypeNode->Nodes->Add(String::Format("(R-{0}\\PTF-{1}) PCM Video Type - {2}",
                                iRIndex, iRDsiIndex, RDataSource->sPcmVideoTypeFormat));
                            } // end if PTF

                        } // end if PCM channel type

                    // Video Attributes
                    if (String::Compare(RDataSource->sChannelDataType, "VIDIN", true) == 0)
                        {
                        // (R-x\VTF-n)
                        if (RDataSource->sVideoDataType != nullptr)
                            {
                            String  ^ VideoDataType;
                            switch (RDataSource->sVideoDataType[0])
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
                        if (RDataSource->sVideoEncodeType != nullptr)
                            {
                            String  ^ VideoEncodeType;
                            switch (RDataSource->sVideoEncodeType[0])
                                {
                                case '0' : VideoEncodeType = "2ON2 (MPEG-2)";  break;
                                case '1' : VideoEncodeType = "264ON2 (H.264)"; break;
                                default  : VideoEncodeType = "UNKNOWN";        break;
                                }
                            ChanTypeNode->Nodes->Add(String::Format("(R-{0}\\VXF-{1}) MPEG-2 Format - {2}",
                                iRIndex, iRDsiIndex, VideoEncodeType));
                            } // end if VXF

                        // (R-x\VST-n)
                        if (RDataSource->sVideoSignalType != nullptr)
                            {
                            String  ^ VideoSignalType;
                            switch (RDataSource->sVideoSignalType[0])
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
                        if (RDataSource->sVideoSignalFormat != nullptr)
                            {
                            String  ^ VideoSignalFormat;
                            switch (RDataSource->sVideoSignalType[0])
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

                    // Analog Attributes
                    if (String::Compare(RDataSource->sChannelDataType, "ANAIN", true) == 0)
                        {
                        // (R-x\ACH\N-n)
                        if (RDataSource->sAnalogChansPerPkt != nullptr)
                            {
                            ChanTypeNode->Nodes->Add(String::Format("(R-{0}\\ACH\\N-{1}) Analog Channels per Packet - {2}",
                                iRIndex, iRDsiIndex, RDataSource->sAnalogChansPerPkt));
                            } // end if ACH\N

                        // (R-1\ASR-n)
                        if (RDataSource->sAnalogSampleRate != nullptr)
                            {
                            ChanTypeNode->Nodes->Add(String::Format("(R-{0}\\ASR-{1}) Analog Sample Rate - {2}",
                                iRIndex, iRDsiIndex, RDataSource->sAnalogSampleRate));
                            } // end if ASR

                        // (R-x\ADP-n)
                        if (RDataSource->sAnalogDataPacking != nullptr)
                            {
                            ChanTypeNode->Nodes->Add(String::Format("(R-{0}\\ADP-{1}) Analog Data Packing - {2}",
                                iRIndex, iRDsiIndex, RDataSource->sAnalogDataPacking));
                            } // end if ACH\N

                        } // end if analog channel type

                    } // end if szChannelDataType not null

                if (RDataSource->sTrackNumber != nullptr)
                    RDataSourceNode->Nodes->Add(String::Format("(R-{0}\\TK1-{1}) Track Number - {2}",
                        iRIndex, iRDsiIndex, RDataSource->sTrackNumber));
                if (RDataSource->sEnabled != nullptr)
                    RDataSourceNode->Nodes->Add(String::Format("(R-{0}\\CHE-{1}) {2}",
                        iRIndex, iRDsiIndex, RDataSource->bEnabled ? "ENABLED" : "DISABLED"));
                if (RDataSource->sChanDataLinkName != nullptr)
                    RDataSourceNode->Nodes->Add(String::Format("(R-{0}\\CDLN-{1}) Channel Data Link Name - {2}",
                        iRIndex, iRDsiIndex, RDataSource->sChanDataLinkName));

                if (RDataSource->PRecord != nullptr)
                    {
                    RDataSourceNode->Nodes->Add(MakePRecordNode(RDataSource->PRecord));
                    }

                // M record
                MRecord = RDataSource->MRecord;
                if (MRecord != nullptr)
                    {
                    iMIndex = MRecord->iRecordNum;
                    TreeNode ^ MRecordNode = gcnew TreeNode(String::Format("(M-{0}\\ID) - {1}",
                        iMIndex, MRecord->sDataSourceID));

                    if (MRecord->sBasebandSignalType != nullptr)
                        MRecordNode->Nodes->Add(String::Format("(M-{0}\\BSG1) {1}",
                            iMIndex, MRecord->sBasebandSignalType));

                    if (MRecord->sBBDataLinkName != nullptr)
                        MRecordNode->Nodes->Add(String::Format("(M-{0}\\BB\\DLN) {1}",
                            iMIndex, MRecord->sBBDataLinkName));

                    RDataSourceNode->Nodes->Add(MRecordNode);

                    // P record
                    if (MRecord->PRecord != nullptr)
                        {
                        MRecordNode->Nodes->Add(MakePRecordNode(MRecord->PRecord));
                        } // end if P record exists

                    } // end if M record exists

                } // for each RDataSource

            } // end if R record exists

        } // end for each GDataSource

    Update();

    return;
    }



/* ------------------------------------------------------------------------ */

TreeNode ^ MakePRecordNode(Irig106DotNet::Tmats::SuPRecord ^ PRecord)

    {
    int                     iPIndex;

    iPIndex = PRecord->iRecordNum;

    TreeNode ^ PRecordNode = gcnew TreeNode(String::Format("(P-{0}\\DLN) Data Link Name - {1}",
        iPIndex, PRecord->sDataLinkName));

    if (PRecord->sPcmCode != nullptr)
        PRecordNode->Nodes->Add(String::Format("(P-{0}\\D1) PCM Code - {1}",
            iPIndex, PRecord->sPcmCode));
    if (PRecord->sBitsPerSec != nullptr)
        PRecordNode->Nodes->Add(String::Format("(P-{0}\\D2) Bits per Second - {1}",
            iPIndex, PRecord->sBitsPerSec));
    if (PRecord->sPolarity != nullptr)
        PRecordNode->Nodes->Add(String::Format("(P-{0}\\D4) Polarity - {1}",
            iPIndex, PRecord->sPolarity));
    if (PRecord->sTypeFormat != nullptr)
        PRecordNode->Nodes->Add(String::Format("(P-{0}\\TF) Type Format - {1}",
            iPIndex, PRecord->sTypeFormat));
    if (PRecord->sCommonWordLen != nullptr)
        PRecordNode->Nodes->Add(String::Format("(P-{0}\\F1) Common Word Length - {1}",
            iPIndex, PRecord->sCommonWordLen));
    if (PRecord->sNumMinorFrames != nullptr)
        PRecordNode->Nodes->Add(String::Format("(P-{0}\\MF\\N) Number of Minor Frames - {1}",
            iPIndex, PRecord->sNumMinorFrames));
    if (PRecord->sWordsInMinorFrame != nullptr)
        PRecordNode->Nodes->Add(String::Format("(P-{0}\\MF1) Words per Minor Frame - {1}",
            iPIndex, PRecord->sWordsInMinorFrame));
    if (PRecord->sBitsInMinorFrame != nullptr)
        PRecordNode->Nodes->Add(String::Format("(P-{0}\\MF2) Bits per Minor Frame - {1}",
            iPIndex, PRecord->sBitsInMinorFrame));
    if (PRecord->sMinorFrameSyncType != nullptr)
        PRecordNode->Nodes->Add(String::Format("(P-{0}\\MF3) Minor Frame Sync Type - {1}",
            iPIndex, PRecord->sMinorFrameSyncType));
    if (PRecord->sMinorFrameSyncPatLen != nullptr)
        {
        PRecordNode->Nodes->Add(String::Format("(P-{0}\\MF4) Minor Frame Sync Pattern Length - {1}",
            iPIndex, PRecord->sMinorFrameSyncPatLen));
        }
    if (PRecord->sMinorFrameSyncPatLen != nullptr)
        {
        // Make a hex version of Frame Sync Pattern to make Ron VK happy
        Int64 iPower = 1;
        Int64 iPattern = 0;
        String ^ sPattern;
        sPattern = PRecord->sMinorFrameSyncPat;
        for (int iIdx=sPattern->Length-1; iIdx>=0; iIdx--)
            {
            if (sPattern[iIdx] == '1')
                iPattern += iPower;
            iPower = iPower << 1;
            } // end for all characters
        PRecordNode->Nodes->Add(String::Format("(P-{0}\\MF5) Minor Frame Sync Pattern - {1} (0x{2:X})",
            iPIndex, PRecord->sMinorFrameSyncPat, iPattern));
        }

    return PRecordNode;
    }



/* ------------------------------------------------------------------------ */

void InputForm::DisplayChannels()
    {
    int                     iGIndex;
    int                     iRIndex;
    int                     iRDsiIndex;
    int                     bEnabled;
    Irig106DotNet::Tmats::SuRRecord           ^ RRecord;

    // Clear out the text box
    this->textChannels->Text = "";

    // Print out the TMATS info
    // ------------------------

    // G record
    this->textChannels->Text += L"Program Name - " + Tmats->GRecord->sProgramName + "\r\n";

    if (Tmats->bCsdwValid)
        switch (Tmats->iCh10Ver)
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

    this->textChannels->Text += L"TMATS Ch 9 Version - " + Tmats->GRecord->sIrig106Rev + "\r\n";

    this->textChannels->Text += L"Channel  Type          Enabled   Data Source         \r\n";
    this->textChannels->Text += L"-------  ------------  --------  --------------------\r\n";

    // Data sources
    for each (Irig106DotNet::Tmats::SuGDataSource ^ GDataSource in Tmats->GRecord->GDataSources)
        {

        // G record data source info
        iGIndex = GDataSource->iDataSourceNum;

        // R record info
        RRecord = GDataSource->RRecord;
        if (RRecord != nullptr)
            {
            iRIndex = RRecord->iRecordNum;

            // R record data sources
            for each (Irig106DotNet::Tmats::SuRDataSource ^ RDataSource in RRecord->RDataSources)
                {
                if ((RDataSource->sEnabled != nullptr) &&
                    (RDataSource->bEnabled == false  ))
                    bEnabled = false;
                else
                    bEnabled = true;

                iRDsiIndex = RDataSource->iDataSourceNum;
                this->textChannels->Text += String::Format("{0,6}   {1,-12} {2}  {3}\r\n",
                    RDataSource->sTrackNumber,
                    RDataSource->sChannelDataType,
                    bEnabled ? " Enabled " : " Disabled",
                    RDataSource->sDataSourceID);
                } // end for each RDataSource

            } // end if RRecord not null

        } // end for each GDataSource

    // Update the dialog box display"
    Update();

    return;
    }


