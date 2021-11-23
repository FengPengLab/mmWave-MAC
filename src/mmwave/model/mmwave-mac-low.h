/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_MAC_LOW_H
#define MMWAVE_MAC_LOW_H
#include <map>
#include "ns3/object.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/mac48-address.h"
#include "ns3/random-variable-stream.h"
#include "mmwave-mac-low-parameters.h"
#include "mmwave-mac-header.h"
#include "mmwave-tx-vector.h"
#include "mmwave.h"

namespace ns3 {
    class MmWaveMac;
    class MmWavePhy;
    class MmWaveTxop;
    class MmWaveMacQueueItem;
    class MmWaveMacQueue;
    class MmWavePsdu;
    class MmWaveRemoteStationManager;

    class MmWaveMacLow : public Object
    {
    public:
        MmWaveMacLow ();
        virtual ~MmWaveMacLow ();
        static TypeId GetTypeId ();
        virtual void DoInitialize ();
        virtual void DoDispose ();
        virtual void ReceiveOk (Ptr<MmWavePsdu> psdu, double rxSnr, Time start, Time duration, MmWaveTxVector txVector) = 0;
        virtual void ReceiveError (Ptr<MmWavePsdu> psdu, double rxSnr, Time start, Time duration, MmWaveTxVector txVector) = 0;
        virtual bool DoNavStartNow (Time duration) = 0;
        virtual void DoNavResetNow (Time duration) = 0;
        virtual void NotifyAckTimeoutStartNow (Time duration) = 0;
        virtual void NotifyAckTimeoutResetNow () = 0;
        virtual void NotifyCtsTimeoutStartNow (Time duration) = 0;
        virtual void NotifyCtsTimeoutResetNow () = 0;
        virtual void NormalAckTimeout () = 0;
        virtual void NotifySwitchingStartNow (Time duration) = 0;
        virtual void NotifySleepNow () = 0;
        virtual void NotifyOffNow () = 0;
        virtual void NotifyNav (Ptr<const Packet> packet,const MmWaveMacHeader &hdr) = 0;
        virtual void CtsTimeout () = 0;
        virtual void WaitIfsAfterEndTxFragment () = 0;
        virtual void WaitIfsAfterEndTxPacket () = 0;
        virtual void EndTxNoAck () = 0;
        virtual void UnrecognizedSignalDetected (double , double , Time start, Time duration) = 0;
        virtual void RecognizedSignalDetected (double , double , Time start, Time duration) = 0;
        virtual void RxStartIndication (MmWaveTxVector txVector, Time psduDuration) = 0;
        virtual void CancelAllEvents () = 0;
        void SetPhy (const Ptr<MmWavePhy> phy);
        void ResetPhy ();
        void SetMac (const Ptr<MmWaveMac> mac);
        void SetRemoteStationManager (const Ptr<MmWaveRemoteStationManager> manager);
        void SetupMacLowListener (const Ptr<MmWavePhy> phy);
        void RemoveMacLowListener (Ptr<MmWavePhy> phy);
        void SetAddress (Mac48Address ad);
        void SetBssid (Mac48Address ad);
        void SetPromisc ();
        void SetRxCallback (Callback<void, Ptr<MmWaveMacQueueItem>> callback);
        void SetTxOkCallback (Callback <void, const MmWaveMacHeader &> callback);
        void SetTxFailedCallback (Callback <void, const MmWaveMacHeader &> callback);
        void ForwardDown (Ptr<const MmWavePsdu> psdu, MmWaveTxVector txVector);
        Ptr<MmWavePhy> GetPhy () const;
        Ptr<MmWaveMac> GetMac () const;
        Mac48Address GetAddress () const;
        Mac48Address GetBssid () const;
        MmWaveTxVector GetDataTxVector (Mac48Address address) const;
        MmWaveTxVector GetCtrlTxVector (Mac48Address address) const;
        MmWaveTxVector GetCtsTxVector (Mac48Address to, MmWaveMode rtsTxMode) const;
        MmWaveTxVector GetAckTxVector (Mac48Address to, MmWaveMode dataTxMode) const;
        MmWaveTxVector GetCtsTxVectorForRts (Mac48Address to, MmWaveMode rtsTxMode) const;
        MmWaveTxVector GetAckTxVectorForData (Mac48Address to, MmWaveMode dataTxMode) const;
        Time GetCtsDuration (const MmWaveTxVector ctsTxVector) const;
        Time GetCtsDuration (Mac48Address to, const MmWaveTxVector rtsTxVector) const;
        Time GetAckDuration (const MmWaveTxVector ackTxVector) const;
        Time GetAckDuration (Mac48Address to, const MmWaveTxVector dataTxVector) const;
        Time GetResponseDuration (const MmWaveMacLowParameters params, const MmWaveTxVector dataTxVector, Mac48Address receiver) const;
        Time GetSifs () const;
        Time GetSlotTime () const;
        Time CalculateOverheadTxTime (Ptr<const MmWaveMacQueueItem> item, const MmWaveMacLowParameters& params) const;
        bool IsNavZero () const;
        bool IsPromisc () const;
        uint32_t GetRtsSize () const;
        uint32_t GetCtsSize () const;
        uint32_t GetAckSize () const;
        uint32_t GetSize (Ptr<const Packet> packet, const MmWaveMacHeader *hdr) const;
        int64_t AssignStreams (int64_t stream);
    public:
        EventId m_normalAckTimeoutEvent;
        EventId m_ctsTimeoutEvent;
        EventId m_sendCtsEvent;
        EventId m_sendAckEvent;
        EventId m_waitIfsEvent;
        EventId m_endTxNoAckEvent;
        EventId m_navCounterReset;
        Ptr<MmWavePhy> m_phy;
        Ptr<MmWaveMac> m_mac;
        Ptr<MmWaveRemoteStationManager> m_stationManager;
        Ptr<MmWavePsdu> m_currentPacket;
        Ptr<UniformRandomVariable> m_rng;
        Callback<void, Ptr<MmWaveMacQueueItem>> m_rxCallback;
        Callback <void, const MmWaveMacHeader &> m_txOk;
        Callback <void, const MmWaveMacHeader &> m_txFailed;
        MmWaveMacLowParameters m_txParams;
        MmWaveTxVector m_currentTxVector;
        Mac48Address m_self;
        Mac48Address m_bssid;
        Time m_lastNavStart;
        Time m_lastNavDuration;
        bool m_promisc;
        class MmWaveMacLowListener * m_macLowListener;
    };

} //namespace ns3
#endif //MMWAVE_MAC_LOW_H
