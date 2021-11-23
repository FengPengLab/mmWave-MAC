/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "mmwave-mac-header.h"

namespace ns3 {
    NS_LOG_COMPONENT_DEFINE ("MmWaveMacHeader");
    NS_OBJECT_ENSURE_REGISTERED (MmWaveMacHeader);
    
    MmWaveMacHeader::MmWaveMacHeader ()
            : m_ctrlMoreData (0),
              m_ctrlWep (0),
              m_ctrlOrder (0)
    {
    }

    MmWaveMacHeader::~MmWaveMacHeader ()
    {
    }

    void
    MmWaveMacHeader::SetDsFrom ()
    {
        m_ctrlFromDs = 1;
    }

    void
    MmWaveMacHeader::SetDsNotFrom ()
    {
        m_ctrlFromDs = 0;
    }

    void
    MmWaveMacHeader::SetDsTo ()
    {
        m_ctrlToDs = 1;
    }

    void
    MmWaveMacHeader::SetDsNotTo ()
    {
        m_ctrlToDs = 0;
    }

    void
    MmWaveMacHeader::SetAddr1 (Mac48Address address)
    {
        m_addr1 = address;
    }

    void
    MmWaveMacHeader::SetAddr2 (Mac48Address address)
    {
        m_addr2 = address;
    }

    void
    MmWaveMacHeader::SetAddr3 (Mac48Address address)
    {
        m_addr3 = address;
    }

    void
    MmWaveMacHeader::SetAddr4 (Mac48Address address)
    {
        m_addr4 = address;
    }

    void
    MmWaveMacHeader::SetType (MmWaveMacType type, bool resetToDsFromDs)
    {
        switch (type)
        {
            case MMWAVE_MAC_CTL_CTLWRAPPER:
                m_ctrlType = MMWAVE_TYPE_CTL;
                m_ctrlSubtype = MMWAVE_SUBTYPE_CTL_CTLWRAPPER;
                break;
            case MMWAVE_MAC_CTL_RTS:
                m_ctrlType = MMWAVE_TYPE_CTL;
                m_ctrlSubtype = MMWAVE_SUBTYPE_CTL_RTS;
                break;
            case MMWAVE_MAC_CTL_CTS:
                m_ctrlType = MMWAVE_TYPE_CTL;
                m_ctrlSubtype = MMWAVE_SUBTYPE_CTL_CTS;
                break;
            case MMWAVE_MAC_CTL_ACK:
                m_ctrlType = MMWAVE_TYPE_CTL;
                m_ctrlSubtype = MMWAVE_SUBTYPE_CTL_ACK;
                break;
            case MMWAVE_MAC_MGT_BEACON:
                m_ctrlType = MMWAVE_TYPE_MGT;
                m_ctrlSubtype = MMWAVE_SUBTYPE_MGT_BEACON;
                break;
            case MMWAVE_MAC_MGT_DETECTION:
                m_ctrlType = MMWAVE_TYPE_MGT;
                m_ctrlSubtype = MMWAVE_SUBTYPE_MGT_DETECTION;
                break;
            case MMWAVE_MAC_MGT_BULK_REQUEST:
                m_ctrlType = MMWAVE_TYPE_MGT;
                m_ctrlSubtype = MMWAVE_SUBTYPE_MGT_BULK_REQUEST;
                break;
            case MMWAVE_MAC_MGT_BULK_RESPONSE:
                m_ctrlType = MMWAVE_TYPE_MGT;
                m_ctrlSubtype = MMWAVE_SUBTYPE_MGT_BULK_RESPONSE;
                break;
            case MMWAVE_MAC_MGT_BULK_ACK:
                m_ctrlType = MMWAVE_TYPE_MGT;
                m_ctrlSubtype = MMWAVE_SUBTYPE_MGT_BULK_ACK;
                break;
            case MMWAVE_MAC_DATA:
                m_ctrlType = MMWAVE_TYPE_DATA;
                m_ctrlSubtype = 0;
                break;
        }
        if (resetToDsFromDs)
        {
            m_ctrlToDs = 0;
            m_ctrlFromDs = 0;
        }
    }

    void
    MmWaveMacHeader::SetRawDuration (uint16_t duration)
    {
        NS_ASSERT (duration <= 32768);
        m_duration = duration;
    }

