/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
#ifndef CR_MAC_LOW_H
#define CR_MAC_LOW_H
#include <map>
#include <utility>
#include <deque>
#include "ns3/simple-ref-count.h"
#include "ns3/mac48-address.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "mmwave.h"
#include "mmwave-psdu.h"
#include "mmwave-phy.h"
#include "mmwave-mac.h"
#include "mmwave-mac-header.h"
#include "mmwave-mac-queue.h"
#include "mmwave-mac-queue-item.h"
#include "mmwave-remote-station-manager.h"
#include "mmwave-mac-low.h"
#include "mmwave-mac-low-parameters.h"
#include "cr-txop.h"
#include "cr-dynamic-channel-access-manager.h"
namespace ns3 {

    class CrMmWaveMacLow : public MmWaveMacLow
    {
    public:
        CrMmWaveMacLow ();
        ~CrMmWaveMacLow ();
        static TypeId GetTypeId ();
        void DoInitialize ();
        void DoDispose ();
        void ReceiveOk (Ptr<MmWavePsdu> psdu, double rxSnr, Time start, Time duration, MmWaveTxVector txVector);
        void ReceiveError (Ptr<MmWavePsdu> psdu, double rxSnr, Time start, Time duration, MmWaveTxVector txVector);
        bool DoNavStartNow (Time duration);
        void DoNavResetNow (Time duration);
        void NotifyAckTimeoutStartNow (Time duration);
        void NotifyAckTimeoutResetNow ();
        void NotifyCtsTimeoutStartNow (Time duration);
        void NotifyCtsTimeoutResetNow ();
        void NotifySwitchingStartNow (Time duration);
        void NotifySleepNow ();
        void NotifyOffNow ();
        void NotifyNav (Ptr<const Packet> packet,const MmWaveMacHeader &hdr);
        void CtsTimeout ();
        void NormalAckTimeout ();
        void WaitIfsAfterEndTxFragment ();
        void WaitIfsAfterEndTxPacket ();
        void EndTxNoAck ();
        void UnrecognizedSignalDetected (double power, double snr, Time start, Time duration);
        void RecognizedSignalDetected (double power, double snr, Time start, Time duration);
        void RxStartIndication (MmWaveTxVector txVector, Time psduDuration);
        void NotifyBulkTimeoutStartNow (Time duration);
        void NotifyBulkTimeoutResetNow ();
        void NotifyUpdateRotationFactor ();
        void BulkAckTimeout ();
        void BulkTimeout ();
        void NoSignalDetected ();
        void UpdateBulkAckInfo (Mac48Address source, uint16_t sequence);
        void SetTypeOfGroup (TypeOfGroup typeOfGroup);
        void SetMacLowState (MmWaveMacState state);
        void SetChannelAccessManager (Ptr<CrDynamicChannelAccessManager> channelAccessManager);
        void SetTxop (Ptr<CrMmWaveTxop> txop);
        void CancelAllEvents ();
        void SendDetectionRequest ();
        void SendBeacon ();
        void SendDataForPacket ();
        void SendBulkRequestForPacket ();
        void SendBulkResponseAfterRequest (Mac48Address source, Time txDuration);
        void SendAckAfterData ();
        void SendPacket ();
        void SendBulkAccessRequest ();
        void StartNewBulkAccess ();
        void StartBeacon ();
        void StartDetection ();
        void StartAccessIfNeed ();
        void StartTransmission (Ptr<MmWaveMacQueueItem> mpdu);
        void StartTxTimers (const MmWaveTxVector dataTxVector);
        void StartDetectionChannel (Time duration);
        void StopDetectionChannel ();
        void SetDetectionStartTime (Time start);
        void SetDetectionDuration (Time duration);
        void StartChannelAccess ();
        void EndChannelAccess ();
        void EndBulkAccess ();
        void DetectionTxEnd ();
        void BeaconTxEnd ();
        void ToSuspend ();
        void ToSwitchChannel (MmWaveChannelNumberStandardPair next);
        void ToTransmission ();
        void ToDetection ();
        bool IsBusy ();
        bool IsAnyPUs (MmWaveChannelNumberStandardPair channel);
        bool IsAnySUs (MmWaveChannelNumberStandardPair channel);
        bool IsWithinSizeAndTimeLimits (uint32_t size, MmWaveTxVector txVector, Time limit);
        bool IsOffMode ();
        bool IsPhyStateTx ();
        TypeOfGroup GetTypeOfGroup () const;
        MmWaveMacState GetMacLowState () const;
        MmWaveChannelNumberStandardPair GetCurrentChannel ();
        MmWaveChannelNumberStandardPair GetNextChannel (MmWaveChannelNumberStandardPair channel);
        MmWaveChannelToFrequencyWidthMap GetChannelToFrequency ();

        Time GetBulkResponseTxDuration (Mac48Address to);
        Time GetBulkAckTxDuration (Mac48Address to);
        Time GetBulkResponseTimeout (Mac48Address to);
        Time CalculateTxDuration (MmWaveMacHeader hdr, uint32_t size);
        Time GetPhyPreambleAndHeaderDuration (MmWaveTxVector txVector);
        Time GetDetectionStartTime () const;
        Time GetDetectionDuration () const;
        uint32_t GetRandomInteger (uint32_t min, uint32_t max);
        void CreateBulkAccessRequests ();
        void ReadySendAckAfterData ();
        EventId m_stateSuspending;
        EventId m_stateSwitching;
        EventId m_stateDetection;
        EventId m_txPacket;
        EventId m_sendBulkAck;
        EventId m_bulkAccess;
        EventId m_bulkResponseTimeout;
        EventId m_bulkAckTimeout;
        EventId m_beaconEvent;
        EventId m_detectionEvent;

    protected:
        Ptr<CrMmWaveTxop> m_txop;
        Ptr<CrDynamicChannelAccessManager> m_channelAccessManager;
        TypeOfGroup m_typeOfGroup;
        MmWaveMacState m_macState;
        Ptr<BulkAccessInfo> m_txBulkAccessInfo;
        Ptr<BulkAccessInfo> m_rxBulkAccessInfo;
        std::deque<Ptr<BulkAccessInfo>> m_bulkAccessRequests;
        Time m_detectionStartTime;
        Time m_detectionDuration;
        MmWaveChannelNumberStandardPair m_switchChannel;
        bool m_switchChannelFlag;
        bool m_makeDecision;
        bool m_noActiveUsers;
        bool m_accessing;
    };
} //namespace ns3
#endif //CR_MAC_LOW_H
