/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/object-base.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "ns3/llc-snap-header.h"
#include "ns3/simulator.h"
#include "jammer-net-device.h"

namespace ns3 {
    NS_LOG_COMPONENT_DEFINE ("JammerNetDevice");
    NS_OBJECT_ENSURE_REGISTERED (JammerNetDevice);

    TypeId
    JammerNetDevice::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::JammerNetDevice")
                .SetParent<NetDevice> ()
                .AddConstructor<JammerNetDevice> ()
                .SetGroupName ("MmWave")
                .AddAttribute ("Mac", "The Mac entities attached to this device.",
                               PointerValue (),
                               MakePointerAccessor (&JammerNetDevice::m_mac),
                               MakePointerChecker<JammerMac> ())
                .AddAttribute ("Phy", "The PHY entities attached to this device.",
                               PointerValue (),
                               MakePointerAccessor (&JammerNetDevice::m_phy),
                               MakePointerChecker<JammerPhy> ())
                .AddAttribute ("Channel", "The channel attached to this device",
                               PointerValue (),
                               MakePointerAccessor (&JammerNetDevice::GetChannel),
                               MakePointerChecker<Channel> ());
        return tid;
    }

    JammerNetDevice::JammerNetDevice ()
            : m_configComplete (false)
    {
        NS_LOG_FUNCTION_NOARGS ();
    }

    JammerNetDevice::~JammerNetDevice ()
    {
        NS_LOG_FUNCTION_NOARGS ();
    }

    void
    JammerNetDevice::DoDispose ()
    {
        NS_LOG_FUNCTION_NOARGS ();
        m_node = 0;
        if (m_mac)
        {
            m_mac->Dispose ();
            m_mac = 0;
        }
        if (m_phy)
        {
            m_phy->Dispose ();
            m_phy = 0;
        }

        NetDevice::DoDispose ();
    }

    void
    JammerNetDevice::DoInitialize ()
    {
        NS_LOG_FUNCTION_NOARGS ();
        if (m_phy)
        {
            m_phy->Initialize ();
        }
        if (m_mac)
        {
            m_mac->Initialize ();
        }
        NetDevice::DoInitialize ();
    }

    void
    JammerNetDevice::CompleteConfig ()
    {
        if (m_mac == 0
            || m_phy == 0
            || m_node == 0
            || m_configComplete)
        {
            return;
        }
        m_mac->SetPhy (m_phy);
        m_phy->SetMac (m_mac);
        m_configComplete = true;
    }

    void
    JammerNetDevice::SetMac (const Ptr <JammerMac> mac)
    {
        m_mac = mac;
        CompleteConfig ();
    }

    void
    JammerNetDevice::SetPhy (const Ptr <JammerPhy> phy)
    {
        m_phy = phy;
        CompleteConfig ();
    }

    Ptr <JammerMac>
    JammerNetDevice::GetMac () const
    {
        return m_mac;
    }

    Ptr <JammerPhy>
    JammerNetDevice::GetPhy () const
    {
        return m_phy;
    }

    void
    JammerNetDevice::SetIfIndex (const uint32_t index)
    {
        m_ifIndex = index;
    }

    uint32_t
    JammerNetDevice::GetIfIndex () const
    {
        return m_ifIndex;
    }

    Ptr <Channel>
    JammerNetDevice::GetChannel () const
    {
        return m_phy->GetChannel ();
    }

    void
    JammerNetDevice::SetAddress (Address address)
    {
        m_mac->SetAddress (Mac48Address::ConvertFrom (address));
    }

    Address
    JammerNetDevice::GetAddress () const
    {
        return m_mac->GetAddress ();
    }

    bool
    JammerNetDevice::SetMtu (const uint16_t mtu)
    {
        m_mtu = mtu;
        return true;
    }

    uint16_t
    JammerNetDevice::GetMtu () const
    {
        return m_mtu;
    }

    bool
    JammerNetDevice::IsLinkUp () const
    {
        return (m_linkUp && m_phy != 0);
    }

    void
    JammerNetDevice::AddLinkChangeCallback (Callback<void> callback)
    {
        m_linkChanges.ConnectWithoutContext (callback);
    }

    bool
    JammerNetDevice::IsBroadcast () const
    {
        return true;
    }

    Address
    JammerNetDevice::GetBroadcast () const
    {
        return Mac48Address::GetBroadcast ();
    }

    bool
    JammerNetDevice::IsMulticast () const
    {
        return true;
    }

    Address
    JammerNetDevice::GetMulticast (Ipv4Address multicastGroup) const
    {
        return Mac48Address::GetMulticast (multicastGroup);
    }

    Address JammerNetDevice::GetMulticast (Ipv6Address addr) const
    {
        return Mac48Address::GetMulticast (addr);
    }

    bool
    JammerNetDevice::IsPointToPoint () const
    {
        return false;
    }

    bool
    JammerNetDevice::IsBridge () const
    {
        return false;
    }

    Ptr <Node>
    JammerNetDevice::GetNode () const
    {
        return m_node;
    }

    void
    JammerNetDevice::SetNode (const Ptr <Node> node)
    {
        m_node = node;
        CompleteConfig ();
    }

    bool
    JammerNetDevice::NeedsArp () const
    {
        return false;
    }

    void
    JammerNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
    {
        m_forwardUp = cb;
    }

    void
    JammerNetDevice::ForwardUp (Ptr <const Packet> packet, Mac48Address from, Mac48Address to)
    {
        NS_LOG_FUNCTION (this << packet << from << to);
    }

    void
    JammerNetDevice::LinkUp ()
    {
        m_linkUp = true;
        m_linkChanges ();
    }

    void
    JammerNetDevice::LinkDown ()
    {
        m_linkUp = false;
        m_linkChanges ();
    }

    bool
    JammerNetDevice::Send (Ptr <Packet> packet, const Address &dest, uint16_t protocolNumber)
    {
        NS_LOG_FUNCTION (this << packet << dest << protocolNumber);
        return false;
    }

    bool
    JammerNetDevice::SendFrom (Ptr <Packet> packet, const Address &source, const Address &dest, uint16_t protocolNumber)
    {
        NS_LOG_FUNCTION (this << packet << source << dest << protocolNumber);
        return false;
    }

    void
    JammerNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
    {
        m_promiscRx = cb;
    }

    bool
    JammerNetDevice::SupportsSendFrom () const
    {
        return false;
    }

}