    void
    MmWaveMacHeader::SetDuration (Time duration)
    {
        int64_t duration_us = static_cast<int64_t> (ceil (static_cast<double> (duration.GetNanoSeconds ()) / 1000));
        NS_ASSERT (duration_us >= 0 && duration_us <= 0x7fff);
        m_duration = static_cast<uint16_t> (duration_us);
    }

    void MmWaveMacHeader::SetId (uint16_t id)
    {
        m_duration = id;
    }

    void MmWaveMacHeader::SetSequenceNumber (uint16_t seq)
    {
        m_seqSeq = seq;
    }

    void MmWaveMacHeader::SetFragmentNumber (uint8_t frag)
    {
        m_seqFrag = frag;
    }

    void MmWaveMacHeader::SetNoMoreFragments ()
    {
        m_ctrlMoreFrag = 0;
    }

    void MmWaveMacHeader::SetMoreFragments ()
    {
        m_ctrlMoreFrag = 1;
    }

    void MmWaveMacHeader::SetOrder ()
    {
        m_ctrlOrder = 1;
    }

    void MmWaveMacHeader::SetNoOrder ()
    {
        m_ctrlOrder = 0;
    }

    void MmWaveMacHeader::SetRetry ()
    {
        m_ctrlRetry = 1;
    }

    void MmWaveMacHeader::SetNoRetry ()
    {
        m_ctrlRetry = 0;
    }

    Mac48Address
    MmWaveMacHeader::GetAddr1 () const
    {
        return m_addr1;
    }

    Mac48Address
    MmWaveMacHeader::GetAddr2 () const
    {
        return m_addr2;
    }

    Mac48Address
    MmWaveMacHeader::GetAddr3 () const
    {
        return m_addr3;
    }

    Mac48Address
    MmWaveMacHeader::GetAddr4 () const
    {
        return m_addr4;
    }

    MmWaveMacType
    MmWaveMacHeader::GetType () const
    {
        switch (m_ctrlType)
        {
            case MMWAVE_TYPE_MGT:
                switch (m_ctrlSubtype)
                {
                    case MMWAVE_SUBTYPE_MGT_BEACON:
                        return MMWAVE_MAC_MGT_BEACON;
                    case MMWAVE_SUBTYPE_MGT_DETECTION:
                        return MMWAVE_MAC_MGT_DETECTION;
                    case MMWAVE_SUBTYPE_MGT_BULK_REQUEST:
                        return MMWAVE_MAC_MGT_BULK_REQUEST;
                    case MMWAVE_SUBTYPE_MGT_BULK_RESPONSE:
                        return MMWAVE_MAC_MGT_BULK_RESPONSE;
                    case MMWAVE_SUBTYPE_MGT_BULK_ACK:
                        return MMWAVE_MAC_MGT_BULK_ACK;
                }
                break;
            case MMWAVE_TYPE_CTL:
                switch (m_ctrlSubtype)
                {
                    case MMWAVE_SUBTYPE_CTL_CTLWRAPPER:
                        return MMWAVE_MAC_CTL_CTLWRAPPER;
                    case MMWAVE_SUBTYPE_CTL_RTS:
                        return MMWAVE_MAC_CTL_RTS;
                    case MMWAVE_SUBTYPE_CTL_CTS:
                        return MMWAVE_MAC_CTL_CTS;
                    case MMWAVE_SUBTYPE_CTL_ACK:
                        return MMWAVE_MAC_CTL_ACK;
                }
                break;
            case MMWAVE_TYPE_DATA:
                switch (m_ctrlSubtype)
                {
                    case 0:
                        return MMWAVE_MAC_DATA;
                }
                break;
        }
        // NOTREACHED
        NS_ASSERT (false);
        return (MmWaveMacType) - 1;
    }

    bool
    MmWaveMacHeader::IsFromDs () const
    {
        return m_ctrlFromDs == 1;
    }

    bool
    MmWaveMacHeader::IsToDs () const
    {
        return m_ctrlToDs == 1;
    }

    bool
    MmWaveMacHeader::IsData () const
    {
        return (m_ctrlType == MMWAVE_TYPE_DATA);

    }

