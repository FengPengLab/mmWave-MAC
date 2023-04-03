/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/packet.h"
#include "ns3/log.h"
#include "mmwave.h"
#include "mmwave-psdu.h"
#include "mmwave-mac-trailer.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("MmWavePsdu");

    MmWavePsdu::MmWavePsdu (Ptr<const Packet> p, const MmWaveMacHeader & header)
    {
        m_mpdu = Create<MmWaveMacQueueItem> (p, header);
        m_size = header.GetSerializedSize () + p->GetSize () + MMWAVE_MAC_FCS_LENGTH;
    }

    MmWavePsdu::~MmWavePsdu ()
    {
    }

    Ptr<const Packet>
    MmWavePsdu::GetPacket () const
    {
        Ptr<Packet> packet = Create<Packet> ();
        if (m_mpdu != 0)
        {
            packet = m_mpdu->GetPacket ()->Copy ();
            packet->AddHeader (m_mpdu->GetHeader ());
            AddMmWaveMacTrailer (packet);
        }
        else 
        {
            NS_ABORT_MSG ("MPDUs size should be 1.");
        }
        return packet;
    }

    Mac48Address
    MmWavePsdu::GetAddr1 () const
    {
        Mac48Address ra = m_mpdu->GetHeader ().GetAddr1 ();
        return ra;
    }

    Mac48Address
    MmWavePsdu::GetAddr2 () const
    {
        Mac48Address ta = m_mpdu->GetHeader ().GetAddr2 ();
        return ta;
    }

    Time
    MmWavePsdu::GetDuration () const
    {
        Time duration = m_mpdu->GetHeader ().GetDuration ();
        return duration;
    }

    void
    MmWavePsdu::SetDuration (Time duration)
    {
        NS_LOG_FUNCTION (this << duration);
        m_mpdu->GetHeader ().SetDuration (duration);
    }

    uint32_t
    MmWavePsdu::GetSize () const
    {
        return m_size;
    }

    const MmWaveMacHeader &
    MmWavePsdu::GetHeader () const
    {
        return m_mpdu->GetHeader ();
    }

    MmWaveMacHeader &
    MmWavePsdu::GetHeader ()
    {
        return m_mpdu->GetHeader ();
    }

    Ptr<const Packet>
    MmWavePsdu::GetPayload () const
    {
        return m_mpdu->GetPacket ();
    }

    Time
    MmWavePsdu::GetTimeStamp () const
    {
        return m_mpdu->GetTimeStamp ();
    }

    Ptr<MmWaveMacQueueItem>
    MmWavePsdu::GetMpdu () const
    {
        return m_mpdu;
    }

    void
    MmWavePsdu::AddMmWaveMacTrailer (Ptr<Packet> packet) const
    {
        MmWaveMacTrailer fcs;
        packet->AddTrailer (fcs);
    }
    
    void
    MmWavePsdu::Print (std::ostream& os) const
    {
        os << "size=" << m_size
           << ", " << "normal MPDU"
           << " (" << *(m_mpdu) << ")";
    }

    std::ostream & operator << (std::ostream &os, const MmWavePsdu &psdu)
    {
        psdu.Print (os);
        return os;
    }

} //namespace ns3