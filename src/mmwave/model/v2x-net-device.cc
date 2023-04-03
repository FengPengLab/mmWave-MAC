/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include <algorithm>
#include "ns3/simulator.h"
#include "ns3/socket.h"
#include "ns3/net-device-queue-interface.h"
#include "ns3/llc-snap-header.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "ns3/object-map.h"
#include "ns3/object-vector.h"
#include "ns3/object-base.h"
#include "ns3/random-variable-stream.h"
#include "ns3/channel-coordinator.h"
#include "ns3/channel-manager.h"
#include "ns3/mac-low.h"
#include "ns3/mac-low-transmission-parameters.h"
#include "ns3/wifi-phy.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/wifi-mac-queue.h"
#include "ns3/wifi-mac-queue-item.h"
#include "ns3/wifi-mac-trailer.h"
#include "mmwave-phy.h"
#include "mmwave-mac-trailer.h"
#include "v2x-vsa-manager.h"
#include "v2x-net-device.h"
#include "v2x-ctrl-mac.h"
#include "v2x-ctrl-mac-low.h"
#include "v2x-data-mac.h"
#include "v2x-data-mac-low.h"
#include "v2x-channel-scheduler.h"
#include "v2x-contention-free-access.h"

namespace ns3 {
    NS_LOG_COMPONENT_DEFINE ("V2xMmWaveNetDevice");
    NS_OBJECT_ENSURE_REGISTERED (V2xMmWaveNetDevice);

