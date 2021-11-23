/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
#ifndef JAMMER_NET_DEVICE_H
#define JAMMER_NET_DEVICE_H
#include "ns3/net-device.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/channel.h"
#include "ns3/node.h"
#include "jammer-mac.h"
#include "jammer-phy.h"
namespace ns3 {
    class JammerNetDevice : public NetDevice
    {
    public:
        JammerNetDevice ();
        virtual ~JammerNetDevice ();
        static TypeId GetTypeId ();

        Address GetAddress () const;
        Address GetBroadcast () const;
        Address GetMulticast (Ipv4Address multicastGroup) const;
        Address GetMulticast (Ipv6Address addr) const;

        Ptr<JammerMac> GetMac () const;
        Ptr<JammerPhy> GetPhy () const;
        Ptr<Channel> GetChannel () const;
        Ptr<Node> GetNode () const;

        uint32_t GetIfIndex () const;
        uint16_t GetMtu () const;

        void AddLinkChangeCallback (Callback<void> callback);
        bool IsBridge () const;
        bool IsBroadcast () const;
        bool IsLinkUp () const;
        bool IsMulticast () const;
        bool IsPointToPoint () const;
        bool NeedsArp () const;
        bool Send (Ptr<Packet> packet, const Address &dest, uint16_t protocolNumber);
        bool SendFrom (Ptr<Packet> packet, const Address &source, const Address &dest, uint16_t protocolNumber);
        void SetMac (const Ptr<JammerMac> mac);
        void SetPhy (const Ptr<JammerPhy> phy);
        void SetAddress (Address address);
        void SetIfIndex (const uint32_t index);
        bool SetMtu (const uint16_t mtu);
        void SetNode (const Ptr <Node> node);
        void SetReceiveCallback (NetDevice::ReceiveCallback cb);
        void SetPromiscReceiveCallback (PromiscReceiveCallback cb);
        bool SupportsSendFrom () const;

    protected:
        void DoDispose ();
        void DoInitialize ();
        void ForwardUp (Ptr <const Packet> packet, Mac48Address from, Mac48Address to);
        void LinkUp ();
        void LinkDown ();
        void CompleteConfig ();
        JammerNetDevice (const JammerNetDevice &o);
        JammerNetDevice &operator= (const JammerNetDevice &o);
    private:
        Ptr<Node> m_node; //!< the node
        Ptr<JammerPhy> m_phy; //!< the phy
        Ptr<JammerMac> m_mac; //! the mac

        NetDevice::ReceiveCallback m_forwardUp; //!< forward up callback
        NetDevice::PromiscReceiveCallback m_promiscRx; //!< promiscious receive callback

        TracedCallback <Ptr<const Packet>, Mac48Address> m_rxLogger; //!< receive trace callback
        TracedCallback <Ptr<const Packet>, Mac48Address> m_txLogger; //!< transmit trace callback
        TracedCallback <> m_linkChanges; //!< link change callback

        uint32_t m_ifIndex; //!< IF index
        mutable uint16_t m_mtu; //!< MTU

        bool m_linkUp; //!< link up
        bool m_configComplete; //!< configuration complete
    };
}
#endif //JAMMER_NET_DEVICE_H
