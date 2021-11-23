/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef V2X_DATA_MAC_LOW_H
#define V2X_DATA_MAC_LOW_H
#include "mmwave-mac-low.h"
#include "mmwave-mac-tx-middle.h"
#include "mmwave-mac-rx-middle.h"
namespace ns3 {

    class V2xDataMacLow : public MmWaveMacLow
    {
    public:
        static TypeId GetTypeId ();
        V2xDataMacLow ();
        virtual ~V2xDataMacLow ();

        bool DoNavStartNow (Time duration);
        void DoNavResetNow (Time duration);
        void NotifyAckTimeoutStartNow (Time duration);
        void NotifyAckTimeoutResetNow ();
        void NotifyCtsTimeoutStartNow (Time duration);
        void NotifyCtsTimeoutResetNow ();
        void NormalAckTimeout ();
        void NotifySwitchingStartNow (Time duration);
        void NotifySleepNow ();
        void NotifyOffNow ();
        void NotifyNav (Ptr<const Packet> packet,const MmWaveMacHeader &hdr);
        void CtsTimeout ();
        void WaitIfsAfterEndTxFragment ();
        void WaitIfsAfterEndTxPacket ();
        void EndTxNoAck ();
        void CancelAllEvents ();
        void ReceiveOk (Ptr<MmWavePsdu> psdu, double rxSnr, Time start, Time duration, MmWaveTxVector txVector);
        void ReceiveError (Ptr<MmWavePsdu> psdu, double rxSnr, Time start, Time duration, MmWaveTxVector txVector);
        void UnrecognizedSignalDetected (double power, double snr, Time start, Time duration);
        void RecognizedSignalDetected (double power, double snr, Time start, Time duration);
        void RxStartIndication (MmWaveTxVector txVector, Time psduDuration);
        void StartTransmissionImmediately (Ptr<MmWaveMacQueueItem> mpdu, MmWaveMacLowParameters params);
        void StartDataTxTimers (const MmWaveTxVector dataTxVector);
        void SendPacket ();
        void SendAckAfterData (Mac48Address source, MmWaveMode dataTxMode, double dataSnr);
    };
}
#endif //V2X_DATA_MAC_LOW_H
