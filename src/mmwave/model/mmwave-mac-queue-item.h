/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_MAC_QUEUE_ITEM_H
#define MMWAVE_MAC_QUEUE_ITEM_H
#include "ns3/nstime.h"
#include "ns3/mac48-address.h"
#include "mmwave-mac-header.h"

namespace ns3 {

    class Packet;
    
    class MmWaveMacQueueItem : public SimpleRefCount<MmWaveMacQueueItem>
    {
    public:
        MmWaveMacQueueItem (Ptr<const Packet> p, const MmWaveMacHeader & header);
        MmWaveMacQueueItem (Ptr<const Packet> p, const MmWaveMacHeader & header, Time tstamp);
        MmWaveMacQueueItem (Ptr<const Packet> p, const MmWaveMacHeader & header, MmWaveChannelNumberStandardPair channel);
        MmWaveMacQueueItem (Ptr<const Packet> p, const MmWaveMacHeader & header, MmWaveChannelNumberStandardPair channel, Time tstamp);
        static void AddMmWaveMacTrailer (Ptr<Packet> packet);

        virtual ~MmWaveMacQueueItem ();
        Ptr<const Packet> GetPacket () const;
        const MmWaveMacHeader & GetHeader () const;
        MmWaveMacHeader & GetHeader ();
        Mac48Address GetDestinationAddress () const;
        MmWaveChannelNumberStandardPair GetChannel () const;
        Time GetTimeStamp () const;
        uint32_t GetSize () const;
        Ptr<Packet> GetProtocolDataUnit () const;
        virtual void Print (std::ostream &os) const;

    private:
        Ptr<const Packet> m_packet;                   //!< The packet (MSDU or A-MSDU) contained in this queue item
        MmWaveMacHeader m_header;                       //!< Wifi MAC header associated with the packet
        MmWaveChannelNumberStandardPair m_channel;
        Time m_tstamp;                                //!< timestamp when the packet arrived at the queue
    };

    std::ostream& operator<< (std::ostream& os, const MmWaveMacQueueItem &item);

} //namespace ns3
#endif //MMWAVE_MAC_QUEUE_ITEM_H
