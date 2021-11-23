/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/simulator.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/mac48-address.h"
#include "mmwave-mac-header.h"
#include "mmwave-psdu.h"
#include "mmwave-phy.h"
#include "jammer-mac.h"

namespace ns3 {
    NS_LOG_COMPONENT_DEFINE ("JammerMac");
    NS_OBJECT_ENSURE_REGISTERED (JammerMac);

    TypeId
    JammerMac::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::JammerMac")
                .SetParent<Object> ()
                .SetGroupName ("MmWave")
                .AddConstructor<JammerMac> ()
                .AddAttribute ("JammerSignalStartTime",
                               "The interval of jamming signal sent as the PUs role.",
                               TimeValue (MicroSeconds (100)),
                               MakeTimeAccessor (&JammerMac::m_jammerSignalStart),
                               MakeTimeChecker ())
                .AddAttribute ("JammerSignalInterval",
                               "The interval of jamming signal sent as the PUs role.",
                               TimeValue (MicroSeconds (200)),
                               MakeTimeAccessor (&JammerMac::m_jammerSignalInterval),
                               MakeTimeChecker ())
                .AddAttribute ("JammerSignalSize",
                               "The packet size of jamming signal sent as the PUs role.",
                               UintegerValue (10000),
                               MakeUintegerAccessor (&JammerMac::m_jammerSignalSize),
                               MakeUintegerChecker<uint32_t> ())
               ;
        return tid;
    }

    JammerMac::JammerMac ()
    {
        NS_LOG_FUNCTION (this);
        m_defaultTxPowerLevel = 1;
        m_guardInterval = 3200;
    }

    JammerMac::~JammerMac ()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    JammerMac::DoDispose ()
    {
        m_device = 0;
        m_phy = 0;
    }

    void
    JammerMac::DoInitialize ()
    {
        m_send = Simulator::Schedule (m_jammerSignalStart, &JammerMac::Send, this);
    }

    void
    JammerMac::SetDevice (const Ptr<NetDevice> device)
    {
        m_device = device;
    }

    Ptr<NetDevice>
    JammerMac::GetDevice () const
    {
        return m_device;
    }

    void
    JammerMac::SetPhy (const Ptr<JammerPhy> phy)
    {
        NS_LOG_FUNCTION (this << phy);
        m_phy = phy;
    }

    Ptr<JammerPhy>
    JammerMac::GetPhy () const
    {
        NS_LOG_FUNCTION (this);
        return m_phy;
    }

    void
    JammerMac::ResetPhy ()
    {
        NS_LOG_FUNCTION (this);
        m_phy = 0;
    }

    void
    JammerMac::SetAddress (Mac48Address address)
    {
        NS_LOG_FUNCTION (this << address);
        m_self = address;
    }

    void
    JammerMac::SetInterval (Time interval)
    {
        m_jammerSignalInterval = interval;
    }

    void
    JammerMac::SetPacketSize (uint32_t size)
    {
        m_jammerSignalSize = size;
    }

    MmWaveTxVector
    JammerMac::GetTxVector ()
    {
        return MmWaveTxVector (m_phy->GetMmWaveMcs0 (),
                               m_defaultTxPowerLevel,
                               MMWAVE_PREAMBLE_UNSPECIFIED,
                               m_guardInterval,
                               m_phy->GetNumberOfAntennas (),
                               m_phy->GetMaxSupportedTxSpatialStreams (),
                               0,
                               m_phy->GetChannelWidth ());
    }

    Mac48Address
    JammerMac::GetAddress () const
    {
        return m_self;
    }

    void
    JammerMac::Send ()
    {
        NS_LOG_FUNCTION (this);
        Ptr<Packet> packet = Create<Packet> (m_jammerSignalSize);
        MmWaveMacHeader hdr;
        hdr.SetType (MMWAVE_MAC_DATA);
        hdr.SetDsNotFrom ();
        hdr.SetDsNotTo ();
        hdr.SetNoMoreFragments ();
        hdr.SetNoRetry ();
        hdr.SetAddr1 (Mac48Address::GetBroadcast ());
        hdr.SetAddr2 (Mac48Address::GetBroadcast ());
        hdr.SetAddr3 (Mac48Address::GetBroadcast ());
        hdr.SetDuration (Seconds (0.0));
        m_phy->SendSignal (Create<const MmWavePsdu> (packet, hdr), GetTxVector ());
    }

    void
    JammerMac::NotifyTxEnd ()
    {
        m_send = Simulator::Schedule (m_jammerSignalInterval, &JammerMac::Send, this);
    }

}