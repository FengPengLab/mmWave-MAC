/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/pointer.h"
#include "jammer-phy.h"
#include "mmwave-mac.h"
namespace ns3 {
    NS_LOG_COMPONENT_DEFINE ("MmWaveMac");
    NS_OBJECT_ENSURE_REGISTERED (MmWaveMac);

    TypeId
    MmWaveMac::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::MmWaveMac")
                .SetParent<Object> ()
                .SetGroupName ("MmWave")
                .AddTraceSource ("MacTx",
                                 "A packet has been received from higher layers and is being processed in preparation for "
                                 "queueing for transmission.",
                                 MakeTraceSourceAccessor (&MmWaveMac::m_macTxTrace),
                                 "ns3::Packet::TracedCallback")
                .AddTraceSource ("MacTxDrop",
                                 "A packet has been dropped in the MAC layer before transmission.",
                                 MakeTraceSourceAccessor (&MmWaveMac::m_macTxDropTrace),
                                 "ns3::Packet::TracedCallback")
                .AddTraceSource ("MacPromiscRx",
                                 "A packet has been received by this device, has been passed up from the physical layer "
                                 "and is being forwarded up the local protocol stack.  This is a promiscuous trace.",
                                 MakeTraceSourceAccessor (&MmWaveMac::m_macPromiscRxTrace),
                                 "ns3::Packet::TracedCallback")
                .AddTraceSource ("MacRx",
                                 "A packet has been received by this device, has been passed up from the physical layer "
                                 "and is being forwarded up the local protocol stack. This is a non-promiscuous trace.",
                                 MakeTraceSourceAccessor (&MmWaveMac::m_macRxTrace),
                                 "ns3::Packet::TracedCallback")
                .AddTraceSource ("MacRxDrop",
                                 "A packet has been dropped in the MAC layer after it has been passed up from the physical layer.",
                                 MakeTraceSourceAccessor (&MmWaveMac::m_macRxDropTrace),
                                 "ns3::Packet::TracedCallback")
                .AddTraceSource ("TxOkHeader",
                                 "The header of successfully transmitted packet.",
                                 MakeTraceSourceAccessor (&MmWaveMac::m_txOkCallback),
                                 "ns3::WifiMacHeader::TracedCallback")
                .AddTraceSource ("TxErrHeader",
                                 "The header of unsuccessfully transmitted packet.",
                                 MakeTraceSourceAccessor (&MmWaveMac::m_txErrCallback),
                                 "ns3::WifiMacHeader::TracedCallback")
        ;
        return tid;
    }

    MmWaveMac::MmWaveMac ()
    {
        NS_LOG_FUNCTION (this);
    }

    MmWaveMac::~MmWaveMac ()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    MmWaveMac::DoInitialize ()
    {
        NS_LOG_FUNCTION (this);
    }
    
    void
    MmWaveMac::DoDispose ()
    {
        NS_LOG_FUNCTION (this);
        m_stationManager = 0;
        m_device = 0;
    }

    void
    MmWaveMac::SetDevice (const Ptr<NetDevice> device)
    {
        NS_LOG_FUNCTION (this);
        m_device = device;
    }

    void
    MmWaveMac::SetRemoteStationManager (const Ptr<MmWaveRemoteStationManager> stationManager)
    {
        NS_LOG_FUNCTION (this);
        m_stationManager = stationManager;
    }

    Ptr<NetDevice>
    MmWaveMac::GetDevice () const
    {
        NS_LOG_FUNCTION (this);
        return m_device;
    }

    Ptr<MmWaveRemoteStationManager>
    MmWaveMac::GetRemoteStationManager () const
    {
        NS_LOG_FUNCTION (this);
        return m_stationManager;
    }

    void
    MmWaveMac::SetForwardUpCallback (Callback<void, Ptr<const Packet>, Mac48Address, Mac48Address> upCallback)
    {
        NS_LOG_FUNCTION (this);
        m_forwardUp = upCallback;
    }

    void
    MmWaveMac::SetLinkUpCallback (Callback<void> linkUp)
    {
        NS_LOG_FUNCTION (this);
        m_linkUp = linkUp;
    }

    void
    MmWaveMac::SetLinkDownCallback (Callback<void> linkDown)
    {
        NS_LOG_FUNCTION (this);
        m_linkDown = linkDown;
    }

    void
    MmWaveMac::TxOk (const MmWaveMacHeader &hdr)
    {
        NS_LOG_FUNCTION (this << hdr);
        m_txOkCallback (hdr);
    }

    void
    MmWaveMac::TxFailed (const MmWaveMacHeader &hdr)
    {
        NS_LOG_FUNCTION (this << hdr);
        m_txErrCallback (hdr);
    }

    void
    MmWaveMac::NotifyTx (Ptr<const Packet> packet)
    {
        NS_LOG_FUNCTION (this);
        m_macTxTrace (packet);
    }

    void
    MmWaveMac::NotifyTxDrop (Ptr<const Packet> packet)
    {
        NS_LOG_FUNCTION (this);
        m_macTxDropTrace (packet);
    }

    void
    MmWaveMac::NotifyRx (Ptr<const Packet> packet)
    {
        NS_LOG_FUNCTION (this);
        m_macRxTrace (packet);
    }

    void
    MmWaveMac::NotifyRxDrop (Ptr<const Packet> packet)
    {
        NS_LOG_FUNCTION (this);
        m_macRxDropTrace (packet);
    }

    void
    MmWaveMac::NotifyPromiscRx (Ptr<const Packet> packet)
    {
        NS_LOG_FUNCTION (this);
        m_macPromiscRxTrace (packet);
    }

    bool
    MmWaveMac::SupportsSendFrom () const
    {
        NS_LOG_FUNCTION (this);
        return false;
    }

    void
    MmWaveMac::ForwardUp (Ptr<const Packet> packet, Mac48Address from, Mac48Address to)
    {
        NS_LOG_FUNCTION (this << packet << from << to);
        m_forwardUp (packet, from, to);
    }

    void
    MmWaveMac::Receive (Ptr<MmWaveMacQueueItem> mpdu)
    {
        NS_LOG_FUNCTION (this << *mpdu);
        const MmWaveMacHeader* hdr = &mpdu->GetHeader ();
        Ptr<Packet> packet = mpdu->GetPacket ()->Copy ();
        if (hdr->GetAddr1 () != GetAddress ())
        {
            return;
        }

        if (hdr->IsMgt ())
        {
            return;
        }

        NS_FATAL_ERROR ("Don't know how to handle frame type=" << hdr->GetType ());
    }

} //namespace ns3