    bool
    MmWaveMacHeader::IsCtl () const
    {
        return (m_ctrlType == MMWAVE_TYPE_CTL);
    }

    bool
    MmWaveMacHeader::IsMgt () const
    {
        return (m_ctrlType == MMWAVE_TYPE_MGT);
    }

    bool
    MmWaveMacHeader::HasData () const
    {
        switch (GetType ())
        {
            case MMWAVE_MAC_DATA:
                return true;
            default:
                return false;
        }
    }

    bool
    MmWaveMacHeader::IsRts () const
    {
        return (GetType () == MMWAVE_MAC_CTL_RTS);
    }

    bool
    MmWaveMacHeader::IsCts () const
    {
        return (GetType () == MMWAVE_MAC_CTL_CTS);
    }

    bool
    MmWaveMacHeader::IsAck () const
    {
        return (GetType () == MMWAVE_MAC_CTL_ACK);
    }

    bool
    MmWaveMacHeader::IsBeacon () const
    {
        return (GetType () == MMWAVE_MAC_MGT_BEACON);
    }

    bool
    MmWaveMacHeader::IsDetectRequest () const
    {
        return (GetType () == MMWAVE_MAC_MGT_DETECTION);
    }

    bool
    MmWaveMacHeader::IsBulkRequest () const
    {
        return (GetType () == MMWAVE_MAC_MGT_BULK_REQUEST);
    }

    bool
    MmWaveMacHeader::IsBulkResponse () const
    {
        return (GetType () == MMWAVE_MAC_MGT_BULK_RESPONSE);
    }

    bool
    MmWaveMacHeader::IsBulkAck () const
    {
        return (GetType () == MMWAVE_MAC_MGT_BULK_ACK);
    }

    uint16_t
    MmWaveMacHeader::GetRawDuration () const
    {
        return m_duration;
    }

    Time
    MmWaveMacHeader::GetDuration () const
    {
        return MicroSeconds (m_duration);
    }

    uint16_t
    MmWaveMacHeader::GetSequenceControl () const
    {
        return (m_seqSeq << 4) | m_seqFrag;
    }

    uint16_t
    MmWaveMacHeader::GetSequenceNumber () const
    {
        return m_seqSeq;
    }

    uint8_t
    MmWaveMacHeader::GetFragmentNumber () const
    {
        return m_seqFrag;
    }

    bool
    MmWaveMacHeader::IsRetry () const
    {
        return (m_ctrlRetry == 1);
    }

    bool
    MmWaveMacHeader::IsMoreFragments () const
    {
        return (m_ctrlMoreFrag == 1);
    }


    uint16_t
    MmWaveMacHeader::GetFrameControl () const
    {
        uint16_t val = 0;
        val |= (m_ctrlType << 2) & (0x3 << 2);
        val |= (m_ctrlSubtype << 4) & (0xf << 4);
        val |= (m_ctrlToDs << 8) & (0x1 << 8);
        val |= (m_ctrlFromDs << 9) & (0x1 << 9);
        val |= (m_ctrlMoreFrag << 10) & (0x1 << 10);
        val |= (m_ctrlRetry << 11) & (0x1 << 11);
        val |= (m_ctrlMoreData << 13) & (0x1 << 13);
        val |= (m_ctrlWep << 14) & (0x1 << 14);
        val |= (m_ctrlOrder << 15) & (0x1 << 15);
        return val;
    }

    void
    MmWaveMacHeader::SetFrameControl (uint16_t ctrl)
    {
        m_ctrlType = (ctrl >> 2) & 0x03;
        m_ctrlSubtype = (ctrl >> 4) & 0x0f;
        m_ctrlToDs = (ctrl >> 8) & 0x01;
        m_ctrlFromDs = (ctrl >> 9) & 0x01;
        m_ctrlMoreFrag = (ctrl >> 10) & 0x01;
        m_ctrlRetry = (ctrl >> 11) & 0x01;
        m_ctrlMoreData = (ctrl >> 13) & 0x01;
        m_ctrlWep = (ctrl >> 14) & 0x01;
        m_ctrlOrder = (ctrl >> 15) & 0x01;
    }
    void
    MmWaveMacHeader::SetSequenceControl (uint16_t seq)
    {
        m_seqFrag = seq & 0x0f;
        m_seqSeq = (seq >> 4) & 0x0fff;
    }

