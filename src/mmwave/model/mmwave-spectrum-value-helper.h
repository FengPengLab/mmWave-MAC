/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_SPECTRUM_VALUE_HELPER_H
#define MMWAVE_SPECTRUM_VALUE_HELPER_H
#include "ns3/ptr.h"
#include "ns3/spectrum-value.h"
#include "ns3/spectrum-model.h"

namespace ns3 {

    typedef std::pair<uint32_t, uint32_t> MmWaveSpectrumBand;

    class MmWaveSpectrumValueHelper
    {
    public:
        virtual ~MmWaveSpectrumValueHelper ();

        static Ptr<SpectrumModel> GetSpectrumModel (uint32_t centerFrequency, uint16_t channelWidth, uint32_t bandBandwidth, uint16_t guardBandwidth);
        static Ptr<SpectrumValue> CreateMmWaveOfdmTxPowerSpectralDensity (uint32_t centerFrequency, uint16_t channelWidth, double txPowerW, uint16_t guardBandwidth, double minInnerBandDbr = -20, double minOuterbandDbr = -28, double lowestPointDbr = -40);
        static Ptr<SpectrumValue> CreateNoisePowerSpectralDensity (uint32_t centerFrequency, uint16_t channelWidth, uint32_t bandBandwidth, double noiseFigure, uint16_t guardBandwidth);
        static Ptr<SpectrumValue> CreateNoisePowerSpectralDensity (double noiseFigure, Ptr<SpectrumModel> spectrumModel);
        static Ptr<SpectrumValue> CreateRfFilter (uint32_t centerFrequency, uint16_t totalChannelWidth, uint32_t bandBandwidth, uint16_t guardBandwidth, MmWaveSpectrumBand band);
        static void CreateSpectrumMaskForOfdm (Ptr<SpectrumValue> c, std::vector <MmWaveSpectrumBand> allocatedSubBands, MmWaveSpectrumBand maskBand, double txPowerPerBandW, uint32_t nGuardBands, uint32_t innerSlopeWidth, double minInnerBandDbr, double minOuterbandDbr, double lowestPointDbr);
        static void NormalizeSpectrumMask (Ptr<SpectrumValue> c, double txPowerW);
        static double DbmToW (double dbm);
    };

} // namespace ns3
#endif //MMWAVE_SPECTRUM_VALUE_HELPER_H
