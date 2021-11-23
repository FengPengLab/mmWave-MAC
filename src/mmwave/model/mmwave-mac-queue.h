/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_MAC_QUEUUE_H
#define MMWAVE_MAC_QUEUUE_H
#include <vector>
#include <utility>
#include "ns3/queue.h"
#include "mmwave.h"
#include "mmwave-mac-queue-item.h"
namespace ns3 {
    extern template class Queue<MmWaveMacQueueItem>;
    class MmWaveMacQueue : public Queue<MmWaveMacQueueItem>
    {
    public:
        static TypeId GetTypeId ();
        MmWaveMacQueue ();
        ~MmWaveMacQueue ();

        enum DropPolicy
        {
            DROP_NEWEST,
            DROP_OLDEST
        };
        
        using Queue<MmWaveMacQueueItem>::ConstIterator;
        using Queue<MmWaveMacQueueItem>::Iterator;
        using Queue<MmWaveMacQueueItem>::begin;
        using Queue<MmWaveMacQueueItem>::end;
        
        void SetMaxDelay (Time delay);
        Time GetMaxDelay () const;
        bool PushFront (Ptr<MmWaveMacQueueItem> item);
        bool IsEmpty ();
        bool Insert (ConstIterator pos, Ptr<MmWaveMacQueueItem> item);
        bool Remove (Ptr<const Packet> packet);
        bool Enqueue (Ptr<MmWaveMacQueueItem> item);
        bool FindByAddress (Mac48Address to);
        bool FindByChannel (MmWaveChannelNumberStandardPair channel);
        bool FindByAddressAndChannel (Mac48Address to, MmWaveChannelNumberStandardPair channel);
        Ptr<MmWaveMacQueueItem> Dequeue ();
        Ptr<MmWaveMacQueueItem> Dequeue (MmWaveMacQueue::ConstIterator pos);
        Ptr<MmWaveMacQueueItem> DequeueFirstAvailable ();
        Ptr<MmWaveMacQueueItem> DequeueByAddress (Mac48Address dest);
        Ptr<MmWaveMacQueueItem> DequeueByChannel (MmWaveChannelNumberStandardPair channel);
        Ptr<MmWaveMacQueueItem> DequeueExcludedChannel (MmWaveChannelNumberStandardPair channel);
        Ptr<MmWaveMacQueueItem> DequeueByAddressAndChannel (Mac48Address dest, MmWaveChannelNumberStandardPair channel);
        Ptr<const MmWaveMacQueueItem> Peek () const;
        Ptr<MmWaveMacQueueItem> Remove ();
        Ptr<const MmWaveMacQueueItem> PeekItemByAddress (Mac48Address addr);
        Ptr<const MmWaveMacQueueItem> PeekItemByChannel (MmWaveChannelNumberStandardPair channel);
        MmWaveChannelNumberStandardPair GetNeedToAccessExcludedChannel (MmWaveChannelNumberStandardPair c);
        std::vector<Ptr<const MmWaveMacQueueItem>> PeekFrontNItemByChannel (uint32_t max, MmWaveChannelNumberStandardPair channel);
        std::vector<Mac48Address> PeekFrontNAddresses (int num);
        std::map<Mac48Address, std::vector<std::pair<MmWaveMacHeader, uint32_t>>> GetBulkAccessRequestsInQueue (uint32_t num, MmWaveChannelNumberStandardPair channel);
        ConstIterator PeekByChannel (MmWaveChannelNumberStandardPair channel) const;
        ConstIterator PeekExcludedChannel (MmWaveChannelNumberStandardPair channel) const;
        ConstIterator PeekByAddress (Mac48Address dest) const;
        ConstIterator PeekFirstAvailable (ConstIterator pos = EMPTY) const;
        ConstIterator PeekByAddressAndChannel (Mac48Address dest, MmWaveChannelNumberStandardPair channel) const;
        ConstIterator Remove (ConstIterator pos, bool removeExpired = false);
        uint32_t GetNPacketsByAddress (Mac48Address dest);
        uint32_t GetNPackets ();
        uint32_t GetNBytes ();
        static const ConstIterator EMPTY;         //!< Invalid iterator to signal an empty queue

    private:
        bool TtlExceeded (ConstIterator &it);
        Time m_maxDelay;                          //!< Time to live for packets in the queue
        DropPolicy m_dropPolicy;                  //!< Drop behavior of queue
        mutable bool m_expiredPacketsPresent;     //!< True if expired packets are in the queue
        TracedCallback<Ptr<const MmWaveMacQueueItem> > m_traceExpired;
        NS_LOG_TEMPLATE_DECLARE;                  //!< redefinition of the log component
    };

} //namespace ns3
#endif //MMWAVE_MAC_QUEUUE_H