    TypeId
    V2xMmWaveNetDevice::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::V2xMmWaveNetDevice")
                .SetParent<NetDevice> ()
                .SetGroupName ("MmWave")
                .AddConstructor<V2xMmWaveNetDevice> ()
                .AddAttribute ("Mtu", "The MAC-level Maximum Transmission Unit",
                               UintegerValue (MAX_MSDU_SIZE - LLC_SNAP_HEADER_LENGTH),
                               MakeUintegerAccessor (&V2xMmWaveNetDevice::SetMtu,
                                                     &V2xMmWaveNetDevice::GetMtu),
                               MakeUintegerChecker<uint16_t> (1,MAX_MSDU_SIZE - LLC_SNAP_HEADER_LENGTH))
                .AddAttribute ("Channel", "The channel attached to this device",
                               PointerValue (),
                               MakePointerAccessor (&V2xMmWaveNetDevice::GetChannel),
                               MakePointerChecker<Channel> ())
                .AddAttribute ("DataPhy", "The PHY layer attached to this device.",
                               PointerValue (),
                               MakePointerAccessor (&V2xMmWaveNetDevice::m_dataPhy),
                               MakePointerChecker<MmWavePhy> ())
                .AddAttribute ("DataMac", "The MAC layer attached to this device.",
                               PointerValue (),
                               MakePointerAccessor (&V2xMmWaveNetDevice::m_dataMac),
                               MakePointerChecker<V2xDataMac> ())
                .AddAttribute ("DataRemoteStationManager", "The station manager attached to this device.",
                               PointerValue (),
                               MakePointerAccessor (&V2xMmWaveNetDevice::m_dataStationManager),
                               MakePointerChecker<MmWaveRemoteStationManager> ())
                .AddAttribute ("CtrlPhyEntities", "The PHY entities attached to this device.",
                               ObjectVectorValue (),
                               MakeObjectVectorAccessor (&V2xMmWaveNetDevice::m_ctrlPhyEntities),
                               MakeObjectVectorChecker<WifiPhy> ())
                .AddAttribute ("CtrlMacEntities", "The MAC layer attached to this device.",
                               ObjectMapValue (),
                               MakeObjectMapAccessor (&V2xMmWaveNetDevice::m_ctrlMacEntities),
                               MakeObjectMapChecker<V2xCtrlMac> ())
                .AddAttribute ("ChannelManager", "The channel manager attached to this device.",
                               PointerValue (),
                               MakePointerAccessor (&V2xMmWaveNetDevice::SetChannelManager,
                                                    &V2xMmWaveNetDevice::GetChannelManager),
                               MakePointerChecker<ChannelManager> ())
                .AddAttribute ("ChannelCoordinator", "The channel coordinator attached to this device.",
                               PointerValue (),
                               MakePointerAccessor (&V2xMmWaveNetDevice::SetChannelCoordinator,
                                                    &V2xMmWaveNetDevice::GetChannelCoordinator),
                               MakePointerChecker<ChannelCoordinator> ())
                .AddAttribute ("ContentionFreeAccess", "The contention free access attached to this device.",
                               PointerValue (),
                               MakePointerAccessor (&V2xMmWaveNetDevice::SetContentionFreeAccess,
                                                    &V2xMmWaveNetDevice::GetContentionFreeAccess),
                               MakePointerChecker<V2xContentionFreeAccess> ())
                .AddAttribute ("ChannelScheduler", "The channel scheduler attached to this device.",
                               PointerValue (),
                               MakePointerAccessor (&V2xMmWaveNetDevice::SetChannelScheduler,
                                                    &V2xMmWaveNetDevice::GetChannelScheduler),
                               MakePointerChecker<V2xChannelScheduler> ())
                .AddAttribute ("VsaManager", "The VSA manager attached to this device.",
                               PointerValue (),
                               MakePointerAccessor (&V2xMmWaveNetDevice::SetVsaManager,
                                                    &V2xMmWaveNetDevice::GetVsaManager),
                               MakePointerChecker<V2xVsaManager> ())
                .AddAttribute ("V2xVsaManager", "The VSA manager attached to this device.",
                               PointerValue (),
                               MakePointerAccessor (&V2xMmWaveNetDevice::SetVsaManager,
                                                    &V2xMmWaveNetDevice::GetVsaManager),
                               MakePointerChecker<V2xVsaManager> ())
        ;
        return tid;
    }

    V2xMmWaveNetDevice::V2xMmWaveNetDevice()
            : m_txProfile(0),
              m_configComplete(false)
    {
        m_queue = CreateObject<MmWaveMacQueue> ();
        m_rng = CreateObject<UniformRandomVariable> ();
        m_maxQueueItemNumber = 10;
        m_beaconInterval = MicroSeconds (100);
        m_txDuration = MicroSeconds (500);
    }

    V2xMmWaveNetDevice::~V2xMmWaveNetDevice ()
    {
    }

    void
    V2xMmWaveNetDevice::DoDispose ()
    {
        if (m_txProfile != 0)
        {
            delete m_txProfile;
            m_txProfile = 0;
        }

        for (PhyEntitiesI i = m_ctrlPhyEntities.begin (); i != m_ctrlPhyEntities.end (); ++i)
        {
            Ptr<WifiPhy> phy = (*i);
            phy->Dispose ();
        }
        m_ctrlPhyEntities.clear ();
        for (MacEntitiesI i = m_ctrlMacEntities.begin (); i != m_ctrlMacEntities.end (); ++i)
        {
            Ptr<V2xCtrlMac> mac = i->second;
            Ptr<WifiRemoteStationManager> stationManager = mac->GetWifiRemoteStationManager ();
            stationManager->Dispose ();
            mac->Dispose ();
        }
        m_ctrlMacEntities.clear ();

        m_dataMac->Dispose ();
        m_dataPhy->Dispose ();
        m_dataStationManager->Dispose ();
        m_channelCoordinator->Dispose ();
        m_channelManager->Dispose ();
        m_channelScheduler->Dispose ();
        m_contentionFreeAccess->Dispose();
        m_vsaManager->Dispose ();
        m_node = 0;
        m_dataMac = 0;
        m_dataPhy = 0;
        m_dataStationManager = 0;
        m_queueInterface = 0;
        m_channelCoordinator = 0;
        m_channelManager = 0;
        m_channelScheduler = 0;
        m_contentionFreeAccess = 0;
        m_vsaManager = 0;
        m_queue = 0;
        m_rng = 0;
        // chain up.
        NetDevice::DoDispose ();
    }

    void
    V2xMmWaveNetDevice::DoInitialize ()
    {
        if (m_ctrlPhyEntities.size () == 0)
        {
            NS_FATAL_ERROR ("there is no PHY entity in this WAVE device");
        }
        for (PhyEntitiesI i = m_ctrlPhyEntities.begin (); i != m_ctrlPhyEntities.end (); ++i)
        {
            Ptr<WifiPhy> phy = (*i);
            phy->Initialize ();
        }
        if (m_ctrlMacEntities.size () == 0)
        {
            NS_FATAL_ERROR ("there is no MAC entity in this WAVE device");
        }
        for (MacEntitiesI i = m_ctrlMacEntities.begin (); i != m_ctrlMacEntities.end (); ++i)
        {
            Ptr<V2xCtrlMac> mac = i->second;
            mac->SetForwardUpCallback (MakeCallback (&V2xMmWaveNetDevice::ForwardUp, this));
            mac->SetLinkUpCallback (MakeCallback (&V2xMmWaveNetDevice::LinkUp, this));
            mac->SetLinkDownCallback (MakeCallback (&V2xMmWaveNetDevice::LinkDown, this));
            mac->SetRxBeaconCallback (MakeCallback (&V2xMmWaveNetDevice::NotifyCtrlRxBeacon, this));
            mac->SetTxNoAckCallback (MakeCallback (&V2xMmWaveNetDevice::NotifyCtrlTxOK, this));
            mac->SetAddRequestCallback (MakeCallback (&V2xMmWaveNetDevice::AddRequest, this));
            mac->SetAddAgreementCallback (MakeCallback (&V2xMmWaveNetDevice::AddAgreement, this));
            mac->SetReplaceBeaconCallback (MakeCallback (&V2xMmWaveNetDevice::ReplaceBeacon, this));
            mac->Suspend ();
            mac->Initialize ();

            Ptr<WifiRemoteStationManager> stationManager = mac->GetWifiRemoteStationManager ();
            stationManager->SetupPhy (m_ctrlPhyEntities[0]);
            stationManager->Initialize ();
        }

        m_channelScheduler->SetV2xMmWaveNetDevice (this);
        m_vsaManager->SetV2xMmWaveNetDevice (this);
        m_channelScheduler->Initialize ();
        m_channelCoordinator->Initialize ();
        m_channelManager->Initialize ();
        m_vsaManager->Initialize ();
        m_dataPhy->Initialize ();
        m_dataMac->Initialize ();
        m_dataStationManager->Initialize ();

        Time start = m_rng->GetInteger (0, 20) * m_ctrlPhyEntities[0]->GetSifs ();
        m_beaconEvent = Simulator::Schedule (start, &V2xMmWaveNetDevice::SendOneBeacon, this);

        NetDevice::DoInitialize ();
    }

    void
    V2xMmWaveNetDevice::CompleteConfig ()
    {
        if (m_dataMac == 0
            || m_dataPhy == 0
            || m_dataStationManager == 0
            || m_node == 0
            || m_configComplete)
        {
            return;
        }

        m_dataMac->SetRemoteStationManager (m_dataStationManager);
        m_dataMac->SetPhy (m_dataPhy);
        m_dataMac->SetForwardUpCallback (MakeCallback (&V2xMmWaveNetDevice::ForwardUp, this));
        m_dataMac->SetLinkUpCallback (MakeCallback (&V2xMmWaveNetDevice::LinkUp, this));
        m_dataMac->SetLinkDownCallback (MakeCallback (&V2xMmWaveNetDevice::LinkDown, this));
        m_dataMac->SetTxOkCallback (MakeCallback (&V2xMmWaveNetDevice::NotifyDataTxOK, this));
        m_dataMac->SetTxFailedCallback (MakeCallback (&V2xMmWaveNetDevice::NotifyDataTxFailed, this));

        m_dataStationManager->SetupMac (m_dataMac);
        m_dataStationManager->InitializePhyParams (m_dataPhy->GetNumberOfAntennas (),
                                                   m_dataPhy->GetMaxSupportedTxSpatialStreams (),
                                                   m_dataPhy->GetChannelWidth ());
        m_configComplete = true;
    }


    void
    V2xMmWaveNetDevice::AddCtrlMac (uint32_t channelNumber, Ptr<V2xCtrlMac> mac)
    {
        if (!ChannelManager::IsWaveChannel (channelNumber))
        {
            NS_FATAL_ERROR ("The channel " << channelNumber << " is not a valid WAVE channel number");
        }
        if (m_ctrlMacEntities.find (channelNumber) != m_ctrlMacEntities.end ())
        {
            NS_FATAL_ERROR ("The MAC entity for channel " << channelNumber << " already exists.");
        }
        m_ctrlMacEntities.insert (std::make_pair (channelNumber, mac));
    }
    
    Ptr<V2xCtrlMac>
    V2xMmWaveNetDevice::GetCtrlMac (uint32_t channelNumber) const
    {
        MacEntitiesI i = m_ctrlMacEntities.find (channelNumber);
        if (i == m_ctrlMacEntities.end ())
        {
            NS_FATAL_ERROR ("there is no available MAC entity for channel " << channelNumber);
        }
        return i->second;
    }

    std::map<uint32_t, Ptr<V2xCtrlMac> >
    V2xMmWaveNetDevice::GetCtrlMacs () const
    {
        return m_ctrlMacEntities;
    }

    void
    V2xMmWaveNetDevice::AddCtrlPhy (Ptr<WifiPhy> phy)
    {
        if (std::find (m_ctrlPhyEntities.begin (), m_ctrlPhyEntities.end (), phy) != m_ctrlPhyEntities.end ())
        {
            NS_FATAL_ERROR ("This PHY entity is already inserted");
        }
        m_ctrlPhyEntities.push_back (phy);
    }
    
    Ptr<WifiPhy>
    V2xMmWaveNetDevice::GetCtrlPhy (uint32_t index) const
    {
        return m_ctrlPhyEntities.at (index);
    }

    std::vector<Ptr<WifiPhy> >
    V2xMmWaveNetDevice::GetCtrlPhys () const
    {
        return m_ctrlPhyEntities;
    }
    
    void
    V2xMmWaveNetDevice::SetDataMac (Ptr<V2xDataMac> mac)
    {
        m_dataMac = mac;
        CompleteConfig ();
    }
    
    void
    V2xMmWaveNetDevice::SetDataPhy (Ptr<MmWavePhy> phy)
    {
        m_dataPhy = phy;
        CompleteConfig ();
    }

    void
    V2xMmWaveNetDevice::SetDataRemoteStationManager (Ptr<MmWaveRemoteStationManager> manager)
    {
        m_dataStationManager = manager;
        CompleteConfig ();
    }

    void
    V2xMmWaveNetDevice::SetNode (Ptr<Node> node)
    {
        m_node = node;
        CompleteConfig ();
    }

    void
    V2xMmWaveNetDevice::SetVsaManager (Ptr<V2xVsaManager> vsaManager)
    {
        m_vsaManager = vsaManager;
    }

    Ptr<MmWaveRemoteStationManager>
    V2xMmWaveNetDevice::GetDataRemoteStationManager () const
    {
        return m_dataStationManager;
    }

    Ptr<V2xVsaManager>
    V2xMmWaveNetDevice::GetVsaManager () const
    {
        return m_vsaManager;
    }

    Ptr<V2xDataMac>
    V2xMmWaveNetDevice::GetDataMac () const
    {
        return m_dataMac;
    }

    Ptr<MmWavePhy>
    V2xMmWaveNetDevice::GetDataPhy () const
    {
        return m_dataPhy;
    }

    bool
    V2xMmWaveNetDevice::SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocol)
    {
        NS_LOG_FUNCTION (this << packet << source << dest << protocol);
        return false;
    }

    void
    V2xMmWaveNetDevice::SetChannelManager (Ptr<ChannelManager> channelManager)
    {
        m_channelManager = channelManager;
    }

    Ptr<ChannelManager>
    V2xMmWaveNetDevice::GetChannelManager () const
    {
        return m_channelManager;
    }

    void
    V2xMmWaveNetDevice::SetChannelScheduler (Ptr<V2xChannelScheduler> channelScheduler)
    {
        m_channelScheduler = channelScheduler;
    }

    Ptr<V2xChannelScheduler>
    V2xMmWaveNetDevice::GetChannelScheduler () const
    {
        return m_channelScheduler;
    }

    void
    V2xMmWaveNetDevice::SetChannelCoordinator (Ptr<ChannelCoordinator> channelCoordinator)
    {
        m_channelCoordinator = channelCoordinator;
    }

    Ptr<ChannelCoordinator>
    V2xMmWaveNetDevice::GetChannelCoordinator () const
    {
        return m_channelCoordinator;
    }

    void
    V2xMmWaveNetDevice::SetContentionFreeAccess (Ptr<V2xContentionFreeAccess> contentionFreeAccess)
    {
        m_contentionFreeAccess = contentionFreeAccess;
        m_contentionFreeAccess->SetDevice(this);
    }

    Ptr<V2xContentionFreeAccess>
    V2xMmWaveNetDevice::GetContentionFreeAccess () const
    {
        return m_contentionFreeAccess;
    }

    void
    V2xMmWaveNetDevice::SetIfIndex (const uint32_t index)
    {
        m_ifIndex = index;
    }

    uint32_t
    V2xMmWaveNetDevice::GetIfIndex () const
    {
        return m_ifIndex;
    }

    Ptr<Channel>
    V2xMmWaveNetDevice::GetChannel () const
    {
        return m_dataPhy->GetChannel ();
    }

    void
    V2xMmWaveNetDevice::SetAddress (Address address)
    {
        for (MacEntitiesI i = m_ctrlMacEntities.begin (); i != m_ctrlMacEntities.end (); ++i)
        {
            i->second->SetAddress (Mac48Address::ConvertFrom (address));
        }
        m_dataMac->SetAddress (Mac48Address::ConvertFrom (address));
    }

    Address
    V2xMmWaveNetDevice::GetAddress () const
    {
        return (GetCtrlMac (CCH))->GetAddress ();
    }

    bool
    V2xMmWaveNetDevice::SetMtu (const uint16_t mtu)
    {
        if (mtu > MAX_MSDU_SIZE - LLC_SNAP_HEADER_LENGTH)
        {
            return false;
        }
        m_mtu = mtu;
        return true;
    }

    uint16_t
    V2xMmWaveNetDevice::GetMtu () const
    {
        return m_mtu;
    }

    bool
    V2xMmWaveNetDevice::IsLinkUp () const
    {
        return true;
    }

    void
    V2xMmWaveNetDevice::AddLinkChangeCallback (Callback<void> callback)
    {
        NS_LOG_WARN ("V2xMmWaveNetDevice is linkup forever, so this callback will be never called");
    }

    bool
    V2xMmWaveNetDevice::IsBroadcast () const
    {
        return true;
    }

    Address
    V2xMmWaveNetDevice::GetBroadcast () const
    {
        return Mac48Address::GetBroadcast ();
    }

    bool
    V2xMmWaveNetDevice::IsMulticast () const
    {
        return true;
    }

    Address
    V2xMmWaveNetDevice::GetMulticast (Ipv4Address multicastGroup) const
    {
        return Mac48Address::GetMulticast (multicastGroup);
    }

    Address
    V2xMmWaveNetDevice::GetMulticast (Ipv6Address addr) const
    {
        return Mac48Address::GetMulticast (addr);
    }

    bool
    V2xMmWaveNetDevice::IsPointToPoint () const
    {
        return false;
    }

    bool
    V2xMmWaveNetDevice::IsBridge () const
    {
        return false;
    }

    Ptr<Node>
    V2xMmWaveNetDevice::GetNode () const
    {
        return m_node;
    }

    bool
    V2xMmWaveNetDevice::NeedsArp () const
    {
        return true;
    }

    void
    V2xMmWaveNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
    {
        m_forwardUp = cb;
    }

    bool
    V2xMmWaveNetDevice::IsAvailableChannel (uint32_t channelNumber) const
    {
        if (!ChannelManager::IsWaveChannel (channelNumber))
        {
            NS_LOG_DEBUG ("this is no a valid WAVE channel for channel " << channelNumber);
            return false;
        }
        if (m_ctrlMacEntities.find (channelNumber) == m_ctrlMacEntities.end ())
        {
            NS_LOG_DEBUG ("this is no available WAVE entity  for channel " << channelNumber);
            return false;
        }
        return true;
    }

    void
    V2xMmWaveNetDevice::SetPromiscReceiveCallback (NetDevice::PromiscReceiveCallback cb)
    {
        m_promiscRx = cb;
        for (MacEntitiesI i = m_ctrlMacEntities.begin (); i != m_ctrlMacEntities.end (); ++i)
        {
            i->second->SetPromisc ();
        }
        m_dataMac->SetPromisc ();
    }

    bool
    V2xMmWaveNetDevice::SupportsSendFrom () const
    {
        return m_dataMac->SupportsSendFrom ();
    }

    void
    V2xMmWaveNetDevice::LinkUp ()
    {
        m_linkUp = true;
        m_linkChanges ();
    }

    void
    V2xMmWaveNetDevice::LinkDown ()
    {
        m_linkUp = false;
        m_linkChanges ();
    }

    int64_t
    V2xMmWaveNetDevice::AssignStreams (int64_t stream)
    {
        m_rng->SetStream (stream);
        return 1;
    }

    void
    V2xMmWaveNetDevice::SetWaveVsaCallback (Callback<bool, Ptr<const Packet>,const Address &, uint32_t, uint32_t> vsaCallback)
    {
        m_vsaManager->SetWaveVsaCallback (vsaCallback);
    }

    bool
    V2xMmWaveNetDevice::RegisterTxProfile (const TxProfile & txprofile)
    {
        if (m_txProfile != 0)
        {
            return false;
        }
        if (!IsAvailableChannel (txprofile.channelNumber))
        {
            return false;
        }
        if (txprofile.txPowerLevel > 8)
        {
            return false;
        }
        // IP-based packets is not allowed to send in the CCH.
        if (txprofile.channelNumber == CCH)
        {
            NS_LOG_DEBUG ("IP-based packets shall not be transmitted on the CCH");
            return false;
        }
        if  (txprofile.dataRate == WifiMode () || txprofile.txPowerLevel == 8)
        {
            // let MAC layer itself determine tx parameters.
            NS_LOG_DEBUG ("High layer does not want to control tx parameters.");
        }
        else
        {
            // if current PHY devices do not support data rate of the tx profile
            for (PhyEntitiesI i = m_ctrlPhyEntities.begin (); i != m_ctrlPhyEntities.end (); ++i)
            {
                if (!((*i)->IsModeSupported (txprofile.dataRate)))
                {
                    NS_LOG_DEBUG ("This data rate " << txprofile.dataRate.GetUniqueName () << " is not supported by current PHY device");
                    return false;
                }
            }
        }

        m_txProfile = new TxProfile ();
        *m_txProfile = txprofile;
        return true;
    }

    bool
    V2xMmWaveNetDevice::DeleteTxProfile (uint32_t channelNumber)
    {
        if (!IsAvailableChannel (channelNumber))
        {
            return false;
        }
        if (m_txProfile == 0)
        {
            return false;
        }
        if (m_txProfile->channelNumber != channelNumber)
        {
            return false;
        }

        delete m_txProfile;
        m_txProfile = 0;
        return true;
    }

    bool
    V2xMmWaveNetDevice::StartSch (const SchInfo & schInfo)
    {
        if (!IsAvailableChannel (schInfo.channelNumber))
        {
            return false;
        }
        return m_channelScheduler->StartSch (schInfo);
    }

    bool
    V2xMmWaveNetDevice::StopSch (uint32_t channelNumber)
    {
        if (!IsAvailableChannel (channelNumber))
        {
            return false;
        }
        return m_channelScheduler->StopSch (channelNumber);
    }

    bool
    V2xMmWaveNetDevice::StartVsa (const VsaInfo & vsaInfo)
    {
        if (!IsAvailableChannel ( vsaInfo.channelNumber))
        {
            return false;
        }
        if (vsaInfo.vsc == 0)
        {
            NS_LOG_DEBUG ("vendor specific information shall not be null");
            return false;
        }
        if (vsaInfo.oi.IsNull () && vsaInfo.managementId >= 16)
        {
            NS_LOG_DEBUG ("when organization identifier is not set, management ID "
                          "shall be in range from 0 to 15");
            return false;
        }

        m_vsaManager->SendVsa (vsaInfo);
        return true;
    }

    bool
    V2xMmWaveNetDevice::StopVsa (uint32_t channelNumber)
    {
        if (!IsAvailableChannel (channelNumber))
        {
            return false;
        }
        m_vsaManager->RemoveByChannel (channelNumber);
        return true;
    }

    void
    V2xMmWaveNetDevice::ChangeAddress (Address newAddress)
    {
        Address oldAddress = GetAddress ();
        if (newAddress == oldAddress)
        {
            return;
        }
        SetAddress (newAddress);
        // Since MAC address is changed, the MAC layer including multiple MAC entities should be reset
        // and internal MAC queues will be flushed.
        for (MacEntitiesI i = m_ctrlMacEntities.begin (); i != m_ctrlMacEntities.end (); ++i)
        {
            i->second->Reset ();
        }
        m_addressChange (oldAddress, newAddress);
    }

    void
    V2xMmWaveNetDevice::CancelTx (uint32_t channelNumber, enum AcIndex ac)
    {
        if (IsAvailableChannel (channelNumber))
        {
            return;
        }
        Ptr<V2xCtrlMac> mac = GetCtrlMac (channelNumber);
        mac->CancleTx (ac);
    }

    bool
    V2xMmWaveNetDevice::SendX (Ptr<Packet> packet, const Address & dest, uint32_t protocol, const TxInfo & txInfo)
    {
        NS_LOG_FUNCTION (this << packet << dest << protocol << &txInfo);
        if (!IsAvailableChannel (txInfo.channelNumber))
        {
            return false;
        }

        if ((txInfo.dataRate == WifiMode ()) ||  (txInfo.txPowerLevel == 8))
        {
            NS_LOG_DEBUG ("High layer does not want to control tx parameters.");
        }
        else
        {
            for (PhyEntitiesI i = m_ctrlPhyEntities.begin (); i != m_ctrlPhyEntities.end (); ++i)
            {
                if ( !((*i)->IsModeSupported (txInfo.dataRate)))
                {
                    return false;
                }
            }
            WifiTxVector txVector;
            txVector.SetChannelWidth (10);
            txVector.SetTxPowerLevel (txInfo.txPowerLevel);
            txVector.SetMode (txInfo.dataRate);
            txVector.SetPreambleType (txInfo.preamble);
            HigherLayerTxVectorTag tag = HigherLayerTxVectorTag (txVector, false);
            packet->AddPacketTag (tag);
        }

        LlcSnapHeader llc;
        llc.SetType (protocol);
        packet->AddHeader (llc);

        SocketPriorityTag prio;
        prio.SetPriority (txInfo.priority);
        packet->ReplacePacketTag (prio);
        Ptr<V2xCtrlMac> mac = GetCtrlMac (txInfo.channelNumber);
        Mac48Address realTo = Mac48Address::ConvertFrom (dest);
        mac->NotifyTx (packet);
        mac->Enqueue (packet, realTo);
        return true;
    }

    uint32_t
    V2xMmWaveNetDevice::GetSize (Ptr<const Packet> packet, const MmWaveMacHeader *hdr)
    {
        MmWaveMacTrailer fcs;
        uint32_t size = packet->GetSize () + hdr->GetSerializedSize () + fcs.GetSerializedSize ();
        return size;
    }

    uint32_t
    V2xMmWaveNetDevice::GetSize (Ptr<const Packet> packet, const WifiMacHeader *hdr)
    {
        WifiMacTrailer fcs;
        uint32_t size = packet->GetSize () + hdr->GetSerializedSize () + fcs.GetSerializedSize ();
        return size;
    }

    Mac48Address
    V2xMmWaveNetDevice::GetMacAddress ()
    {
        return m_dataMac->GetAddress ();
    }

    void
    V2xMmWaveNetDevice::SendOneBeacon ()
    {
        RxBeaconHeader rxBeaconHeader;
        rxBeaconHeader.SetDuration (0);
        rxBeaconHeader.SetDelay (0);

        Ptr<Packet> packet = Create<Packet>();
        packet->AddHeader (rxBeaconHeader);

        Ptr<V2xCtrlMac> mac = GetCtrlMac (CCH);
        Mac48Address to = Mac48Address::GetBroadcast ();
        mac->SendBeacon (packet, to);
        m_beaconEvent = Simulator::Schedule (m_beaconInterval, &V2xMmWaveNetDevice::SendOneBeacon, this);
    }

    bool
    V2xMmWaveNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocol)
    {
        NS_LOG_FUNCTION (this << packet << dest << protocol);
        LlcSnapHeader llc;
        llc.SetType (protocol);
        packet->AddHeader (llc);
        Mac48Address realTo = Mac48Address::ConvertFrom (dest);
        Enqueue (packet, realTo);
        return true;
    }

    void
    V2xMmWaveNetDevice::Enqueue (Ptr<const Packet> packet, Mac48Address to)
    {
        MmWaveMacHeader hdr;
        hdr.SetType (MMWAVE_MAC_DATA);
        hdr.SetAddr1 (to);
        hdr.SetAddr2 (GetMacAddress ());
        hdr.SetAddr3 (Mac48Address::GetBroadcast ());
        hdr.SetDsNotFrom ();
        hdr.SetDsNotTo ();
        hdr.SetDuration (Seconds (0.0));

        MmWaveMacLowParameters params;
        if (to.IsGroup ())
        {
            params.DisableAck ();
        }
        else
        {
            params.EnableAck ();
        }
        MmWaveTxVector dataTxVector = m_dataMac->GetMacLow ()->GetDataTxVector (to);
        Time duration = m_dataMac->GetResponseDuration (params, dataTxVector, to);
        hdr.SetDuration (duration);

        m_queue->Enqueue (Create<MmWaveMacQueueItem> (packet, hdr));

        TryToStartContentionFreeDuration ();
    }

    void
    V2xMmWaveNetDevice::AddRequest (Mac48Address tx, Mac48Address rx, Time duration)
    {
        m_contentionFreeAccess->AddRequest (tx, rx, duration);
    }

    void
    V2xMmWaveNetDevice::AddAgreement (Mac48Address tx, Mac48Address rx, Time start, Time duration)
    {
        m_contentionFreeAccess->AddAgreement (tx, rx, start, duration);
    }

    Ptr<MmWaveMacQueue>
    V2xMmWaveNetDevice::GetMacQueue ()
    {
        return m_queue;
    }

    void
    V2xMmWaveNetDevice::ForwardUp (Ptr<const Packet> packet, Mac48Address from, Mac48Address to)
    {
        NS_LOG_FUNCTION (this << packet << from << to);
        LlcSnapHeader llc;
        enum NetDevice::PacketType type;
        if (to.IsBroadcast ())
        {
            type = NetDevice::PACKET_BROADCAST;
        }
        else if (to.IsGroup ())
        {
            type = NetDevice::PACKET_MULTICAST;
        }
        else if (to == GetAddress ())
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
            m_dataMac->NotifyRx (copy);
            copy->RemoveHeader (llc);
            m_forwardUp (this, copy, llc.GetType (), from);
        }
        else
        {
            copy->RemoveHeader (llc);
        }

        if (!m_promiscRx.IsNull ())
        {
            m_dataMac->NotifyPromiscRx (copy);
            m_promiscRx (this, copy, llc.GetType (), from, to, type);
        }
    }

    Time
    V2xMmWaveNetDevice::GetDataPhyTxDuration (Ptr<const Packet> packet, const MmWaveMacHeader hdr)
    {
        Ptr<V2xDataMacLow> low = m_dataMac->GetMacLow ();
        uint32_t size = GetSize (packet, &hdr);
        Time txTime = m_dataPhy->CalculateTxDuration (size, low->GetDataTxVector (hdr.GetAddr1()), m_dataPhy->GetPhyBand ());
        return txTime;
    }

    Time
    V2xMmWaveNetDevice::GetCtrlPhyRxBeaconDuration (Mac48Address to)
    {
        Ptr<Packet> packet = Create<Packet>();
        RxBeaconHeader beacon;
        beacon.SetDuration (0);
        beacon.SetDelay (0);
        packet->AddHeader (beacon);

        WifiMacHeader hdr;
        hdr.SetType (WIFI_MAC_MGT_RX_BEACON);
        hdr.SetAddr1 (to);
        hdr.SetAddr2 (GetMacAddress ());
        hdr.SetAddr3 (Mac48Address::GetBroadcast ());
        hdr.SetDsNotFrom ();
        hdr.SetDsNotTo ();

        Ptr<V2xCtrlMac> mac = GetCtrlMac (CCH);
        MacLowTransmissionParameters params;
        uint32_t size = GetSize (packet, &hdr);
        if (to.IsGroup ())
        {
            params.DisableRts();
            params.DisableAck();
            params.DisableNextData();
        }
        else
        {
            if (mac->GetWifiRemoteStationManager ()->NeedRts (hdr, size))
            {
                params.EnableRts ();
                params.EnableAck ();
                params.DisableNextData();
            }
            else
            {
                params.DisableRts ();
                params.EnableAck ();
                params.DisableNextData();
            }
        }
        Ptr<const WifiMacQueueItem> item = Create<const WifiMacQueueItem> (packet, hdr);
        Time txTime = mac->GetMacLow ()->CalculateOverheadTxTime (item, params);
        Time duration = mac->GetWifiPhy ()->CalculateTxDuration (size, mac->GetMacLow ()->GetDataTxVector (item), mac->GetWifiPhy ()->GetPhyBand ());
        txTime += duration;
        return txTime;
    }

    Ptr<WifiMacQueueItem>
    V2xMmWaveNetDevice::ReplaceBeacon (Ptr<WifiMacQueueItem> item)
    {
        item = 0;
        if (m_contentionFreeAccess->IsAnyRequest ())
        {
            item = SendRxBeacon ();
            return item;
        }

        if (!m_queue->IsEmpty ())
        {
            item = SendTxBeacon ();
            return item;
        }
        return item;
    }

    Ptr<WifiMacQueueItem>
    V2xMmWaveNetDevice::SendTxBeacon ()
    {
        Ptr<WifiMacQueueItem> item = 0;
        Ptr<Packet> packet = Create<Packet>();
        WifiMacHeader hdr;
        NS_ASSERT (!m_queue->IsEmpty ());
        std::vector<Mac48Address> rx = m_queue->PeekFrontNAddresses (50);
        NS_ASSERT (!rx.empty ());
        auto firstA = rx.begin ();
        Mac48Address to = (*firstA);
        Mac48Address from = GetMacAddress ();
        if (to.IsGroup ())
        {
            if (!m_contentionFreeAccess->FindAgreement (from, to))
            {
                Time idleStart = Simulator::Now () + GetCtrlPhyRxBeaconDuration (to);
                Time txStart = m_contentionFreeAccess->NewAgreement (from, to, idleStart, m_txDuration);
                Time txDelay = txStart - idleStart;
                NS_ASSERT (txDelay.IsPositive ());
                RxBeaconHeader rxBeaconHeader;
                rxBeaconHeader.SetDuration (m_txDuration.GetNanoSeconds ());
                rxBeaconHeader.SetDelay (txDelay.GetNanoSeconds ());
                packet->AddHeader (rxBeaconHeader);
                hdr.SetType (WIFI_MAC_MGT_RX_BEACON);
                hdr.SetAddr1 (to);
                hdr.SetAddr2 (from);
                hdr.SetAddr3 (Mac48Address::GetBroadcast ());
                hdr.SetDsNotFrom ();
                hdr.SetDsNotTo ();
                item = Create<WifiMacQueueItem> (packet, hdr);
//                NS_LOG_DEBUG ("SendRxBeacon " << hdr);
            }
        }
        else
        {
            for (auto i = rx.begin (); i != rx.end (); )
            {
                if (m_contentionFreeAccess->FindAgreement (from, to))
                {
                    i = rx.erase (i);
                }
                else
                {
                    i++;
                }
            }
            if (!rx.empty())
            {
                TxBeaconHeader txBeaconHeader;
                txBeaconHeader.SetRx (rx);
                txBeaconHeader.SetDuration (m_txDuration.GetNanoSeconds ());
                packet->AddHeader (txBeaconHeader);
                hdr.SetType (WIFI_MAC_MGT_TX_BEACON);
                hdr.SetAddr1 (Mac48Address::GetBroadcast ());
                hdr.SetAddr2 (from);
                hdr.SetAddr3 (Mac48Address::GetBroadcast ());
                hdr.SetDsNotFrom ();
                hdr.SetDsNotTo ();
                item = Create<WifiMacQueueItem> (packet, hdr);
//                NS_LOG_DEBUG ("SendTxBeacon " << hdr);
            }
        }
        return item;
    }

    Ptr<WifiMacQueueItem>
    V2xMmWaveNetDevice::SendRxBeacon ()
    {
        Ptr<WifiMacQueueItem> item;
        Ptr<Packet> packet = Create<Packet>();
        NS_ASSERT (m_contentionFreeAccess->IsAnyRequest ());

        AgreementInfo info = m_contentionFreeAccess->DequeueRequest ();
        Mac48Address to = info.m_tx;
        Mac48Address from = info.m_rx;
        Time txDuration = info.m_duration;

        NS_ASSERT (!to.IsGroup ());

        WifiMacHeader hdr;
        hdr.SetType (WIFI_MAC_MGT_RX_BEACON);
        hdr.SetAddr1 (to);
        hdr.SetAddr2 (from);
        hdr.SetAddr3 (Mac48Address::GetBroadcast ());
        hdr.SetDsNotFrom ();
        hdr.SetDsNotTo ();

        Time idleStart = Simulator::Now () + GetCtrlPhyRxBeaconDuration (to);
        Time txStart = m_contentionFreeAccess->NewAgreement (to, from, idleStart, txDuration);
        Time txDelay = txStart - idleStart;

        NS_ASSERT (txDelay.IsPositive ());

        RxBeaconHeader rxBeaconHeader;
        rxBeaconHeader.SetDuration (txDuration.GetNanoSeconds ());
        rxBeaconHeader.SetDelay (txDelay.GetNanoSeconds ());
        packet->AddHeader (rxBeaconHeader);
        item = Create<WifiMacQueueItem> (packet, hdr);
//        NS_LOG_DEBUG ("SendRxBeacon " << hdr);
        return item;
    }

    Ptr<WifiMacQueueItem>
    V2xMmWaveNetDevice::SendNullBeacon ()
    {
        Ptr<WifiMacQueueItem> item;
        Ptr<Packet> packet = Create<Packet>();
        Mac48Address to = Mac48Address::GetBroadcast ();

        RxBeaconHeader rxBeaconHeader;
        rxBeaconHeader.SetDuration (0);
        rxBeaconHeader.SetDelay (0);
        packet->AddHeader (rxBeaconHeader);

        WifiMacHeader hdr;
        hdr.SetType (WIFI_MAC_MGT_NULL_BEACON);
        hdr.SetAddr1 (to);
        hdr.SetAddr2 (GetMacAddress ());
        hdr.SetAddr3 (Mac48Address::GetBroadcast ());
        hdr.SetDsNotFrom ();
        hdr.SetDsNotTo ();

        item = Create<WifiMacQueueItem> (packet, hdr);
        return item;
    }

    void
    V2xMmWaveNetDevice::WaitContentionFreeDuration (Time delay, Time duration, Mac48Address to)
    {
        if (m_startContentionFreeDuration.IsRunning ())
        {
            return;
        }

        if (m_waitContentionFreeDuration.IsRunning ())
        {
            return;
        }
        m_waitContentionFreeDuration = Simulator::Schedule (delay, &V2xMmWaveNetDevice::StartContentionFreeDuration, this, duration, to);
        m_contentionFreeRx = to;
    }

    void
    V2xMmWaveNetDevice::StartContentionFreeDuration (Time duration, Mac48Address to)
    {
//        NS_LOG_FUNCTION (this);
        if (m_startContentionFreeDuration.IsRunning ())
        {
            return;
        }

        if (m_waitContentionFreeDuration.IsRunning ())
        {
            return;
        }
        
        m_startContentionFreeDuration = Simulator::Schedule (duration, &V2xMmWaveNetDevice::StopContentionFreeDuration, this);
        m_contentionFreeRx = to;
        ReadyToTransmitFrame ();
    }

    void
    V2xMmWaveNetDevice::StopContentionFreeDuration ()
    {
//        NS_LOG_FUNCTION (this);
        m_contentionFreeRx = GetMacAddress ();
        m_contentionFreeAccess->StartContentionFreeDurationIfNeed ();
    }

    bool
    V2xMmWaveNetDevice::IsAnyActiveContentionFreeDuration ()
    {
        if (m_startContentionFreeDuration.IsRunning () || m_waitContentionFreeDuration.IsRunning ())
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    void
    V2xMmWaveNetDevice::ReadyToTransmitFrame ()
    {
//        NS_LOG_FUNCTION (this);
        if (m_queue->IsEmpty ())
        {
            return;
        }

        if (m_dataMac->IsWaitToTransmit ())
        {
            return;
        }

        if (m_dataMac->IsStateIdle ())
        {
            Time delayLeft = Simulator::GetDelayLeft (m_startContentionFreeDuration);
            Time txDuration;

            Ptr<MmWaveMacQueueItem> item;
            Ptr<Packet> packet;
            if (m_contentionFreeRx != Mac48Address::GetBroadcast ())
            {
                item = m_queue->DequeueByAddress (m_contentionFreeRx);
                if (item != 0)
                {
                    txDuration = m_dataPhy->GetSifs () + GetDataPhyTxDuration (item->GetPacket (), item->GetHeader ());
                    Mac48Address to = item->GetHeader ().GetAddr1 ();
                    if (!to.IsGroup ())
                    {
                        txDuration += m_dataPhy->GetSifs ();
                        txDuration += m_dataMac->GetAckDuration (to);
                    }

                    if (delayLeft >= txDuration)
                    {
                        packet = item->GetPacket ()->Copy ();
                        m_dataMac->NotifyTx (packet);
                        m_dataMac->Enqueue (packet, m_contentionFreeRx, m_dataPhy->GetSifs ());
                        return;
                    }
                    else
                    {
                        m_queue->PushFront (item);
                    }
                }
            }

            Mac48Address group = Mac48Address::GetBroadcast ();
            item = m_queue->DequeueByAddress (group);
            if (item != 0)
            {
                txDuration = m_dataPhy->GetSifs () + GetDataPhyTxDuration (item->GetPacket (), item->GetHeader ());
                if (delayLeft >= txDuration)
                {
                    packet = item->GetPacket ()->Copy ();
                    m_dataMac->NotifyTx (packet);
                    m_dataMac->Enqueue (packet, group, m_dataPhy->GetSifs ());
                }
                else
                {
                    m_queue->PushFront (item);
                }
            }
        }
        else
        {
            if (!m_tryAgain.IsRunning ())
            {
                m_tryAgain = Simulator::Schedule (m_dataPhy->GetDelayUntilIdle (), &V2xMmWaveNetDevice::TryToStartContentionFreeDuration, this);
            }
        }
    }

    void
    V2xMmWaveNetDevice::NotifyCtrlTxOK (WifiMacHeader &hdr)
    {
        if ((hdr.GetType () == WIFI_MAC_MGT_RX_BEACON)
            && (hdr.GetAddr1 () == Mac48Address::GetBroadcast ()))
        {
            TryToStartContentionFreeDuration ();
        }
    }

    void
    V2xMmWaveNetDevice::NotifyCtrlRxBeacon (const WifiMacHeader &hdr)
    {
        if ((hdr.GetType () == WIFI_MAC_MGT_RX_BEACON)
            && (hdr.GetAddr1 () == GetMacAddress ()))
        {
            TryToStartContentionFreeDuration ();
        }
    }

    void
    V2xMmWaveNetDevice::NotifyDataTxOK (const MmWaveMacHeader &hdr)
    {
        TryToStartContentionFreeDuration ();
    }

    void
    V2xMmWaveNetDevice::NotifyDataTxFailed (const MmWaveMacHeader &hdr)
    {
        TryToStartContentionFreeDuration ();
    }

    void
    V2xMmWaveNetDevice::TryToStartContentionFreeDuration ()
    {
        if (!m_startContentionFreeDuration.IsRunning () && !m_waitContentionFreeDuration.IsRunning ())
        {
            m_contentionFreeAccess->StartContentionFreeDurationIfNeed ();
        }
        else if (m_startContentionFreeDuration.IsRunning ())
        {
            ReadyToTransmitFrame ();
        }
    }

}