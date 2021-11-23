/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_SPECTRUM_SIGNAL_PARAMETERS_H
#define MMWAVE_SPECTRUM_SIGNAL_PARAMETERS_H
#include "ns3/spectrum-signal-parameters.h"
#include "ns3/simple-ref-count.h"
#include "mmwave.h"
namespace ns3 {

    class MmWavePpdu;

    class MmWaveSpectrumChannel : public SimpleRefCount<MmWaveSpectrumChannel>
    {
    public:
        MmWaveSpectrumChannel (MmWavePhyStandard standard, MmWavePhyBand band, uint8_t channelNumber, uint16_t channelFrequency, uint16_t channelWidth);
        MmWaveSpectrumChannel (MmWaveChannelNumberStandardPair channelNumberStandardPair, MmWaveFrequencyWidthPair frequencyWidthPair);
        bool IsMatch (MmWaveChannelNumberStandardPair channelNumberStandardPair);
        bool IsMatch (MmWaveFrequencyWidthPair frequencyWidthPair);
        bool IsMatch (MmWavePhyStandard standard, MmWavePhyBand band, uint8_t channelNumber, uint16_t channelFrequency, uint16_t channelWidth);
        MmWaveChannelNumberStandardPair GetChannelNumberStandardPair ();

    private:
        MmWavePhyStandard m_standard;
        MmWavePhyBand m_band;
        uint16_t m_channelWidth;
        uint16_t m_channelFrequency;
        uint8_t m_channelNumber;
    };

    struct MmWaveSpectrumSignalParameters : public SpectrumSignalParameters
    {
        virtual Ptr<SpectrumSignalParameters> Copy ();
        MmWaveSpectrumSignalParameters ();
        MmWaveSpectrumSignalParameters (const MmWaveSpectrumSignalParameters& p);

        void SetMmWaveSpectrumChannel (Ptr<MmWaveSpectrumChannel> c);

        Ptr<MmWavePpdu> ppdu; ///< The PPDU being transmitted
        Ptr<MmWaveSpectrumChannel> channel;
    };

}  // namespace ns3

#endif //MMWAVE_SPECTRUM_SIGNAL_PARAMETERS_H
