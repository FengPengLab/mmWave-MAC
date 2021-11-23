/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
#include <cmath>
#include "ns3/simulator.h"
#include "ns3/vector.h"
#include "ns3/mobility-model.h"
#include "mmwave-snr-tag.h"
#include "mmwave-mac-trailer.h"
#include "mmwave-phy.h"
#include "cr-mac.h"
#include "cr-mac-low.h"
#include "cr-dynamic-channel-access-manager.h"

#undef NS_LOG_APPEND_CONTEXT
#define NS_LOG_APPEND_CONTEXT std::clog << "[" << m_typeOfGroup << "] "

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("CrMmWaveMacLow");
    NS_OBJECT_ENSURE_REGISTERED (CrMmWaveMacLow);

    CrMmWaveMacLow::CrMmWaveMacLow ()
    {
        NS_LOG_FUNCTION (this);

        m_detectionStartTime = Seconds (0.0);
        m_detectionDuration = Seconds (0.0);

        m_macState = INTRA_SUSPEND;
        m_accessing = false;
        m_noActiveUsers = false;
        m_switchChannelFlag = false;
        m_makeDecision = false;
        m_noActiveUsers = false;
        m_accessing = false;
    }

    CrMmWaveMacLow::~CrMmWaveMacLow ()
    {
        NS_LOG_FUNCTION (this);
    }

    TypeId
    CrMmWaveMacLow::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::CrMmWaveMacLow")
                .SetParent<MmWaveMacLow> ()
                .SetGroupName ("MmWave")
                .AddConstructor<CrMmWaveMacLow> ();
        return tid;
    }

    void
    CrMmWaveMacLow::DoInitialize()
    {
        ToSuspend ();
    }

    void
    CrMmWaveMacLow::DoDispose ()
    {
        CancelAllEvents ();
        m_bulkAccessRequests.clear ();
        m_channelAccessManager = 0;
        m_txop = 0;
        MmWaveMacLow::DoDispose ();
    }

    void
    CrMmWaveMacLow::CancelAllEvents ()
    {
        if (m_beaconEvent.IsRunning ())
        {
            m_beaconEvent.Cancel ();
        }
        if (m_detectionEvent.IsRunning ())
        {
            m_detectionEvent.Cancel ();
        }
        if (m_sendBulkAck.IsRunning ())
        {
            m_sendBulkAck.Cancel ();
        }
        if (m_txPacket.IsRunning ())
        {
            m_txPacket.Cancel ();
        }
        if (m_waitIfsEvent.IsRunning ())
        {
            m_waitIfsEvent.Cancel ();
        }
        if (m_bulkAccess.IsRunning ())
        {
            m_bulkAccess.Cancel ();
        }
        if (m_bulkResponseTimeout.IsRunning ())
        {
            m_bulkResponseTimeout.Cancel ();
        }
        if (m_bulkAckTimeout.IsRunning ())
        {
            m_bulkAckTimeout.Cancel ();
        }
    }

    void
    CrMmWaveMacLow::SetTypeOfGroup (TypeOfGroup typeOfGroup)
    {
        m_typeOfGroup = typeOfGroup;
    }

    TypeOfGroup
    CrMmWaveMacLow::GetTypeOfGroup () const
    {
        return m_typeOfGroup;
    }

    void
    CrMmWaveMacLow::SetMacLowState (MmWaveMacState state)
    {
        m_macState = state;
    }

    MmWaveMacState
    CrMmWaveMacLow::GetMacLowState () const
    {
        return m_macState;
    }

    void
    CrMmWaveMacLow::SetChannelAccessManager (Ptr<CrDynamicChannelAccessManager> channelAccessManager)
    {
        m_channelAccessManager = channelAccessManager;
    }

    void
    CrMmWaveMacLow::SetTxop (Ptr<CrMmWaveTxop> txop)
    {
        m_txop = txop;
    }

    MmWaveChannelToFrequencyWidthMap
    CrMmWaveMacLow::GetChannelToFrequency ()
    {
        MmWaveChannelToFrequencyWidthMap list = m_phy->GetChannelToFrequency ();
        NS_ASSERT(list.size() != 0);
        return list;
    }

    MmWaveChannelNumberStandardPair
    CrMmWaveMacLow::GetCurrentChannel ()
    {
        return m_phy->GetCurrentChannel ();
    }

    MmWaveChannelNumberStandardPair
    CrMmWaveMacLow::GetNextChannel (MmWaveChannelNumberStandardPair channel)
    {
        MmWaveChannelToFrequencyWidthMap list = m_phy->GetChannelToFrequency ();
        NS_ASSERT(list.size() != 0);
        auto f = list.find (channel);
        if (f == list.end ())
        {
            NS_FATAL_ERROR ("cannot find!");
        }
        else
        {
            for (auto i = list.begin(); i != list.end (); i++)
            {
                if (i->first == channel)
                {
                    i++;
                    if (i == list.end ())
                    {
                        return (list.begin ()->first);
                    }
                    else
                    {
                        return (i->first);
                    }
                }
            }
        }
        return {{0,MMWAVE_PHY_BAND_UNSPECIFIED}, MMWAVE_PHY_STANDARD_UNSPECIFIED};
    }

    void
    CrMmWaveMacLow::CreateBulkAccessRequests ()
    {
        Time duration;
        uint32_t num;
        auto requirments = m_txop->GetBulkAccessRequestsInQueue (GetTypeOfGroup ());
        NS_ASSERT (requirments.size () > 0);
        m_bulkAccessRequests.clear ();
        Ptr<BulkAccessInfo> request;
        for (auto & i : requirments)
        {
            duration = Seconds (0.0);
            num = 0;
            for (auto & j : i.second)
            {
                num++;
                duration += CalculateTxDuration (j.first, j.second);
            }
            request = Create<BulkAccessInfo> ();
            request->m_address = i.first;
            request->m_txDuration = duration;
            request->m_numOfPackets = num;
            request->m_startingSeq = m_txop->GetStartingSequenceNumber (GetTypeOfGroup ());
            request->m_bitmap = 0;
            m_bulkAccessRequests.push_back (request);
        }
    }

    void
    CrMmWaveMacLow::StartNewBulkAccess ()
    {
        m_txBulkAccessInfo = 0;
        if (m_bulkAccessRequests.empty ())
        {
            if (m_bulkAckTimeout.IsExpired () && m_bulkResponseTimeout.IsExpired ())
            {
                EndChannelAccess ();
                StartAccessIfNeed ();
            }
            return;
        }
        if (m_bulkAckTimeout.IsRunning ())
        {
            return;
        }
        else if (m_bulkResponseTimeout.IsRunning ())
        {
            return;
        }
        else 
        {
            if (!IsStateIdle ())
            {
                EndChannelAccess ();
                StartAccessIfNeed ();
            }
            else 
            {
                m_txBulkAccessInfo = m_bulkAccessRequests.front ();
                m_bulkAccessRequests.pop_front ();
                NS_ASSERT (m_txBulkAccessInfo != 0);
                NS_ASSERT (IsStateIdle ());
                m_txBulkAccessInfo->m_startingSeq = m_txop->GetStartingSequenceNumber (GetTypeOfGroup ());
                if (m_txBulkAccessInfo->m_address.IsGroup ())
                {
                    SendDataForPacket ();
                }
                else
                {
                    SendBulkRequestForPacket ();
                }
            }
        }
    }

    void
    CrMmWaveMacLow::SendBulkAccessRequest ()
    {
        Ptr<CrMmWaveMac> mac = DynamicCast<CrMmWaveMac> (m_mac);
        if ((mac->IsAnyConflictBetweenGroup ()) && (GetTypeOfGroup() == INTER_GROUP))
        {
            MmWaveChannelNumberStandardPair c;
            c = mac->GetChannelNeedToAccessForInterGroup ();
            if (c != GetCurrentChannel ())
            {
                if (!IsStateTx ())
                {
                    ToSwitchChannel (c);
                }
            }
            else
            {
                ToTransmission ();
            }
            return;
        }
        bool r1 = !m_txop->IsQueueEmpty (GetTypeOfGroup ());
        bool r2 = m_channelAccessManager->IsRequestAccess ();
        bool r3 = m_bulkAccess.IsRunning ();
        if (r1 || r2 || r3)
        {
            return;
        }
        if (!IsStateIdle ())
        {
            return;
        }
        NS_ASSERT (IsStateIdle ());
        NS_ASSERT (m_bulkAckTimeout.IsExpired ());
        NS_ASSERT (m_bulkResponseTimeout.IsExpired ());
        CreateBulkAccessRequests ();
        StartChannelAccess ();
        m_txBulkAccessInfo = m_bulkAccessRequests.front ();
        m_bulkAccessRequests.pop_front ();
        NS_ASSERT (m_txBulkAccessInfo != 0);
        m_txBulkAccessInfo->m_startingSeq = m_txop->GetStartingSequenceNumber (GetTypeOfGroup ());
        if (m_txBulkAccessInfo->m_address.IsGroup ())
        {
            SendDataForPacket ();
        }
        else
        {
            SendBulkRequestForPacket ();
        }
    }

    void
    CrMmWaveMacLow::SendDetectionRequest ()
    {
        NS_LOG_FUNCTION (this);
        NS_ASSERT (IsStateIdle ());
        Ptr<CrMmWaveMac> mac = DynamicCast<CrMmWaveMac> (m_mac);
        StartChannelAccess ();
        Ptr<Packet> packet = Create<Packet> ();
        MmWaveMacHeader hdr;
        hdr.SetType (MMWAVE_MAC_MGT_DETECTION);
        hdr.SetAddr1 (Mac48Address::GetBroadcast ());
        hdr.SetAddr2 (m_self);
        hdr.SetDsNotFrom ();
        hdr.SetDsNotTo ();
        hdr.SetNoRetry ();
        hdr.SetNoMoreFragments ();
        hdr.SetDuration (mac->GetFastDetectionDuration ());
        StartTransmission (Create<MmWaveMacQueueItem> (packet, hdr, GetCurrentChannel ()));
    }

    void
    CrMmWaveMacLow::SendBeacon ()
    {
        MmWaveChannelNumberStandardPair c;
        Ptr<CrMmWaveMac> mac = DynamicCast<CrMmWaveMac> (m_mac);
        if (m_makeDecision)
        {
            m_makeDecision = false;
            c = mac->GetChannelNeedToAccessForIntraGroup ();
            if (c == GetCurrentChannel())
            {
                m_switchChannelFlag = false;
                m_switchChannel = GetCurrentChannel ();
            }
            else
            {
                m_switchChannelFlag = true;
                m_switchChannel = c;
            }
        }
        else
        {
            m_switchChannelFlag = false;
            m_switchChannel = GetCurrentChannel ();
        }
        NS_LOG_FUNCTION (this << m_switchChannel);
        NS_ASSERT (IsStateIdle ());
        StartChannelAccess ();
        BeaconHeader beaconHeader;
        beaconHeader.SetChannelNumber (m_switchChannel.first.first);
        Ptr<Packet> packet = Create<Packet> ();
        packet->AddHeader (beaconHeader);
        MmWaveMacHeader hdr;
        hdr.SetType (MMWAVE_MAC_MGT_BEACON);
        hdr.SetAddr1 (Mac48Address::GetBroadcast ());
        hdr.SetAddr2 (m_self);
        hdr.SetDsNotFrom ();
        hdr.SetDsNotTo ();
        hdr.SetNoRetry ();
        hdr.SetNoMoreFragments ();
        hdr.SetDuration (Seconds (0.0));
        StartTransmission (Create<MmWaveMacQueueItem> (packet, hdr, GetCurrentChannel ()));
    }

    void
    CrMmWaveMacLow::SendDataForPacket ()
    {
        MmWaveChannelNumberStandardPair c;
        c = GetCurrentChannel();
        NS_LOG_FUNCTION (this << c);
        NS_ASSERT (IsStateIdle ());
        NS_ASSERT (m_bulkAccess.IsExpired ());
        NS_ASSERT (m_txBulkAccessInfo != 0);
        NS_ASSERT (m_txBulkAccessInfo->m_address.IsGroup ());
        NS_ASSERT (m_txBulkAccessInfo->m_txDuration.IsPositive ());
        m_bulkAccess = Simulator::Schedule (m_txBulkAccessInfo->m_txDuration - GetSifs (), &CrMmWaveMacLow::EndBulkAccess, this);
        m_txop->SetAccessBuffer (GetTypeOfGroup (), m_txBulkAccessInfo->m_address, m_txBulkAccessInfo->m_txDuration);

        NS_ASSERT (GetMacLowState() == INTRA_TRANSMISSION || GetMacLowState() == INTER_TRANSMISSION);
        NS_ASSERT (!m_waitIfsEvent.IsRunning ());
        if (m_bulkAccess.IsRunning ())
        {
            m_txop->NotifyAccessGranted (GetTypeOfGroup());
        }

    }

    void
    CrMmWaveMacLow::SendBulkRequestForPacket ()
    {
        MmWaveChannelNumberStandardPair c;
        c = GetCurrentChannel();
        NS_LOG_FUNCTION (this << c);
        NS_ASSERT (IsStateIdle ());
        NS_ASSERT (m_txBulkAccessInfo != 0);
        BulkRequestHeader requestHeader;
        requestHeader.SetTxDuration(m_txBulkAccessInfo->m_txDuration);
        requestHeader.SetStartingSequenceControl(m_txBulkAccessInfo->m_startingSeq);
        requestHeader.SetNum(m_txBulkAccessInfo->m_numOfPackets);
        Ptr<Packet> packet = Create<Packet> ();
        packet->AddHeader (requestHeader);
        MmWaveMacHeader hdr;
        hdr.SetType (MMWAVE_MAC_MGT_BULK_REQUEST);
        hdr.SetAddr1 (m_txBulkAccessInfo->m_address);
        hdr.SetAddr2 (m_self);
        hdr.SetDsNotFrom ();
        hdr.SetDsNotTo ();
        hdr.SetNoRetry ();
        hdr.SetNoMoreFragments ();
        hdr.SetDuration (GetSifs () + GetBulkResponseTxDuration (m_self));
        StartTransmission (Create<MmWaveMacQueueItem> (packet, hdr, GetCurrentChannel ()));
    }

    void
    CrMmWaveMacLow::SendBulkResponseAfterRequest (Mac48Address source, Time txDuration)
    {
        MmWaveChannelNumberStandardPair c;
        c = GetCurrentChannel();
        NS_LOG_FUNCTION (this << c << source << txDuration);
        
        if (!IsStateIdle ())
        {
            if (m_sendBulkAck.IsRunning ())
            {
                m_sendBulkAck.Cancel ();
            }
        }
        else 
        {
            NS_ASSERT (IsStateIdle ());
            BulkResponseHeader responseHeader;
            responseHeader.SetTxDuration(txDuration);
            Ptr<Packet> packet = Create<Packet> ();
            packet->AddHeader (responseHeader);
            MmWaveMacHeader hdr;
            hdr.SetType (MMWAVE_MAC_MGT_BULK_RESPONSE);
            hdr.SetAddr1 (source);
            hdr.SetAddr2 (m_self);
            hdr.SetDsNotFrom ();
            hdr.SetDsNotTo ();
            hdr.SetNoRetry ();
            hdr.SetNoMoreFragments ();
            hdr.SetDuration (txDuration + GetSifs() + GetBulkAckTxDuration (source));
            StartTransmission (Create<MmWaveMacQueueItem> (packet, hdr, GetCurrentChannel ()));
        }
       
    }

    void
    CrMmWaveMacLow::SendAckAfterData ()
    {
        MmWaveChannelNumberStandardPair c;
        c = GetCurrentChannel();
        NS_LOG_FUNCTION (this << c);
        if (IsStateIdle ())
        {
            NS_ASSERT (IsStateIdle ());
            NS_ASSERT (m_rxBulkAccessInfo != 0);
            Mac48Address source = m_rxBulkAccessInfo->m_address;
            uint16_t seqControl = m_rxBulkAccessInfo->m_startingSeq;
            uint64_t bitmap = m_rxBulkAccessInfo->m_bitmap;
            m_rxBulkAccessInfo = 0;
            BulkAckHeader ackHeader;
            ackHeader.SetStartingSequenceControl (seqControl);
            ackHeader.SetBitmap (bitmap);
            Ptr<Packet> packet = Create<Packet> ();
            packet->AddHeader (ackHeader);
            MmWaveMacHeader hdr;
            hdr.SetType (MMWAVE_MAC_MGT_BULK_ACK);
            hdr.SetAddr1 (source);
            hdr.SetAddr2 (m_self);
            hdr.SetDsNotFrom ();
            hdr.SetDsNotTo ();
            hdr.SetNoRetry ();
            hdr.SetNoMoreFragments ();
            hdr.SetDuration (Seconds (0.0));
            StartTransmission (Create<MmWaveMacQueueItem> (packet, hdr, GetCurrentChannel ()));
        }
    }

    Time
    CrMmWaveMacLow::GetBulkResponseTxDuration (Mac48Address to)
    {
        MmWaveTxVector txVector = GetDataTxVector (to);
        MmWaveMacTrailer fcs;
        MmWaveMacHeader hdr;
        BulkResponseHeader header;
        hdr.SetType(MMWAVE_MAC_MGT_BULK_RESPONSE);
        uint32_t size = hdr.GetSize () + header.GetSerializedSize () + fcs.GetSerializedSize ();
        Time txDuration = m_phy->CalculateTxDuration (size, txVector, m_phy->GetPhyBand ());
        return txDuration;
    }

    Time
    CrMmWaveMacLow::GetBulkAckTxDuration (Mac48Address to)
    {
        MmWaveTxVector txVector = GetDataTxVector (to);
        MmWaveMacTrailer fcs;
        MmWaveMacHeader hdr;
        BulkAckHeader header;
        hdr.SetType(MMWAVE_MAC_MGT_BULK_ACK);
        uint32_t size = hdr.GetSize () + header.GetSerializedSize () + fcs.GetSerializedSize ();
        Time txDuration = m_phy->CalculateTxDuration (size, txVector, m_phy->GetPhyBand ());
        return txDuration;
    }

    Time
    CrMmWaveMacLow::GetBulkResponseTimeout (Mac48Address to)
    {
        MmWaveTxVector txVector;
        txVector = GetDataTxVector (to);
        Time timeout = GetSifs ()
                       + GetBulkResponseTxDuration (to)
                       + GetSifs ()
                       + GetSlotTime ()
                       + GetPhyPreambleAndHeaderDuration (txVector);
        return timeout;
    }

    Time
    CrMmWaveMacLow::GetDetectionStartTime () const
    {
        return m_detectionStartTime;
    }

    Time
    CrMmWaveMacLow::GetDetectionDuration () const
    {
        return m_detectionDuration;
    }

    Time
    CrMmWaveMacLow::GetPhyPreambleAndHeaderDuration (MmWaveTxVector txVector)
    {
        return m_phy->CalculatePhyPreambleAndHeaderDuration (txVector);
    }

    Time
    CrMmWaveMacLow::CalculateTxDuration (MmWaveMacHeader hdr, uint32_t packetSize)
    {
        NS_ASSERT (packetSize > 0);
        MmWaveMacTrailer fcs;
        Mac48Address to = hdr.GetAddr1 ();
        MmWaveTxVector txVector;
        if (hdr.IsCtl ())
        {
            txVector = GetCtrlTxVector (to);
        }
        else
        {
            txVector = GetDataTxVector (to);
        }
        uint32_t size = packetSize;
        NS_ASSERT (m_stationManager->GetFragmentationThreshold () == 65534);
        uint32_t fragment = m_stationManager->GetFragmentationThreshold () - hdr.GetSize () - fcs.GetSerializedSize ();

        Time txDuration = Seconds (0.0);
        if (to.IsGroup ())
        {
            size = hdr.GetSize () + fcs.GetSerializedSize ();
            txDuration += GetSifs ();
            txDuration += m_phy->CalculateTxDuration (size, txVector, m_phy->GetPhyBand ());
        }
        else
        {
            while (size > fragment)
            {
                size -= fragment;
                txDuration += GetSifs ();
                txDuration += m_phy->CalculateTxDuration (m_stationManager->GetFragmentationThreshold (), txVector, m_phy->GetPhyBand ());
            }
            NS_ASSERT (0 < size && size < fragment);
            txDuration += GetSifs ();
            txDuration += m_phy->CalculateTxDuration (size, txVector, m_phy->GetPhyBand ());
        }
        return txDuration;
    }

    bool
    CrMmWaveMacLow::IsAnyPUs (MmWaveChannelNumberStandardPair channel)
    {
        Ptr<CrMmWaveMac> mac = DynamicCast<CrMmWaveMac> (m_mac);
        Ptr<MmWaveSpectrumStatistical> result = mac->GetStatisticalInfo (channel);
        if (result->IsValid ())
        {
            return result->IsAnyPUs ();
        }
        else
        {
            return false;
        }
    }

    bool
    CrMmWaveMacLow::IsAnySUs (MmWaveChannelNumberStandardPair channel)
    {
        Ptr<CrMmWaveMac> mac = DynamicCast<CrMmWaveMac> (m_mac);
        Ptr<MmWaveSpectrumStatistical> result = mac->GetStatisticalInfo (channel);
        if (result->IsValid ())
        {
            return result->IsAnySUs ();
        }
        else
        {
            return false;
        }
    }

    bool
    CrMmWaveMacLow::IsWithinSizeAndTimeLimits (uint32_t size, MmWaveTxVector txVector, Time limit)
    {
        Time txDuration = m_phy->CalculateTxDuration (size, txVector, m_phy->GetPhyBand ());
        if (limit.IsStrictlyPositive () && txDuration <= limit)
        {
            return true;
        }
        return false;
    }

    bool
    CrMmWaveMacLow::IsBusy ()
    {
        return m_channelAccessManager->IsBusy ();
    }

    bool
    CrMmWaveMacLow::IsStateOff ()
    {
        return m_phy->IsStateOff ();
    }

    bool
    CrMmWaveMacLow::IsStateTx ()
    {
        return m_phy->IsStateTx ();
    }

    bool
    CrMmWaveMacLow::IsStateRx ()
    {
        return m_phy->IsStateRx ();
    }

    bool
    CrMmWaveMacLow::IsStateIdle ()
    {
        return m_phy->IsStateIdle ();
    }

    uint32_t
    CrMmWaveMacLow::GetRandomInteger (uint32_t min, uint32_t max)
    {
        return m_rng->GetInteger (min, max);
    }

    void
    CrMmWaveMacLow::DoNavResetNow (Time duration)
    {
        m_channelAccessManager->NotifyNavResetNow (duration);
        m_lastNavStart = Simulator::Now ();
        m_lastNavDuration = duration;
    }

    void
    CrMmWaveMacLow::NotifyAckTimeoutStartNow (Time duration)
    {
        m_channelAccessManager->NotifyAckTimeoutStartNow (duration);
    }

    void
    CrMmWaveMacLow::NotifyAckTimeoutResetNow ()
    {
        m_channelAccessManager->NotifyAckTimeoutResetNow ();
    }

    void
    CrMmWaveMacLow::NotifyCtsTimeoutStartNow (Time duration)
    {
        NS_FATAL_ERROR ("should not be here");
    }

    void
    CrMmWaveMacLow::NotifyCtsTimeoutResetNow ()
    {
        NS_FATAL_ERROR ("should not be here");
    }

    void
    CrMmWaveMacLow::NotifyBulkTimeoutStartNow (Time duration)
    {
        m_channelAccessManager->NotifyBulkTimeoutStartNow (duration);
    }

    void
    CrMmWaveMacLow::NotifyBulkTimeoutResetNow ()
    {
        m_channelAccessManager->NotifyBulkTimeoutResetNow ();
    }

    void
    CrMmWaveMacLow::NotifyNav (Ptr<const Packet> packet,const MmWaveMacHeader &hdr)
    {
        if (hdr.GetRawDuration () > 32767)
        {
            return;
        }

        if (hdr.IsMgt () || hdr.IsData ())
        {
            if (hdr.GetAddr2 () != m_self)
            {
                DoNavStartNow (hdr.GetDuration ());
            }
        }
    }

    void
    CrMmWaveMacLow::NotifyUpdateRotationFactor ()
    {
        if (m_channelAccessManager->IsRequestAccess ())
        {
            m_channelAccessManager->UpdateRotationFactor ();
        }
    }


    void
    CrMmWaveMacLow::NotifySwitchingStartNow (Time duration)
    {
        m_stationManager->Reset ();
        CancelAllEvents ();
        if (m_navCounterReset.IsRunning ())
        {
            m_navCounterReset.Cancel ();
        }
        m_lastNavStart = Simulator::Now ();
        m_lastNavDuration = Seconds (0.0);
        m_currentPacket = 0;
    }

    void
    CrMmWaveMacLow::NotifySleepNow ()
    {
        CancelAllEvents ();
        if (m_navCounterReset.IsRunning ())
        {
            m_navCounterReset.Cancel ();
        }
        m_lastNavStart = Simulator::Now ();
        m_lastNavDuration = Seconds (0.0);
        m_currentPacket = 0;
    }

    void
    CrMmWaveMacLow::NotifyOffNow ()
    {
        CancelAllEvents ();
        if (m_navCounterReset.IsRunning ())
        {
            m_navCounterReset.Cancel ();
        }
        m_lastNavStart = Simulator::Now ();
        m_lastNavDuration = Seconds (0.0);
        m_currentPacket = 0;
    }

    void
    CrMmWaveMacLow::WaitIfsAfterEndTxFragment ()
    {
        NS_ASSERT (GetMacLowState() == INTRA_TRANSMISSION || GetMacLowState() == INTER_TRANSMISSION);
        NS_ASSERT (!m_waitIfsEvent.IsRunning ());
        if (IsStateIdle () && m_bulkAccess.IsRunning ())
        {
            m_txop->NotifyAccessGranted (GetTypeOfGroup());
        }
    }

    void
    CrMmWaveMacLow::WaitIfsAfterEndTxPacket ()
    {
        NS_ASSERT (GetMacLowState() == INTRA_TRANSMISSION || GetMacLowState() == INTER_TRANSMISSION);
        NS_ASSERT (!m_waitIfsEvent.IsRunning ());
        if (IsStateIdle () && m_bulkAccess.IsRunning ())
        {
            m_txop->NotifyAccessGranted (GetTypeOfGroup ());
        }
    }

    void
    CrMmWaveMacLow::UnrecognizedSignalDetected (double power, double snr, Time start, Time duration)
    {
        NS_ASSERT (m_phy->IsSignalDetectionMode ());
        Ptr<CrMmWaveMac> mac = DynamicCast<CrMmWaveMac> (m_mac);
        Time detectionStart = GetDetectionStartTime ();
        Time detectionDuration = GetDetectionDuration ();
        NS_ASSERT (detectionStart <= start);
        if (start + duration > detectionStart + detectionDuration)
        {
            duration = detectionStart + detectionDuration - start;
        }
        mac->AddSpectrumActivityToRepository (GetCurrentChannel (), UTILIZED_BY_PUs, power, start, duration, detectionStart, detectionDuration);
        if (m_noActiveUsers)
        {
            m_noActiveUsers = false;
        }
    }

    void
    CrMmWaveMacLow::RecognizedSignalDetected (double power, double snr, Time start, Time duration)
    {
        NS_ASSERT (m_phy->IsSignalDetectionMode ());
        Ptr<CrMmWaveMac> mac = DynamicCast<CrMmWaveMac> (m_mac);
        Time detectionStart = GetDetectionStartTime ();
        Time detectionDuration = GetDetectionDuration ();
        NS_ASSERT (detectionStart <= start);
        if (start + duration > detectionStart + detectionDuration)
        {
            duration = detectionStart + detectionDuration - start;
        }
        mac->AddSpectrumActivityToRepository (GetCurrentChannel (), UTILIZED_BY_SUs, power, start, duration, detectionStart, detectionDuration);
        if (m_noActiveUsers)
        {
            m_noActiveUsers = false;
        }
    }

    void
    CrMmWaveMacLow::RxStartIndication (MmWaveTxVector txVector, Time psduDuration)
    {
        if (psduDuration.IsZero ())
        {
            return;
        }
        NS_ASSERT (psduDuration.IsStrictlyPositive ());

        if (m_bulkResponseTimeout.IsRunning ())
        {
            m_bulkResponseTimeout.Cancel ();
            NotifyBulkTimeoutResetNow ();
            m_bulkResponseTimeout = Simulator::Schedule (psduDuration + NanoSeconds (400), &CrMmWaveMacLow::BulkTimeout, this);
        }
        else if (m_bulkAckTimeout.IsRunning ())
        {
            m_bulkAckTimeout.Cancel ();
            NotifyAckTimeoutResetNow ();
            m_bulkAckTimeout = Simulator::Schedule (psduDuration + NanoSeconds (400), &CrMmWaveMacLow::BulkAckTimeout, this);
        }
        else if (m_navCounterReset.IsRunning ())
        {
            m_navCounterReset.Cancel ();
        }
    }

    void
    CrMmWaveMacLow::NoSignalDetected ()
    {
        NS_ASSERT (m_phy->IsSignalDetectionMode ());
        Ptr<CrMmWaveMac> mac = DynamicCast<CrMmWaveMac> (m_mac);
        Time detectionStart = GetDetectionStartTime ();
        Time detectionDuration = GetDetectionDuration ();
        mac->AddSpectrumActivityToRepository (GetCurrentChannel (), UNUTILIZED, 0, Seconds (0.0), Seconds (0.0), detectionStart, detectionDuration);
    }

    void
    CrMmWaveMacLow::StartChannelAccess ()
    {
        NS_ASSERT (!m_accessing);
        m_accessing = true;
    }

    void
    CrMmWaveMacLow::EndChannelAccess ()
    {
        NS_ASSERT (m_accessing);
        m_accessing = false;
    }

    void
    CrMmWaveMacLow::EndBulkAccess ()
    {
        Time timerDelay;
        MmWaveTxVector txVector;
        NS_ASSERT (m_txBulkAccessInfo != 0);
        NS_ASSERT (m_bulkAccess.IsExpired ());
        Mac48Address to = m_txBulkAccessInfo->m_address;

        if (!to.IsGroup ())
        {
            txVector = GetDataTxVector (to);
            timerDelay = GetSifs () + GetBulkAckTxDuration (m_self) + GetSifs () + GetSlotTime () + GetPhyPreambleAndHeaderDuration (txVector);
            m_bulkAckTimeout = Simulator::Schedule (timerDelay, &CrMmWaveMacLow::BulkAckTimeout, this);
            NotifyAckTimeoutStartNow (timerDelay);
        }
        else
        {
            m_waitIfsEvent = Simulator::Schedule (GetSifs (), &CrMmWaveMacLow::StartNewBulkAccess, this);
        }
    }

    void
    CrMmWaveMacLow::SetDetectionStartTime (Time start)
    {
        m_detectionStartTime = start;
    }

    void
    CrMmWaveMacLow::SetDetectionDuration (Time duration)
    {
        m_detectionDuration = duration;
    }

    bool
    CrMmWaveMacLow::DoNavStartNow (Time duration)
    {
        m_channelAccessManager->NotifyNavStartNow (duration);
        Time newNavEnd = Simulator::Now () + duration;
        Time oldNavEnd = m_lastNavStart + m_lastNavDuration;
        if (newNavEnd > oldNavEnd)
        {
            m_lastNavStart = Simulator::Now ();
            m_lastNavDuration = duration;
            return true;
        }
        return false;
    }

    void
    CrMmWaveMacLow::BulkTimeout ()
    {
        m_txop->MissedBulkResponse (GetTypeOfGroup ());
        NS_ASSERT (m_waitIfsEvent.IsExpired ());
        m_waitIfsEvent = Simulator::Schedule (GetSifs (), &CrMmWaveMacLow::StartNewBulkAccess, this);
    }

    void
    CrMmWaveMacLow::BulkAckTimeout ()
    {
        m_txop->MissedBulkAck (GetTypeOfGroup ());
        NS_ASSERT (m_waitIfsEvent.IsExpired ());
        m_waitIfsEvent = Simulator::Schedule (GetSifs (), &CrMmWaveMacLow::StartNewBulkAccess, this);
    }

    void
    CrMmWaveMacLow::StartAccessIfNeed ()
    {
        MmWaveChannelNumberStandardPair c;
        Ptr<CrMmWaveMac> mac = DynamicCast<CrMmWaveMac> (m_mac);
        switch (GetTypeOfGroup ())
        {
            case INTRA_GROUP:
                if (GetMacLowState() == INTRA_TRANSMISSION)
                {
                    if ((!m_accessing) && (m_txop->IsQueueEmpty (INTRA_GROUP)))
                    {
                        if (m_txop->HasNextPacket (INTRA_GROUP))
                        {
                            m_txop->MissedBulkResponse (INTRA_GROUP);
                        }
                        if (!m_channelAccessManager->IsRequestAccess ())
                        {
                            m_channelAccessManager->StartRequestAccess (BULK_ACCESS);
                        }
                    }
                }
                break;
            case INTER_GROUP:
                if (mac->GetAccessMode() == MMWAVE_MULTI_CHANNEL)
                {
                    if (GetMacLowState() == INTER_TRANSMISSION)
                    {
                        c = mac->GetChannelNeedToAccessForInterGroup ();
                        if (c == GetCurrentChannel ())
                        {
                            if ((!m_accessing) && (m_txop->IsQueueEmpty (INTER_GROUP)))
                            {
                                if (m_txop->HasNextPacket (INTER_GROUP))
                                {
                                    m_txop->MissedBulkResponse (INTER_GROUP);
                                }
                                if (!m_channelAccessManager->IsRequestAccess ())
                                {
                                    m_channelAccessManager->StartRequestAccess (BULK_ACCESS);
                                }
                            }
                        }
                        else
                        {
                            if (!IsStateTx ())
                            {
                                ToSwitchChannel (c);
                            }
                        }
                    }
                }
                break;
            default:
                NS_FATAL_ERROR ("TypeOfGroup is error");
                break;
        }
    }

    void
    CrMmWaveMacLow::UpdateBulkAckInfo (Mac48Address source, uint16_t sequence)
    {
        NS_ASSERT (m_rxBulkAccessInfo != 0);
        m_rxBulkAccessInfo->m_numOfReceived++;
        if (source == m_rxBulkAccessInfo->m_address)
        {
            if (sequence >= m_rxBulkAccessInfo->m_startingSeq)
            {
                uint16_t pos = sequence - m_rxBulkAccessInfo->m_startingSeq;
                uint64_t bit = 1;
                if (pos >= 0)
                {
                    m_rxBulkAccessInfo->m_bitmap = m_rxBulkAccessInfo->m_bitmap | (bit << pos);
                }
            }
        }
    }

    void
    CrMmWaveMacLow::ToSuspend ()
    {
        MmWaveChannelNumberStandardPair c;
        CancelAllEvents ();
        Ptr<CrMmWaveMac> mac = DynamicCast<CrMmWaveMac> (m_mac);

        if (mac->GetAccessMode () == MMWAVE_SINGLE_CHANNE)
        {
            switch (GetTypeOfGroup ())
            {
                case INTRA_GROUP:
                    SetMacLowState (INTRA_SUSPEND);
                    ToTransmission ();
                    break;
                case INTER_GROUP:
                    SetMacLowState (INTER_SUSPEND);
                    m_phy->SetOffMode ();
                    break;
                case PROBE_GROUP:
                    SetMacLowState (PROBE_SUSPEND);
                    m_phy->SetOffMode ();
                    break;
                default:
                    NS_FATAL_ERROR("TypeOfGroup is error");
                    break;
            }
        }
        else
        {
            switch (GetTypeOfGroup ())
            {
                case INTRA_GROUP:
                    c = mac->GetChannelNeedToAccessForIntraGroup ();
                    if (c != GetCurrentChannel ())
                    {
                        if (!IsStateTx ())
                        {
                            ToSwitchChannel (c);
                        }
                    }
                    else
                    {
                        ToTransmission ();
                    }
                    break;
                case INTER_GROUP:
                    c = mac->GetChannelNeedToAccessForInterGroup ();
                    if (c != GetCurrentChannel ())
                    {
                        if (!IsStateTx ())
                        {
                            ToSwitchChannel (c);
                        }
                    }
                    else
                    {
                        ToTransmission ();
                    }
                    break;
                case PROBE_GROUP:
                    ToDetection ();
                    break;
                default:
                    NS_FATAL_ERROR("TypeOfGroup is error");
                    break;
            }
        }
    }

    void
    CrMmWaveMacLow::ToSwitchChannel (MmWaveChannelNumberStandardPair next)
    {
        Ptr<CrMmWaveMac> mac = DynamicCast<CrMmWaveMac> (m_mac);
        NS_ASSERT (!IsStateTx ());
        NS_ASSERT (mac->GetAccessMode () == MMWAVE_MULTI_CHANNEL);
        CancelAllEvents ();

        switch (GetTypeOfGroup ())
        {
            case INTRA_GROUP:
                SetMacLowState (INTRA_SWITCH);
                m_phy->SetChannelNumber (next.first.first);
                m_stateSwitching = Simulator::Schedule (m_phy->GetChannelSwitchDelay (), &CrMmWaveMacLow::ToTransmission, this);
                break;
            case INTER_GROUP:
                SetMacLowState (INTER_SWITCH);
                m_phy->SetChannelNumber (next.first.first);
                m_stateSwitching = Simulator::Schedule (m_phy->GetChannelSwitchDelay (), &CrMmWaveMacLow::ToTransmission, this);
                break;
            case PROBE_GROUP:
                SetMacLowState (PROBE_SWITCH);
                m_phy->SetChannelNumber (next.first.first);
                m_stateSwitching = Simulator::Schedule (m_phy->GetChannelSwitchDelay (), &CrMmWaveMacLow::ToDetection, this);
                break;
            default:
                NS_FATAL_ERROR ("TypeOfGroup is error");
                break;
        }
    }

    void
    CrMmWaveMacLow::ToTransmission ()
    {
        CancelAllEvents ();
        Ptr<CrMmWaveMac> mac = DynamicCast<CrMmWaveMac> (m_mac);
        Time rngDelay = GetRandomInteger (1, 64) * m_phy->GetSifs ();

        switch (GetTypeOfGroup())
        {
            case INTRA_GROUP:
                SetMacLowState (INTRA_TRANSMISSION);
                m_beaconEvent = Simulator::Schedule (rngDelay, &CrMmWaveMacLow::StartBeacon, this);
                if (mac->GetAccessMode () == MMWAVE_MULTI_CHANNEL)
                {
                    m_detectionEvent = Simulator::Schedule (rngDelay + mac->GetDetectionInterval (), &CrMmWaveMacLow::StartDetection, this);
                }
                break;
            case INTER_GROUP:
                NS_ASSERT (mac->GetAccessMode () == MMWAVE_MULTI_CHANNEL);
                SetMacLowState (INTER_TRANSMISSION);
                if (m_txop->IsQueueEmpty (INTER_GROUP))
                {
                    m_channelAccessManager->StartRequestAccess (BULK_ACCESS);
                }
                break;
            case PROBE_GROUP:
            default:
                NS_FATAL_ERROR ("TypeOfGroup is error");
                break;
        }
    }

    void
    CrMmWaveMacLow::ToDetection ()
    {
        Ptr<CrMmWaveMac> mac = DynamicCast<CrMmWaveMac> (m_mac);
        NS_ASSERT (mac->GetAccessMode () == MMWAVE_MULTI_CHANNEL);
        if (m_channelAccessManager->IsRequestAccess ())
        {
            m_channelAccessManager->CancelRequestAccess ();
        }
        CancelAllEvents ();
        switch (GetTypeOfGroup())
        {
            case INTRA_GROUP:
                SetMacLowState (INTRA_DETECTION);
                StartDetectionChannel (mac->GetFastDetectionDuration ());
                m_stateDetection = Simulator::Schedule (mac->GetFastDetectionDuration (), &CrMmWaveMacLow::StopDetectionChannel, this);
                break;
            case PROBE_GROUP:
                SetMacLowState (PROBE_DETECTION);
                StartDetectionChannel (mac->GetFineDetectionDuration ());
                m_stateDetection = Simulator::Schedule (mac->GetFineDetectionDuration (), &CrMmWaveMacLow::StopDetectionChannel, this);
                break;
            case INTER_GROUP:
            default:
                NS_FATAL_ERROR ("TypeOfGroup is error");
                break;
        }
    }

    void
    CrMmWaveMacLow::StartBeacon ()
    {
        m_channelAccessManager->StartRequestAccess (BEACON_ACCESS);
    }

    void
    CrMmWaveMacLow::StartDetection ()
    {
        m_channelAccessManager->StartRequestAccess (DETECTION_ACCESS);
    }

    void
    CrMmWaveMacLow::DetectionTxEnd ()
    {
        NS_ASSERT (GetTypeOfGroup () == INTRA_GROUP);
        EndChannelAccess ();
        Ptr<CrMmWaveMac> mac = DynamicCast<CrMmWaveMac> (m_mac);
        m_detectionEvent = Simulator::Schedule (mac->GetDetectionInterval (), &CrMmWaveMacLow::StartDetection, this);
        ToDetection ();
    }

    void
    CrMmWaveMacLow::BeaconTxEnd ()
    {
        NS_ASSERT (GetTypeOfGroup () == INTRA_GROUP);
        EndChannelAccess ();
        if (m_switchChannelFlag)
        {
            ToSwitchChannel (m_switchChannel);
            return;
        }

        Ptr<CrMmWaveMac> mac = DynamicCast<CrMmWaveMac> (m_mac);
        m_beaconEvent = Simulator::Schedule (mac->GetBeaconInterval (), &CrMmWaveMacLow::StartBeacon, this);
        if (m_txop->IsQueueEmpty (GetTypeOfGroup ()) && IsStateIdle ())
        {
            NS_ASSERT (m_waitIfsEvent.IsExpired ());
            m_waitIfsEvent = Simulator::Schedule (GetSifs (), &CrMmWaveMacLow::SendBulkAccessRequest, this);
        }
    }

    void
    CrMmWaveMacLow::StartDetectionChannel (Time duration)
    {
        TypeOfGroup typeOfGroup = GetTypeOfGroup ();
        NS_ASSERT (typeOfGroup != INTER_GROUP);
        m_noActiveUsers = true;
        SetDetectionStartTime (Simulator::Now ());
        SetDetectionDuration (duration);
        m_phy->SetSignalDetectionMode ();
    }

    void
    CrMmWaveMacLow::StopDetectionChannel ()
    {
        MmWaveChannelNumberStandardPair c;
        Ptr<CrMmWaveMac> mac = DynamicCast<CrMmWaveMac> (m_mac);
        NS_ASSERT (mac->GetAccessMode () == MMWAVE_MULTI_CHANNEL);
        NS_ASSERT (!IsStateTx ());

        if (m_noActiveUsers)
        {
            NoSignalDetected ();
        }
        m_noActiveUsers = false;
        m_phy->ResetSignalDetectionMode ();

        switch (GetTypeOfGroup ())
        {
            case INTRA_GROUP:
                m_makeDecision = true;
                ToTransmission();
                if (m_beaconEvent.IsRunning())
                {
                    m_beaconEvent.Cancel();
                }
                StartBeacon ();
                break;
            case PROBE_GROUP:
                c = GetNextChannel (GetCurrentChannel ());
                if (c == GetCurrentChannel ())
                {
                    ToDetection ();
                }
                else
                {
                    if (!IsStateTx ())
                    {
                        ToSwitchChannel (c);
                    }
                }
                break;
            case INTER_GROUP:
            default:
                NS_FATAL_ERROR ("TypeOfGroup is error");
                break;
        }
    }

    void
    CrMmWaveMacLow::StartTransmission (Ptr<MmWaveMacQueueItem> mpdu)
    {
        MmWaveMacState state = GetMacLowState();
        NS_LOG_FUNCTION (this << state);
        NS_ASSERT ((state == INTRA_TRANSMISSION) || (state == INTER_TRANSMISSION));
        NS_ASSERT (!IsStateOff ());
        NS_ASSERT (IsStateIdle ());
        const MmWaveMacHeader& hdr = mpdu->GetHeader ();
        if (hdr.IsCtl ())
        {
            m_currentTxVector = GetCtrlTxVector (mpdu->GetHeader().GetAddr1 ());
        }
        else
        {
            m_currentTxVector = GetDataTxVector (mpdu->GetHeader().GetAddr1 ());
        }
        m_currentPacket = Create<MmWavePsdu> (mpdu->GetPacket (), mpdu->GetHeader ());
        SendPacket ();
        NS_ASSERT (IsStateTx ());
    }

    void
    CrMmWaveMacLow::SendPacket ()
    {
        StartTxTimers (m_currentTxVector);
        Time limit;
        Mac48Address to;
        if (m_currentPacket->GetHeader().IsData ())
        {
            to = m_currentPacket->GetHeader().GetAddr1 ();
            limit = Simulator::GetDelayLeft (m_bulkAccess);
            NS_ASSERT (limit.IsStrictlyPositive ());
            NS_ASSERT (IsWithinSizeAndTimeLimits (m_currentPacket->GetSize (), m_currentTxVector, limit));

            limit -= m_phy->CalculateTxDuration (m_currentPacket->GetSize (), m_currentTxVector, m_phy->GetPhyBand ());
            NS_ASSERT (limit.IsPositive ());
            if (!to.IsGroup ())
            {
                limit += GetSifs ();
                limit += GetBulkAckTxDuration (to);
            }
            m_currentPacket->SetDuration (limit);
        }
        ForwardDown (m_currentPacket, m_currentTxVector);
    }

    void
    CrMmWaveMacLow::StartTxTimers (const MmWaveTxVector txVector)
    {
        Time txDuration = m_phy->CalculateTxDuration (m_currentPacket->GetSize (), txVector, m_phy->GetPhyBand ());
        if (m_currentPacket->GetHeader ().IsData ())
        {
            NS_ASSERT (m_waitIfsEvent.IsExpired ());
            if (m_txop->HasNextFragment (GetTypeOfGroup ()))
            {
                m_waitIfsEvent = Simulator::Schedule (txDuration + GetSifs (), &CrMmWaveMacLow::WaitIfsAfterEndTxFragment, this);
            }
            else if (m_txop->HasNextPacket (GetTypeOfGroup ()))
            {
                m_waitIfsEvent = Simulator::Schedule (txDuration + GetSifs (), &CrMmWaveMacLow::WaitIfsAfterEndTxPacket, this);
            }
        }
        else if (m_currentPacket->GetHeader ().IsBeacon ())
        {
            m_txPacket = Simulator::Schedule (txDuration, &CrMmWaveMacLow::BeaconTxEnd, this);
        }
        else if (m_currentPacket->GetHeader ().IsDetectRequest ())
        {
            m_txPacket = Simulator::Schedule (txDuration, &CrMmWaveMacLow::DetectionTxEnd, this);
        }
        else if (m_currentPacket->GetHeader ().IsBulkRequest ())
        {
            NS_ASSERT (m_bulkResponseTimeout.IsExpired ());
            Time timerDelay = txDuration + GetBulkResponseTimeout (m_self);
            NotifyBulkTimeoutStartNow (timerDelay);
            m_bulkResponseTimeout = Simulator::Schedule (timerDelay, &CrMmWaveMacLow::BulkTimeout, this);
        }
    }

    void
    CrMmWaveMacLow::ReceiveOk (Ptr<MmWavePsdu> psdu, double rxSnr, Time start, Time duration, MmWaveTxVector txVector)
    {
        Ptr<CrMmWaveMac> mac = DynamicCast<CrMmWaveMac> (m_mac);
        Ptr<MmWaveMacQueueItem> mpdu = psdu->GetMpdu ();
        const MmWaveMacHeader& hdr = mpdu->GetHeader ();
        MmWaveMacType type = hdr.GetType ();
        NS_LOG_FUNCTION (this << type << hdr << rxSnr << start << duration);

        if ((mac->GetAccessMode () == MMWAVE_MULTI_CHANNEL)
            && (GetTypeOfGroup () == INTER_GROUP)
            && (mac->IsAnyConflictBetweenGroup ()))
        {
            MmWaveChannelNumberStandardPair c;
            c = mac->GetChannelNeedToAccessForInterGroup ();
            if (c != GetCurrentChannel ())
            {
                if (!IsStateTx ())
                {
                    ToSwitchChannel (c);
                }
            }
            else
            {
                ToTransmission ();
            }
            return;
        }
        Ptr<Packet> packet = mpdu->GetPacket ()->Copy ();
        bool isPrevNavZero = IsNavZero ();
        Mac48Address to = hdr.GetAddr1 ();
        Mac48Address from;
        Time timerDelay;
        if (hdr.IsMgt () || hdr.IsData ())
        {
            from = hdr.GetAddr2 ();
        }

        if (from == GetAddress ())
        {
            return;
        }

        NS_ASSERT(mac != 0);
        NS_ASSERT (!IsStateOff ());
        switch (GetTypeOfGroup ())
        {
            case INTRA_GROUP:
            case INTER_GROUP:
                if (hdr.IsBulkRequest () 
                    && ((GetMacLowState () == INTRA_TRANSMISSION) 
                        ||(GetMacLowState () == INTER_TRANSMISSION)))
                {
                    if (isPrevNavZero && (to == m_self) && (GetTypeOfGroup () == INTRA_GROUP))
                    {
                        m_stationManager->ReportRxOk (from, rxSnr, txVector.GetMode ());
                        
                        BulkRequestHeader requestHeader;
                        packet->RemoveHeader (requestHeader);

                        m_rxBulkAccessInfo = Create<BulkAccessInfo> ();
                        m_rxBulkAccessInfo->m_address = from;
                        m_rxBulkAccessInfo->m_startingSeq = requestHeader.GetStartingSequenceControl ();
                        m_rxBulkAccessInfo->m_numOfPackets = requestHeader.GetNum ();
                        m_rxBulkAccessInfo->m_numOfReceived = 0;
                        m_rxBulkAccessInfo->m_txDuration = requestHeader.GetTxDuration ();
                        m_rxBulkAccessInfo->m_bitmap = 0;
                        m_rxBulkAccessInfo->m_numOfReceived = 0;

                        NS_ASSERT (m_txPacket.IsExpired ());
                        NS_ASSERT (m_sendBulkAck.IsExpired ());
                        m_txPacket = Simulator::Schedule (GetSifs (), &CrMmWaveMacLow::SendBulkResponseAfterRequest, this, m_rxBulkAccessInfo->m_address, m_rxBulkAccessInfo->m_txDuration);
                        timerDelay = GetSifs ()
                                     + GetBulkResponseTxDuration (from)
                                     + m_rxBulkAccessInfo->m_txDuration
                                     + GetSifs ();
                        m_sendBulkAck = Simulator::Schedule (timerDelay, &CrMmWaveMacLow::SendAckAfterData, this);
                        NS_LOG_DEBUG ("rx bulk request from=" << from << ", txDuration=" << m_rxBulkAccessInfo->m_txDuration << ", send bulk ack timer=" << timerDelay);
                    }
                    NotifyUpdateRotationFactor ();
                }
                else if (hdr.IsBulkResponse ()
                         && ((GetMacLowState () == INTRA_TRANSMISSION) 
                             ||(GetMacLowState () == INTER_TRANSMISSION)))
                {
                    if (to == m_self && m_bulkResponseTimeout.IsRunning ())
                    {
                        m_stationManager->ReportRxOk (from, rxSnr, txVector.GetMode ());

                        BulkResponseHeader responseHeader;
                        packet->RemoveHeader (responseHeader);

                        m_bulkResponseTimeout.Cancel ();
                        NotifyBulkTimeoutResetNow ();

                        NS_ASSERT (m_bulkAccess.IsExpired ());
                        NS_ASSERT (m_txBulkAccessInfo != 0);

                        m_bulkAccess = Simulator::Schedule (m_txBulkAccessInfo->m_txDuration, &CrMmWaveMacLow::EndBulkAccess, this);
                        NS_ASSERT (m_txBulkAccessInfo->m_txDuration.IsPositive ());
                        m_txop->SetAccessBuffer (GetTypeOfGroup (), m_txBulkAccessInfo->m_address, m_txBulkAccessInfo->m_txDuration);

                        NS_ASSERT (m_waitIfsEvent.IsExpired ());
                        m_waitIfsEvent = Simulator::Schedule (GetSifs (), &CrMmWaveMacLow::WaitIfsAfterEndTxPacket, this);

                        NS_LOG_DEBUG ("rx bulk response from=" << from << ", txDuration=" << m_txBulkAccessInfo->m_txDuration);
                    }
                }
                else if (hdr.IsBulkAck ()
                         && ((GetMacLowState () == INTRA_TRANSMISSION) 
                             ||(GetMacLowState () == INTER_TRANSMISSION)))
                {
                    if (to == m_self && m_bulkAckTimeout.IsRunning ())
                    {
                        m_stationManager->ReportRxOk (from, rxSnr, txVector.GetMode ());

                        BulkAckHeader ackHeader;
                        packet->RemoveHeader (ackHeader);

                        m_bulkAckTimeout.Cancel ();
                        NotifyAckTimeoutResetNow ();

                        m_txop->GotBulkAck (GetTypeOfGroup (), ackHeader.GetStartingSequenceControl (), ackHeader.GetBitmap ());
                        NS_ASSERT (m_waitIfsEvent.IsExpired ());
                        m_waitIfsEvent = Simulator::Schedule (GetSifs (), &CrMmWaveMacLow::StartNewBulkAccess, this);
                        NS_LOG_DEBUG ("rx bulk ack from=" << from);
                    }
                }
                else if (hdr.IsBeacon ())
                {
                    m_stationManager->ReportRxOk (from, rxSnr, txVector.GetMode ());
                    BeaconHeader beaconHeader;
                    packet->RemoveHeader (beaconHeader);
                    MmWaveChannelNumberStandardPair c;
                    c = m_phy->GetChannelFromChannelNumber (beaconHeader.GetChannelNumber());
                    mac->UpdateNeighborDevice (from, c, Simulator::Now ());
                    NotifyUpdateRotationFactor ();
                    NS_LOG_DEBUG ("rx Beacon from=" << from);
                }
                else if (hdr.IsDetectRequest ())
                {
                    m_stationManager->ReportRxOk (from, rxSnr, txVector.GetMode ());
                    mac->UpdateNeighborDevice (from, GetCurrentChannel (), Simulator::Now ());
                    if (GetTypeOfGroup() == INTRA_GROUP)
                    {
                        ToDetection ();
                    }
                    NotifyUpdateRotationFactor ();
                    NS_LOG_DEBUG ("rx Detection Request from=" << from);
                }
                else if (hdr.IsData ())
                {
                    if ((to == m_self)
                        || (hdr.GetAddr1 ().IsGroup ())
                        || (m_promisc))
                    {
                        NS_LOG_DEBUG ("rx DATA from=" << from);
                        m_stationManager->ReportRxOk (from, rxSnr, txVector.GetMode ());
                        m_rxCallback (mpdu);

                        if (m_sendBulkAck.IsRunning ()
                            && ((GetMacLowState () == INTRA_TRANSMISSION) 
                                 ||(GetMacLowState () == INTER_TRANSMISSION)))
                        {
                            NS_ASSERT (m_rxBulkAccessInfo != 0);
                            if (m_rxBulkAccessInfo->m_address == from)
                            {
                                UpdateBulkAckInfo (from, hdr.GetSequenceNumber ());
                            }
                            if (m_rxBulkAccessInfo->m_numOfPackets == m_rxBulkAccessInfo->m_numOfReceived)
                            {
                                m_sendBulkAck.Cancel ();
                                m_sendBulkAck = Simulator::Schedule (GetSifs (), &CrMmWaveMacLow::SendAckAfterData, this);
                            }
                        }
                    }
                }
                NotifyNav (packet, hdr);
                break;
            case PROBE_GROUP:
                if (hdr.IsBeacon ())
                {
                    m_stationManager->ReportRxOk (from, rxSnr, txVector.GetMode ());
                    BeaconHeader beaconHeader;
                    packet->RemoveHeader (beaconHeader);
                    MmWaveChannelNumberStandardPair c;
                    c = m_phy->GetChannelFromChannelNumber (beaconHeader.GetChannelNumber());
                    mac->UpdateNeighborDevice (from, c, Simulator::Now ());

                }
                else if (hdr.IsDetectRequest ())
                {
                    m_stationManager->ReportRxOk (from, rxSnr, txVector.GetMode ());
                    mac->UpdateNeighborDevice (from, GetCurrentChannel (), Simulator::Now ());
                }
                else if (hdr.IsData () && m_promisc)
                {
                    m_stationManager->ReportRxOk (from, rxSnr, txVector.GetMode ());
                    m_rxCallback (mpdu);
                }
                NotifyNav (packet, hdr);
                break;
            default:
                NS_FATAL_ERROR ("TypeOfGroup is error");
                break;
        }
    }

    void
    CrMmWaveMacLow::ReceiveError (Ptr<MmWavePsdu> psdu, double rxSnr, Time start, Time duration, MmWaveTxVector txVector)
    {
        NS_LOG_FUNCTION (this << GetTypeOfGroup() << *psdu << rxSnr << start << duration << txVector);
    }

    void
    CrMmWaveMacLow::EndTxNoAck ()
    {
        NS_FATAL_ERROR ("should not be here");
    }

    void
    CrMmWaveMacLow::CtsTimeout ()
    {
        NS_FATAL_ERROR ("should not be here");
    }

    void
    CrMmWaveMacLow::NormalAckTimeout ()
    {
        NS_FATAL_ERROR ("should not be here");
    }

} //namespace ns3