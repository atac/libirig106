/****************************************************************************

 irig106_dotnet_tmats.cpp - A .NET TMATS reader module

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

using namespace System;
using namespace System::Diagnostics;
using namespace System::Text;
using namespace System::Runtime::InteropServices;

#include "irig106_dotnet.h"
#include "irig106_dotnet_tmats.h"

#include <stdlib.h>
#include <memory.h>
#include "config.h"
#include "irig106ch10.h"
#include "i106_decode_tmats.h"

using namespace Irig106DotNet;

// Local functions
// ---------------
void Copy_GRecord     (Irig106::SuGRecord     * pFirstGRecord,          Tmats::SuGRecord       ^% GRecord);
void Copy_GDataSources(Irig106::SuGDataSource * pFirstGDataSource, List<Tmats::SuGDataSource ^> % GDataSources);
void Copy_RRecords    (Irig106::SuRRecord     * pFirstRRecord,     List<Tmats::SuRRecord     ^> % RRecords);
void Copy_RDataSources(Irig106::SuRDataSource * pFirstRDataSource, List<Tmats::SuRDataSource ^> % RDataSources);
void Copy_MRecords    (Irig106::SuMRecord     * pFirstMRecord,     List<Tmats::SuMRecord     ^> % MRecords);
void Copy_BRecords    (Irig106::SuBRecord     * pFirstBRecord,     List<Tmats::SuBRecord     ^> % BRecords);
void Copy_PRecords    (Irig106::SuPRecord     * pFirstPRecord,     List<Tmats::SuPRecord     ^> % PRecords);

void ConnectG(Irig106DotNet::Tmats ^ TmatsInfo);
void ConnectR(Irig106DotNet::Tmats ^ TmatsInfo);
void ConnectM(Irig106DotNet::Tmats ^ TmatsInfo);

void TmatsBufferToString(array<SByte> ^ DataBuff, int DataLen, String ^% StringBuff);
void TmatsBufferToLines (String ^ sDataBuff, List<Tmats::SuTmatsLine ^> ^ Lines);


namespace Irig106DotNet
    {
    namespace DLL
        {
// MAKE psuTmatsInfo A VOID POINTER SO I CAN GATHER THESE DLL DEFS INTO ONE FILE, AND DO IT
// WITHOUT DRAGGING IN ALL THE REGULAR IRIG LIBRARY HEADERS
        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" Irig106DotNet::ReturnStatus enI106_Decode_Tmats(
//                  Irig106::SuI106Ch10Header           * Header,
                    Irig106DotNet::SuI106Ch10Header     ^ Header,
                    void                                * pvBuff,
                    Irig106::SuTmatsInfo                * psuTmatsInfo);

        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" Irig106DotNet::ReturnStatus enI106_Decode_Tmats_Text(
                    void                                * pvBuff,
                    uint32_t                              ulDataLen,
                    Irig106::SuTmatsInfo                * psuTmatsInfo);

        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" void enI106_Free_TmatsInfo(Irig106::SuTmatsInfo * psuTmatsInfo);

        [DllImport("irig106.dll", CharSet=CharSet::Ansi)]
            extern "C" Irig106DotNet::ReturnStatus enI106_Tmats_Signature(
                    void                                * pvBuff,
                    uint32_t                              ulDataLen,
                    int                                   iSigVersion,
                    int                                   iSigFlags,
                    uint8_t                             * piOpCode,
                    uint32_t                            * piSignature);
 
        } // end namespace DLL

    } // end Irig106DotNet namespace



namespace Irig106DotNet 
{
//=========================================================================

// Constructor / destructor

Tmats::Tmats(void)
    {
    bCsdwValid = false;
    }

Tmats::~Tmats(void)
    {
    }



//=========================================================================

// Copy a standard C szString to a .NET sString
#define COPY_STRING(szString, sString)                                  \
    if (szString != NULL)                                               \
        sString = Marshal::PtrToStringAnsi((System::IntPtr)szString);



// Decode a TMATS buffer as read from a Ch 10 data file, including the CSDW
ReturnStatus Tmats::DecodeTmats(Irig106DotNet::SuI106Ch10Header ^ Header, array<SByte> ^ DataBuff)
    {
    // IRIG DLL data and structures
    Irig106::SuTmatsInfo            suTmatsInfo;

    // .NET data and structures
    Irig106DotNet::ReturnStatus     Status;

    // Convert buffer to a .NET string
    this->sDataBuff = "";
    TmatsBufferToString(DataBuff, Header->ulDataLen, this->sDataBuff);

    // Convert buffer to a bunch of lines
    this->Lines = gcnew List<SuTmatsLine ^>;
    TmatsBufferToLines(sDataBuff, this->Lines);

    // Decode the TMATS buffer into the linked tree
    memset(&suTmatsInfo, 0, sizeof(Irig106::SuTmatsInfo));
    pin_ptr<SByte>                           pDataBuff = &DataBuff[0];
    Status = DLL::enI106_Decode_Tmats(Header, pDataBuff, &suTmatsInfo);

    if (Status != ReturnStatus::OK)
        return Status;

    // Copy the info over to our .NET tree
    bCsdwValid = true;
    iCh10Ver         = suTmatsInfo.iCh10Ver;
    bConfigChange    = suTmatsInfo.bConfigChange;

    // Get the various record types
    Copy_GRecord (suTmatsInfo.psuFirstGRecord, this->GRecord);
    Copy_RRecords(suTmatsInfo.psuFirstRRecord, this->RRecords);
    Copy_MRecords(suTmatsInfo.psuFirstMRecord, this->MRecords);
    Copy_BRecords(suTmatsInfo.psuFirstBRecord, this->BRecords);
    Copy_PRecords(suTmatsInfo.psuFirstPRecord, this->PRecords);

    // Link the records
    ConnectG(this);
    ConnectR(this);
    ConnectM(this);

    // Free up memory the decoder might have malloc'ed
    DLL::enI106_Free_TmatsInfo(&suTmatsInfo);

    return Status;
    }



// ------------------------------------------------------------------------

// Decode a TMATS buffer as read from text file, no CSDW
ReturnStatus Tmats::DecodeTmats(String ^ sDataBuff)
    {
    // IRIG DLL data and structures
    Irig106::SuTmatsInfo            suTmatsInfo;

    // .NET data and structures
    Irig106DotNet::ReturnStatus     Status;

    // Convert buffer to a bunch of lines
    this->Lines = gcnew List<SuTmatsLine ^>;
    TmatsBufferToLines(sDataBuff, this->Lines);

    // Decode the TMATS buffer into the linked tree
    memset(&suTmatsInfo, 0, sizeof(Irig106::SuTmatsInfo));

    char * szDataBuff = (char *)(void *)Marshal::StringToHGlobalAnsi(sDataBuff);
    Status = DLL::enI106_Decode_Tmats_Text(szDataBuff, sDataBuff->Length, &suTmatsInfo);
	Marshal::FreeHGlobal((System::IntPtr)(void*)szDataBuff);

    if (Status != ReturnStatus::OK)
        return Status;

    bCsdwValid = false;

    // Get the various record types
    Copy_GRecord (suTmatsInfo.psuFirstGRecord, this->GRecord);
    Copy_RRecords(suTmatsInfo.psuFirstRRecord, this->RRecords);
    Copy_MRecords(suTmatsInfo.psuFirstMRecord, this->MRecords);
    Copy_BRecords(suTmatsInfo.psuFirstBRecord, this->BRecords);
    Copy_PRecords(suTmatsInfo.psuFirstPRecord, this->PRecords);

    // Link the records
    ConnectG(this);
    ConnectR(this);
    ConnectM(this);

    // Free up memory the decoder might have malloc'ed
    DLL::enI106_Free_TmatsInfo(&suTmatsInfo);

    return Status;
    }

}; // end Irig106DotNet namespace



// ------------------------------------------------------------------------
// G Records
// ------------------------------------------------------------------------

void Copy_GRecord(Irig106::SuGRecord * pFirstGRecord, Tmats::SuGRecord ^% GRecord)
    {
    Irig106::SuGRecord      * pGRecord;

    pGRecord = pFirstGRecord;

    GRecord = gcnew Tmats::SuGRecord;
    COPY_STRING(pGRecord->szProgramName,    GRecord->sProgramName)
    COPY_STRING(pGRecord->szIrig106Rev,     GRecord->sIrig106Rev)
    COPY_STRING(pGRecord->szNumDataSources, GRecord->sNumDataSources)

    // Go get a copy of the G record data sources
    Copy_GDataSources(pGRecord->psuFirstGDataSource, GRecord->GDataSources);

    } // end Copy_GRecords()



// ------------------------------------------------------------------------

void Copy_GDataSources(Irig106::SuGDataSource * pFirstGDataSource, List<Tmats::SuGDataSource ^> % GDataSources)
    {
    Irig106::SuGDataSource      *  pGDataSource;
    Tmats::SuGDataSource        ^ GDataSource;

    pGDataSource = pFirstGDataSource;

    while (pGDataSource != NULL)
        {
        // Make a new data source record
        GDataSource = gcnew Tmats::SuGDataSource;

        // Copy the data
        GDataSource->iDataSourceNum = pGDataSource->iDataSourceNum;

        COPY_STRING(pGDataSource->szDataSourceID,   GDataSource->sDataSourceID)
        COPY_STRING(pGDataSource->szDataSourceType, GDataSource->sDataSourceType)

        // Keep track of any links to other records
        if (pGDataSource->psuRRecord != NULL)
            GDataSource->iRRecordNum = pGDataSource->psuRRecord->iRecordNum;
        else
            GDataSource->iRRecordNum = 0;

        // Add the record to the list
        GDataSources.Add(GDataSource);

        // Next G data source
        pGDataSource = pGDataSource->psuNextGDataSource;
        }

    } // end Copy_GDataSources()



// ------------------------------------------------------------------------

void ConnectG(Irig106DotNet::Tmats ^ TmatsInfo)
    {
    // Step through the G data source records
    for each (Tmats::SuGDataSource ^ CurrGDataSrc in TmatsInfo->GRecord->GDataSources)
        {
        if (CurrGDataSrc->iRRecordNum != 0)
            {
            // Step through R records
            for each (Tmats::SuRRecord ^ CurrRRecord in TmatsInfo->RRecords)
                if (CurrGDataSrc->iRRecordNum == CurrRRecord->iRecordNum)
                    CurrGDataSrc->RRecord = CurrRRecord;
            } // end if good record number
        } // end for each G data source

    return;

    }



// ------------------------------------------------------------------------
// R Records
// ------------------------------------------------------------------------

void Copy_RRecords(Irig106::SuRRecord * pFirstRRecord, List<Tmats::SuRRecord ^> % RRecords)
    {
    Irig106::SuRRecord      *  pRRecord;
    Tmats::SuRRecord        ^  RRecord;

    pRRecord = pFirstRRecord;

    while (pRRecord != NULL)
        {
        // Make a new record
        RRecord = gcnew Tmats::SuRRecord;

        // Copy the data
        RRecord->iRecordNum     =  pRRecord->iRecordNum;
        RRecord->bIndexEnabled  = (pRRecord->bIndexEnabled  != 0);
        RRecord->bEventsEnabled = (pRRecord->bEventsEnabled != 0);

        COPY_STRING(pRRecord->szDataSourceID,   RRecord->sDataSourceID)
        COPY_STRING(pRRecord->szNumDataSources, RRecord->sNumDataSources)
        COPY_STRING(pRRecord->szIndexEnabled,   RRecord->sIndexEnabled)
        COPY_STRING(pRRecord->szEventsEnabled,  RRecord->sEventsEnabled)

        // Go get a copy of the R data sources
        Copy_RDataSources(pRRecord->psuFirstDataSource, RRecord->RDataSources);

        // Add the record to the list
        RRecords.Add(RRecord);

        // Next R record
        pRRecord = pRRecord->psuNextRRecord;
        }

    } // end Copy_RRecords()


// ------------------------------------------------------------------------

void Copy_RDataSources(Irig106::SuRDataSource * pFirstRDataSource, List<Tmats::SuRDataSource ^> % RDataSources)
    {
    Irig106::SuRDataSource      *  pRDataSource;
    Tmats::SuRDataSource        ^  RDataSource;

    pRDataSource = pFirstRDataSource;

    while (pRDataSource != NULL)
        {
        RDataSource = gcnew Tmats::SuRDataSource;

        RDataSource->iDataSourceNum =  pRDataSource->iDataSourceNum;
        RDataSource->iTrackNumber   =  pRDataSource->iTrackNumber;
        RDataSource->bEnabled       = (pRDataSource->bEnabled != 0);

        COPY_STRING(pRDataSource->szDataSourceID,        RDataSource->sDataSourceID)
        COPY_STRING(pRDataSource->szChannelDataType,     RDataSource->sChannelDataType)
        COPY_STRING(pRDataSource->szTrackNumber,         RDataSource->sTrackNumber)
        COPY_STRING(pRDataSource->szEnabled,             RDataSource->sEnabled)
        COPY_STRING(pRDataSource->szPcmDataLinkName,     RDataSource->sPcmDataLinkName)
        COPY_STRING(pRDataSource->szBusDataLinkName,     RDataSource->sBusDataLinkName)
        COPY_STRING(pRDataSource->szChanDataLinkName,    RDataSource->sChanDataLinkName)

        // Video channel attributes
        COPY_STRING(pRDataSource->szVideoDataType,       RDataSource->sVideoDataType)
        COPY_STRING(pRDataSource->szVideoEncodeType,     RDataSource->sVideoEncodeType)
        COPY_STRING(pRDataSource->szVideoSignalType,     RDataSource->sVideoSignalType)
        COPY_STRING(pRDataSource->szVideoSignalFormat,   RDataSource->sVideoSignalFormat)
        COPY_STRING(pRDataSource->szVideoConstBitRate,   RDataSource->sVideoConstBitRate)
        COPY_STRING(pRDataSource->szVideoVarPeakBitRate, RDataSource->sVideoVarPeakBitRate)
        COPY_STRING(pRDataSource->szVideoEncodingDelay,  RDataSource->sVideoEncodingDelay)

        // PCM channel attributes
        COPY_STRING(pRDataSource->szPcmDataTypeFormat,   RDataSource->sPcmDataTypeFormat)
        COPY_STRING(pRDataSource->szPcmDataPacking,      RDataSource->sPcmDataPacking)
        COPY_STRING(pRDataSource->szPcmInputClockEdge,   RDataSource->sPcmInputClockEdge)
        COPY_STRING(pRDataSource->szPcmInputSignalType,  RDataSource->sPcmInputSignalType)
        COPY_STRING(pRDataSource->szPcmInputThreshold,   RDataSource->sPcmInputThreshold)
        COPY_STRING(pRDataSource->szPcmInputTermination, RDataSource->sPcmInputTermination)
        COPY_STRING(pRDataSource->szPcmVideoTypeFormat,  RDataSource->sPcmVideoTypeFormat)

        // Analog channel attributes
        COPY_STRING(pRDataSource->szAnalogChansPerPkt,   RDataSource->sAnalogChansPerPkt)
        COPY_STRING(pRDataSource->szAnalogSampleRate,    RDataSource->sAnalogSampleRate)
        COPY_STRING(pRDataSource->szAnalogDataPacking,   RDataSource->sAnalogDataPacking)

        // Keep track of any links to other records
        if (pRDataSource->psuMRecord != NULL)
            RDataSource->iMRecordNum = pRDataSource->psuMRecord->iRecordNum;
        else
            RDataSource->iMRecordNum = 0;

        if (pRDataSource->psuPRecord != NULL)
            RDataSource->iPRecordNum = pRDataSource->psuPRecord->iRecordNum;
        else
            RDataSource->iPRecordNum = 0;

        // Add the record to the list
        RDataSources.Add(RDataSource);

        // Next R data source
        pRDataSource = pRDataSource->psuNextRDataSource;
        }

    } // end Copy_RDataSources()


// ------------------------------------------------------------------------

void ConnectR(Irig106DotNet::Tmats ^ TmatsInfo)
    {
    // Step through the R records and data source records
    for each (Tmats::SuRRecord ^ CurrRRecord in TmatsInfo->RRecords)
        {
        for each (Tmats::SuRDataSource ^ CurrRDataSrc in CurrRRecord->RDataSources)
            {
            // If the connecting M record number valid then find and connect to the M record
            if (CurrRDataSrc->iMRecordNum != 0)
                for each (Tmats::SuMRecord ^ CurrMRecord in TmatsInfo->MRecords)
                    if (CurrRDataSrc->iMRecordNum == CurrMRecord->iRecordNum)
                        CurrRDataSrc->MRecord = CurrMRecord;
            // If the connecting P record number valid then find and connect to the P record
            if (CurrRDataSrc->iPRecordNum != 0)
                for each (Tmats::SuPRecord ^ CurrPRecord in TmatsInfo->PRecords)
                    if (CurrRDataSrc->iPRecordNum == CurrPRecord->iRecordNum)
                        CurrRDataSrc->PRecord = CurrPRecord;
            } // end for each R data source
        } // end for each R record

    return;

    } // end ConnectR()



// ------------------------------------------------------------------------
// M Records
// ------------------------------------------------------------------------

void Copy_MRecords(Irig106::SuMRecord * pFirstMRecord, List<Tmats::SuMRecord ^> % MRecords)
    {
    Irig106::SuMRecord      *  pMRecord;
    Tmats::SuMRecord        ^  MRecord;

    pMRecord = pFirstMRecord;

    while (pMRecord != NULL)
        {
        MRecord = gcnew Tmats::SuMRecord;

        // Copy the data
        MRecord->iRecordNum = pMRecord->iRecordNum;

        COPY_STRING(pMRecord->szDataSourceID,       MRecord->sDataSourceID)
        COPY_STRING(pMRecord->szBBDataLinkName,     MRecord->sBBDataLinkName)
        COPY_STRING(pMRecord->szBasebandSignalType, MRecord->sBasebandSignalType)

        // Keep track of any links to other records
        if (pMRecord->psuBRecord != NULL)
            MRecord->iBRecordNum = pMRecord->psuBRecord->iRecordNum;
        else
            MRecord->iBRecordNum = 0;

        if (pMRecord->psuPRecord != NULL)
            MRecord->iPRecordNum = pMRecord->psuPRecord->iRecordNum;
        else
            MRecord->iPRecordNum = 0;

        // Add the record to the list
        MRecords.Add(MRecord);

        // Next M record
        pMRecord = pMRecord->psuNextMRecord;
        }

    } // end Copy_MRecords()



// ------------------------------------------------------------------------

void ConnectM(Irig106DotNet::Tmats ^ TmatsInfo)
    {
    // Step through the M records
    for each (Tmats::SuMRecord ^ CurrMRecord in TmatsInfo->MRecords)
        {
        // If the connecting P record number valid then find and connect to the P record
        if (CurrMRecord->iPRecordNum != 0)
            for each (Tmats::SuPRecord ^ CurrPRecord in TmatsInfo->PRecords)
                if (CurrMRecord->iPRecordNum == CurrPRecord->iRecordNum)
                    CurrMRecord->PRecord = CurrPRecord;
        // If the connecting P record number valid then find and connect to the P record
        if (CurrMRecord->iBRecordNum != 0)
            for each (Tmats::SuBRecord ^ CurrBRecord in TmatsInfo->BRecords)
                if (CurrMRecord->iPRecordNum == CurrBRecord->iRecordNum)
                    CurrMRecord->BRecord = CurrBRecord;
        } // end for each M record

    return;
    } // end ConnectM()



// ------------------------------------------------------------------------
// B Records
// ------------------------------------------------------------------------

void Copy_BRecords(Irig106::SuBRecord * pFirstBRecord, List<Tmats::SuBRecord ^> % BRecords)
    {
    Irig106::SuBRecord      *  pBRecord;
    Tmats::SuBRecord        ^  BRecord;

    pBRecord = pFirstBRecord;

    while (pBRecord != NULL)
        {
        BRecord = gcnew Tmats::SuBRecord;

        // Copy the data
        BRecord->iRecordNum = pBRecord->iRecordNum;
        BRecord->iNumBuses  = pBRecord->iNumBuses;

        COPY_STRING(pBRecord->szDataLinkName, BRecord->sDataLinkName)

        // Add the record to the list
        BRecords.Add(BRecord);

        // Next B record
        pBRecord = pBRecord->psuNextBRecord;
        }

    return;
    } // end CopyBRecords()



// ------------------------------------------------------------------------
// P Records
// ------------------------------------------------------------------------

void Copy_PRecords(Irig106::SuPRecord * pFirstPRecord, List<Tmats::SuPRecord ^> % PRecords)
    {
    Irig106::SuPRecord      *  pPRecord;
    Tmats::SuPRecord        ^  PRecord;

    pPRecord = pFirstPRecord;

    while (pPRecord != NULL)
        {
        PRecord = gcnew Tmats::SuPRecord;

        // Copy the data
        PRecord->iRecordNum = pPRecord->iRecordNum;

        COPY_STRING(pPRecord->szDataLinkName,         PRecord->sDataLinkName)
        COPY_STRING(pPRecord->szPcmCode,              PRecord->sPcmCode)
        COPY_STRING(pPRecord->szBitsPerSec,           PRecord->sBitsPerSec)
        COPY_STRING(pPRecord->szPolarity,             PRecord->sPolarity)
        COPY_STRING(pPRecord->szTypeFormat,           PRecord->sTypeFormat)
        COPY_STRING(pPRecord->szCommonWordLen,        PRecord->sCommonWordLen)
        COPY_STRING(pPRecord->szNumMinorFrames,       PRecord->sNumMinorFrames)
        COPY_STRING(pPRecord->szWordsInMinorFrame,    PRecord->sWordsInMinorFrame)
        COPY_STRING(pPRecord->szBitsInMinorFrame,     PRecord->sBitsInMinorFrame)
        COPY_STRING(pPRecord->szMinorFrameSyncType,   PRecord->sMinorFrameSyncType)
        COPY_STRING(pPRecord->szMinorFrameSyncPatLen, PRecord->sMinorFrameSyncPatLen)
        COPY_STRING(pPRecord->szMinorFrameSyncPat,    PRecord->sMinorFrameSyncPat)

        // Add the record to the list
        PRecords.Add(PRecord);

        // Next M record
        pPRecord = pPRecord->psuNextPRecord;
        }

    return;
    } // end CopyPRecords()



// ------------------------------------------------------------------------
// Other utilities
// ------------------------------------------------------------------------

#define CR_CHAR      (13)
#define LF_CHAR      (10)
#define NULL_CHAR     (0)

/* This is really stupid.  I've spent *days* trying to figure out the .NET
   blessed way of converting a buffer of ASCII characters into a .NET String
   object.  I've looked at the various String constructors, Marshal::, Encoder::,
   Encoding::, Decoder::, Convert::, and on and on.  I'll be damned if I can figure
   out a clean, one line way of doing it.  So here I do it the hard way.  If someday
   I actually figure it out then I can replace all this with the one magic line of
   code.  Sheesh.
 */

