/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/object.h"
#include "mmwave-phy-header.h"
namespace ns3 {
    
    NS_OBJECT_ENSURE_REGISTERED (MmWaveSigHeader);

    MmWaveSigHeader::MmWaveSigHeader ()
            : m_format (1),
              m_bssColor (0),
              m_ul_dl (0),
              m_mcs (0),
              m_spatialReuse (0),
              m_bandwidth (0),
              m_gi_ltf_size (0),
              m_nsts (0)
    {
    }

    MmWaveSigHeader::~MmWaveSigHeader ()
    {
    }

    TypeId
    MmWaveSigHeader::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::MmWaveSigHeader")
                .SetParent<Header> ()
                .SetGroupName ("MmWave")
                .AddConstructor<MmWaveSigHeader> ()
        ;
        return tid;
    }

    TypeId
    MmWaveSigHeader::GetInstanceTypeId () const
    {
        return GetTypeId ();
    }

    void
    MmWaveSigHeader::Print (std::ostream &os) const
    {
        os << "MCS=" << +m_mcs
           << " CHANNEL_WIDTH=" << GetChannelWidth ()
           << " GI=" << GetGuardInterval ()
           << " NSTS=" << +m_nsts
           << " bssColor=" << +m_bssColor
           << " CODING=" << (m_coding ? "LDPC" : "BCC");
    }

    uint32_t
    MmWaveSigHeader::GetSerializedSize () const
    {
        uint32_t size = 0;
        size += 4; //SIG-A1
        size += 4; //SIG-A2

        return size;
    }

    void
    MmWaveSigHeader::SetLength (uint16_t length)
    {
        NS_ASSERT_MSG (length < 4096, "Invalid length");
        m_length = length;
    }

    uint16_t
    MmWaveSigHeader::GetLength () const
    {
        return m_length;
    }

    void
    MmWaveSigHeader::SetMcs (uint8_t mcs)
    {
        NS_ASSERT (mcs <= 11);
        m_mcs = mcs;
    }

    uint8_t
    MmWaveSigHeader::GetMcs () const
    {
        return m_mcs;
    }

    void
    MmWaveSigHeader::SetBssColor (uint8_t bssColor)
    {
        NS_ASSERT (bssColor < 64);
        m_bssColor = bssColor;
    }

    uint8_t
    MmWaveSigHeader::GetBssColor () const
    {
        return m_bssColor;
    }

    void
    MmWaveSigHeader::SetChannelWidth (uint16_t channelWidth)
    {
        if (channelWidth == 1280)
        {
            m_bandwidth = 3;
        }
        else if (channelWidth == 640)
        {
            m_bandwidth = 2;
        }
        else if (channelWidth == 320)
        {
            m_bandwidth = 1;
        }
        else
        {
            m_bandwidth = 0;
        }
    }

    uint16_t
    MmWaveSigHeader::GetChannelWidth () const
    {
        if (m_bandwidth == 3)
        {
            return 1280;
        }
        else if (m_bandwidth == 2)
        {
            return 640;
        }
        else if (m_bandwidth == 1)
        {
            return 320;
        }
        else
        {
            return 160;
        }
    }

    void
    MmWaveSigHeader::SetGuardIntervalAndLtfSize (uint16_t gi, uint8_t ltf)
    {
        if (gi == 800 && ltf == 1)
        {
            m_gi_ltf_size = 0;
        }
        else if (gi == 800 && ltf == 2)
        {
            m_gi_ltf_size = 1;
        }
        else if (gi == 1600)
        {
            m_gi_ltf_size = 2;
        }
        else
        {
            m_gi_ltf_size = 3;
        }
    }

    uint16_t
    MmWaveSigHeader::GetGuardInterval () const
    {
        if (m_gi_ltf_size == 3)
        {
            //we currently do not consider DCM nor STBC fields
            return 3200;
        }
        else if (m_gi_ltf_size == 2)
        {
            return 1600;
        }
        else
        {
            return 800;
        }
    }

    void
    MmWaveSigHeader::SetNStreams (uint8_t nStreams)
    {
        NS_ASSERT (nStreams <= 8);
        m_nsts = (nStreams - 1);
    }

    uint8_t
    MmWaveSigHeader::GetNStreams () const
    {
        return (m_nsts + 1);
    }

    void
    MmWaveSigHeader::SetCoding (bool ldpc)
    {
        m_coding = ldpc ? 1 : 0;
    }

    bool
    MmWaveSigHeader::IsLdpcCoding () const
    {
        return m_coding ? true : false;

    }

    void
    MmWaveSigHeader::Serialize (Buffer::Iterator start) const
    {
        //SIG-A1
        uint8_t byte = m_format & 0x01;
        byte |= ((m_ul_dl & 0x01) << 2);
        byte |= ((m_mcs & 0x0f) << 3);
        start.WriteU8 (byte);
        uint16_t bytes = (m_bssColor & 0x3f);
        bytes |= (0x01 << 6); //Reserved set to 1
        bytes |= ((m_spatialReuse & 0x0f) << 7);
        bytes |= ((m_bandwidth & 0x03) << 11);
        bytes |= ((m_gi_ltf_size & 0x03) << 13);
        bytes |= ((m_nsts & 0x01) << 15);
        start.WriteU16 (bytes);
        start.WriteU8 ((m_nsts >> 1) & 0x03);

        //SIG-A2
        uint32_t sigA2 = 0;
        sigA2 |= (m_length & 0x0fff);
        sigA2 |= ((m_coding & 0x01) << 13);
        start.WriteU32 (sigA2);
    }

    uint32_t
    MmWaveSigHeader::Deserialize (Buffer::Iterator start)
    {
        Buffer::Iterator i = start;

        //SIG-A1
        uint8_t byte = i.ReadU8 ();
        m_format = (byte & 0x01);
        m_ul_dl = ((byte >> 2) & 0x01);
        m_mcs = ((byte >> 3) & 0x0f);
        uint16_t bytes = i.ReadU16 ();
        m_bssColor = (bytes & 0x3f);
        m_spatialReuse = ((bytes >> 7) & 0x0f);
        m_bandwidth = ((bytes >> 11) & 0x03);
        m_gi_ltf_size = ((bytes >> 13) & 0x03);
        m_nsts = ((bytes >> 15) & 0x01);
        byte = i.ReadU8 ();
        m_nsts |= (byte & 0x03) << 1;

        //SIG-A2
        uint32_t sigA2 = i.ReadU32 ();
        m_length = (sigA2 & 0x0fff);
        m_coding = ((sigA2 >> 13) & 0x01);

        return i.GetDistanceFrom (start);
    }

} //namespace ns3