#include "StdAfx.h"
#include <malloc.h>
#include <memory.h>

#include "CopyStatus.h"

using namespace System::Diagnostics;

using namespace Irig106;

namespace i106Dub {


// --------------------------------------------------------------------------

System::Void CopyStatus::DubFile(BackgroundWorker^ bCaller, DoWorkEventArgs^ mArgs)
    {
    Irig106Lib        * IrigIn;
    Irig106Lib        * IrigOut;
    EnI106Status        enStatus;
    int64_t             llPastTmats;
    int64_t             llBeginRtcTime;
    int64_t             llEndRtcTime;
    int64_t             llCurrTime;
    int64_t             llCurrOffset;
    int                 iPercentDone;
    SuCopyParams      ^ mParams;

    mParams = safe_cast<SuCopyParams^>(mArgs->Argument);

    // Some stuff for writing out packets
    SuI106Ch10Header        * psuOutHeader = new SuI106Ch10Header;
    ChanSeqNums = gcnew array<uint8_t>(0x10000);
    Array::Clear(ChanSeqNums,0,ChanSeqNums->Length);

    // Lists of indexed packets
    SuPacketIndex           ^ PacketIndex;
    List<SuPacketIndex ^>   ^ NodeIndexList = gcnew List<SuPacketIndex ^>;
    List<SuPacketIndex ^>   ^ RootIndexList = gcnew List<SuPacketIndex ^>;

    // Needed to link root index packets
    SuPacketIndex             PrevRootPacketIndex;

    // One time loop for easy breakout
    do
        {

        // Put up the status
        // -----------------
        bCaller->ReportProgress(0, "Status - Open input file");

        // Open the input file and get some info
        // -------------------------------------
        IrigIn = new Irig106Lib;
        IrigIn->Open(mParams->sInFile);

        // Get begin RTC
        IrigIn->SetPos(mParams->llBeginOffset);
        IrigIn->ReadNextHeader();
        IrigIn->TimeArray2LLInt(&llBeginRtcTime);

        // Get end RTC
        IrigIn->SetPos(mParams->llEndOffset);
        IrigIn->ReadPrevHeader();
        IrigIn->TimeArray2LLInt(&llEndRtcTime);

        // Open output file
        // ----------------
        bCaller->ReportProgress(0, "Status - Open output file");
        IrigOut = new Irig106Lib;
        IrigOut->Open(mParams->sOutFile, I106_OVERWRITE);

        // Read, modify, write TMATS
        // -------------------------
        bCaller->ReportProgress(0, "Status - Update and write TMATS");
        IrigIn->FirstMsg();
        IrigIn->ReadNextHeader();
        IrigIn->ReadData();
        if (mParams->bCh0TMATSCopy)
            {
            if (IrigIn->pHeader->ubyDataType != I106CH10_DTYPE_TMATS)
                {
                MessageBox::Show( "TMATS data not found", "Error", 
                    MessageBoxButtons::OK, MessageBoxIcon::Exclamation );
                // throw
                }
//{
//SuTmatsEditInfo     suEditInfo;
//InitTmatsEditInfo(&(IrigIn->pDataBuff), IrigIn->pHeader->ulDataLen, uint32_t iBuffLength, &suEditInfo)
//}
                FixSeqNum(IrigIn->pHeader);
                IrigIn->SetHeaderChecksum();
                IrigOut->WriteMsg(IrigIn->pHeader, IrigIn->pDataBuff);
            } // end if copy TMATS

        // Save this position in case we need it later
        IrigIn->GetPos(&llPastTmats);

        // Find closest time packet
        // ------------------------

        // Only get time if a time channel is enabled
        if (mParams->bSkipTime == false)
            {
            bCaller->ReportProgress(0, "Status : Find and write the nearest time packet");

            IrigIn->SetPos(mParams->llBeginOffset);

            // Scan backwards
            while (true)
                {
                // If cancel requested then bail
                if (bCaller->CancellationPending)
                    {
                    mArgs->Cancel = true;
                    break;
                    }

                enStatus = IrigIn->ReadPrevHeader();

                // If we can't read backwards any more then break out
                if  (enStatus != I106_OK)
                    break;

                // If too much time has gone by then break out
                IrigIn->TimeArray2LLInt(&llCurrTime);
                if (llCurrTime < (llBeginRtcTime - 100000000))  // 10 seconds
                    {
                    enStatus = I106_TIME_NOT_FOUND;
                    break;
                    }

                // If packet is time then write it
                if (IrigIn->pHeader->ubyDataType == I106CH10_DTYPE_IRIG_TIME)
                    {
                    IrigIn->ReadData();

                    // If index enabled for this channel then make an index entry
                    if (mParams->ChanIndex[IrigIn->pHeader->uChID] != 0)
                        {
                        PacketIndex = gcnew SuPacketIndex;
                        IrigIn->TimeArray2LLInt(PacketIndex->llTime);
                        PacketIndex->uChannelID = IrigIn->pHeader->uChID;
                        PacketIndex->uDataType  = IrigIn->pHeader->ubyDataType;
                        IrigOut->GetPos(PacketIndex->uOffset);
                        NodeIndexList->Add(PacketIndex);
                        } // end if index enabled

                    FixSeqNum(IrigIn->pHeader);
                    IrigIn->SetHeaderChecksum();
                    IrigOut->WriteMsg(IrigIn->pHeader, IrigIn->pDataBuff);
                    IrigIn->SetPos(mParams->llBeginOffset);
                    break;
                    } // end if IRIG time

                } // end while scanning backwards

            // Scan backwards didn't work so try forward
            if (enStatus != I106_OK)
                {
                IrigIn->SetPos(mParams->llBeginOffset);        

                 while (true)
                    {
                    // If cancel requested then bail
                    if (bCaller->CancellationPending)
                        {
                        mArgs->Cancel = true;
                        break;
                        }

                    enStatus = IrigIn->ReadNextHeader();

                    // If we can't read backwards any more then break out
                    if  (enStatus != I106_OK)
                        break;

                    // If too much time has gone by then break out
                    IrigIn->TimeArray2LLInt(&llCurrTime);
                    if (llCurrTime > (llBeginRtcTime + 100000000))  // 10 seconds
                        {
                        enStatus = I106_TIME_NOT_FOUND;
                        }

                    // If packet is time then write it
                    if (IrigIn->pHeader->ubyDataType == I106CH10_DTYPE_IRIG_TIME)
                        {
                        IrigIn->ReadData();

                        // If index enabled for this channel then make an index entry
                        if (mParams->ChanIndex[IrigIn->pHeader->uChID] != 0)
                            {
                            PacketIndex = gcnew SuPacketIndex;
                            IrigIn->TimeArray2LLInt(PacketIndex->llTime);
                            PacketIndex->uChannelID = IrigIn->pHeader->uChID;
                            PacketIndex->uDataType  = IrigIn->pHeader->ubyDataType;
                            IrigOut->GetPos(PacketIndex->uOffset);
                            NodeIndexList->Add(PacketIndex);
                            } // end if index enabled

                        FixSeqNum(IrigIn->pHeader);
                        IrigIn->SetHeaderChecksum();
                        IrigOut->WriteMsg(IrigIn->pHeader, IrigIn->pDataBuff);
                        IrigIn->SetPos(mParams->llBeginOffset);
                        break;
                        } // end if IRIG time

                    } // end while scanning forwards
                } // end if scan backwards not OK

            // Scan forward didn't work so get time from the beginning of the file
            if (enStatus != I106_OK)
                {
                MessageBox::Show( "Suitable time packet not found", "Error", 
                    MessageBoxButtons::OK, MessageBoxIcon::Exclamation );
                // throw
                }

            } // end if time channel enabled

        // Now copy until end point
        // ------------------------

        // Get the index list setup
        //PacketIndexList = gcnew List;

        // Loop until done
        while (true)
            {
            // Get the next IRIG packet header
            enStatus = IrigIn->ReadNextHeader();

            // Get a copy of the current packet time for later use
            IrigIn->TimeArray2LLInt(&llCurrTime);

            // If anything but OK we're done
            if  (enStatus != I106_OK)
                break;

            // If past dub end then we're done
            if (llCurrTime > llEndRtcTime)
                break;

            // If cancel requested then bail
            if (bCaller->CancellationPending)
                {
                mArgs->Cancel = true;
                break;
                }

            // Put up current status
            // PROBABLY SHOULD OPTIMIZE THIS, GUI UPDATES CAN BE SLOW
            IrigIn->GetPos(&llCurrOffset);
            iPercentDone = int(((llCurrOffset - mParams->llBeginOffset) * 100) / (mParams->llEndOffset - mParams->llBeginOffset));
            bCaller->ReportProgress(iPercentDone, "Status : Dubbing file");

            // Not done so do some processing and maybe write packet to output file
            // --------------------------------------------------------------------

            // Handle special Channel ID 0 case
            if (IrigIn->pHeader->uChID == 0)
                {
                switch (IrigIn->pHeader->ubyDataType)
                    {
                    case I106CH10_DTYPE_TMATS :
                        if (mParams->bCh0TMATSCopy)
                            {
                            IrigIn->ReadData();
                            if (mParams->bCh0TMATSIndex)
                                {
                                PacketIndex = gcnew SuPacketIndex;
                                IrigIn->TimeArray2LLInt(PacketIndex->llTime);
                                PacketIndex->uChannelID = 0;
                                PacketIndex->uDataType  = IrigIn->pHeader->ubyDataType;
                                IrigOut->GetPos(PacketIndex->uOffset);
                                NodeIndexList->Add(PacketIndex);
                                } // end if index enabled
                            FixSeqNum(IrigIn->pHeader);
                            IrigIn->SetHeaderChecksum();
                            IrigOut->WriteMsg(IrigIn->pHeader, IrigIn->pDataBuff);
                            }
                        break;

                    case I106CH10_DTYPE_RECORDING_INDEX :
                        // Don't copy indexes, they need to be fixed later
                        break;

                    case I106CH10_DTYPE_RECORDING_EVENT :
                        if (mParams->bCh0EventsCopy)
                            {
                            IrigIn->ReadData();
                            if (mParams->bCh0EventsIndex)
                                {
                                PacketIndex = gcnew SuPacketIndex;
                                IrigIn->TimeArray2LLInt(PacketIndex->llTime);
                                PacketIndex->uChannelID = 0;
                                PacketIndex->uDataType  = IrigIn->pHeader->ubyDataType;
                                IrigOut->GetPos(PacketIndex->uOffset);
                                NodeIndexList->Add(PacketIndex);
                                } // end if index enabled
                            FixSeqNum(IrigIn->pHeader);
                            IrigIn->SetHeaderChecksum();
                            IrigOut->WriteMsg(IrigIn->pHeader, IrigIn->pDataBuff);
                            }
                        break;

                    default :
                        if (mParams->bCh0OtherCopy)
                            {
                            IrigIn->ReadData();
                            if (mParams->bCh0OtherIndex)
                                {
                                PacketIndex = gcnew SuPacketIndex;
                                IrigIn->TimeArray2LLInt(PacketIndex->llTime);
                                PacketIndex->uChannelID = 0;
                                PacketIndex->uDataType  = IrigIn->pHeader->ubyDataType;
                                IrigOut->GetPos(PacketIndex->uOffset);
                                NodeIndexList->Add(PacketIndex);
                                } // end if index enabled
                            FixSeqNum(IrigIn->pHeader);
                            IrigIn->SetHeaderChecksum();
                            IrigOut->WriteMsg(IrigIn->pHeader, IrigIn->pDataBuff);
                            }
                        break;
                    } // end switch on data type                
                } // end if Channel ID 0

            // Channel ID other than 0
            else
                {
                if (mParams->ChanEnabled[IrigIn->pHeader->uChID] != 0)
                    {
                    IrigIn->ReadData();
                    if (mParams->ChanIndex[IrigIn->pHeader->uChID] != 0)
                        {
                        PacketIndex = gcnew SuPacketIndex;
                        IrigIn->TimeArray2LLInt(PacketIndex->llTime);
                        PacketIndex->uChannelID = IrigIn->pHeader->uChID;
                        PacketIndex->uDataType  = IrigIn->pHeader->ubyDataType;
                        IrigOut->GetPos(PacketIndex->uOffset);
                        NodeIndexList->Add(PacketIndex);

//Debug::Print("Write Packet - Time {0} Chan ID {1} Data Type {2} Offset {3:X}", 
//    PacketIndex->llTime, PacketIndex->uChannelID,
//    PacketIndex->uDataType, PacketIndex->uOffset);

                        } // end if index enabled
                    FixSeqNum(IrigIn->pHeader);
                    IrigIn->SetHeaderChecksum();
                    IrigOut->WriteMsg(IrigIn->pHeader, IrigIn->pDataBuff);
                    } // end if channel enabled
                } // end if Channel ID not 0

            } // end while copying

        // Write indexes
        // -------------

        // Only write index packets if the index list has items
        if (NodeIndexList->Count > 0)
            {

            // -- Make node index packet(s) --

            SuIndex_NodePacket * psuNodeIndexPacket;
            SuIndex_RootPacket * psuRootIndexPacket;

            int PacketIndexIdx;     // Index into the list of index records
            int NodePacketIdx = 0;  // Index into the IRIG node index packet
            int RootPacketIdx = 0;  // Index into the IRIG root index packet
            int NodesToWrite;       // Number of indexes to write to a single node index packet
            int MaxDataBuffSize;    // Maximum size of the index packet in bytes
            int IndexDataBuffSize;  // Total size of the index packet in bytes
            int IndexListCount;
//            int64_t PrevRootPacketPos;

int max = 10;

            // Make most of the header
//            memset(IrigOut->pHeader, 0, sizeof(SuI106Ch10Header));
            IrigOut->pHeader->uSync          = IRIG106_SYNC;
            IrigOut->pHeader->uChID          = 0;
            IrigOut->pHeader->ubyHdrVer      = 0x03;
            IrigOut->pHeader->ubyPacketFlags = I106CH10_PFLAGS_CHKSUM_32;
            IrigOut->pHeader->ubyDataType    = I106CH10_DTYPE_RECORDING_INDEX;

            // Make some of the data buffer
            MaxDataBuffSize = IrigOut->CalcDataBuffReqSize(sizeof(SuIndex_ChanSpec) + max * sizeof(SuIndex_NodeEntry));
            psuNodeIndexPacket = (SuIndex_NodePacket *)malloc(MaxDataBuffSize);
            IrigOut->pDataBuff = (void *)psuNodeIndexPacket;
//            memset(psuNodeIndexPacket, 0, MaxDataBuffSize); // MAY NOT REALLY NEED THIS

            // Make and write Node Index packets
            IndexListCount = NodeIndexList->Count;
            for (PacketIndexIdx=0; PacketIndexIdx<IndexListCount; PacketIndexIdx++)
                {
                // If first index in node packet then initialize a new node index packet
                if (NodePacketIdx == 0)
                    {
                    // Figure out how many nodes to write, data size, and packet size
                    NodesToWrite = Math::Min((IndexListCount - PacketIndexIdx), max);
                    IrigOut->pHeader->ulDataLen = sizeof(SuIndex_ChanSpec) + NodesToWrite * sizeof(SuIndex_NodeEntry);
                    IndexDataBuffSize = IrigOut->CalcDataBuffReqSize();
                    IrigOut->pHeader->ulPacketLen = HEADER_SIZE + IndexDataBuffSize;

                    // Set some other header junk
                    llCurrTime++; // Some validators whine if two time stamps are the same
                    IrigOut->LLInt2TimeArray(&llCurrTime);  // TIME??????????
                    FixSeqNum(IrigOut->pHeader);
                    IrigOut->SetHeaderChecksum();

                    // Initialize the Channel Specific Data Word
                    psuNodeIndexPacket->IndexCSDW.uIndexType   = 1;
                    psuNodeIndexPacket->IndexCSDW.bFileSize    = 0;
                    psuNodeIndexPacket->IndexCSDW.bIntraPckHdr = 0;
                    psuNodeIndexPacket->IndexCSDW.uIdxEntCount = NodesToWrite;

                    // Save some root index info
                    PacketIndex = gcnew SuPacketIndex;
                    PacketIndex->llTime = NodeIndexList[PacketIndexIdx]->llTime;
                    IrigOut->GetPos(PacketIndex->uOffset);
                    RootIndexList->Add(PacketIndex);
                    
                    } // end if first packet in node index

                // Get setup for one node index packet
                psuNodeIndexPacket->NodeEntry[NodePacketIdx].suTime.llTime     = NodeIndexList[PacketIndexIdx]->llTime;
                psuNodeIndexPacket->NodeEntry[NodePacketIdx].suData.uChannelID = NodeIndexList[PacketIndexIdx]->uChannelID;
                psuNodeIndexPacket->NodeEntry[NodePacketIdx].suData.uDataType  = NodeIndexList[PacketIndexIdx]->uDataType;
                psuNodeIndexPacket->NodeEntry[NodePacketIdx].suData.uOffset    = NodeIndexList[PacketIndexIdx]->uOffset;

//Debug::Print("Time {0} Chan ID {1} Data Type {2} Offset {3:X}", 
//    psuNodeIndexPacket->NodeEntry[NodePacketIdx].suTime.llTime,
//    psuNodeIndexPacket->NodeEntry[NodePacketIdx].suData.uChannelID,
//    psuNodeIndexPacket->NodeEntry[NodePacketIdx].suData.uDataType,
//    psuNodeIndexPacket->NodeEntry[NodePacketIdx].suData.uOffset);

                // If last one in current index packet, write it and setup for another
                if (NodePacketIdx == NodesToWrite-1)
                    {
                    // Keep info for root index

                    // Make the trailer
                    IrigOut->AddDataFillerChecksum();

                    // Write the Node Index packet
                    IrigOut->WriteMsg();

                    // Get setup for the next node index packet
                    NodePacketIdx = 0;
                    } // end if node packet full

                // Else not done making an Node Index packet
                else
                    {
                    NodePacketIdx++;
                    }

                } // end writing Node Index packets

            free(psuNodeIndexPacket);

            // -- Make and write Root Index packets --

            // Make some of the data buffer
            MaxDataBuffSize    = IrigOut->CalcDataBuffReqSize(sizeof(SuIndex_ChanSpec) + max * sizeof(SuIndex_RootEntry));
            psuRootIndexPacket = (SuIndex_RootPacket *)malloc(MaxDataBuffSize);
            IrigOut->pDataBuff = (void *)psuRootIndexPacket;
//            memset(psuNodeIndexPacket, 0, MaxDataBuffSize); // MAY NOT REALLY NEED THIS

            // Save some root index info for linked list
            IrigOut->GetPos(PrevRootPacketIndex.uOffset);
            PrevRootPacketIndex.llTime  = llCurrTime;

            // Make and write Root Index packets
            IndexListCount = RootIndexList->Count;
            for (PacketIndexIdx=0; PacketIndexIdx<IndexListCount; PacketIndexIdx++)
                {
                // If first index in root packet then initialize a new root index packet
                if (RootPacketIdx == 0)
                    {
                    // Figure out how many nodes to write, data size, and packet size
                    NodesToWrite = Math::Min((IndexListCount - PacketIndexIdx + 1), max);
                    IrigOut->pHeader->ulDataLen = sizeof(SuIndex_ChanSpec) + NodesToWrite * sizeof(SuIndex_RootEntry);
                    IndexDataBuffSize = IrigOut->CalcDataBuffReqSize();
                    IrigOut->pHeader->ulPacketLen = HEADER_SIZE + IndexDataBuffSize;

                    // Set some other header junk
                    llCurrTime++; // Some validators whine if two time stamps are the same
                    IrigOut->LLInt2TimeArray(&llCurrTime);
                    FixSeqNum(IrigOut->pHeader);
                    IrigOut->SetHeaderChecksum();

                    // Initialize the Channel Specific Data Word
                    psuRootIndexPacket->IndexCSDW.uIndexType   = 0;
                    psuRootIndexPacket->IndexCSDW.bFileSize    = 0;
                    psuRootIndexPacket->IndexCSDW.bIntraPckHdr = 0;
                    psuRootIndexPacket->IndexCSDW.uIdxEntCount = NodesToWrite;

                    } // end if first packet in root index

                // Get setup for one root index packet
                psuRootIndexPacket->RootEntry[RootPacketIdx].suTime.llTime = RootIndexList[PacketIndexIdx]->llTime;
                psuRootIndexPacket->RootEntry[RootPacketIdx].uOffset       = RootIndexList[PacketIndexIdx]->uOffset;

//Debug::Print("Time {0} Offset {3:X}", 
//    psuNodeIndexPacket->NodeEntry[NodePacketIdx].suTime.llTime,
//    psuNodeIndexPacket->NodeEntry[NodePacketIdx].suData.uOffset);

                // If next to last one in current index packet, make the link to the previous
                // root index packet, write it, and setup for another packet

                if (RootPacketIdx == NodesToWrite-2)
                    {
                    // Make link to previous root index packet
                    RootPacketIdx++;
                    psuRootIndexPacket->RootEntry[RootPacketIdx].suTime.llTime = PrevRootPacketIndex.llTime;
                    psuRootIndexPacket->RootEntry[RootPacketIdx].uOffset       = PrevRootPacketIndex.uOffset;
                    
                    // Save some root index info for linked list
                    IrigOut->GetPos(PrevRootPacketIndex.uOffset);
                    PrevRootPacketIndex.llTime  = llCurrTime;

                    // Make the trailer
                    IrigOut->AddDataFillerChecksum();

                    // Write the Node Index packet
                    IrigOut->WriteMsg();

                    // Get setup for the next node index packet
                    RootPacketIdx = 0;
                    } // end if root packet full

                // Else not done making an Root Index packet
                else
                    {
                    RootPacketIdx++;
                    }

                } // end for each node packet

            free(psuRootIndexPacket);

            } // end if writing node and root index packets

        } while (false);

    // Clean up
    // --------

    IrigIn->Close();
    delete IrigIn;

    IrigOut->Close();
    delete IrigOut;

    bCaller->ReportProgress(100, "Status : Done!");

    return;
    } // end DubFile background thread



// ------------------------------------------------------------------------

// Put the next sequence number in the IRIG header

System::Void CopyStatus::FixSeqNum(SuI106Ch10Header * pIrigHeader)
    {
    pIrigHeader->ubySeqNum = ChanSeqNums[pIrigHeader->uChID];
    ChanSeqNums[pIrigHeader->uChID]++;
    return;
    }


} // end namespace i106_dub