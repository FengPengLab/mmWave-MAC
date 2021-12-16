/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/llc-snap-header.h"
#include "ns3/arp-header.h"
#include "cr-net-device.h"
namespace ns3 {
    NS_LOG_COMPONENT_DEFINE ("CrMmWaveNetDevice");
    NS_OBJECT_ENSURE_REGISTERED (CrMmWaveNetDevice);

    TypeId
    CrMmWaveNetDevice::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::CrMmWaveNetDevice")
                .SetParent<NetDevice> ()
                .AddConstructor<CrMmWaveNetDevice> ()
                .SetGroupName ("MmWave")
                .AddAttribute ("Mtu", "The MAC-level Maximum Transmission Unit",
                               UintegerValue (MAX_MSDU_SIZE - LLC_SNAP_HEADER_LENGTH),
                               MakeUintegerAccessor (&CrMmWaveNetDevice::SetMtu, &CrMmWaveNetDevice::GetMtu),
                               MakeUintegerChecker<uint16_t> (1, MAX_MSDU_SIZE - LLC_SNAP_HEADER_LENGTH))
                .AddAttribute ("Channel", "The channel attached to this device",
                               PointerValue (),
                               MakePointerAccessor (&CrMmWaveNetDevice::GetChannel),
                               MakePointerChecker<Channel> ())
                .AddAttribute ("PhyIntraGroup", "The PHY entities attached to this device.",
                               PointerValue (),
                               MakePointerAccessor (&CrMmWaveNetDevice::m_phyForIntraGroup),
                               MakePointerChecker<MmWavePhy> ())
                .AddAttribute ("PhyInterGroup", "The PHY entities attached to this device.",
                               PointerValue (),
                               MakePointerAccessor (&CrMmWaveNetDevice::m_phyForInterGroup),
                               MakePointerChecker<MmWavePhy> ())
                .AddAttribute ("PhyProbeGroup", "The PHY entities attached to this device.",
                               PointerValue (),
                               MakePointerAccessor (&CrMmWaveNetDevice::m_phyForProbeGroup),
                               MakePointerChecker<MmWavePhy> ())
                .AddAttribute ("Mac", "The MAC layer attached to this device.",
                               PointerValue (),
                               MakePointerAccessor (&CrMmWaveNetDevice::m_mac),
                               MakePointerChecker<CrMmWaveMac> ())
                .AddAttribute ("RemoteStationManager", "The station manager attached to this device.",
                               PointerValue (),
                               MakePointerAccessor (&CrMmWaveNetDevice::m_stationManager),
                               MakePointerChecker<MmWaveRemoteStationManager> ())
                .AddAttribute ("MmWaveSpectrumRepository",
                               "The Cognitive Radio Spectrum Info Repository.",
                               PointerValue (),
                               MakePointerAccessor (&CrMmWaveNetDevice::m_repository),
                               MakePointerChecker<MmWaveSpectrumRepository> ());
        return tid;
    }

    CrMmWaveNetDevice::CrMmWaveNetDevice ()
    {
        NS_LOG_FUNCTION (this);
        m_configComplete = false;
        m_standard = MMWAVE_UNSPECIFIED;
    }

    CrMmWaveNetDevice::~CrMmWaveNetDevice ()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    CrMmWaveNetDevice::DoDispose ()
    {
        NS_LOG_FUNCTION (this);
        m_node = 0;
        if (m_mac)
        {
            m_mac->Dispose ();
            m_mac = 0;
        }
        if (m_phyForIntraGroup)
        {
            m_phyForIntraGroup->Dispose ();
            m_phyForIntraGroup = 0;
        }
        if (m_phyForInterGroup)
        {
            m_phyForInterGroup->Dispose ();
            m_phyForInterGroup = 0;
        }
        if (m_phyForProbeGroup)
        {
            m_phyForProbeGroup->Dispose ();
            m_phyForProbeGroup = 0;
        }
        if (m_stationManager)
        {
            m_stationManager->Dispose ();
            m_stationManager = 0;
        }
        if (m_repository)
        {
            m_repository->Dispose ();
            m_repository = 0;
        }
        NetDevice::DoDispose ();
    }

    void
    CrMmWaveNetDevice::DoInitialize ()
    {
        NS_LOG_FUNCTION (this);
        if (m_phyForIntraGroup)
        {
            m_phyForIntraGroup->Initialize ();
        }
        if (m_phyForInterGroup)
        {
            m_phyForInterGroup->Initialize ();
        }
        if (m_phyForProbeGroup)
        {
            m_phyForProbeGroup->Initialize ();
        }
        if (m_stationManager)
        {
            m_stationManager->Initialize ();
        }
        if (m_repository)
        {
            m_repository->Initialize ();
        }
        if (m_mac)
        {
            m_mac->Initialize ();
        }
        NetDevice::DoInitialize ();
    }

    void
    CrMmWaveNetDevice::CompleteConfig ()
    {
        if (m_repository == 0
            || m_mac == 0
            || m_stationManager == 0
            || m_node == 0
            || m_repository == 0
            || m_phyForIntraGroup == 0
            || m_phyForInterGroup == 0
            || m_phyForProbeGroup == 0
            || m_standard == MMWAVE_UNSPECIFIED
            || m_configComplete)
        {
            return;
        }
        m_phyForIntraGroup->ConfigureStandardAndBand (mmWaveStandards.at(m_standard).m_phyStandard, mmWaveStandards.at(m_standard).m_phyBand);
        m_phyForInterGroup->ConfigureStandardAndBand (mmWaveStandards.at(m_standard).m_phyStandard, mmWaveStandards.at(m_standard).m_phyBand);
        m_phyForProbeGroup->ConfigureStandardAndBand (mmWaveStandards.at(m_standard).m_phyStandard, mmWaveStandards.at(m_standard).m_phyBand);
        m_repository->SetNetDevice (this);
        m_repository->SetChannelToFrequencyWidth (mmWaveStandards.at(m_standard).m_phyStandard, mmWaveStandards.at(m_standard).m_phyBand);
        m_mac->SetRemoteStationManager (m_stationManager);
        m_mac->SetPhy (INTRA_GROUP, m_phyForIntraGroup);
        m_mac->SetPhy (INTER_GROUP, m_phyForInterGroup);
        m_mac->SetPhy (PROBE_GROUP, m_phyForProbeGroup);
        m_mac->SetSpectrumInfoRepository (m_repository);
        m_mac->SetForwardUpCallback (MakeCallback (&CrMmWaveNetDevice::ForwardUp, this));
        m_mac->SetLinkUpCallback (MakeCallback (&CrMmWaveNetDevice::LinkUp, this));
        m_mac->SetLinkDownCallback (MakeCallback (&CrMmWaveNetDevice::LinkDown, this));
        m_stationManager->SetupMac (m_mac);
        m_stationManager->InitializePhyParams (m_phyForIntraGroup->GetNumberOfAntennas (),
                                               m_phyForIntraGroup->GetMaxSupportedTxSpatialStreams (),
                                               m_phyForIntraGroup->GetChannelWidth ());

        m_phyForIntraGroup->m_typeOfGroup = INTRA_GROUP;
        m_phyForInterGroup->m_typeOfGroup = INTER_GROUP;
        m_phyForProbeGroup->m_typeOfGroup = PROBE_GROUP;
        m_configComplete = true;
    }

    void
    CrMmWaveNetDevice::SetMac (const Ptr <CrMmWaveMac> mac)
    {
        m_mac = mac;
        CompleteConfig ();
    }

    void
    CrMmWaveNetDevice::SetPhy (TypeOfGroup typeOfGroup, const Ptr <MmWavePhy> phy)
    {
        switch (typeOfGroup)
        {
            case INTRA_GROUP:
                m_phyForIntraGroup = phy;
                CompleteConfig ();
                break;
            case INTER_GROUP:
                m_phyForInterGroup = phy;
                CompleteConfig ();
                break;
            case PROBE_GROUP:
                m_phyForProbeGroup = phy;
                CompleteConfig ();
                break;
            default:
                NS_FATAL_ERROR ("TypeOfGroup is error");
                break;
        }
    }

    void
    CrMmWaveNetDevice::SetRemoteStationManager (const Ptr <MmWaveRemoteStationManager> manager)
    {
        m_stationManager = manager;
        CompleteConfig ();
    }

    void
    CrMmWaveNetDevice::SetSpectrumRepository (const Ptr <MmWaveSpectrumRepository> repo)
    {
        m_repository = repo;
        CompleteConfig ();
    }

    void
    CrMmWaveNetDevice::SetStandard (MmWaveStandard standard)
    {
        m_standard = standard;
    }

    Ptr <CrMmWaveMac>
    CrMmWaveNetDevice::GetMac () const
    {
        return m_mac;
    }

    Ptr <MmWavePhy>
    CrMmWaveNetDevice::GetPhy (TypeOfGroup typeOfGroup) const
    {
        switch (typeOfGroup)
        {
            case INTRA_GROUP:
                return m_phyForIntraGroup;
            case INTER_GROUP:
                return m_phyForInterGroup;
            case PROBE_GROUP:
                return m_phyForProbeGroup;
            default:
                NS_FATAL_ERROR ("TypeOfGroup is error");
                return 0;
        }
    }

    Ptr<MmWaveRemoteStationManager>
    CrMmWaveNetDevice::GetRemoteStationManager () const
    {
        return m_stationManager;
    }

    Ptr<MmWaveSpectrumRepository>
    CrMmWaveNetDevice::GetSpectrumRepository () const
    {
        return m_repository;
    }

    void
    CrMmWaveNetDevice::SetIfIndex (const uint32_t index)
    {
        m_ifIndex = index;
    }

    uint32_t
    CrMmWaveNetDevice::GetIfIndex () const
    {
        return m_ifIndex;
    }

    Ptr<Channel>
    CrMmWaveNetDevice::GetChannel () const
    {
        return m_phyForIntraGroup->GetChannel ();
    }

    void
    CrMmWaveNetDevice::SetAddress (Address address)
    {
        m_mac->SetAddress (Mac48Address::ConvertFrom (address));
    }

    Address
    CrMmWaveNetDevice::GetAddress () const
    {
        return m_mac->GetAddress ();
    }

    bool
    CrMmWaveNetDevice::SetMtu (const uint16_t mtu)
    {
        if (mtu > MAX_MSDU_SIZE - LLC_SNAP_HEADER_LENGTH)
        {
            return false;
        }
        m_mtu = mtu;
        return true;
    }

    uint16_t
    CrMmWaveNetDevice::GetMtu () const
    {
        return m_mtu;
    }

    bool
    CrMmWaveNetDevice::IsLinkUp () const
    {
        return (m_linkUp && (m_phyForIntraGroup != 0) && (m_phyForInterGroup != 0) && (m_phyForProbeGroup != 0));
    }

    void
    CrMmWaveNetDevice::AddLinkChangeCallback (Callback<void> callback)
    {
        m_linkChanges.ConnectWithoutContext (callback);
    }

    bool
    CrMmWaveNetDevice::IsBroadcast () const
    {
        return true;
    }

    Address
    CrMmWaveNetDevice::GetBroadcast () const
    {
        return Mac48Address::GetBroadcast ();
    }

    bool
    CrMmWaveNetDevice::IsMulticast () const
    {
        return true;
    }

    Address
    CrMmWaveNetDevice::GetMulticast (Ipv4Address multicastGroup) const
    {
        return Mac48Address::GetMulticast (multicastGroup);
    }

    Address CrMmWaveNetDevice::GetMulticast (Ipv6Address addr) const
    {
        return Mac48Address::GetMulticast (addr);
    }

    bool
    CrMmWaveNetDevice::IsPointToPoint () const
    {
        NS_LOG_FUNCTION (this);
        return false;
    }

    bool
    CrMmWaveNetDevice::IsBridge () const
    {
        return false;
    }

    Ptr <Node>
    CrMmWaveNetDevice::GetNode () const
    {
        return m_node;
    }

    void
    CrMmWaveNetDevice::SetNode (const Ptr <Node> node)
    {
        m_node = node;
        CompleteConfig ();
    }

    bool
    CrMmWaveNetDevice::NeedsArp () const
    {
        return true;
    }

    void
    CrMmWaveNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
    {
        m_forwardUp = cb;
    }

    void
    CrMmWaveNetDevice::ForwardUp (Ptr <const Packet> packet, Mac48Address from, Mac48Address to)
    {
        NS_LOG_FUNCTION (this << packet << from << to);
        LlcSnapHeader llc;
        NetDevice::PacketType type;
        if (to.IsBroadcast ())
        {
            type = NetDevice::PACKET_BROADCAST;
        }
        else if (to.IsGroup ())
        {
            type = NetDevice::PACKET_MULTICAST;
        }
        else if (to == m_mac->GetAddress ())
        {
            type = NetDevice::PACKET_HOST;
        }
        else
        {
            type = NetDevice::PACKET_OTHERHOST;
        }
        Ptr<Packet> copy = packet->Copy ();
        if (type != NetDevice::PACKET_OTHERHOST)
        {
            m_mac->NotifyRx (packet);
            copy->RemoveHeader (llc);
            m_forwardUp (this, copy, llc.GetType (), from);
        }
        else
        {
            copy->RemoveHeader (llc);
        }

        if (!m_promiscRx.IsNull ())
        {
            m_mac->NotifyPromiscRx (copy);
            m_promiscRx (this, copy, llc.GetType (), from, to, type);
        }
    }

    void
    CrMmWaveNetDevice::LinkUp ()
    {
        m_linkUp = true;
        m_linkChanges ();
    }

    void
    CrMmWaveNetDevice::LinkDown ()
    {
        NS_LOG_FUNCTION (this);
        m_linkUp = false;
        m_linkChanges ();
    }

    bool
    CrMmWaveNetDevice::Send (Ptr <Packet> packet, const Address &dest, uint16_t protocolNumber)
    {
        NS_LOG_FUNCTION (this << packet << dest << protocolNumber);
        NS_ASSERT (Mac48Address::IsMatchingType (dest));
        Mac48Address realTo = Mac48Address::ConvertFrom (dest);
        LlcSnapHeader llc;
        llc.SetType (protocolNumber);
        packet->AddHeader (llc);
        m_mac->NotifyTx (packet);
        m_mac->Enqueue (packet, realTo);
        return true;
    }

    bool
    CrMmWaveNetDevice::SendFrom (Ptr <Packet> packet, const Address &source, const Address &dest, uint16_t protocolNumber)
    {
        NS_LOG_FUNCTION (this << packet << source << dest << protocolNumber);
        NS_ASSERT (Mac48Address::IsMatchingType (dest));
        NS_ASSERT (Mac48Address::IsMatchingType (source));
        Mac48Address realTo = Mac48Address::ConvertFrom (dest);
        Mac48Address realFrom = Mac48Address::ConvertFrom (source);
        LlcSnapHeader llc;
        llc.SetType (protocolNumber);
        packet->AddHeader (llc);
        m_mac->NotifyTx (packet);
        m_mac->Enqueue (packet, realTo, realFrom);
        return true;
    }

    void
    CrMmWaveNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
    {
        m_promiscRx = cb;
        m_mac->SetPromisc ();
    }

    bool
    CrMmWaveNetDevice::SupportsSendFrom () const
    {
        return m_mac->SupportsSendFrom ();
    }
}