    uint32_t
    MmWaveMacHeader::GetSize () const
    {
        uint32_t size = 0;
        switch (m_ctrlType)
        {
            case MMWAVE_TYPE_MGT:
                switch (m_ctrlSubtype)
                {
                    case MMWAVE_SUBTYPE_MGT_BEACON:
                    case MMWAVE_SUBTYPE_MGT_DETECTION:
                    case MMWAVE_SUBTYPE_MGT_BULK_REQUEST:
                    case MMWAVE_SUBTYPE_MGT_BULK_RESPONSE:
                    case MMWAVE_SUBTYPE_MGT_BULK_ACK:
                        size = 2 + 2 + 6 + 6;
                        break;
                }
                break;
            case MMWAVE_TYPE_CTL:
                switch (m_ctrlSubtype)
                {
                    case MMWAVE_SUBTYPE_CTL_RTS:
                        size = 2 + 2 + 6 + 6;
                        break;
                    case MMWAVE_SUBTYPE_CTL_CTS:
                    case MMWAVE_SUBTYPE_CTL_ACK:
                        size = 2 + 2 + 6;
                        break;
                    case MMWAVE_SUBTYPE_CTL_CTLWRAPPER:
                        size = 2 + 2 + 6 + 2 + 4;
                        break;
                }
                break;
            case MMWAVE_TYPE_DATA:
                size = 2 + 2 + 6 + 6 + 6 + 2;
                if (m_ctrlToDs && m_ctrlFromDs)
                {
                    size += 6;
                }
                break;
        }
        return size;
    }

    const char *
    MmWaveMacHeader::GetTypeString () const
    {
#define FOO(x) \
case MMWAVE_MAC_ ## x: \
  return # x; \
  break;
        switch (GetType ())
        {
            FOO (CTL_RTS);
            FOO (CTL_CTS);
            FOO (CTL_ACK);
            FOO (MGT_BEACON);
            FOO (MGT_DETECTION);
            FOO (MGT_BULK_REQUEST);
            FOO (MGT_BULK_RESPONSE);
            FOO (MGT_BULK_ACK);
            FOO (DATA);
            default:
                return "ERROR";
        }
#undef FOO
#ifndef _WIN32
        return "BIG_ERROR";
#endif
    }

