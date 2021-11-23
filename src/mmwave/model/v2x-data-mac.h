/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef V2X_DATA_MAC_H
#define V2X_DATA_MAC_H
#include "ns3/net-device.h"
#include "ns3/mac48-address.h"
#include "ns3/event-id.h"
#include "ns3/packet.h"
#include "ns3/traced-callback.h"
#include "mmwave-mac.h"
#include "mmwave-mac-tx-middle.h"
#include "mmwave-mac-rx-middle.h"
#include "v2x-data-mac-low.h"
namespace ns3  {
    class MmWavePhy;

    class V2xDataMac : public MmWaveMac
    {
    public:
        static TypeId GetTypeId ();

        V2xDataMac ();
        virtual ~V2xDataMac ();

        virtual void DoInitialize ();
        virtual void DoDispose ();

        Mac48Address GetAddress () const;
        Mac48Address GetBssid () const;
        Ptr<V2xDataMacLow> GetMacLow () const;
        Ptr<MmWavePhy> GetPhy () const;

        void SetAddress (Mac48Address address);
        void SetBssid (Mac48Address bssid);
        void SetLinkUpCallback (Callback<void> linkUp);
        void SetPromisc ();
        void SetPhy (const Ptr<MmWavePhy> phy);
        void SetTxOkCallback (Callback <void, const MmWaveMacHeader &> callback);
        void SetTxFailedCallback (Callback <void, const MmWaveMacHeader &> callback);
        void ResetPhy ();
        void SetRemoteStationManager (Ptr<MmWaveRemoteStationManager> stationManager);
        void Enqueue (Ptr<Packet> packet, Mac48Address to);
        void Enqueue (Ptr<Packet> packet, Mac48Address to, Time delay);
        void Enqueue (Ptr<Packet> packet, Mac48Address to, Mac48Address from);
        void TransmitImmediately (Ptr<Packet> packet, Mac48Address to);
        void Receive (Ptr<MmWaveMacQueueItem> mpdu);
        bool IsWaitToTransmit ();
        bool IsStateIdle ();
        Time GetResponseDuration (const MmWaveMacLowParameters& params, MmWaveTxVector dataTxVector, Mac48Address receiver) const;
        Time GetAckDuration (Mac48Address to);
        void TxOk (const MmWaveMacHeader &hdr);
        void TxFailed (const MmWaveMacHeader &hdr);
    protected:
        Ptr<MmWaveMacRxMiddle> m_rxMiddle;
        Ptr<MmWaveMacTxMiddle> m_txMiddle;
        Ptr<MmWavePhy> m_phy;
        Ptr<V2xDataMacLow> m_low;

        Callback <void, const MmWaveMacHeader &> m_txOk;
        Callback <void, const MmWaveMacHeader &> m_txFailed;
        EventId m_waitToAccess;
        EventId m_waitToSend;
        EventId m_tryAgain;
    };

} // namespace ns3
#endif //V2X_DATA_MAC_H
