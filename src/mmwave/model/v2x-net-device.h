/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef V2X_NET_DEVICE_H
#define V2X_NET_DEVICE_H
#include <map>
#include <vector>
#include "ns3/net-device.h"
#include "ns3/type-id.h"
#include "ns3/packet.h"
#include "ns3/mac48-address.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/traced-callback.h"
#include "ns3/net-device-queue-interface.h"
#include "ns3/queue-item.h"
#include "ns3/nstime.h"
#include "ns3/channel.h"
#include "ns3/node.h"
#include "ns3/mac48-address.h"
#include "ns3/higher-tx-tag.h"
#include "ns3/channel-coordinator.h"
#include "ns3/channel-manager.h"
#include "ns3/wave-net-device.h"
#include "ns3/wifi-phy.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-remote-station-manager.h"
#include "mmwave-mac-queue.h"
#include "mmwave-phy.h"

#include "mmwave-remote-station-manager.h"
namespace ns3 {

    struct SchInfo;
    struct VsaInfo;
    class NetDevice;
    class V2xCtrlMac;
    class V2xDataMac;
    class V2xChannelScheduler;
    class V2xContentionFreeAccess;
    class V2xVsaManager;

    class V2xMmWaveNetDevice : public WifiNetDevice
    {
    public:
        static TypeId GetTypeId ();
        V2xMmWaveNetDevice ();
        virtual ~V2xMmWaveNetDevice ();

        void SetDataMac (Ptr<V2xDataMac> mac);
        void SetDataPhy (Ptr<MmWavePhy> phy);
        void SetDataRemoteStationManager (Ptr<MmWaveRemoteStationManager> manager);
        void SetVsaManager (Ptr<V2xVsaManager> vsaManager);
        void SetChannelScheduler (Ptr<V2xChannelScheduler> channelScheduler);
        void SetChannelManager (Ptr<ChannelManager> channelManager);
        void SetChannelCoordinator (Ptr<ChannelCoordinator> channelCoordinator);
        void SetContentionFreeAccess (Ptr<V2xContentionFreeAccess> contentionFreeAccess);
        void AddCtrlMac (uint32_t channelNumber, Ptr<V2xCtrlMac> mac);
        void AddCtrlPhy (Ptr<WifiPhy> phy);

        std::map<uint32_t, Ptr<V2xCtrlMac> >  GetCtrlMacs () const;
        std::vector<Ptr<WifiPhy> > GetCtrlPhys () const;
        Ptr<V2xCtrlMac> GetCtrlMac (uint32_t channelNumber) const;
        Ptr<WifiPhy> GetCtrlPhy (uint32_t index) const;
        Ptr<V2xDataMac> GetDataMac () const;
        Ptr<MmWavePhy> GetDataPhy () const;
        Ptr<MmWaveRemoteStationManager> GetDataRemoteStationManager () const;
        Ptr<V2xChannelScheduler> GetChannelScheduler () const;
        Ptr<ChannelManager> GetChannelManager () const;
        Ptr<ChannelCoordinator> GetChannelCoordinator () const;
        Ptr<V2xContentionFreeAccess> GetContentionFreeAccess () const;
        Ptr<V2xVsaManager> GetVsaManager () const;

        bool StartSch (const SchInfo & schInfo);
        bool StopSch (uint32_t channelNumber);
        bool StartVsa (const VsaInfo & vsaInfo);
        bool StopVsa (uint32_t channelNumber);
        void SetWaveVsaCallback (Callback<bool, Ptr<const Packet>,const Address &, uint32_t, uint32_t> vsaCallback);
        bool RegisterTxProfile (const TxProfile &txprofile);
        bool DeleteTxProfile (uint32_t channelNumber);
        void ChangeAddress (Address newAddress);
        void CancelTx (uint32_t channelNumber, enum AcIndex ac);
        bool Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);
        bool SendX (Ptr<Packet> packet, const Address& dest, uint32_t protocol, const TxInfo & txInfo);