    TypeId
    MmWaveMacHeader::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::MmWaveMacHeader")
                .SetParent<Header> ()
                .SetGroupName ("MmWave")
                .AddConstructor<MmWaveMacHeader> ()
        ;
        return tid;
    }

    TypeId
    MmWaveMacHeader::GetInstanceTypeId () const
    {
        return GetTypeId ();
    }

    void
    MmWaveMacHeader::PrintFrameControl (std::ostream &os) const
    {
        os << "ToDS=" << std::hex << (int) m_ctrlToDs << ", FromDS=" << std::hex << (int) m_ctrlFromDs
           << ", MoreFrag=" <<  std::hex << (int) m_ctrlMoreFrag << ", Retry=" << std::hex << (int) m_ctrlRetry
           << ", MoreData=" <<  std::hex << (int) m_ctrlMoreData << std::dec;
    }

    void
    MmWaveMacHeader::Print (std::ostream &os) const
    {
        os << GetTypeString () << " ";
        switch (GetType ())
        {
            case MMWAVE_MAC_CTL_RTS:
                os << "Duration=" << m_duration << "us"
                   << ", RA=" << m_addr1 << ", TA=" << m_addr2;
                break;
            case MMWAVE_MAC_CTL_CTS:
            case MMWAVE_MAC_CTL_ACK:
                os << "Duration=" << m_duration << "us"
                   << ", RA=" << m_addr1;
                break;
            case MMWAVE_MAC_MGT_BEACON:
            case MMWAVE_MAC_MGT_DETECTION:
            case MMWAVE_MAC_MGT_BULK_REQUEST:
            case MMWAVE_MAC_MGT_BULK_RESPONSE:
            case MMWAVE_MAC_MGT_BULK_ACK:
                os << "Duration=" << m_duration << "us"
                   << ", DA=" << m_addr1 << ", SA=" << m_addr2
                   << std::dec;
                break;
            case MMWAVE_MAC_DATA:
                PrintFrameControl (os);
                os << "Duration/ID=" << m_duration << "us";
                if (!m_ctrlToDs && !m_ctrlFromDs)
                {
                    os << ", DA=" << m_addr1 << ", SA=" << m_addr2 << ", ID=" << m_addr3;
                }
                else if (!m_ctrlToDs && m_ctrlFromDs)
                {
                    os << ", DA=" << m_addr1 << ", SA=" << m_addr3 << ", ID=" << m_addr2;
                }
                else if (m_ctrlToDs && !m_ctrlFromDs)
                {
                    os << ", DA=" << m_addr3 << ", SA=" << m_addr2 << ", ID=" << m_addr1;
                }
                else if (m_ctrlToDs && m_ctrlFromDs)
                {
                    os << ", DA=" << m_addr3 << ", SA=" << m_addr4 << ", RA=" << m_addr1 << ", TA=" << m_addr2;
                }
                else
                {
                    NS_FATAL_ERROR ("Impossible ToDs and FromDs flags combination");
                }
                os << ", FragNumber=" << std::hex << (int) m_seqFrag << std::dec
                   << ", SeqNumber=" << m_seqSeq;
                break;
            case MMWAVE_MAC_CTL_CTLWRAPPER:
            default:
                break;
        }
    }

    uint32_t
    MmWaveMacHeader::GetSerializedSize () const
    {
        return GetSize ();
    }

    void
    MmWaveMacHeader::Serialize (Buffer::Iterator i) const
    {
        i.WriteHtolsbU16 (GetFrameControl ());
        i.WriteHtolsbU16 (m_duration);
        WriteTo (i, m_addr1);
        switch (m_ctrlType)
        {
            case MMWAVE_TYPE_MGT:
                switch (m_ctrlSubtype)
                {
                    case MMWAVE_SUBTYPE_MGT_BEACON:
                    case MMWAVE_SUBTYPE_MGT_DETECTION:
                    case MMWAVE_SUBTYPE_MGT_BULK_REQUEST:
                    case MMWAVE_SUBTYPE_MGT_BULK_RESPONSE:
                    case MMWAVE_SUBTYPE_MGT_BULK_ACK:
                        WriteTo (i, m_addr2);
                        break;
                }
                break;
            case MMWAVE_TYPE_CTL:
                switch (m_ctrlSubtype)
                {
                    case MMWAVE_SUBTYPE_CTL_RTS:
                        WriteTo (i, m_addr2);
                        break;
                    case MMWAVE_SUBTYPE_CTL_CTS:
                    case MMWAVE_SUBTYPE_CTL_ACK:
                        break;
                    default:
                        //NOTREACHED
                        NS_ASSERT (false);
                        break;
                }
                break;
            case MMWAVE_TYPE_DATA:
                WriteTo (i, m_addr2);
                WriteTo (i, m_addr3);
                i.WriteHtolsbU16 (GetSequenceControl ());
                if (m_ctrlToDs && m_ctrlFromDs)
                {
                    WriteTo (i, m_addr4);
                }
                break;
            default:
                //NOTREACHED
                NS_ASSERT (false);
                break;
        }
    }

    uint32_t
    MmWaveMacHeader::Deserialize (Buffer::Iterator start)
    {
        Buffer::Iterator i = start;
        uint16_t frame_control = i.ReadLsbtohU16 ();
        SetFrameControl (frame_control);
        m_duration = i.ReadLsbtohU16 ();
        ReadFrom (i, m_addr1);
        switch (m_ctrlType)
        {
            case MMWAVE_TYPE_MGT:
                switch (m_ctrlSubtype)
                {
                    case MMWAVE_SUBTYPE_MGT_BEACON:
                    case MMWAVE_SUBTYPE_MGT_DETECTION:
                    case MMWAVE_SUBTYPE_MGT_BULK_REQUEST:
                    case MMWAVE_SUBTYPE_MGT_BULK_RESPONSE:
                    case MMWAVE_SUBTYPE_MGT_BULK_ACK:
                        ReadFrom (i, m_addr2);
                        break;
                }
                break;
            case MMWAVE_TYPE_CTL:
                switch (m_ctrlSubtype)
                {
                    case MMWAVE_SUBTYPE_CTL_RTS:
                        ReadFrom (i, m_addr2);
                        break;
                    case MMWAVE_SUBTYPE_CTL_CTS:
                    case MMWAVE_SUBTYPE_CTL_ACK:
                        break;
                }
                break;
            case MMWAVE_TYPE_DATA:
                ReadFrom (i, m_addr2);
                ReadFrom (i, m_addr3);
                SetSequenceControl (i.ReadLsbtohU16 ());
                if (m_ctrlToDs && m_ctrlFromDs)
                {
                    ReadFrom (i, m_addr4);
                }
                break;
        }
        return i.GetDistanceFrom (start);
    }

    NS_OBJECT_ENSURE_REGISTERED (BeaconHeader);

    BeaconHeader::BeaconHeader ()
    {
    }

    BeaconHeader::~BeaconHeader ()
    {
    }

    TypeId
    BeaconHeader::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::BeaconHeader")
                .SetParent<Header> ()
                .SetGroupName ("MmWave")
                .AddConstructor<BeaconHeader> ();
        return tid;
    }

    TypeId
    BeaconHeader::GetInstanceTypeId () const
    {
        return GetTypeId ();
    }

    uint32_t
    BeaconHeader::GetSerializedSize () const
    {
        uint32_t size = 0;
        size += 1;
        return size;
    }

    void
    BeaconHeader::Print (std::ostream &os) const
    {
        os << "ChannelNumber =" << +m_num;
    }

    void
    BeaconHeader::Serialize (Buffer::Iterator start) const
    {
        Buffer::Iterator i = start;
        i.WriteU8 (m_num);
    }

    uint32_t
    BeaconHeader::Deserialize (Buffer::Iterator start)
    {
        Buffer::Iterator i = start;
        m_num = i.ReadU8 ();
        return i.GetDistanceFrom (start);
    }

    void
    BeaconHeader::SetChannelNumber (uint8_t number)
    {
        m_num = number;
    }

    uint8_t
    BeaconHeader::GetChannelNumber () const
    {
        return m_num;
    }

    NS_OBJECT_ENSURE_REGISTERED (BulkRequestHeader);

    BulkRequestHeader::BulkRequestHeader ()
    {
    }

    BulkRequestHeader::~BulkRequestHeader ()
    {
    }

    TypeId
    BulkRequestHeader::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::BulkRequestHeader")
                .SetParent<Header> ()
                .SetGroupName ("MmWave")
                .AddConstructor<BulkRequestHeader> ();
        return tid;
    }

    TypeId
    BulkRequestHeader::GetInstanceTypeId () const
    {
        return GetTypeId ();
    }

    uint32_t
    BulkRequestHeader::GetSerializedSize () const
    {
        uint32_t size = 0;
        size += 2;
        size += 2;
        size += 1;
        return size;
    }

    void
    BulkRequestHeader::Print (std::ostream &os) const
    {
        os << "TxDuration =" << +m_txDuration << "us, "
           << " StartingSequenceControl =" << +m_startingSeq << ", "
           << " num =" << +m_num;
    }

    void
    BulkRequestHeader::Serialize (Buffer::Iterator start) const
    {
        Buffer::Iterator i = start;
        i.WriteHtolsbU16 (m_txDuration);
        i.WriteHtolsbU16 (m_startingSeq);
        i.WriteU8 (m_num);
    }

    uint32_t
    BulkRequestHeader::Deserialize (Buffer::Iterator start)
    {
        Buffer::Iterator i = start;
        m_txDuration = i.ReadLsbtohU16 ();
        m_startingSeq = i.ReadLsbtohU16 ();
        m_num = i.ReadU8 ();
        return i.GetDistanceFrom (start);
    }

    void
    BulkRequestHeader::SetTxDuration (Time txDuration)
    {
        int64_t txDuration_us = static_cast<int64_t> (ceil (static_cast<double> (txDuration.GetNanoSeconds ()) / 1000));
        NS_ASSERT (txDuration_us >= 0 && txDuration_us <= 0x7fff);
        m_txDuration = static_cast<uint16_t> (txDuration_us);
    }

    Time
    BulkRequestHeader::GetTxDuration () const
    {
        return MicroSeconds (m_txDuration);
    }

    void
    BulkRequestHeader::SetStartingSequenceControl (uint16_t seqControl)
    {
        m_startingSeq = seqControl;
    }

    uint16_t
    BulkRequestHeader::GetStartingSequenceControl () const
    {
        return m_startingSeq;
    }

    void
    BulkRequestHeader::SetNum (uint8_t num)
    {
        m_num = num;
    }

    uint8_t
    BulkRequestHeader::GetNum () const
    {
        return m_num;
    }

    NS_OBJECT_ENSURE_REGISTERED (BulkResponseHeader);

    BulkResponseHeader::BulkResponseHeader ()
    {
    }

    BulkResponseHeader::~BulkResponseHeader ()
    {
    }

    TypeId
    BulkResponseHeader::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::BulkResponseHeader")
                .SetParent<Header> ()
                .SetGroupName ("MmWave")
                .AddConstructor<BulkResponseHeader> ();
        return tid;
    }

    TypeId
    BulkResponseHeader::GetInstanceTypeId () const
    {
        return GetTypeId ();
    }

    uint32_t
    BulkResponseHeader::GetSerializedSize () const
    {
        uint32_t size = 0;
        size += 2;
        return size;
    }

    void
    BulkResponseHeader::Print (std::ostream &os) const
    {
        os << "TxDuration =" << +m_txDuration << "us";
    }

    void
    BulkResponseHeader::Serialize (Buffer::Iterator start) const
    {
        Buffer::Iterator i = start;
        i.WriteHtolsbU16 (m_txDuration);
    }

    uint32_t
    BulkResponseHeader::Deserialize (Buffer::Iterator start)
    {
        Buffer::Iterator i = start;
        m_txDuration = i.ReadLsbtohU16 ();
        return i.GetDistanceFrom (start);
    }

    void
    BulkResponseHeader::SetTxDuration (Time txDuration)
    {
        int64_t txDuration_us = static_cast<int64_t> (ceil (static_cast<double> (txDuration.GetNanoSeconds ()) / 1000));
        NS_ASSERT (txDuration_us >= 0 && txDuration_us <= 0x7fff);
        m_txDuration = static_cast<uint16_t> (txDuration_us);
    }

    Time
    BulkResponseHeader::GetTxDuration () const
    {
        return MicroSeconds (m_txDuration);
    }

    NS_OBJECT_ENSURE_REGISTERED (BulkAckHeader);

    BulkAckHeader::BulkAckHeader ()
    {
    }

    BulkAckHeader::~BulkAckHeader ()
    {
    }

    TypeId
    BulkAckHeader::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::BulkAckHeader")
                .SetParent<Header> ()
                .SetGroupName ("MmWave")
                .AddConstructor<BulkAckHeader> ();
        return tid;
    }

    TypeId
    BulkAckHeader::GetInstanceTypeId () const
    {
        return GetTypeId ();
    }

    uint32_t
    BulkAckHeader::GetSerializedSize () const
    {
        uint32_t size = 0;
        size += 2;
        size += 8;
        return size;
    }

    void
    BulkAckHeader::Print (std::ostream &os) const
    {
        os << "StartingSequenceControl =" << +m_startingSeq;
    }

    void
    BulkAckHeader::Serialize (Buffer::Iterator start) const
    {
        Buffer::Iterator i = start;
        i.WriteHtolsbU16 (m_startingSeq);
        i.WriteHtolsbU64 (m_bitmap);
    }

    uint32_t
    BulkAckHeader::Deserialize (Buffer::Iterator start)
    {
        Buffer::Iterator i = start;
        m_startingSeq = i.ReadLsbtohU16 ();
        m_bitmap = i.ReadLsbtohU64 ();
        return i.GetDistanceFrom (start);
    }

    void
    BulkAckHeader::SetStartingSequenceControl (uint16_t seqControl)
    {
        m_startingSeq = seqControl;
    }

    uint16_t
    BulkAckHeader::GetStartingSequenceControl () const
    {
        return m_startingSeq;
    }

    void
    BulkAckHeader::SetBitmap (uint64_t bitmap)
    {
        m_bitmap = bitmap;
    }

    uint64_t
    BulkAckHeader::GetBitmap () const
    {
        return m_bitmap;
    }

} //namespace ns3