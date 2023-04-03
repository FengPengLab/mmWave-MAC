/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_PHY_TAG_H
#define MMWAVE_PHY_TAG_H
#include "ns3/tag.h"
#include "ns3/type-id.h"
#include "mmwave.h"
#include "mmwave-mode.h"

namespace ns3 {

    class MmWavePhyTag : public Tag
    {
    public:
        static TypeId GetTypeId (void);
        TypeId GetInstanceTypeId (void) const;

        MmWavePhyTag ();
        MmWavePhyTag (MmWavePreamble preamble, MmWaveModulationClass modulation, uint8_t frameComplete);
        MmWavePreamble GetPreambleType (void) const;
        MmWaveModulationClass GetModulation (void) const;
        uint8_t GetFrameComplete (void) const;

        // From class Tag
        uint32_t GetSerializedSize (void) const;
        void Serialize (TagBuffer i) const;
        void Deserialize (TagBuffer i);
        void Print (std::ostream &os) const;


    private:
        MmWavePreamble m_preamble;          ///< preamble type
        MmWaveModulationClass m_modulation; ///< modulation used for transmission
        uint8_t m_frameComplete;          ///< Used to indicate that TX stopped sending before the end of the frame
    };

} // namespace ns3
#endif //MMWAVE_PHY_TAG_H
