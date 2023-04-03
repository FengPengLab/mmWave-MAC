/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_MAC_H
#define MMWAVE_MAC_H
#include "ns3/net-device.h"
#include "ns3/traced-callback.h"
#include "mmwave.h"
#include "mmwave-mac-queue-item.h"
#include "mmwave-remote-station-manager.h"

namespace ns3 {

    class MmWaveMac : public Object
    {
    public:
        static TypeId GetTypeId ();
        MmWaveMac ();
        virtual ~MmWaveMac ();
        virtual void DoInitialize ();
        virtual void DoDispose ();
        virtual Ptr<NetDevice> GetDevice () const;
        virtual Ptr<MmWaveRemoteStationManager> GetRemoteStationManager () const;
        virtual void SetDevice (const Ptr<NetDevice> device);
        virtual void SetRemoteStationManager (Ptr<MmWaveRemoteStationManager> stationManager);
        virtual void Receive (Ptr<MmWaveMacQueueItem> mpdu);
        virtual bool SupportsSendFrom () const;
        virtual void Enqueue (Ptr<Packet> packet, Mac48Address to, Mac48Address from) = 0;
        virtual void Enqueue (Ptr<Packet> packet, Mac48Address to) = 0;
        virtual void SetPromisc () = 0;
        virtual void SetAddress (Mac48Address address) = 0;
        virtual void SetBssid (Mac48Address bssid) = 0;
        virtual Mac48Address GetAddress () const = 0;
        virtual Mac48Address GetBssid () const = 0;
        virtual void SetForwardUpCallback (Callback<void, Ptr<const Packet>, Mac48Address, Mac48Address> upCallback);
        virtual void SetLinkUpCallback (Callback<void> linkUp);
        virtual void SetLinkDownCallback (Callback<void> linkDown);
        virtual void ForwardUp (Ptr<const Packet> packet, Mac48Address from, Mac48Address to);
        virtual void TxOk (const MmWaveMacHeader &hdr);
        virtual void TxFailed (const MmWaveMacHeader &hdr);
        virtual void NotifyTx (Ptr<const Packet> packet);
        virtual void NotifyTxDrop (Ptr<const Packet> packet);
        virtual void NotifyRx (Ptr<const Packet> packet);
        virtual void NotifyRxDrop (Ptr<const Packet> packet);
        virtual void NotifyPromiscRx (Ptr<const Packet> packet);
    protected:
        Ptr<MmWaveRemoteStationManager> m_stationManager;
        Ptr<NetDevice> m_device;

        MmWaveMac (const MmWaveMac &);
        MmWaveMac & operator= (const MmWaveMac & mac);

        Callback<void, Ptr<const Packet>, Mac48Address, Mac48Address> m_forwardUp; //!< Callback to forward packet up the stack
        Callback<void> m_linkUp;       //!< Callback when a link is up
        Callback<void> m_linkDown;     //!< Callback when a link is down

        TracedCallback<const MmWaveMacHeader &> m_txOkCallback; ///< transmit OK callback
        TracedCallback<const MmWaveMacHeader &> m_txErrCallback; ///< transmit error callback
        TracedCallback<Ptr<const Packet> > m_macTxTrace;
        TracedCallback<Ptr<const Packet> > m_macTxDropTrace;
        TracedCallback<Ptr<const Packet> > m_macPromiscRxTrace;
        TracedCallback<Ptr<const Packet> > m_macRxTrace;
        TracedCallback<Ptr<const Packet> > m_macRxDropTrace;
    };

} //namespace ns3

#endif //MMWAVE_MAC_H
