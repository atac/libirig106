/****************************************************************************

 i106_dotnet_tmats.h - 

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

#pragma once

using namespace System::Collections::Generic;

namespace Irig106DotNet 
    {

	public ref class Tmats
	    {

        // Data structures
        // ===============

        public:

        // P Records
        // ---------

        ref struct SuPRecord
            {
            int                         iRecordNum;             // P-x
            String                    ^ sDataLinkName;          // P-x\DLN
            String                    ^ sPcmCode;               // P-x\D1
            String                    ^ sBitsPerSec;            // P-x\D2
            String                    ^ sPolarity;              // P-x\D4
            String                    ^ sTypeFormat;            // P-x\TF
            String                    ^ sCommonWordLen;         // P-x\F1
            String                    ^ sNumMinorFrames;        // P-x\MF\N
            String                    ^ sWordsInMinorFrame;     // P-x\MF1
            String                    ^ sBitsInMinorFrame;      // P-x\MF2
            String                    ^ sMinorFrameSyncType;    // P-x\MF3
            String                    ^ sMinorFrameSyncPatLen;  // P-x\MF4
            String                    ^ sMinorFrameSyncPat;     // P-x\MF5
            };

        // B Records
        // ---------

        ref struct SuBRecord
            {
            int                         iRecordNum;             // B-x
            String                    ^ sDataLinkName;          // B-x\DLN
            int                         iNumBuses;              // B-x\NBS\N
            };


        // M Records
        // ---------

        ref struct SuMRecord
            {
            int                                 iRecordNum;             // M-x
            String                            ^ sDataSourceID;          // M-x\ID
            String                            ^ sBBDataLinkName;        // M-x\BB\DLN
            String                            ^ sBasebandSignalType;    // M-x\BSG1
            int                                 iPRecordNum;            // Corresponding P record number
            SuPRecord                         ^ PRecord;                // Corresponding P record
            int                                 iBRecordNum;            // Corresponding P record number
            SuBRecord                         ^ BRecord;                // Corresponding B record
            };


        // R Records
        // ---------

        // R record data source
        ref struct SuRDataSource
            {
            int                                 iDataSourceNum;         // R-x\XXX-n
            String                            ^ sDataSourceID;          // R-x\DSI-n
            String                            ^ sChannelDataType;       // R-x\CDT-n
            String                            ^ sTrackNumber;           // R-x\TK1-n
            int                                 iTrackNumber;           // Only valid if sTrackNumber != NULL
            String                            ^ sEnabled;               // R-x\CHE-n
            bool                                bEnabled;               // Only valid if sEnabled != NULL
            String                            ^ sPcmDataLinkName;       // R-x\PDLN-n (-04, -05)
            String                            ^ sBusDataLinkName;       // R-x\BDLN-n (-04, -05)
            String                            ^ sChanDataLinkName;      // R-x\CDLN-n (-07, -09)
            // Video channel attributes
            String                            ^ sVideoDataType;         // (R-x\VTF-n)
            String                            ^ sVideoEncodeType;       // (R-x\VXF-n)
            String                            ^ sVideoSignalType;       // (R-x\VST-n)
            String                            ^ sVideoSignalFormat;     // (R-x\VSF-n)
            String                            ^ sVideoConstBitRate;     // (R-x\CBR-n)
            String                            ^ sVideoVarPeakBitRate;   // (R-x\VBR-n)
            String                            ^ sVideoEncodingDelay;    // (R-x\VED-n)
            // PCM channel attributes
            String                            ^ sPcmDataTypeFormat;     // (R-x\PDTF-n)
            String                            ^ sPcmDataPacking;        // (R-x\PDP-n)
            String                            ^ sPcmInputClockEdge;     // (R-x\ICE-n)
            String                            ^ sPcmInputSignalType;    // (R-x\IST-n)
            String                            ^ sPcmInputThreshold;     // (R-x\ITH-n)
            String                            ^ sPcmInputTermination;   // (R-x\ITM-n)
            String                            ^ sPcmVideoTypeFormat;    // (R-x\PTF-n)
            // Analog channel attributes
            String                            ^ sAnalogChansPerPkt;     // (R-1\ACH\N-n)
            String                            ^ sAnalogSampleRate;      // (R-1\ASR-n)
            String                            ^ sAnalogDataPacking;     // (R-1\ADP-n)
            // Links to other records
            int                                 iMRecordNum;            // Corresponding R record number
            SuMRecord                         ^ MRecord;                // Corresponding M record
            int                                 iPRecordNum;            // Corresponding P record number
            SuPRecord                         ^ PRecord;                // Corresponding P record
            };

        // R record
        ref struct SuRRecord
            {
            int                                 iRecordNum;             // R-x
            String                            ^ sDataSourceID;          // R-x\ID
            String                            ^ sNumDataSources;        // R-x\N
            String                            ^ sIndexEnabled;          // R-x\IDX\E
            bool                                bIndexEnabled;          // Only valid if sIndexEnabled != NULL
            String                            ^ sEventsEnabled;         // R-x\EVE\E
            bool                                bEventsEnabled;         // Only valid if sEventsEnabled != NULL
            List<SuRDataSource ^>               RDataSources;
            };


        // G Records
        // ---------

        // G record, data source
        ref struct SuGDataSource
            {
            int                                 iDataSourceNum;         // G\XXX-n
            String                            ^ sDataSourceID;          // G\DSI-n
            String                            ^ sDataSourceType;        // G\DST-n
            SuRRecord                         ^ RRecord;                // Corresponding R record
            int                                 iRRecordNum;            // Corresponding R record number
            };

        // G record
        ref struct SuGRecord
            {
            String                            ^ sProgramName;           // G\PN
            String                            ^ sIrig106Rev;            // G\106
            String                            ^ sNumDataSources;        // G\DSI\N
            List<SuGDataSource ^>               GDataSources;
            };

        // A TMATS line
        // ------------

        ref struct SuTmatsLine
            {
            String                            ^ CodeName;   // Left hand side of TMATS line
            String                            ^ DataItem;   // Right hand side of TMATS line
            };

        // Class data
        // ==========

        public:

        List<SuTmatsLine ^>                   ^ Lines;
        String                                ^ sDataBuff;

        // CSDW stuff
        bool                                    bCsdwValid;
        int                                     iCh10Ver;
        int                                     bConfigChange;

        int                                     TmatsVersion;

        SuGRecord                             ^ GRecord;
        List<SuRRecord ^>                       RRecords;
        List<SuMRecord ^>                       MRecords;
        List<SuBRecord ^>                       BRecords;
        List<SuPRecord ^>                       PRecords;

        // Constructor / Destructor
        // ========================

        public:

        Tmats(void);

        ~Tmats(void);


        // Methods
        // =======

        public:

        ReturnStatus DecodeTmats(Irig106DotNet::SuI106Ch10Header ^ Header, array<SByte> ^ DataBuff);
        ReturnStatus DecodeTmats(String ^ sDataBuff);

        }; // end class Tmats

    } // end namespace Irig106DotNet


