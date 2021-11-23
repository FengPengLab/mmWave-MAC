/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/log.h"
#include "ns3/object-base.h"
#include "ns3/pointer.h"
#include "ns3/queue.h"
#include "ns3/socket.h"
#include "ns3/random-variable-stream.h"
#include "mmwave.h"
#include "mmwave-mac-low.h"
#include "mmwave-mac-queue.h"
#include "mmwave-mac-trailer.h"
#include "mmwave-mac-tx-middle.h"
#include "mmwave-remote-station-manager.h"
#include "cr-mac.h"
#include "cr-mac-low.h"
#include "cr-txop.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("CrMmWaveTxop");
    NS_OBJECT_ENSURE_REGISTERED (CrMmWaveTxop);

    TypeId
    CrMmWaveTxop::GetTypeId()
    {
        static TypeId tid = TypeId("ns3::CrMmWaveTxop")
                .SetParent<Object>()
                .SetGroupName("MmWave")
                .AddConstructor<CrMmWaveTxop>()
                .AddAttribute("Queue", "The MmWaveMacQueue object",
                              PointerValue(),
                              MakePointerAccessor(&CrMmWaveTxop::GetMacQueue),
                              MakePointerChecker<MmWaveMacQueue>())
        ;
        return tid;
    }

    CrMmWaveTxop::CrMmWaveTxop ()
            : m_currentItemOfIntraGroup (0),
              m_currentItemOfInterGroup (0),
              m_accessRequestedOfIntraGroup (false),
              m_accessRequestedOfInterGroup (false),
              m_fragmentNumberOfIntraGroup (0),
              m_fragmentNumberOfInterGroup (0),
              m_bufferSize (64)
    {
        NS_LOG_FUNCTION (this);
        m_queue = CreateObject<MmWaveMacQueue>();
    }

    CrMmWaveTxop::~CrMmWaveTxop ()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    CrMmWaveTxop::DoDispose ()
    {
        NS_LOG_FUNCTION (this);
        m_queue = 0;
        m_lowOfIntraGroup = 0;
        m_lowOfInterGroup = 0;
        m_stationManager = 0;
        m_txMiddleOfIntraGroup = 0;
        m_txMiddleOfInterGroup = 0;
    }

    void
    CrMmWaveTxop::DoInitialize ()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    CrMmWaveTxop::SetTxMiddle (TypeOfGroup typeOfGroup, const Ptr<MmWaveMacTxMiddle> txMiddle)
    {
        NS_LOG_FUNCTION (this);
        switch (typeOfGroup)
        {
            case INTRA_GROUP:
                m_txMiddleOfIntraGroup = txMiddle;
                break;
            case INTER_GROUP:
                m_txMiddleOfInterGroup = txMiddle;
                break;
            case PROBE_GROUP:
            default:
                NS_FATAL_ERROR ("TypeOfGroup is error");
                break;
        }

    }

    void
    CrMmWaveTxop::SetMac (const Ptr<CrMmWaveMac> mac)
    {
        NS_LOG_FUNCTION (this);
        m_mac = mac;
    }

    void
    CrMmWaveTxop::SetMacLowOfIntraGroup (const Ptr<CrMmWaveMacLow> low)
    {
        NS_LOG_FUNCTION (this);
        m_lowOfIntraGroup = low;
        low->SetTxop (this);
    }

    void
    CrMmWaveTxop::SetMacLowOfInterGroup (const Ptr<CrMmWaveMacLow> low)
    {
        NS_LOG_FUNCTION (this);
        m_lowOfInterGroup = low;
        low->SetTxop (this);
    }

    void
    CrMmWaveTxop::SetRemoteStationManager (const Ptr<MmWaveRemoteStationManager> remoteManager)
    {
        NS_LOG_FUNCTION (this);
        m_stationManager = remoteManager;
        m_stationManager->UpdateFragmentationThreshold ();
    }

    void
    CrMmWaveTxop::SetTxOkCallback (Callback <void, const MmWaveMacHeader&> callback)
    {
        NS_LOG_FUNCTION (this << &callback);
        m_txOkCallback = callback;
    }

    void
    CrMmWaveTxop::SetTxFailedCallback (Callback <void, const MmWaveMacHeader&> callback)
    {
        NS_LOG_FUNCTION (this << &callback);
        m_txFailedCallback = callback;
    }

    void
    CrMmWaveTxop::SetTxDroppedCallback (Callback <void, Ptr<const Packet>> callback)
    {
        NS_LOG_FUNCTION (this << &callback);
        m_txDroppedCallback = callback;
        m_queue->TraceConnectWithoutContext ("Drop", MakeCallback (&CrMmWaveTxop::TxDroppedPacket, this));
    }

    Ptr<MmWaveMacQueue>
    CrMmWaveTxop::GetMacQueue () const
    {
        NS_LOG_FUNCTION (this);
        return m_queue;
    }

    void
    CrMmWaveTxop::Queue (Ptr<Packet> packet, MmWaveMacHeader hdr)
    {
        NS_LOG_FUNCTION (this << packet << hdr);
        std::vector<MmWaveChannelNumberStandardPair> channels = GetChannelsNeedToAccess (hdr.GetAddr1 ());
        NS_ASSERT (!channels.empty ());
        if (channels.empty ())
        {
            TxDroppedPacket (Create<MmWaveMacQueueItem> (packet, hdr));
        }
        else
        {
            for (auto i = channels.begin (); i != channels.end (); i++)
            {
                m_queue->Enqueue (Create<MmWaveMacQueueItem> (packet->Copy (), hdr, (*i)));
            }
        }

        if (HasAnyAccessRequest(INTRA_GROUP))
        {
            m_lowOfIntraGroup->StartAccessIfNeed ();
        }

        if (HasAnyAccessRequest (INTER_GROUP))
        {
            m_lowOfInterGroup->StartAccessIfNeed ();
        }
    }



    std::vector<MmWaveChannelNumberStandardPair>
    CrMmWaveTxop::GetChannelsNeedToAccess (Mac48Address to)
    {
        std::vector<MmWaveChannelNumberStandardPair> list;
        if (to.IsGroup ())
        {
            MmWaveChannelToFrequencyWidthMap l = m_lowOfIntraGroup->GetChannelToFrequency ();
            for (auto i = l.begin (); i != l.end (); i++)
            {
                list.push_back (i->first);
            }
        }
        else
        {
            MmWaveNeighborDevices neighbors = m_mac->GetAllNeighborDevices ();
            auto f = neighbors.find (to);
            if (f != neighbors.end ())
            {
                list.push_back (neighbors[to]->m_channel);
            }
            else
            {
                list.push_back (m_lowOfIntraGroup->GetCurrentChannel ());
            }
        }
        return list;
    }

    std::map<Mac48Address, std::vector<std::pair<MmWaveMacHeader, uint32_t>>>
    CrMmWaveTxop::GetBulkAccessRequestsInQueue (TypeOfGroup typeOfGroup)
    {
        NS_LOG_FUNCTION (this);
        NS_ASSERT (!m_queue->IsEmpty ());
        std::map<Mac48Address, std::vector<std::pair<MmWaveMacHeader, uint32_t>>> requirments;
        MmWaveChannelNumberStandardPair channel;
        switch (typeOfGroup)
        {
            case INTRA_GROUP:
                channel = m_lowOfIntraGroup->GetCurrentChannel ();
                requirments = m_queue->GetBulkAccessRequestsInQueue (m_bufferSize, channel);
                break;
            case INTER_GROUP:
                channel = m_lowOfInterGroup->GetCurrentChannel ();
                requirments = m_queue->GetBulkAccessRequestsInQueue (m_bufferSize, channel);
                break;
            case PROBE_GROUP:
            default:
                NS_FATAL_ERROR ("TypeOfGroup is error");
                break;
        }
        return requirments;
    }

    bool
    CrMmWaveTxop::HasAnyAccessRequest (TypeOfGroup typeOfGroup)
    {
        NS_LOG_FUNCTION (this << typeOfGroup);
        bool ret = false;
        switch (typeOfGroup)
        {
            case INTRA_GROUP:
                if (m_bufferOfIntraGroup.empty ())
                {
                    ret = m_queue->FindByChannel (m_lowOfIntraGroup->GetCurrentChannel ());
                }
                else
                {
                    ret = true;
                }
                break;
            case INTER_GROUP:
                if (m_mac->GetAccessMode () == MMWAVE_MULTI_CHANNEL)
                {
                    if (m_bufferOfInterGroup.empty ())
                    {
                        ret = m_queue->FindByChannel (m_lowOfInterGroup->GetCurrentChannel ());
                    }
                    else
                    {
                        ret = true;
                    }
                }
                else
                {
                    ret = false;
                }
                break;
            case PROBE_GROUP:
            default:
                NS_FATAL_ERROR ("TypeOfGroup is error");
                return false;
        }
        return ret;
    }

    bool
    CrMmWaveTxop::IsQueueEmpty (TypeOfGroup typeOfGroup)
    {
        NS_LOG_FUNCTION (this << typeOfGroup);
        bool ret = false;
        switch (typeOfGroup)
        {
            case INTRA_GROUP:
                ret = m_queue->FindByChannel (m_lowOfIntraGroup->GetCurrentChannel ());
                break;
            case INTER_GROUP:
                if (m_mac->GetAccessMode () == MMWAVE_MULTI_CHANNEL)
                {
                    ret = m_queue->FindByChannel (m_lowOfInterGroup->GetCurrentChannel ());
                }
                else
                {
                    ret = false;
                }
                break;
            case PROBE_GROUP:
            default:
                NS_FATAL_ERROR ("TypeOfGroup is error");
                return false;
        }
        return ret;
    }

    MmWaveChannelNumberStandardPair
    CrMmWaveTxop::GetNeedToAccessForInterGroup ()
    {
        return m_queue->GetNeedToAccessExcludedChannel (m_lowOfIntraGroup->GetCurrentChannel ());
    }
    
    void
    CrMmWaveTxop::NotifyAccessGranted (TypeOfGroup typeOfGroup)
    {
        NS_LOG_FUNCTION (this << typeOfGroup);
        Ptr<MmWaveMacQueueItem> item = 0;
        Mac48Address to;
        MmWaveMacHeader hdr;
        Ptr<Packet> fragment;
        switch (typeOfGroup)
        {
            case INTRA_GROUP:
                if (m_currentItemOfIntraGroup == 0)
                {
                    NS_ASSERT (m_lowOfIntraGroup->IsStateIdle ());
                    NS_ASSERT (!m_bufferOfIntraGroup.empty ());
                    item = m_bufferOfIntraGroup.front ();
                    m_recordsOfIntraGroup.push_back (m_bufferOfIntraGroup.front ());
                    m_bufferOfIntraGroup.pop_front ();
                    NS_ASSERT (item != 0);
                    item->GetHeader().SetSequenceNumber (m_txMiddleOfIntraGroup->GetNextSequenceNumberFor (&item->GetHeader ()));
                    item->GetHeader().SetFragmentNumber (0);
                    item->GetHeader().SetNoMoreFragments ();
                    item->GetHeader().SetNoRetry ();
                    m_fragmentNumberOfIntraGroup = 0;

                    to = item->GetHeader().GetAddr1 ();
                    if (!to.IsGroup ())
                    {
                        if (m_stationManager->NeedFragmentation (to, &item->GetHeader(), item->GetPacket ()))
                        {
                            m_currentItemOfIntraGroup = item;
                            fragment = GetFragmentPacket (INTRA_GROUP, &hdr);
                            item = Create<MmWaveMacQueueItem> (fragment, hdr, m_lowOfIntraGroup->GetCurrentChannel ());
                        }
                    }
                    m_lowOfIntraGroup->StartTransmission (item);
                }
                else 
                {
                    NS_ASSERT (m_lowOfIntraGroup->IsStateIdle ());
                    m_fragmentNumberOfIntraGroup++;
                    fragment = GetFragmentPacket (INTRA_GROUP, &hdr);
                    m_lowOfIntraGroup->StartTransmission (Create<MmWaveMacQueueItem> (fragment, hdr, m_lowOfIntraGroup->GetCurrentChannel ()));
                    if (m_stationManager->IsLastFragment (m_currentItemOfIntraGroup->GetHeader().GetAddr1(),
                                                          &m_currentItemOfIntraGroup->GetHeader(),
                                                          m_currentItemOfIntraGroup->GetPacket(),
                                                          m_fragmentNumberOfIntraGroup))
                    {
                        m_currentItemOfIntraGroup = 0;
                    }
                }
                break;
            case INTER_GROUP:
                if (m_currentItemOfInterGroup == 0)
                {
                    NS_ASSERT (m_lowOfInterGroup->IsStateIdle ());
                    NS_ASSERT (!m_bufferOfInterGroup.empty ());
                    item = m_bufferOfInterGroup.front ();
                    m_recordsOfInterGroup.push_back (m_bufferOfInterGroup.front ());
                    m_bufferOfInterGroup.pop_front ();
                    NS_ASSERT (item != 0);
                    item->GetHeader().SetSequenceNumber (m_txMiddleOfInterGroup->GetNextSequenceNumberFor (&item->GetHeader ()));
                    item->GetHeader().SetFragmentNumber (0);
                    item->GetHeader().SetNoMoreFragments ();
                    item->GetHeader().SetNoRetry ();
                    m_fragmentNumberOfInterGroup = 0;

                    to = item->GetHeader().GetAddr1 ();
                    if (!to.IsGroup ())
                    {
                        if (m_stationManager->NeedFragmentation (to, &item->GetHeader(), item->GetPacket ()))
                        {
                            m_currentItemOfInterGroup = item;
                            fragment = GetFragmentPacket (INTER_GROUP, &hdr);
                            item = Create<MmWaveMacQueueItem> (fragment, hdr, m_lowOfInterGroup->GetCurrentChannel ());
                        }
                    }
                    m_lowOfInterGroup->StartTransmission (item);
                }
                else
                {
                    NS_ASSERT (m_lowOfInterGroup->IsStateIdle ());
                    m_fragmentNumberOfInterGroup++;
                    fragment = GetFragmentPacket (INTER_GROUP, &hdr);
                    m_lowOfInterGroup->StartTransmission (Create<MmWaveMacQueueItem> (fragment, hdr, m_lowOfInterGroup->GetCurrentChannel ()));
                    if (m_stationManager->IsLastFragment (m_currentItemOfInterGroup->GetHeader().GetAddr1(),
                                                          &m_currentItemOfInterGroup->GetHeader(),
                                                          m_currentItemOfInterGroup->GetPacket(),
                                                          m_fragmentNumberOfInterGroup))
                    {
                        m_currentItemOfInterGroup = 0;
                    }
                }
                break;
            case PROBE_GROUP:
            default:
                NS_FATAL_ERROR ("TypeOfGroup is error");
                break;
        }
    }

    MmWaveChannelNumberStandardPair
    CrMmWaveTxop::GetCurrentChannel (TypeOfGroup typeOfGroup)
    {
        switch (typeOfGroup)
        {
            case INTRA_GROUP:
                return m_lowOfIntraGroup->GetCurrentChannel ();
            case INTER_GROUP:
                return m_lowOfInterGroup->GetCurrentChannel ();
            case PROBE_GROUP:
            default:
                NS_FATAL_ERROR ("TypeOfGroup is error");
                break;
        }
    }

    void
    CrMmWaveTxop::SetAccessBuffer (TypeOfGroup typeOfGroup, Mac48Address to, Time txDuration)
    {
        NS_LOG_FUNCTION (this << typeOfGroup);
        Ptr<MmWaveMacQueueItem> item = 0;
        Time duration;
        switch (typeOfGroup)
        {
            case INTRA_GROUP:
                m_bufferOfIntraGroup.clear ();
                m_recordsOfIntraGroup.clear ();
                NS_ASSERT (txDuration.IsStrictlyPositive ());
                while (m_queue->FindByAddressAndChannel (to, m_lowOfIntraGroup->GetCurrentChannel ()))
                {
                    item = m_queue->DequeueByAddressAndChannel (to, m_lowOfIntraGroup->GetCurrentChannel ());
                    NS_ASSERT (item != 0);
                    duration = m_lowOfIntraGroup->CalculateTxDuration (item->GetHeader(), item->GetPacket()->GetSize ());
                    if (txDuration.IsStrictlyPositive () && (duration <= txDuration))
                    {
                        txDuration -= duration;
                        m_bufferOfIntraGroup.push_back (item);
                    }
                    else
                    {
                        m_queue->PushFront (item);
                        break;
                    }
                }
                break;
            case INTER_GROUP:
                m_bufferOfInterGroup.clear ();
                m_recordsOfInterGroup.clear ();
                NS_ASSERT (txDuration.IsStrictlyPositive ());
                while (m_queue->FindByAddressAndChannel (to, m_lowOfInterGroup->GetCurrentChannel ()))
                {
                    item = m_queue->DequeueByAddressAndChannel (to, m_lowOfInterGroup->GetCurrentChannel ());
                    NS_ASSERT (item != 0);
                    duration = m_lowOfInterGroup->CalculateTxDuration (item->GetHeader(), item->GetPacket()->GetSize ());
                    if (txDuration.IsStrictlyPositive () && (duration <= txDuration))
                    {
                        txDuration -= duration;
                        m_bufferOfInterGroup.push_back (item);
                    }
                    else
                    {
                        m_queue->PushFront (item);
                        break;
                    }
                }
                break;
            case PROBE_GROUP:
            default:
                NS_FATAL_ERROR ("TypeOfGroup is error");
                break;
        }
    }

    void
    CrMmWaveTxop::ClearAllPackets (TypeOfGroup typeOfGroup)
    {
        NS_LOG_FUNCTION (this << typeOfGroup);
        switch (typeOfGroup)
        {
            case INTRA_GROUP:
                m_bufferOfIntraGroup.clear ();
                m_recordsOfIntraGroup.clear ();
                if (m_queue->FindByChannel (m_lowOfIntraGroup->GetCurrentChannel ()))
                {
                    bool go_on = true;
                    Ptr<MmWaveMacQueueItem> item;
                    while (go_on)
                    {
                        NS_ASSERT (!m_queue->IsEmpty ());
                        item = m_queue->DequeueByChannel (m_lowOfIntraGroup->GetCurrentChannel ());
                        TxDroppedPacket (item);
                        go_on = m_queue->FindByChannel (m_lowOfIntraGroup->GetCurrentChannel ());
                    }
                }
                break;
            case INTER_GROUP:
                m_bufferOfInterGroup.clear ();
                m_recordsOfInterGroup.clear ();
                if (m_queue->FindByChannel (m_lowOfInterGroup->GetCurrentChannel ()))
                {
                    bool go_on = true;
                    Ptr<MmWaveMacQueueItem> item;
                    while (go_on)
                    {
                        NS_ASSERT (!m_queue->IsEmpty ());
                        item = m_queue->DequeueByChannel (m_lowOfInterGroup->GetCurrentChannel ());
                        TxDroppedPacket (item);
                        go_on = m_queue->FindByChannel (m_lowOfInterGroup->GetCurrentChannel ());
                    }
                }
                break;
            case PROBE_GROUP:
            default:
                NS_FATAL_ERROR("TypeOfGroup is error");
                break;
        }
    }

    void
    CrMmWaveTxop::GotBulkAck (TypeOfGroup typeOfGroup, uint16_t startingSeq, uint64_t bitmap)
    {
        NS_LOG_FUNCTION (this << typeOfGroup);
        uint16_t seq;
        uint64_t flag;
        std::vector<uint16_t> seqVector;
        for (uint16_t i = 1; i <= 64; i++)
        {
            seq = startingSeq + i - 1;
            flag = 1;
            flag = bitmap & (flag << (i - 1));
            if (flag != 0)
            {
                seqVector.push_back (seq);
            }
        }
        switch (typeOfGroup)
        {
            case INTRA_GROUP:
                for (auto & i : m_recordsOfIntraGroup)
                {
                    seq = i->GetHeader ().GetSequenceNumber ();
                    auto f = std::find (seqVector.begin (), seqVector.end (), seq);
                    if (f != seqVector.end ())
                    {
                        TxOk (i->GetHeader ());
                    }
                    else
                    {
                        TxFailed (i->GetHeader (), i->GetPacket ()->GetSize ());
                    }
                }
                m_recordsOfIntraGroup.clear ();
                break;
            case INTER_GROUP:
                for (auto & i : m_recordsOfInterGroup)
                {
                    seq = i->GetHeader ().GetSequenceNumber ();
                    auto f = std::find (seqVector.begin (), seqVector.end (), seq);
                    if (f != seqVector.end ())
                    {
                        TxOk (i->GetHeader ());
                    }
                    else
                    {
                        TxFailed (i->GetHeader (), i->GetPacket ()->GetSize ());
                    }
                }
                m_recordsOfInterGroup.clear ();
                break;
            case PROBE_GROUP:
            default:
                NS_FATAL_ERROR("TypeOfGroup is error");
                break;
        }
    }

    void
    CrMmWaveTxop::MissedBulkResponse (TypeOfGroup typeOfGroup)
    {
        NS_LOG_FUNCTION (this << typeOfGroup);
        switch (typeOfGroup)
        {
            case INTRA_GROUP:
                m_bufferOfIntraGroup.clear ();
                m_recordsOfIntraGroup.clear ();
                break;
            case INTER_GROUP:
                m_bufferOfInterGroup.clear ();
                m_recordsOfInterGroup.clear ();
                break;
            case PROBE_GROUP:
            default:
                NS_FATAL_ERROR ("TypeOfGroup is error");
                break;
        }
    }

    void
    CrMmWaveTxop::MissedBulkAck (TypeOfGroup typeOfGroup)
    {
        NS_LOG_FUNCTION (this << typeOfGroup);
        switch (typeOfGroup)
        {
            case INTRA_GROUP:
                for (auto & i : m_recordsOfIntraGroup)
                {
                    TxFailed (i->GetHeader (), i->GetPacket ()->GetSize ());
                }
                m_recordsOfIntraGroup.clear ();
                break;
            case INTER_GROUP:
                for (auto & i : m_recordsOfInterGroup)
                {
                    TxFailed (i->GetHeader (), i->GetPacket ()->GetSize ());
                }
                m_recordsOfInterGroup.clear ();
                break;
            case PROBE_GROUP:
            default:
                NS_FATAL_ERROR("TypeOfGroup is error");
                break;
        }
    }

    void
    CrMmWaveTxop::TxOk (MmWaveMacHeader hdr)
    {
        NS_LOG_FUNCTION (this << hdr);
        if (!m_txOkCallback.IsNull ())
        {
            m_txOkCallback (hdr);
        }
    }

    void
    CrMmWaveTxop::TxFailed (MmWaveMacHeader hdr, uint32_t packetSize)
    {
        NS_LOG_FUNCTION (this << hdr << packetSize);
        if (!hdr.GetAddr1 ().IsGroup ())
        {
            m_stationManager->ReportFinalDataFailed (hdr.GetAddr1 (), &hdr, packetSize);
            if (!m_txFailedCallback.IsNull ())
            {
                m_txFailedCallback (hdr);
            }
        }
    }

    void
    CrMmWaveTxop::TxDroppedPacket (Ptr<const MmWaveMacQueueItem> item)
    {
        NS_LOG_FUNCTION (this << item);
        if (!m_txDroppedCallback.IsNull ())
        {
            m_txDroppedCallback (item->GetPacket ());
        }
    }

    Ptr<Packet>
    CrMmWaveTxop::GetFragmentPacket (TypeOfGroup typeOfGroup, MmWaveMacHeader *hdr)
    {
        NS_LOG_FUNCTION (this << *hdr);
        uint32_t startOffset;
        uint32_t length;
        Ptr<Packet> fragment;
        Mac48Address to;
        switch (typeOfGroup)
        {
            case INTRA_GROUP:
                *hdr = m_currentItemOfIntraGroup->GetHeader();
                to = m_currentItemOfIntraGroup->GetHeader().GetAddr1 ();
                hdr->SetFragmentNumber (m_fragmentNumberOfIntraGroup);
                startOffset = m_stationManager->GetFragmentOffset (to,
                                                                   &m_currentItemOfIntraGroup->GetHeader (),
                                                                   m_currentItemOfIntraGroup->GetPacket (),
                                                                   m_fragmentNumberOfIntraGroup);
                if (m_stationManager->IsLastFragment (to,
                                                      &m_currentItemOfIntraGroup->GetHeader (),
                                                      m_currentItemOfIntraGroup->GetPacket (),
                                                      m_fragmentNumberOfIntraGroup))
                {
                    hdr->SetNoMoreFragments ();
                }
                else
                {
                    hdr->SetMoreFragments ();
                }
                length = m_stationManager->GetFragmentSize (to,
                                                            &m_currentItemOfIntraGroup->GetHeader (),
                                                            m_currentItemOfIntraGroup->GetPacket (),
                                                            m_fragmentNumberOfIntraGroup);
                fragment = m_currentItemOfIntraGroup->GetPacket()->CreateFragment (startOffset,length);
                break;
            case INTER_GROUP:
                *hdr = m_currentItemOfInterGroup->GetHeader();
                to = m_currentItemOfInterGroup->GetHeader().GetAddr1 ();
                hdr->SetFragmentNumber (m_fragmentNumberOfInterGroup);
                startOffset = m_stationManager->GetFragmentOffset (to,
                                                                   &m_currentItemOfInterGroup->GetHeader (),
                                                                   m_currentItemOfInterGroup->GetPacket (),
                                                                   m_fragmentNumberOfInterGroup);
                if (m_stationManager->IsLastFragment (to,
                                                      &m_currentItemOfInterGroup->GetHeader (),
                                                      m_currentItemOfInterGroup->GetPacket (),
                                                      m_fragmentNumberOfInterGroup))
                {
                    hdr->SetNoMoreFragments ();
                }
                else
                {
                    hdr->SetMoreFragments ();
                }
                length = m_stationManager->GetFragmentSize (to,
                                                            &m_currentItemOfInterGroup->GetHeader (),
                                                            m_currentItemOfInterGroup->GetPacket (),
                                                            m_fragmentNumberOfInterGroup);
                fragment = m_currentItemOfInterGroup->GetPacket()->CreateFragment (startOffset,length);
                break;
            case PROBE_GROUP:
            default:
                NS_FATAL_ERROR ("TypeOfGroup is error");
                break;
        }
        return fragment;
    }

    bool
    CrMmWaveTxop::HasNextPacket (TypeOfGroup typeOfGroup)
    {
        switch (typeOfGroup)
        {
            case INTRA_GROUP:
                if (m_bufferOfIntraGroup.empty ())
                {
                    return false;
                }
                else
                {
                    return true;
                }
                break;
            case INTER_GROUP:
                if (m_bufferOfInterGroup.empty ())
                {
                    return false;
                }
                else
                {
                    return true;
                }
                break;
            case PROBE_GROUP:
            default:
                NS_FATAL_ERROR ("TypeOfGroup is error");
                break;
        }
    }

    bool
    CrMmWaveTxop::HasNextFragment (TypeOfGroup typeOfGroup)
    {
        switch (typeOfGroup)
        {
            case INTRA_GROUP:
                if (m_currentItemOfIntraGroup != 0)
                {
                    return true;
                }
                else
                {
                    return false;
                }
                break;
            case INTER_GROUP:
                if (m_currentItemOfInterGroup != 0)
                {
                    return true;
                }
                else
                {
                    return false;
                }
                break;
            case PROBE_GROUP:
            default:
                NS_FATAL_ERROR ("TypeOfGroup is error");
                break;
        }
    }

    uint16_t
    CrMmWaveTxop::GetStartingSequenceNumber (TypeOfGroup typeOfGroup)
    {
        switch (typeOfGroup)
        {
            case INTRA_GROUP:
                return m_txMiddleOfIntraGroup->GetStartingSequenceNumber ();
                break;
            case INTER_GROUP:
                return m_txMiddleOfInterGroup->GetStartingSequenceNumber ();
                break;
            case PROBE_GROUP:
            default:
                NS_FATAL_ERROR ("TypeOfGroup is error");
                break;
        }
    }
}