void TmatsBufferToString(array<SByte> ^ DataBuff, int DataLen, String ^% StringBuff)
    {
    int           iInBuffIdx = 4;   // Skip the CSDW

//    StringBuff = "";

    // Loop until we get to the end of the buffer
    while (true)
        {

        // If at the end of the buffer then break out of the big loop
        if (iInBuffIdx >= DataLen)
            break;

        if (DataBuff[iInBuffIdx] == NULL_CHAR)
            break;

        StringBuff = StringBuff + Convert::ToChar(DataBuff[iInBuffIdx]);

        iInBuffIdx++;

        } // end while filling complete line

    return;
    }



// ------------------------------------------------------------------------

// Copy a TMATS buffer into a list of strings, skipping the CSDW at the front

void TmatsBufferToLines(String ^ sDataBuff, List<Irig106DotNet::Tmats::SuTmatsLine ^> ^ Lines)
    {
    int           iInBuffIdx = 0;
    String      ^ sLine;
    int           iLine = 0;

    // Loop until we get to the end of the buffer
    while (true)
        {

        // If at the end of the buffer then break out of the big loop
        if (iInBuffIdx >= sDataBuff->Length)
            break;

        sLine = "";
        iLine++;

        // Fill a local buffer with one line
        // ---------------------------------

        while (true)
            {
            // If at the end of the buffer then break out
            if (iInBuffIdx >= sDataBuff->Length)
                break;

            // If not CR, LF, or ';' then copy character to line buffer
            if ((sDataBuff[iInBuffIdx] != NULL_CHAR )  &&
                (sDataBuff[iInBuffIdx] != CR_CHAR   )  &&
                (sDataBuff[iInBuffIdx] != LF_CHAR   )  &&
                (sDataBuff[iInBuffIdx] != ';'       ))
                {
                sLine = sLine + sDataBuff[iInBuffIdx];
                }

            // If line terminator and line buffer not empty then break out
            if (sDataBuff[iInBuffIdx] == ';')
                {
                iInBuffIdx++;
                if (sLine->Length != 0)
                    break;
                } // end if line terminator
            else
                iInBuffIdx++;

            } // end while filling complete line

        // Decode the TMATS line
        // ---------------------

        // Go ahead and split the line into left hand and right hand sides
        Irig106DotNet::Tmats::SuTmatsLine ^ suLine = gcnew Irig106DotNet::Tmats::SuTmatsLine;
        array<String^> ^ SplitLine = nullptr;

        int iPosColon;

        // Get the position of the code name / data item field delimiter
        iPosColon = sLine->IndexOf(':');

        // Code name and data item fields
        if (iPosColon != -1)
            {
            suLine->CodeName = sLine->Substring(0,iPosColon);
            suLine->DataItem = sLine->Substring(iPosColon+1);
            }
        // No colon so no data item field. It's wrong but it happens
        else
            {
            suLine->CodeName = sLine;
            suLine->DataItem = "";
            }

        // Add it to the array of lines
        Lines->Add(suLine);

        // If errors tokenizing the line then skip over them
//        if ((szCodeName == NULL) || (szDataItem == NULL))
//            continue;

        } // end while reading from input buffer

    return;
    }



