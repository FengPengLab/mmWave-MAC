/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include "mmwave-mac-queue-item.h"
#include "mmwave-mac-trailer.h"
#include "mmwave.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("MmWaveMacQueueItem");

    MmWaveMacQueueItem::MmWaveMacQueueItem (Ptr<const Packet> p, const MmWaveMacHeader & header)
            : MmWaveMacQueueItem (p, header, Simulator::Now ())
    {
    }

    MmWaveMacQueueItem::MmWaveMacQueueItem (Ptr<const Packet> p, const MmWaveMacHeader & header, Time tstamp)
            : m_packet (p),
              m_header (header),
              m_channel ({{0, MMWAVE_PHY_BAND_UNSPECIFIED}, MMWAVE_PHY_STANDARD_UNSPECIFIED}),
              m_tstamp (tstamp)
    {
    }

    MmWaveMacQueueItem::MmWaveMacQueueItem (Ptr<const Packet> p, const MmWaveMacHeader & header, MmWaveChannelNumberStandardPair channel)
            : MmWaveMacQueueItem (p, header, channel, Simulator::Now ())
    {
    }

    MmWaveMacQueueItem::MmWaveMacQueueItem (Ptr<const Packet> p, const MmWaveMacHeader & header, MmWaveChannelNumberStandardPair channel, Time tstamp)
            : m_packet (p),
              m_header (header),
              m_channel (channel),
              m_tstamp (tstamp)
    {
    }

    MmWaveMacQueueItem::~MmWaveMacQueueItem ()
    {
    }

    Ptr<const Packet>
    MmWaveMacQueueItem::GetPacket () const
    {
        return m_packet;
    }

    const MmWaveMacHeader&
    MmWaveMacQueueItem::GetHeader () const
    {
        return m_header;
    }

    MmWaveMacHeader&
    MmWaveMacQueueItem::GetHeader ()
    {
        return m_header;
    }

    Mac48Address
    MmWaveMacQueueItem::GetDestinationAddress () const
    {
        return m_header.GetAddr1 ();
    }

    Time
    MmWaveMacQueueItem::GetTimeStamp () const
    {
        return m_tstamp;
    }

    MmWaveChannelNumberStandardPair
    MmWaveMacQueueItem::GetChannel () const
    {
        return m_channel;
    }

    uint32_t
    MmWaveMacQueueItem::GetSize () const
    {
        return m_packet->GetSize () + m_header.GetSerializedSize () + MMWAVE_MAC_FCS_LENGTH;
    }

    Ptr<Packet>
    MmWaveMacQueueItem::GetProtocolDataUnit () const
    {
        Ptr<Packet> mpdu = m_packet->Copy ();
        mpdu->AddHeader (m_header);
        AddMmWaveMacTrailer (mpdu);
        return mpdu;
    }

    void
    MmWaveMacQueueItem::AddMmWaveMacTrailer (Ptr<Packet> packet)
    {
        MmWaveMacTrailer fcs;
        packet->AddTrailer (fcs);
    }
    
    void
    MmWaveMacQueueItem::Print (std::ostream& os) const
    {
        os << "size=" << m_packet->GetSize ()
           << ", to=" << m_header.GetAddr1 ()
           << ", seqN=" << m_header.GetSequenceNumber ()
           << ", lifetime=" << (Simulator::Now () - m_tstamp).As (Time::US);
    }

    std::ostream & operator << (std::ostream &os, const MmWaveMacQueueItem &item)
    {
        item.Print (os);
        return os;
    }

} //namespace ns3