/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/double.h"
#include "mmwave-snr-tag.h"
namespace ns3 {

    NS_OBJECT_ENSURE_REGISTERED (MmWaveSnrTag);

    TypeId
    MmWaveSnrTag::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::MmWaveSnrTag")
                .SetParent<Tag> ()
                .SetGroupName ("MmWave")
                .AddConstructor<MmWaveSnrTag> ()
                .AddAttribute ("Snr", "The SNR of the last packet received",
                               DoubleValue (0.0),
                               MakeDoubleAccessor (&MmWaveSnrTag::Get),
                               MakeDoubleChecker<double> ())
        ;
        return tid;
    }

    TypeId
    MmWaveSnrTag::GetInstanceTypeId () const
    {
        return GetTypeId ();
    }

    MmWaveSnrTag::MmWaveSnrTag ()
            : m_snr (0)
    {
    }

    uint32_t
    MmWaveSnrTag::GetSerializedSize () const
    {
        return sizeof (double);
    }

    void
    MmWaveSnrTag::Serialize (TagBuffer i) const
    {
        i.WriteDouble (m_snr);
    }

    void
    MmWaveSnrTag::Deserialize (TagBuffer i)
    {
        m_snr = i.ReadDouble ();
    }

    void
    MmWaveSnrTag::Print (std::ostream &os) const
    {
        os << "Snr=" << m_snr;
    }

    void
    MmWaveSnrTag::Set (double snr)
    {
        m_snr = snr;
    }

    double
    MmWaveSnrTag::Get () const
    {
        return m_snr;
    }

}