        // inherited from NetDevice base class.
        virtual void SetIfIndex (const uint32_t index);
        virtual uint32_t GetIfIndex () const;
        virtual Ptr<Channel> GetChannel () const;
        virtual void SetAddress (Address address);
        virtual Address GetAddress () const;
        virtual bool SetMtu (const uint16_t mtu);
        virtual uint16_t GetMtu () const;
        virtual bool IsLinkUp () const;
        virtual void AddLinkChangeCallback (Callback<void> callback);
        virtual bool IsBroadcast () const;
        virtual Address GetBroadcast () const;
        virtual bool IsMulticast () const;
        virtual Address GetMulticast (Ipv4Address multicastGroup) const;
        virtual bool IsPointToPoint () const;
        virtual bool IsBridge () const;
        virtual Ptr<Node> GetNode () const;
        virtual void SetNode (Ptr<Node> node);
        virtual bool NeedsArp () const;
        virtual void SetReceiveCallback (NetDevice::ReceiveCallback cb);
        virtual Address GetMulticast (Ipv6Address addr) const;
        virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);
        virtual void SetPromiscReceiveCallback (NetDevice::PromiscReceiveCallback cb);
        virtual bool SupportsSendFrom () const;
        virtual void DoDispose ();
        virtual void DoInitialize ();

        void Enqueue (Ptr<const Packet> packet, Mac48Address to);
        void SendOneBeacon ();
        Ptr<WifiMacQueueItem> SendRxBeacon ();
        Ptr<WifiMacQueueItem> SendTxBeacon ();
        Ptr<WifiMacQueueItem> SendNullBeacon ();
        Ptr<MmWaveMacQueue> GetMacQueue ();
        Time GetDataPhyTxDuration (Ptr<const Packet> packet, const MmWaveMacHeader hdr);
        Time GetCtrlPhyRxBeaconDuration (Mac48Address to);
        Mac48Address GetMacAddress ();
        Ptr<WifiMacQueueItem> ReplaceBeacon (Ptr<WifiMacQueueItem> item);
        void AddRequest (Mac48Address tx, Mac48Address rx, Time duration);
        void AddAgreement (Mac48Address tx, Mac48Address rx, Time start, Time duration);
        void ReadyToTransmitFrame ();
        void NotifyCtrlTxOK (WifiMacHeader &hdr);
        void NotifyCtrlRxBeacon (const WifiMacHeader &hdr);
        void NotifyDataTxOK (const MmWaveMacHeader &hdr);
        void NotifyDataTxFailed (const MmWaveMacHeader &hdr);
        void WaitContentionFreeDuration (Time delay, Time duration, Mac48Address to);
        void StartContentionFreeDuration (Time duration, Mac48Address to);
        void StopContentionFreeDuration ();
        bool IsAnyActiveContentionFreeDuration ();
        void TryToStartContentionFreeDuration ();
        bool IsAvailableChannel (uint32_t channelNumber) const;
        void ForwardUp (Ptr<const Packet> packet, Mac48Address from, Mac48Address to);
        void LinkUp ();
        void LinkDown ();
        void CompleteConfig ();
        uint32_t GetSize (Ptr<const Packet> packet, const MmWaveMacHeader *hdr);
        uint32_t GetSize (Ptr<const Packet> packet, const WifiMacHeader *hdr);
        int64_t AssignStreams (int64_t stream);

        V2xMmWaveNetDevice (const V2xMmWaveNetDevice &o);
        V2xMmWaveNetDevice &operator = (const V2xMmWaveNetDevice &o);

    protected:
        static const uint16_t MAX_MSDU_SIZE = 2304;
        static const uint16_t IPv4_PROT_NUMBER = 0x0800;
        static const uint16_t IPv6_PROT_NUMBER = 0x86DD;

        typedef std::map<uint32_t, Ptr<V2xCtrlMac> > MacEntities;
        typedef std::map<uint32_t, Ptr<V2xCtrlMac> >::const_iterator MacEntitiesI;
        typedef std::vector<Ptr<WifiPhy> > PhyEntities;
        typedef std::vector<Ptr<WifiPhy> >::const_iterator PhyEntitiesI;
        MacEntities m_ctrlMacEntities;
        PhyEntities m_ctrlPhyEntities;

        Ptr<Node> m_node; //!< the node
        Ptr<MmWaveMacQueue> m_queue;
        Ptr<UniformRandomVariable> m_rng;
        Ptr<V2xDataMac> m_dataMac; //!< the MAC
        Ptr<MmWavePhy> m_dataPhy; //!< the phy
        Ptr<MmWaveRemoteStationManager> m_dataStationManager; //!< the station manager
        Ptr<ChannelManager> m_channelManager; ///< the channel manager
        Ptr<ChannelCoordinator> m_channelCoordinator; ///< the channel coordinator
        Ptr<V2xChannelScheduler> m_channelScheduler; ///< the channel scheduler
        Ptr<V2xVsaManager> m_vsaManager; ///< the VSA manager
        Ptr<V2xContentionFreeAccess> m_contentionFreeAccess;
        Ptr<NetDeviceQueueInterface> m_queueInterface;   //!< NetDevice queue interface

        NetDevice::ReceiveCallback m_forwardUp; ///< forward up receive callback
        NetDevice::PromiscReceiveCallback m_promiscRx; ///< promiscious receive callback

        TracedCallback<Ptr<const Packet>, Mac48Address> m_rxLogger; //!< receive trace callback
        TracedCallback<Ptr<const Packet>, Mac48Address> m_txLogger; //!< transmit trace callback

        TxProfile *m_txProfile; ///< transmit profile
        TracedCallback<Address, Address> m_addressChange;
        TracedCallback<> m_linkChanges; //!< link change callback

        uint32_t m_ifIndex; ///< IF index
        bool m_linkUp; //!< link up
        mutable uint16_t m_mtu; ///< MTU
        bool m_configComplete; //!< configuration complete
        int m_maxQueueItemNumber;
        Mac48Address m_contentionFreeRx;
        Time m_beaconInterval;
        Time m_txDuration;
        EventId m_tryAgain;
        EventId m_beaconEvent;
        EventId m_waitToSend;
        EventId m_startContentionFreeDuration;
        EventId m_waitContentionFreeDuration;
    };
}
#endif //V2X_NET_DEVICE_H
