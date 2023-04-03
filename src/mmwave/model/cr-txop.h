/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef CR_TXOP_H
#define CR_TXOP_H
#include <vector>
#include <map>
#include <deque>
#include <algorithm>
#include "ns3/type-id.h"
#include "ns3/object.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/mac48-address.h"
#include "mmwave.h"
#include "mmwave-mac-low-parameters.h"
#include "mmwave-mac-header.h"
#include "mmwave-mode.h"
#include "mmwave-mac-queue.h"
#include "mmwave-mac-queue-item.h"
#include "mmwave-mac-tx-middle.h"
#include "mmwave-remote-station-manager.h"
#include "mmwave-tx-vector.h"
#include "cr-dynamic-channel-access-manager.h"
namespace ns3 {

    class Packet;
    class UniformRandomVariable;
    class CrMmWaveMac;
    class CrMmWaveMacLow;

    class CrMmWaveTxop : public Object
    {
    public:
        CrMmWaveTxop ();
        virtual ~CrMmWaveTxop ();
        static TypeId GetTypeId ();
        void DoDispose ();
        void DoInitialize ();
        void SetMac (const Ptr<CrMmWaveMac> mac);
        void SetMacLowOfIntraGroup (const Ptr<CrMmWaveMacLow> low);
        void SetMacLowOfInterGroup (const Ptr<CrMmWaveMacLow> low);
        void SetRemoteStationManager (const Ptr<MmWaveRemoteStationManager> remoteManager);
        void SetTxMiddle (TypeOfGroup typeOfGroup, const Ptr<MmWaveMacTxMiddle> txMiddle);
        void SetTxOkCallback (Callback <void, const MmWaveMacHeader&> callback);
        void SetTxFailedCallback (Callback <void, const MmWaveMacHeader&> callback);
        void SetTxDroppedCallback (Callback <void, Ptr<const Packet>> callback);
        void Queue (Ptr<Packet> packet, MmWaveMacHeader hdr);
        void GotBulkAck (TypeOfGroup typeOfGroup, uint16_t startingSeq, uint64_t bitmap);
        void MissedBulkAck (TypeOfGroup typeOfGroup);
        void MissedBulkResponse (TypeOfGroup typeOfGroup);
        void TxOk (MmWaveMacHeader hdr);
        void TxFailed (MmWaveMacHeader hdr, uint32_t packetSize);
        void TxDroppedPacket (Ptr<const MmWaveMacQueueItem> item);
        void NotifyAccessGranted (TypeOfGroup typeOfGroup);
        void SetAccessBuffer (TypeOfGroup typeOfGroup, Mac48Address to, Time txDuration);
        void ClearAllPackets (TypeOfGroup typeOfGroup);
        bool HasNextPacket (TypeOfGroup typeOfGroup);
        bool HasNextFragment (TypeOfGroup typeOfGroup);
        bool HasAnyAccessRequest (TypeOfGroup typeOfGroup);
        uint16_t GetStartingSequenceNumber (TypeOfGroup typeOfGroup);
        std::vector<MmWaveChannelNumberStandardPair> GetChannelsNeedToAccess (Mac48Address to);
        std::map<Mac48Address, std::vector<std::pair<MmWaveMacHeader, uint32_t>>> GetBulkAccessRequestsInQueue (TypeOfGroup typeOfGroup);
        MmWaveChannelNumberStandardPair GetNeedToAccessForInterGroup ();
        MmWaveChannelNumberStandardPair GetCurrentChannel (TypeOfGroup typeOfGroup);
        Ptr<Packet> GetFragmentPacket (TypeOfGroup typeOfGroup, MmWaveMacHeader *hdr);
        Ptr<MmWaveMacQueue> GetMacQueue () const;
    protected:
        Callback <void, const MmWaveMacHeader&> m_txOkCallback;
        Callback <void, const MmWaveMacHeader&> m_txFailedCallback;
        Callback <void, Ptr<const Packet>> m_txDroppedCallback;
        Ptr<MmWaveRemoteStationManager> m_stationManager;
        Ptr<MmWaveMacQueue> m_queue;
        Ptr<MmWaveMacTxMiddle> m_txMiddleOfIntraGroup;
        Ptr<MmWaveMacTxMiddle> m_txMiddleOfInterGroup;
        Ptr<CrMmWaveMac> m_mac;
        Ptr<CrMmWaveMacLow> m_lowOfIntraGroup;
        Ptr<CrMmWaveMacLow> m_lowOfInterGroup;
        std::deque<Ptr<MmWaveMacQueueItem>> m_bufferOfIntraGroup;
        std::deque<Ptr<MmWaveMacQueueItem>> m_bufferOfInterGroup;

        std::deque<Ptr<MmWaveMacQueueItem>> m_recordsOfIntraGroup;
        std::deque<Ptr<MmWaveMacQueueItem>> m_recordsOfInterGroup;

        Ptr<MmWaveMacQueueItem> m_currentItemOfIntraGroup;
        Ptr<MmWaveMacQueueItem> m_currentItemOfInterGroup;
        bool m_accessRequestedOfIntraGroup;
        bool m_accessRequestedOfInterGroup;
        uint8_t m_fragmentNumberOfIntraGroup;
        uint8_t m_fragmentNumberOfInterGroup;
        uint32_t m_bufferSize;
    };
}
#endif //CR_TXOP_H
