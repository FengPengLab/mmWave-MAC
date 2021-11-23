/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/log.h"
#include "ns3/sequence-number.h"
#include "ns3/packet.h"
#include "mmwave-mac-rx-middle.h"
#include "mmwave-mac-queue-item.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("MmWaveMacRxMiddle");

    class MmWaveOriginatorRxStatus
    {
    private:
        typedef std::list<Ptr<const Packet> > Fragments;
        typedef std::list<Ptr<const Packet> >::const_iterator FragmentsCI;

        bool m_defragmenting; ///< flag to indicate whether we are defragmenting
        uint16_t m_lastSequenceControl; ///< last sequence control
        Fragments m_fragments; ///< fragments
        
    public:
        MmWaveOriginatorRxStatus ()
        {
            m_lastSequenceControl = 0xffff;
            m_defragmenting = false;
        }
        ~MmWaveOriginatorRxStatus ()
        {
            m_fragments.clear ();
        }

        bool IsDeFragmenting () const
        {
            return m_defragmenting;
        }
        
        void AccumulateFirstFragment (Ptr<const Packet> packet)
        {
            NS_ASSERT (!m_defragmenting);
            m_defragmenting = true;
            m_fragments.push_back (packet);
        }

        Ptr<Packet> AccumulateLastFragment (Ptr<const Packet> packet)
        {
            NS_ASSERT (m_defragmenting);
            m_fragments.push_back (packet);
            m_defragmenting = false;
            Ptr<Packet> full = Create<Packet> ();
            for (FragmentsCI i = m_fragments.begin (); i != m_fragments.end (); i++)
            {
                full->AddAtEnd (*i);
            }
            m_fragments.erase (m_fragments.begin (), m_fragments.end ());
            return full;
        }
       
        void AccumulateFragment (Ptr<const Packet> packet)
        {
            NS_ASSERT (m_defragmenting);
            m_fragments.push_back (packet);
        }

        bool IsNextFragment (uint16_t sequenceControl) const
        {
            if ((sequenceControl >> 4) == (m_lastSequenceControl >> 4)
                && (sequenceControl & 0x0f) == ((m_lastSequenceControl & 0x0f) + 1))
            {
                return true;
            }
            else
            {
                return false;
            }
        }
  
        uint16_t GetLastSequenceControl () const
        {
            return m_lastSequenceControl;
        }
  
        void SetSequenceControl (uint16_t sequenceControl)
        {
            m_lastSequenceControl = sequenceControl;
        }
    };


    MmWaveMacRxMiddle::MmWaveMacRxMiddle ()
    {
        NS_LOG_FUNCTION_NOARGS ();
    }

    MmWaveMacRxMiddle::~MmWaveMacRxMiddle ()
    {
        NS_LOG_FUNCTION_NOARGS ();
        for (OriginatorsI i = m_originatorStatus.begin (); i != m_originatorStatus.end (); i++)
        {
            delete (*i).second;
        }
        m_originatorStatus.erase (m_originatorStatus.begin (), m_originatorStatus.end ());
    }

    void
    MmWaveMacRxMiddle::SetForwardCallback (ForwardUpCallback callback)
    {
        NS_LOG_FUNCTION_NOARGS ();
        m_callback = callback;
    }

    MmWaveOriginatorRxStatus *
    MmWaveMacRxMiddle::Lookup (const MmWaveMacHeader *hdr)
    {
        NS_LOG_FUNCTION (hdr);
        MmWaveOriginatorRxStatus *originator;
        Mac48Address source = hdr->GetAddr2 ();
        originator = m_originatorStatus[source];
        if (originator == 0)
        {
            originator = new MmWaveOriginatorRxStatus ();
            m_originatorStatus[source] = originator;
        }
        return originator;
    }

    bool
    MmWaveMacRxMiddle::IsDuplicate (const MmWaveMacHeader* hdr, MmWaveOriginatorRxStatus *originator) const
    {
        NS_LOG_FUNCTION (hdr << originator);
        if (hdr->IsRetry () && originator->GetLastSequenceControl () == hdr->GetSequenceControl ())
        {
            return true;
        }
        return false;
    }

    Ptr<const Packet>
    MmWaveMacRxMiddle::HandleFragments (Ptr<const Packet> packet, const MmWaveMacHeader *hdr, MmWaveOriginatorRxStatus *originator)
    {
        NS_LOG_FUNCTION (packet << hdr << originator);
        if (originator->IsDeFragmenting ())
        {
            if (hdr->IsMoreFragments ())
            {
                if (originator->IsNextFragment (hdr->GetSequenceControl ()))
                {
                    NS_LOG_DEBUG ("accumulate fragment seq=" << hdr->GetSequenceNumber () <<
                                                             ", frag=" << +hdr->GetFragmentNumber () <<
                                                             ", size=" << packet->GetSize ());
                    originator->AccumulateFragment (packet);
                    originator->SetSequenceControl (hdr->GetSequenceControl ());
                }
                else
                {
                    NS_LOG_DEBUG ("non-ordered fragment");
                }
                return 0;
            }
            else
            {
                if (originator->IsNextFragment (hdr->GetSequenceControl ()))
                {
                    NS_LOG_DEBUG ("accumulate last fragment seq=" << hdr->GetSequenceNumber () <<
                                                                  ", frag=" << +hdr->GetFragmentNumber () <<
                                                                  ", size=" << hdr->GetSize ());
                    Ptr<Packet> p = originator->AccumulateLastFragment (packet);
                    originator->SetSequenceControl (hdr->GetSequenceControl ());
                    return p;
                }
                else
                {
                    NS_LOG_DEBUG ("non-ordered fragment");
                    return 0;
                }
            }
        }
        else
        {
            if (hdr->IsMoreFragments ())
            {
                NS_LOG_DEBUG ("accumulate first fragment seq=" << hdr->GetSequenceNumber () <<
                                                               ", frag=" << +hdr->GetFragmentNumber () <<
                                                               ", size=" << packet->GetSize ());
                originator->AccumulateFirstFragment (packet);
                originator->SetSequenceControl (hdr->GetSequenceControl ());
                return 0;
            }
            else
            {
                return packet;
            }
        }
    }

    void
    MmWaveMacRxMiddle::Receive (Ptr<MmWaveMacQueueItem> mpdu)
    {
        NS_LOG_FUNCTION (*mpdu);
        const MmWaveMacHeader* hdr = &mpdu->GetHeader ();
        NS_ASSERT (hdr->IsData () || hdr->IsMgt ());
        MmWaveOriginatorRxStatus *originator = Lookup (hdr);

        if (!(SequenceNumber16 (originator->GetLastSequenceControl ()) < SequenceNumber16 (hdr->GetSequenceControl ())))
        {
            NS_LOG_DEBUG ("Sequence numbers have looped back. last recorded=" << originator->GetLastSequenceControl () <<
                                                                              " currently seen=" << hdr->GetSequenceControl ());
        }
        //filter duplicates.
        if (IsDuplicate (hdr, originator))
        {
            NS_LOG_DEBUG ("duplicate from=" << hdr->GetAddr2 () <<
                                            ", seq=" << hdr->GetSequenceNumber () <<
                                            ", frag=" << +hdr->GetFragmentNumber ());
            return;
        }
        Ptr<const Packet> aggregate = HandleFragments (mpdu->GetPacket (), hdr, originator);
        if (aggregate == 0)
        {
            return;
        }
        NS_LOG_DEBUG ("forwarding data from=" << hdr->GetAddr2 () <<
                                              ", seq=" << hdr->GetSequenceNumber () <<
                                              ", frag=" << +hdr->GetFragmentNumber ());
        if (!hdr->GetAddr1 ().IsGroup ())
        {
            originator->SetSequenceControl (hdr->GetSequenceControl ());
        }
        if (aggregate == mpdu->GetPacket ())
        {
            m_callback (mpdu);
        }
        else
        {
            m_callback (Create<MmWaveMacQueueItem> (aggregate, *hdr));
        }
    }

} //namespace ns3
