/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
#ifndef CR_NET_DEVICE_H
#define CR_NET_DEVICE_H
#include "ns3/channel.h"
#include "ns3/mac48-address.h"
#include "ns3/net-device.h"
#include "ns3/traced-callback.h"
#include "ns3/type-id.h"
#include "ns3/node.h"
#include "ns3/net-device.h"
#include "mmwave.h"
#include "mmwave-phy.h"
#include "mmwave-spectrum-repository.h"
#include "mmwave-remote-station-manager.h"
#include "cr-mac.h"

namespace ns3 {

    class CrMmWaveNetDevice : public NetDevice
    {
    public:
        static TypeId GetTypeId ();
        CrMmWaveNetDevice ();
        ~CrMmWaveNetDevice ();
        void SetMac (const Ptr<CrMmWaveMac> mac);
        void SetPhy (TypeOfGroup typeOfGroup, const Ptr <MmWavePhy> phy);
        void SetRemoteStationManager (const Ptr<MmWaveRemoteStationManager> manager);
        void SetSpectrumRepository (const Ptr<MmWaveSpectrumRepository> repo);
        void SetStandard (MmWaveStandard standard);
        Ptr<CrMmWaveMac> GetMac () const;
        Ptr<MmWavePhy> GetPhy (TypeOfGroup typeOfGroup) const;
        Ptr<MmWaveRemoteStationManager> GetRemoteStationManager () const;
        Ptr<MmWaveSpectrumRepository> GetSpectrumRepository () const;

        //inherited from NetDevice base class.
        void SetIfIndex (const uint32_t index);
        uint32_t GetIfIndex () const;
        Ptr<Channel> GetChannel () const;
        void SetAddress (Address address);
        Address GetAddress () const;
        bool SetMtu (const uint16_t mtu);
        uint16_t GetMtu () const;
        bool IsLinkUp () const;
        void AddLinkChangeCallback (Callback<void> callback);
        bool IsBroadcast () const;
        bool IsMulticast () const;
        Address GetBroadcast () const;
        Address GetMulticast (Ipv4Address multicastGroup) const;
        Address GetMulticast (Ipv6Address addr) const;
        bool IsPointToPoint () const;
        bool IsBridge () const;
        bool Send (Ptr<Packet> packet, const Address &dest, uint16_t protocolNumber);
        Ptr<Node> GetNode () const;
        void SetNode (const Ptr<Node> node);
        bool NeedsArp () const;
        void SetReceiveCallback (NetDevice::ReceiveCallback cb);
        bool SendFrom (Ptr <Packet> packet, const Address &source, const Address &dest, uint16_t protocolNumber);
        void SetPromiscReceiveCallback (PromiscReceiveCallback cb);
        bool SupportsSendFrom () const;
        void ForwardUp (Ptr <const Packet> packet, Mac48Address from, Mac48Address to);

    protected:
        static const uint16_t MAX_MSDU_SIZE = 2304;
        static const uint16_t IPv4_PROT_NUMBER = 0x0800;
        static const uint16_t IPv6_PROT_NUMBER = 0x86DD;

        void DoDispose ();
        void DoInitialize ();
        void LinkUp ();
        void LinkDown ();
        void CompleteConfig ();

        CrMmWaveNetDevice (const CrMmWaveNetDevice &o);
        CrMmWaveNetDevice &operator= (const CrMmWaveNetDevice &o);

        Ptr<Node> m_node; //!< the node
        Ptr<MmWaveSpectrumRepository> m_repository;
        Ptr<CrMmWaveMac> m_mac; //!< the MAC
        Ptr<MmWavePhy> m_phyForIntraGroup; //!< the phy
        Ptr<MmWavePhy> m_phyForInterGroup; //!< the phy
        Ptr<MmWavePhy> m_phyForProbeGroup; //!< the phy
        Ptr<MmWaveRemoteStationManager> m_stationManager; //!< the station manager
        NetDevice::ReceiveCallback m_forwardUp; //!< forward up callback
        NetDevice::PromiscReceiveCallback m_promiscRx; //!< promiscious receive callback
        MmWaveStandard m_standard;
        TracedCallback <Ptr<const Packet>, Mac48Address> m_rxLogger; //!< receive trace callback
        TracedCallback <Ptr<const Packet>, Mac48Address> m_txLogger; //!< transmit trace callback
        TracedCallback<> m_linkChanges; //!< link change callback

        uint32_t m_ifIndex; //!< IF index
        bool m_linkUp; //!< link up
        mutable uint16_t m_mtu; //!< MTU
        bool m_configComplete; //!< configuration complete
    };
}
#endif //CR_NET_DEVICE_H
