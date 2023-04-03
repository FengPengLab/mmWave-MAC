/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef V2X_CTRL_MAC_H
#define V2X_CTRL_MAC_H
#include "ns3/object-factory.h"
#include "ns3/regular-wifi-mac.h"
#include "ns3/wifi-mac-queue.h"
#include "ns3/wifi-mac-queue-item.h"
#include "ns3/qos-utils.h"
#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/mac48-address.h"
#include "ns3/vendor-specific-action.h"

namespace ns3 {
    class OrganizationIdentifier;
    class V2xMmWaveNetDevice;

    class V2xCtrlMac : public RegularWifiMac
    {
    public:
        static TypeId GetTypeId ();
        V2xCtrlMac ();
        ~V2xCtrlMac ();

        Ssid GetSsid () const;
        void SetSsid (Ssid ssid);
        void SetBssid (Mac48Address bssid);
        Mac48Address GetBssid () const;
        void Enqueue (Ptr<Packet> packet, Mac48Address to);
        void ConfigureStandard (enum WifiStandard standard);
        void SetLinkUpCallback (Callback<void> linkUp);
        void SetLinkDownCallback (Callback<void> linkDown);
        void ConfigureEdca (uint32_t cwmin, uint32_t cwmax, uint32_t aifsn, enum AcIndex ac);
        void EnableForWave (Ptr<V2xMmWaveNetDevice> device);
        void Suspend ();
        void Resume ();
        void MakeVirtualBusy (Time duration);
        void CancleTx (enum AcIndex ac);
        void Reset ();
        void SendVsc (Ptr<Packet> vsc, Mac48Address peer, OrganizationIdentifier oi);
        void AddReceiveVscCallback (OrganizationIdentifier oi, VscCallback cb);
        void RemoveReceiveVscCallback (OrganizationIdentifier oi);
        void SetReplaceBeaconCallback (Callback <Ptr<WifiMacQueueItem>, Ptr<WifiMacQueueItem>> callback);
        void SetAddRequestCallback (Callback <void, Mac48Address, Mac48Address, Time> callback);
        void SetAddAgreementCallback (Callback <void, Mac48Address, Mac48Address, Time, Time> callback);
        void SetRxBeaconCallback (Callback <void, const WifiMacHeader &> callback);
        void SetTxNoAckCallback (Callback <void, WifiMacHeader &> callback);
        void SendBeacon (Ptr<Packet> packet, Mac48Address to);
        void NotifyEndTxNoAck (WifiMacHeader & hdr);
        Ptr<WifiMacQueueItem> ReplaceBeacon (Ptr<WifiMacQueueItem> item);

    protected:
        void Receive (Ptr<WifiMacQueueItem> mpdu);
        VendorSpecificContentManager m_vscManager;
        Callback <void, Mac48Address, Mac48Address, Time> m_addRequest;
        Callback <void, Mac48Address, Mac48Address, Time, Time> m_addAgreement;
        Callback <void, const WifiMacHeader &> m_rxBeacon;
        Callback <void, WifiMacHeader &> m_endTxNoAck;
        Callback <Ptr<WifiMacQueueItem>, Ptr<WifiMacQueueItem>> m_replaceBeacon;
    };

}
#endif /* V2X_CTRL_MAC_H */