// ------------------------------------------------------------------------

/// Generate TMATS signature from a buffer of ASCII TMATS

ReturnStatus Tmats::Signature(
        array<SByte>  ^ DataBuff,       ///< Buffer of ASCII TMATS, no CSWD
        int             BuffLen,        ///< Length of DataBuff
        int             SigVersion,     ///< Signature version
        int             SigFlags,       ///< Optional signature flags
        SByte         % OpCode,         ///< Returned op code
        UInt32        % Signature)      ///< Returned signature
    {
    uint8_t                         OpCodeNative;
    uint32_t                        SignatureNative;
    Irig106DotNet::ReturnStatus     Status;
    pin_ptr<SByte>                  pDataBuff = &DataBuff[0];

    Status = DLL::enI106_Tmats_Signature(
                    pDataBuff,
                    BuffLen,
                    SigVersion,
                    SigFlags,
                    &OpCodeNative,
                    &SignatureNative);
    if (Status != ReturnStatus::OK)
        {
        OpCode    = 0;
        Signature = 0;
        return Status;
        }

    OpCode    = OpCodeNative;
    Signature = SignatureNative;

    return ReturnStatus::OK;
    }



// ------------------------------------------------------------------------

/// Generate TMATS signature from a string containing TMATS

ReturnStatus Tmats::Signature(
        String    ^ sDataBuff,      ///< String with TMATS
        int         SigVersion,     ///< Signature version
        int         SigFlags,       ///< Optional signature flags
        SByte     % OpCode,         ///< Returned op code
        UInt32    % Signature)      ///< Returned signature
    {
    uint8_t                         OpCodeNative;
    uint32_t                        SignatureNative;
    Irig106DotNet::ReturnStatus     Status;

    char * pDataBuff = (char *)(void *)Marshal::StringToHGlobalAnsi(sDataBuff);

    Status = DLL::enI106_Tmats_Signature(
                    pDataBuff,
                    sDataBuff->Length,
                    SigVersion,
                    SigFlags,
                    &OpCodeNative,
                    &SignatureNative);
    if (Status != ReturnStatus::OK)
        {
        OpCode    = 0;
        Signature = 0;
        return Status;
        }

    OpCode    = OpCodeNative;
    Signature = SignatureNative;

    return ReturnStatus::OK;
    }
