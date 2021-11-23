/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include <algorithm>
#include "ns3/simulator.h"
#include "ns3/enum.h"
#include "ns3/queue-size.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "mmwave-mac-queue.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("MmWaveMacQueue");
    NS_OBJECT_ENSURE_REGISTERED (MmWaveMacQueue);
    NS_OBJECT_TEMPLATE_CLASS_DEFINE (Queue, MmWaveMacQueueItem);

    TypeId
    MmWaveMacQueue::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::MmWaveMacQueue")
                                    .SetParent<Queue<MmWaveMacQueueItem> > ()
                                    .SetGroupName ("MmWave")
                                    .AddConstructor<MmWaveMacQueue> ()
                                    .AddAttribute ("MaxSize",
                                                   "The max queue size",
                                                   QueueSizeValue (QueueSize ("500p")),
                                                   MakeQueueSizeAccessor (&QueueBase::SetMaxSize,
                                                                          &QueueBase::GetMaxSize),
                                                   MakeQueueSizeChecker ())
                                    .AddAttribute ("MaxDelay", "If a packet stays longer than this delay in the queue, it is dropped.",
                                                   TimeValue (MilliSeconds (500)),
                                                   MakeTimeAccessor (&MmWaveMacQueue::SetMaxDelay),
                                                   MakeTimeChecker ())
                                    .AddAttribute ("DropPolicy", "Upon enqueue with full queue, drop oldest (DropOldest) or newest (DropNewest) packet",
                                                   EnumValue (DROP_NEWEST),
                                                   MakeEnumAccessor (&MmWaveMacQueue::m_dropPolicy),
                                                   MakeEnumChecker (MmWaveMacQueue::DROP_OLDEST, "DropOldest",
                                                                    MmWaveMacQueue::DROP_NEWEST, "DropNewest"))
                                    .AddTraceSource ("Expired", "MPDU dropped because its lifetime expired.",
                                                     MakeTraceSourceAccessor (&MmWaveMacQueue::m_traceExpired),
                                                     "ns3::MmWaveMacQueueItem::TracedCallback")
        ;
        return tid;
    }

    MmWaveMacQueue::MmWaveMacQueue ()
            : m_expiredPacketsPresent (false),
              NS_LOG_TEMPLATE_DEFINE ("MmWaveMacQueue")
    {
    }

    MmWaveMacQueue::~MmWaveMacQueue ()
    {
        NS_LOG_FUNCTION_NOARGS ();
    }

    static std::list<Ptr<MmWaveMacQueueItem>> g_emptyMmWaveMacQueue;

    const MmWaveMacQueue::ConstIterator MmWaveMacQueue::EMPTY = g_emptyMmWaveMacQueue.end ();

    void
    MmWaveMacQueue::SetMaxDelay (Time delay)
    {
        NS_LOG_FUNCTION (this << delay);
        m_maxDelay = delay;
    }

    Time
    MmWaveMacQueue::GetMaxDelay () const
    {
        NS_LOG_FUNCTION (this);
        return m_maxDelay;
    }

    bool
    MmWaveMacQueue::TtlExceeded (ConstIterator &it)
    {
        NS_LOG_FUNCTION (this);

        if (Simulator::Now () > (*it)->GetTimeStamp () + m_maxDelay)
        {
            NS_LOG_DEBUG ("Removing packet that stayed in the queue for too long (" << Simulator::Now () - (*it)->GetTimeStamp () << ")");
            m_traceExpired (*it);
            auto curr = it++;
            DoRemove (curr);
            return true;
        }
        return false;
    }

    bool
    MmWaveMacQueue::Enqueue (Ptr<MmWaveMacQueueItem> item)
    {
        NS_LOG_FUNCTION (this << *item);

        return Insert (end (), item);
    }

    bool
    MmWaveMacQueue::PushFront (Ptr<MmWaveMacQueueItem> item)
    {
        NS_LOG_FUNCTION (this << *item);

        return Insert (begin (), item);
    }

    bool
    MmWaveMacQueue::Insert (ConstIterator pos, Ptr<MmWaveMacQueueItem> item)
    {
        NS_LOG_FUNCTION (this << *item);
        NS_ASSERT_MSG (GetMaxSize ().GetUnit () == QueueSizeUnit::PACKETS,
                       "MmWaveMacQueues must be in packet mode");

        // insert the item if the queue is not full
        if (QueueBase::GetNPackets () < GetMaxSize ().GetValue ())
        {
            return DoEnqueue (pos, item);
        }

        // the queue is full; scan the list in the attempt to remove stale packets
        ConstIterator it = begin ();
        while (it != end ())
        {
            if (it == pos && TtlExceeded (it))
            {
                return DoEnqueue (it, item);
            }
            if (TtlExceeded (it))
            {
                return DoEnqueue (pos, item);
            }
            it++;
        }

        // the queue is still full, remove the oldest item if the policy is drop oldest
        if (m_dropPolicy == DROP_OLDEST)
        {
            NS_LOG_DEBUG ("Remove the oldest item in the queue");
            DoRemove (begin ());
        }

        return DoEnqueue (pos, item);
    }

    Ptr<MmWaveMacQueueItem>
    MmWaveMacQueue::Dequeue ()
    {
        NS_LOG_FUNCTION (this);
        for (ConstIterator it = begin (); it != end (); )
        {
            if (!TtlExceeded (it))
            {
                return DoDequeue (it);
            }
        }
        NS_LOG_DEBUG ("The queue is empty");
        return 0;
    }

    Ptr<MmWaveMacQueueItem>
    MmWaveMacQueue::DequeueByAddress (Mac48Address dest)
    {
        NS_LOG_FUNCTION (this << dest);
        ConstIterator it = PeekByAddress (dest);

        if (it == end ())
        {
            return 0;
        }
        return Dequeue (it);
    }

    Ptr<MmWaveMacQueueItem>
    MmWaveMacQueue::DequeueByChannel (MmWaveChannelNumberStandardPair channel)
    {
        NS_LOG_FUNCTION (this);
        ConstIterator it = PeekByChannel (channel);

        if (it == end ())
        {
            return 0;
        }
        return Dequeue (it);
    }

    Ptr<MmWaveMacQueueItem>
    MmWaveMacQueue::DequeueByAddressAndChannel (Mac48Address dest, MmWaveChannelNumberStandardPair channel)
    {
        NS_LOG_FUNCTION (this);
        ConstIterator it = PeekByAddressAndChannel (dest, channel);

        if (it == end ())
        {
            return 0;
        }
        return Dequeue (it);
    }

    Ptr<MmWaveMacQueueItem>
    MmWaveMacQueue::DequeueFirstAvailable ()
    {
        NS_LOG_FUNCTION (this);
        ConstIterator it = PeekFirstAvailable ();

        if (it == end ())
        {
            return 0;
        }
        return Dequeue (it);
    }

    Ptr<MmWaveMacQueueItem>
    MmWaveMacQueue::DequeueExcludedChannel (MmWaveChannelNumberStandardPair channel)
    {
        NS_LOG_FUNCTION (this << channel);
        ConstIterator it = PeekExcludedChannel (channel);

        if (it == end ())
        {
            return 0;
        }
        return Dequeue (it);
    }

    Ptr<MmWaveMacQueueItem>
    MmWaveMacQueue::Dequeue (ConstIterator pos)
    {
        NS_LOG_FUNCTION (this);

        if (!m_expiredPacketsPresent)
        {
            if (TtlExceeded (pos))
            {
                NS_LOG_DEBUG ("Packet lifetime expired");
                return 0;
            }
            return DoDequeue (pos);
        }

        // remove stale items queued before the given position
        ConstIterator it = begin ();
        while (it != end ())
        {
            if (it == pos)
            {
                // reset the flag signaling the presence of expired packets before returning
                m_expiredPacketsPresent = false;

                if (TtlExceeded (it))
                {
                    return 0;
                }
                return DoDequeue (it);
            }
            else if (!TtlExceeded (it))
            {
                it++;
            }
        }
        NS_LOG_DEBUG ("Invalid iterator");
        return 0;
    }

    Ptr<const MmWaveMacQueueItem>
    MmWaveMacQueue::Peek () const
    {
        NS_LOG_FUNCTION (this);
        for (auto it = begin (); it != end (); it++)
        {
            // skip packets that stayed in the queue for too long. They will be
            // actually removed from the queue by the next call to a non-const method
            if (Simulator::Now () <= (*it)->GetTimeStamp () + m_maxDelay)
            {
                return DoPeek (it);
            }
            // signal the presence of expired packets
            m_expiredPacketsPresent = true;
        }
        NS_LOG_DEBUG ("The queue is empty");
        return 0;
    }

    MmWaveMacQueue::ConstIterator
    MmWaveMacQueue::PeekByAddress (Mac48Address dest) const
    {
        NS_LOG_FUNCTION (this << dest);
        ConstIterator it = begin ();
        while (it != end ())
        {
            // skip packets that stayed in the queue for too long. They will be
            // actually removed from the queue by the next call to a non-const method
            if (Simulator::Now () <= (*it)->GetTimeStamp () + m_maxDelay)
            {
                if ((*it)->GetHeader ().IsData () && ((*it)->GetDestinationAddress () == dest))
                {
                    return it;
                }
            }
            else
            {
                // signal the presence of expired packets
                m_expiredPacketsPresent = true;
            }
            it++;
        }
        NS_LOG_DEBUG ("The queue is empty");
        return end ();
    }

    MmWaveMacQueue::ConstIterator
    MmWaveMacQueue::PeekByChannel (MmWaveChannelNumberStandardPair channel) const
    {
        NS_LOG_FUNCTION (this);
        ConstIterator it = begin ();
        while (it != end ())
        {
            // skip packets that stayed in the queue for too long. They will be
            // actually removed from the queue by the next call to a non-const method
            if (Simulator::Now () <= (*it)->GetTimeStamp () + m_maxDelay)
            {
                if ((*it)->GetHeader ().IsData () && ((*it)->GetChannel () == channel))
                {
                    return it;
                }
            }
            else
            {
                // signal the presence of expired packets
                m_expiredPacketsPresent = true;
            }
            it++;
        }
        NS_LOG_DEBUG ("The queue is empty");
        return end ();
    }

    MmWaveMacQueue::ConstIterator
    MmWaveMacQueue::PeekByAddressAndChannel (Mac48Address dest, MmWaveChannelNumberStandardPair channel) const
    {
        NS_LOG_FUNCTION (this);
        ConstIterator it = begin ();
        while (it != end ())
        {
            // skip packets that stayed in the queue for too long. They will be
            // actually removed from the queue by the next call to a non-const method
            if (Simulator::Now () <= (*it)->GetTimeStamp () + m_maxDelay)
            {
                if ((*it)->GetHeader ().IsData ()
                    && ((*it)->GetChannel () == channel)
                    && ((*it)->GetDestinationAddress () == dest))
                {
                    return it;
                }
            }
            else
            {
                // signal the presence of expired packets
                m_expiredPacketsPresent = true;
            }
            it++;
        }
        NS_LOG_DEBUG ("The queue is empty");
        return end ();
    }

    MmWaveMacQueue::ConstIterator
    MmWaveMacQueue::PeekExcludedChannel (MmWaveChannelNumberStandardPair channel) const
    {
        NS_LOG_FUNCTION (this);
        ConstIterator it = begin ();
        while (it != end ())
        {
            // skip packets that stayed in the queue for too long. They will be
            // actually removed from the queue by the next call to a non-const method
            if (Simulator::Now () <= (*it)->GetTimeStamp () + m_maxDelay)
            {
                if ((*it)->GetHeader ().IsData () && ((*it)->GetChannel () != channel))
                {
                    return it;
                }
            }
            else
            {
                // signal the presence of expired packets
                m_expiredPacketsPresent = true;
            }
            it++;
        }
        NS_LOG_DEBUG ("The queue is empty");
        return end ();
    }

    MmWaveMacQueue::ConstIterator
    MmWaveMacQueue::PeekFirstAvailable (ConstIterator pos) const
    {
        NS_LOG_FUNCTION (this);
        ConstIterator it = begin ();
        while (it != end ())
        {
            if (Simulator::Now () <= (*it)->GetTimeStamp () + m_maxDelay)
            {
                return it;
            }
            else
            {
                // signal the presence of expired packets
                m_expiredPacketsPresent = true;
            }
            it++;
        }
        NS_LOG_DEBUG ("The queue is empty");
        return end ();
    }

    Ptr<MmWaveMacQueueItem>
    MmWaveMacQueue::Remove ()
    {
        NS_LOG_FUNCTION (this);

        for (ConstIterator it = begin (); it != end (); )
        {
            if (!TtlExceeded (it))
            {
                return DoRemove (it);
            }
        }
        NS_LOG_DEBUG ("The queue is empty");
        return 0;
    }

    bool
    MmWaveMacQueue::Remove (Ptr<const Packet> packet)
    {
        NS_LOG_FUNCTION (this << packet);
        for (ConstIterator it = begin (); it != end (); )
        {
            if (!TtlExceeded (it))
            {
                if ((*it)->GetPacket () == packet)
                {
                    DoRemove (it);
                    return true;
                }

                it++;
            }
        }
        NS_LOG_DEBUG ("Packet " << packet << " not found in the queue");
        return false;
    }

    MmWaveMacQueue::ConstIterator
    MmWaveMacQueue::Remove (ConstIterator pos, bool removeExpired)
    {
        NS_LOG_FUNCTION (this);

        if (!removeExpired)
        {
            ConstIterator curr = pos++;
            DoRemove (curr);
            return pos;
        }

        // remove stale items queued before the given position
        ConstIterator it = begin ();
        while (it != end ())
        {
            if (it == pos)
            {
                // reset the flag signaling the presence of expired packets before returning
                m_expiredPacketsPresent = false;

                ConstIterator curr = pos++;
                DoRemove (curr);
                return pos;
            }
            else if (!TtlExceeded (it))
            {
                it++;
            }
        }
        NS_LOG_DEBUG ("Invalid iterator");
        return end ();
    }

    uint32_t
    MmWaveMacQueue::GetNPacketsByAddress (Mac48Address dest)
    {
        NS_LOG_FUNCTION (this << dest);

        uint32_t nPackets = 0;

        for (ConstIterator it = begin (); it != end (); )
        {
            if (!TtlExceeded (it))
            {
                if ((*it)->GetHeader ().IsData () && (*it)->GetDestinationAddress () == dest)
                {
                    nPackets++;
                }

                it++;
            }
        }
        NS_LOG_DEBUG ("returns " << nPackets);
        return nPackets;
    }

    bool
    MmWaveMacQueue::IsEmpty ()
    {
        NS_LOG_FUNCTION (this);
        for (ConstIterator it = begin (); it != end (); )
        {
            if (!TtlExceeded (it))
            {
                NS_LOG_DEBUG ("returns false");
                return false;
            }
        }
        NS_LOG_DEBUG ("returns true");
        return true;
    }

    uint32_t
    MmWaveMacQueue::GetNPackets ()
    {
        NS_LOG_FUNCTION (this);
        // remove packets that stayed in the queue for too long
        for (ConstIterator it = begin (); it != end (); )
        {
            if (!TtlExceeded (it))
            {
                it++;
            }
        }
        return QueueBase::GetNPackets ();
    }

    uint32_t
    MmWaveMacQueue::GetNBytes ()
    {
        NS_LOG_FUNCTION (this);
        // remove packets that stayed in the queue for too long
        for (ConstIterator it = begin (); it != end (); )
        {
            if (!TtlExceeded (it))
            {
                it++;
            }
        }
        return QueueBase::GetNBytes ();
    }

    bool
    MmWaveMacQueue::FindByAddress (Mac48Address to)
    {
        NS_LOG_FUNCTION (this);
        for (ConstIterator it = begin (); it != end (); )
        {
            if (!TtlExceeded (it))
            {
                if ((*it)->GetHeader ().GetAddr1 () == to)
                {
                    return true;
                }

                it++;
            }
        }
        NS_LOG_DEBUG ("The queue is empty");
        return false;
    }

    bool
    MmWaveMacQueue::FindByChannel (MmWaveChannelNumberStandardPair channel)
    {
        NS_LOG_FUNCTION (this);
        for (ConstIterator it = begin (); it != end (); )
        {
            if (!TtlExceeded (it))
            {
                if ((*it)->GetChannel() == channel)
                {
                    return true;
                }

                it++;
            }
        }
        NS_LOG_DEBUG ("The queue is empty");
        return false;
    }

    bool
    MmWaveMacQueue::FindByAddressAndChannel (Mac48Address to, MmWaveChannelNumberStandardPair channel)
    {
        NS_LOG_FUNCTION (this);
        for (ConstIterator it = begin (); it != end (); )
        {
            if (!TtlExceeded (it))
            {
                if ((*it)->GetChannel() == channel && (*it)->GetHeader ().GetAddr1 () == to)
                {
                    return true;
                }

                it++;
            }
        }
        NS_LOG_DEBUG ("The queue is empty");
        return false;
    }

    std::vector<Mac48Address>
    MmWaveMacQueue::PeekFrontNAddresses (int num)
    {
        NS_LOG_FUNCTION (this);
        std::vector<Mac48Address> list;
        for (ConstIterator it = begin (); it != end (); )
        {
            if (!TtlExceeded (it))
            {
                if (list.size () >= static_cast<uint32_t>(num))
                {
                    break;
                }
                Mac48Address to = (*it)->GetHeader ().GetAddr1 ();
                if (to == Mac48Address::GetBroadcast ())
                {
                    if (list.size () == 0)
                    {
                        list.push_back (to);
                        return list;
                    }
                    else
                    {
                        return list;
                    }
                }
                else
                {
                    auto result = std::find (list.begin (), list.end (), to);
                    if (result == list.end ())
                    {
                        list.push_back (to);
                    }
                }

                it++;
            }
        }
        return list;
    }

    Ptr<const MmWaveMacQueueItem>
    MmWaveMacQueue::PeekItemByAddress (Mac48Address addr)
    {
        NS_LOG_FUNCTION (this);
        for (ConstIterator it = begin (); it != end (); )
        {
            if (!TtlExceeded (it))
            {
                if ((*it)->GetDestinationAddress () == addr)
                {
                    return DoPeek (it);
                }

                it++;
            }
        }
        NS_LOG_DEBUG ("The queue is empty");
        return 0;
    }

    Ptr<const MmWaveMacQueueItem>
    MmWaveMacQueue::PeekItemByChannel (MmWaveChannelNumberStandardPair channel)
    {
        NS_LOG_FUNCTION (this);
        for (ConstIterator it = begin (); it != end (); )
        {
            if (!TtlExceeded (it))
            {
                if ((*it)->GetChannel () == channel)
                {
                    return DoPeek (it);
                }

                it++;
            }
        }
        NS_LOG_DEBUG ("The queue is empty");
        return 0;
    }

    std::vector<Ptr<const MmWaveMacQueueItem>>
    MmWaveMacQueue::PeekFrontNItemByChannel (uint32_t max, MmWaveChannelNumberStandardPair channel)
    {
        NS_LOG_FUNCTION (this);
        uint32_t count = 0;
        std::vector<Ptr<const MmWaveMacQueueItem>> list;
        for (ConstIterator it = begin (); it != end (); )
        {
            if (count >= max)
            {
                return list;
            }
            if (!TtlExceeded (it))
            {
                if ((*it)->GetChannel () == channel && count < max)
                {
                    list.push_back (DoPeek (it));
                    count++;
                }
                it++;
            }
        }
        NS_LOG_DEBUG ("The queue is empty");
        return list;
    }

    MmWaveChannelNumberStandardPair
    MmWaveMacQueue::GetNeedToAccessExcludedChannel (MmWaveChannelNumberStandardPair c)
    {
        NS_LOG_FUNCTION (this);
        for (ConstIterator it = begin (); it != end (); )
        {
            if (!TtlExceeded (it))
            {
                if ((*it)->GetChannel () != c)
                {
                    return (*it)->GetChannel ();
                }
                it++;
            }
        }

        return c;
    }

    std::map<Mac48Address, std::vector<std::pair<MmWaveMacHeader, uint32_t>>>
    MmWaveMacQueue::GetBulkAccessRequestsInQueue (uint32_t num, MmWaveChannelNumberStandardPair channel)
    {
        std::map<Mac48Address, std::vector<std::pair<MmWaveMacHeader, uint32_t>>> items;
        Mac48Address to;
        uint32_t index = 0;
        for (ConstIterator it = begin (); it != end (); )
        {
            NS_ASSERT (index < num);
            if (!TtlExceeded (it))
            {
                if ((*it)->GetChannel () == channel)
                {
                    index++;
                    to = (*it)->GetHeader ().GetAddr1 ();
                    items[to].push_back (std::make_pair ((*it)->GetHeader (), (*it)->GetPacket ()->GetSize ()));
                    if (index >= num)
                    {
                        return items;
                    }
                }
                it++;
            }
        }

        return items;
    }
} //namespace ns3