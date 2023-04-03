/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
#include "mmwave-phy-tag.h"


namespace ns3 {

    TypeId
    MmWavePhyTag::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::MmWavePhyTag")
                .SetParent<Tag> ()
        ;
        return tid;
    }

    TypeId
    MmWavePhyTag::GetInstanceTypeId (void) const
    {
        return GetTypeId ();
    }

    uint32_t
    MmWavePhyTag::GetSerializedSize (void) const
    {
        return 3;
    }

    void
    MmWavePhyTag::Serialize (TagBuffer i) const
    {
        i.WriteU8 (static_cast<uint8_t> (m_preamble));
        i.WriteU8 (static_cast<uint8_t> (m_modulation));
        i.WriteU8 (m_frameComplete);
    }

    void
    MmWavePhyTag::Deserialize (TagBuffer i)
    {
        m_preamble = static_cast<MmWavePreamble> (i.ReadU8 ());
        m_modulation = static_cast<MmWaveModulationClass> (i.ReadU8 ());
        m_frameComplete = i.ReadU8 ();
    }

    void
    MmWavePhyTag::Print (std::ostream &os) const
    {
        os << +m_preamble << " " << +m_modulation << " " << m_frameComplete;
    }

    MmWavePhyTag::MmWavePhyTag ()
    {
    }

    MmWavePhyTag::MmWavePhyTag (MmWavePreamble preamble, MmWaveModulationClass modulation, uint8_t frameComplete)
            : m_preamble (preamble),
              m_modulation (modulation),
              m_frameComplete (frameComplete)
    {
    }

    MmWavePreamble
    MmWavePhyTag::GetPreambleType (void) const
    {
        return m_preamble;
    }

    MmWaveModulationClass
    MmWavePhyTag::GetModulation (void) const
    {
        return m_modulation;
    }

    uint8_t
    MmWavePhyTag::GetFrameComplete (void) const
    {
        return m_frameComplete;
    }

} // namespace ns3