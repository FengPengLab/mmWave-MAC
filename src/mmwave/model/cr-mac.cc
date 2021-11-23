/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/simulator.h"
#include "ns3/integer.h"
#include "ns3/node.h"
#include "ns3/mobility-model.h"
#include "ns3/socket.h"
#include "cr-mac.h"

namespace ns3 {
    NS_LOG_COMPONENT_DEFINE ("CrMmWaveMac");
    NS_OBJECT_ENSURE_REGISTERED (CrMmWaveMac);

    TypeId
    CrMmWaveMac::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::CrMmWaveMac")
                .SetParent<MmWaveMac> ()
                .SetGroupName ("MmWave")
                .AddConstructor<CrMmWaveMac> ();
        return tid;
    }

    CrMmWaveMac::CrMmWaveMac ()
    {
        NS_LOG_FUNCTION (this);
        m_rng = CreateObject<UniformRandomVariable> ();
        m_bssid = Mac48Address::GetBroadcast ();

        m_rxMiddle = Create<MmWaveMacRxMiddle> ();
        m_rxMiddle->SetForwardCallback (MakeCallback (&CrMmWaveMac::Receive, this));

        m_txMiddleOfIntraGroup = Create<MmWaveMacTxMiddle> ();
        m_txMiddleOfInterGroup = Create<MmWaveMacTxMiddle> ();

        m_lowOfIntraGroup = CreateObject<CrMmWaveMacLow> ();
        m_lowOfInterGroup = CreateObject<CrMmWaveMacLow> ();
        m_lowOfProbeGroup = CreateObject<CrMmWaveMacLow> ();

        m_lowOfIntraGroup->SetRxCallback (MakeCallback (&MmWaveMacRxMiddle::Receive, m_rxMiddle));
        m_lowOfInterGroup->SetRxCallback (MakeCallback (&MmWaveMacRxMiddle::Receive, m_rxMiddle));
        m_lowOfProbeGroup->SetRxCallback (MakeCallback (&MmWaveMacRxMiddle::Receive, m_rxMiddle));

        m_lowOfIntraGroup->SetMac (this);
        m_lowOfInterGroup->SetMac (this);
        m_lowOfProbeGroup->SetMac (this);

        m_lowOfIntraGroup->SetTypeOfGroup (INTRA_GROUP);
        m_lowOfInterGroup->SetTypeOfGroup (INTER_GROUP);
        m_lowOfProbeGroup->SetTypeOfGroup (PROBE_GROUP);

        m_channelManagerOfIntraGroup = CreateObject<CrDynamicChannelAccessManager> ();
        m_channelManagerOfInterGroup = CreateObject<CrDynamicChannelAccessManager> ();
        m_channelManagerOfProbeGroup = CreateObject<CrDynamicChannelAccessManager> ();

        m_channelManagerOfIntraGroup->SetupMacLow (m_lowOfIntraGroup);
        m_channelManagerOfInterGroup->SetupMacLow (m_lowOfInterGroup);
        m_channelManagerOfProbeGroup->SetupMacLow (m_lowOfProbeGroup);

        m_txop = CreateObject<CrMmWaveTxop> ();
        m_txop->SetTxMiddle (INTRA_GROUP, m_txMiddleOfIntraGroup);
        m_txop->SetTxMiddle (INTER_GROUP, m_txMiddleOfInterGroup);
        m_txop->SetMac (this);
        m_txop->SetMacLowOfIntraGroup (m_lowOfIntraGroup);
        m_txop->SetMacLowOfInterGroup (m_lowOfInterGroup);
        m_txop->SetTxOkCallback (MakeCallback (&CrMmWaveMac::TxOk, this));
        m_txop->SetTxFailedCallback (MakeCallback (&CrMmWaveMac::TxFailed, this));
        m_txop->SetTxDroppedCallback (MakeCallback (&CrMmWaveMac::NotifyTxDrop, this));

//        m_beaconInterval = MilliSeconds (1);
//        m_detectionInterval = MilliSeconds (10);
//        m_fineDetectionDuration = MicroSeconds (800);
//        m_fastDetectionDuration = MicroSeconds (200);

//        m_beaconInterval = MicroSeconds (400);
//        m_detectionInterval = MicroSeconds (800);
//        m_fineDetectionDuration = MicroSeconds (800);
//        m_fastDetectionDuration = MicroSeconds (200);

        m_beaconInterval = MilliSeconds (5);
        m_detectionInterval = MilliSeconds (10);
        m_fineDetectionDuration = MilliSeconds (10);
        m_fastDetectionDuration = MilliSeconds (1);
    }

    CrMmWaveMac::~CrMmWaveMac ()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    CrMmWaveMac::DoInitialize ()
    {
        NS_LOG_FUNCTION (this);
        MmWaveMac::DoInitialize ();
        m_txop->Initialize ();
        m_channelManagerOfIntraGroup->Initialize ();
        m_channelManagerOfInterGroup->Initialize ();
        m_channelManagerOfProbeGroup->Initialize ();
        m_lowOfIntraGroup->Initialize ();
        m_lowOfInterGroup->Initialize ();
        m_lowOfProbeGroup->Initialize ();
    }

    void
    CrMmWaveMac::DoDispose ()
    {
        NS_LOG_FUNCTION (this);
        m_neighborDevices.clear ();

        m_lowOfIntraGroup->Dispose ();
        m_lowOfInterGroup->Dispose ();
        m_lowOfProbeGroup->Dispose ();

        m_channelManagerOfIntraGroup->Dispose ();
        m_channelManagerOfInterGroup->Dispose ();
        m_channelManagerOfProbeGroup->Dispose ();

        m_txop->Dispose ();

        m_rxMiddle = 0;
        m_txMiddleOfIntraGroup = 0;
        m_txMiddleOfInterGroup = 0;

        m_lowOfIntraGroup = 0;
        m_lowOfInterGroup = 0;
        m_lowOfProbeGroup = 0;

        m_channelManagerOfIntraGroup = 0;
        m_channelManagerOfInterGroup = 0;
        m_channelManagerOfProbeGroup = 0;

        m_phyOfIntraGroup = 0;
        m_phyOfInterGroup = 0;
        m_phyOfProbeGroup = 0;

        m_stationManager = 0;
        m_repos = 0;
        m_txop = 0;

        MmWaveMac::DoDispose ();
    }

    void
    CrMmWaveMac::SetAddress (Mac48Address address)
    {
        m_self = address;
        m_lowOfIntraGroup->SetAddress (address);
        m_lowOfInterGroup->SetAddress (address);
        m_lowOfProbeGroup->SetAddress (address);
    }

    void
    CrMmWaveMac::SetBssid (Mac48Address bssid)
    {
        m_bssid = bssid;
    }

    void
    CrMmWaveMac::SetSpectrumInfoRepository (Ptr<MmWaveSpectrumRepository> info)
    {
        m_repos = info;
        m_channelManagerOfIntraGroup->SetupSpectrumRepository (info);
        m_channelManagerOfInterGroup->SetupSpectrumRepository (info);
        m_channelManagerOfProbeGroup->SetupSpectrumRepository (info);
    }

    void
    CrMmWaveMac::SetRemoteStationManager (Ptr<MmWaveRemoteStationManager> stationManager)
    {
        m_stationManager = stationManager;
        m_lowOfIntraGroup->SetRemoteStationManager (stationManager);
        m_lowOfInterGroup->SetRemoteStationManager (stationManager);
        m_lowOfProbeGroup->SetRemoteStationManager (stationManager);
        m_txop->SetRemoteStationManager (stationManager);
    }

    void
    CrMmWaveMac::SetPromisc ()
    {
        m_lowOfIntraGroup->SetPromisc ();
        m_lowOfInterGroup->SetPromisc ();
        m_lowOfProbeGroup->SetPromisc ();
    }

    Ptr <UniformRandomVariable>
    CrMmWaveMac::GetRandomVariable ()
    {
        return m_rng;
    }

    void
    CrMmWaveMac::SetPhy (TypeOfGroup type, Ptr<MmWavePhy> phy)
    {
        switch (type)
        {
            case INTRA_GROUP:
                m_phyOfIntraGroup = phy;
                m_lowOfIntraGroup->SetPhy (phy);
                m_channelManagerOfIntraGroup->SetupChannelAccessListener (phy);
                break;
            case INTER_GROUP:
                m_phyOfInterGroup = phy;
                m_lowOfInterGroup->SetPhy (phy);
                m_channelManagerOfInterGroup->SetupChannelAccessListener (phy);
                break;
            case PROBE_GROUP:
                m_phyOfProbeGroup = phy;
                m_lowOfProbeGroup->SetPhy (phy);
                m_channelManagerOfProbeGroup->SetupChannelAccessListener (phy);
                break;
            default:
                NS_FATAL_ERROR ("TypeOfGroup is error");
                break;
        }

        if (phy->GetChannelToFrequency ().size () == 1)
        {
            SetAccessMode (MMWAVE_SINGLE_CHANNE);
        }
        else if (phy->GetChannelToFrequency ().size () > 1)
        {
            SetAccessMode (MMWAVE_MULTI_CHANNEL);
        }
        else
        {
            NS_FATAL_ERROR ("Number of channels available is error");
        }
    }

    void
    CrMmWaveMac::ResetPhy (TypeOfGroup type)
    {
        switch (type)
        {
            case INTRA_GROUP:
                m_lowOfIntraGroup->ResetPhy ();
                m_channelManagerOfIntraGroup->RemoveChannelAccessListener (m_phyOfIntraGroup);
                m_phyOfIntraGroup = 0;
            case INTER_GROUP:
                m_lowOfInterGroup->ResetPhy ();
                m_channelManagerOfInterGroup->RemoveChannelAccessListener (m_phyOfInterGroup);
                m_phyOfInterGroup = 0;
            case PROBE_GROUP:
                m_lowOfProbeGroup->ResetPhy ();
                m_channelManagerOfProbeGroup->RemoveChannelAccessListener (m_phyOfProbeGroup);
                m_phyOfProbeGroup = 0;
            default:
                NS_FATAL_ERROR ("TypeOfGroup is error");
                break;
        }
    }
    int64_t
    CrMmWaveMac::AssignStreams (int64_t stream)
    {
        m_rng->SetStream (stream);
        return 1;
    }

    Time
    CrMmWaveMac::GetDetectionInterval () const
    {
        return m_detectionInterval;
    }

    Time
    CrMmWaveMac::GetBeaconInterval () const
    {
        return m_beaconInterval;
    }

    Time
    CrMmWaveMac::GetFineDetectionDuration () const
    {
        return m_fineDetectionDuration;
    }

    Time
    CrMmWaveMac::GetFastDetectionDuration () const
    {
        return m_fastDetectionDuration;
    }

    Mac48Address
    CrMmWaveMac::GetAddress () const
    {
        return m_self;
    }

    Mac48Address
    CrMmWaveMac::GetBssid () const
    {
        return m_bssid;
    }

    Ptr<MmWavePhy>
    CrMmWaveMac::GetPhy (TypeOfGroup type) const
    {
        switch (type)
        {
            case INTRA_GROUP:
                return m_phyOfIntraGroup;
            case INTER_GROUP:
                return m_phyOfInterGroup;
            case PROBE_GROUP:
                return m_phyOfProbeGroup;
            default:
                NS_FATAL_ERROR ("TypeOfGroup is error");
                return 0;
        }
    }

    Ptr<CrMmWaveMacLow>
    CrMmWaveMac::GetMacLow (TypeOfGroup type) const
    {
        switch (type)
        {
            case INTRA_GROUP:
                return m_lowOfIntraGroup;
            case INTER_GROUP:
                return m_lowOfInterGroup;
            case PROBE_GROUP:
                return m_lowOfProbeGroup;
            default:
                NS_FATAL_ERROR ("TypeOfGroup is error");
                return 0;
        }
    }

    Ptr<CrDynamicChannelAccessManager>
    CrMmWaveMac::GetChannelAccessManager (TypeOfGroup type) const
    {
        switch (type)
        {
            case INTRA_GROUP:
                return m_channelManagerOfIntraGroup;
            case INTER_GROUP:
                return m_channelManagerOfInterGroup;
            case PROBE_GROUP:
                return m_channelManagerOfProbeGroup;
            default:
                NS_FATAL_ERROR ("TypeOfGroup is error");
                return 0;
        }
    }

    Ptr<MmWaveSpectrumRepository>
    CrMmWaveMac::GetSpectrumRepository () const
    {
        return m_repos;
    }

    Ptr<CrMmWaveTxop>
    CrMmWaveMac::GetTxop () const
    {
        return m_txop;
    }

    void
    CrMmWaveMac::SetLinkUpCallback (Callback<void> linkUp)
    {
        MmWaveMac::SetLinkUpCallback (linkUp);
        linkUp ();
    }

    void
    CrMmWaveMac::SetAccessMode (MmWaveAccessMode mode)
    {
        m_accessMode = mode;
    }

    MmWaveAccessMode
    CrMmWaveMac::GetAccessMode () const
    {
        return m_accessMode;
    }

    bool
    CrMmWaveMac::IsAnyConflictBetweenGroup ()
    {
        NS_LOG_FUNCTION (this);
        return (GetCurrentChannel (INTRA_GROUP) == GetCurrentChannel (INTER_GROUP));
    }

    void
    CrMmWaveMac::Enqueue (Ptr<Packet> packet, Mac48Address to, Mac48Address from)
    {
        NS_LOG_FUNCTION (this);
        NS_FATAL_ERROR ("This MAC entity (" << this << ", " << GetAddress () << ") does not support Enqueue() with from address");
    }

    void
    CrMmWaveMac::Enqueue (Ptr<Packet> packet, Mac48Address to)
    {
        NS_LOG_FUNCTION (this << packet << to);
        SocketPriorityTag priorityTag;
        packet->RemovePacketTag (priorityTag);
        MmWaveMacHeader hdr;
        hdr.SetType (MMWAVE_MAC_DATA);
        hdr.SetAddr1 (to);
        hdr.SetAddr2 (GetAddress ());
        hdr.SetAddr3 (GetAddress ());
        hdr.SetDsNotFrom ();
        hdr.SetDsNotTo ();
        hdr.SetDuration (Seconds (0.0));
        m_txop->Queue (packet, hdr);
    }

    void
    CrMmWaveMac::Receive (Ptr<MmWaveMacQueueItem> mpdu)
    {
        NS_LOG_FUNCTION (this << *mpdu);
        MmWaveMacHeader* hdr = &mpdu->GetHeader ();
        NS_ASSERT (!hdr->IsCtl ());
        Mac48Address to = hdr->GetAddr1 ();
        Mac48Address from = hdr->GetAddr2 ();

        if (hdr->IsData ())
        {
            ForwardUp (mpdu->GetPacket ()->Copy (), from, to);
            return;
        }

        MmWaveMac::Receive (mpdu);
    }

    MmWaveNeighborDevices
    CrMmWaveMac::GetAllNeighborDevices ()
    {
        NS_LOG_FUNCTION (this);
        CheckNeighborDevice ();
        return m_neighborDevices;
    }

    void
    CrMmWaveMac::CheckNeighborDevice ()
    {
        NS_LOG_FUNCTION (this);
        for (auto it = m_neighborDevices.begin (); it != m_neighborDevices.end (); it++)
        {
            NS_LOG_DEBUG("befor CheckNeighborDevice mac=" << (*it).first << ", channel=" << (*it).second->m_channel);
        }
        for (auto it = m_neighborDevices.begin (); it != m_neighborDevices.end (); )
        {
            if (Simulator::Now () > (*it).second->m_stamp + (5 * GetBeaconInterval ()))
            {
                auto curr = it++;
                m_neighborDevices.erase (curr);
            }
            else if (GetAddress() == (*it).first)
            {
                auto curr = it++;
                m_neighborDevices.erase (curr);
            }
            else
            {
                it++;
            }
        }
        for (auto it = m_neighborDevices.begin (); it != m_neighborDevices.end (); it++)
        {
            NS_LOG_DEBUG("after CheckNeighborDevice mac=" << (*it).first << ", channel=" << (*it).second->m_channel);
        }
    }

    void
    CrMmWaveMac::UpdateNeighborDevice (Mac48Address addr, MmWaveChannelNumberStandardPair channel, Time stamp)
    {
        NS_LOG_FUNCTION (this);
        if (GetAddress() == addr)
        {
            return;
        }

        Ptr<MmWaveNeighborDevice> neighbor = Create<MmWaveNeighborDevice> ();
        neighbor->m_address = addr;
        neighbor->m_channel = channel;
        neighbor->m_stamp = stamp;
        m_neighborDevices[addr] = neighbor;

        for (auto it = m_neighborDevices.begin (); it != m_neighborDevices.end (); it++)
        {
            NS_LOG_DEBUG("UpdateNeighborDevice mac=" << (*it).first << ", channel=" << (*it).second->m_channel);
        }
    }

    MmWaveChannelNumberStandardPair
    CrMmWaveMac::GetChannelNeedToAccessForIntraGroup ()
    {
        MmWaveChannelNumberStandardPair c;
        c = m_repos->GetRecommendedChannel (GetCurrentChannel(INTRA_GROUP),
                                            m_phyOfIntraGroup->GetMobility ()->GetPosition (),
                                            GetAllNeighborDevices ());
        NS_LOG_FUNCTION (this << c);
        return c;
    }

    MmWaveChannelNumberStandardPair
    CrMmWaveMac::GetChannelNeedToAccessForInterGroup ()
    {
        NS_LOG_FUNCTION (this);
        MmWaveChannelNumberStandardPair c;
        MmWaveChannelNumberStandardPair u;
        c = m_txop->GetNeedToAccessForInterGroup ();
        if (c == m_lowOfIntraGroup->GetCurrentChannel ())
        {
            if (c == GetCurrentChannel (INTER_GROUP))
            {
                return m_lowOfInterGroup->GetNextChannel (c);
            }
            else
            {
                return GetCurrentChannel (INTER_GROUP);
            }
        }
        else
        {
            return c;
        }
    }

    MmWaveChannelNumberStandardPair
    CrMmWaveMac::GetCurrentChannel (TypeOfGroup type)
    {
        MmWaveChannelNumberStandardPair c;
        switch (type)
        {
            case INTRA_GROUP:
                c = m_lowOfIntraGroup->GetCurrentChannel ();
                break;
            case INTER_GROUP:
                c = m_lowOfInterGroup->GetCurrentChannel ();
                break;
            case PROBE_GROUP:
                c = m_lowOfProbeGroup->GetCurrentChannel ();
                break;
            default:
                NS_FATAL_ERROR ("TypeOfGroup is error");
                break;
        }
        return c;
    }

    MmWaveChannelToFrequencyWidthMap
    CrMmWaveMac::GetChannelToFrequency ()
    {
        MmWaveChannelToFrequencyWidthMap list = m_phyOfIntraGroup->GetChannelToFrequency ();
        NS_ASSERT(list.size() != 0);
        return list;
    }

    void
    CrMmWaveMac::AddSpectrumActivityToRepository (MmWaveChannelNumberStandardPair channel, MmWaveChannelState state, double rxSnr,
                                                  Time start, Time duration, Time detectionStart, Time detectionDuration)
    {
        NS_LOG_FUNCTION (this << channel << state << rxSnr << start << duration << detectionStart << detectionDuration);
        NS_ASSERT (m_repos != 0);
        Ptr<MmWaveSpectrumData> data = Create<MmWaveSpectrumData> ();
        data->SetSnr (rxSnr);
        data->SetChannelState (state);
        data->SetRelativeStart (start);
        data->SetOccupiedDuration (duration);
        data->SetDetectionStart (detectionStart);
        data->SetDetectionEnd (detectionStart + detectionDuration);
        data->SetPosition (m_phyOfIntraGroup->GetMobility ()->GetPosition ());
        data->SetUpdateTime (Simulator::Now ());
        m_repos->AddActivityInfoToRepository (channel, data);
    }

    Ptr<MmWaveSpectrumStatistical>
    CrMmWaveMac::GetStatisticalInfo (MmWaveChannelNumberStandardPair channel)
    {
        NS_LOG_FUNCTION (this);
        return m_repos->GetStatisticalInfo (m_phyOfIntraGroup->GetMobility ()->GetPosition (), channel);
    }
}