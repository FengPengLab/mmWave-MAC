/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/log.h"
#include "mmwave-ppdu.h"
#include "mmwave-spectrum-signal-parameters.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("MmWaveSpectrumSignalParameters");

    MmWaveSpectrumChannel::MmWaveSpectrumChannel (MmWavePhyStandard standard, MmWavePhyBand band, uint8_t channelNumber, uint16_t channelFrequency, uint16_t channelWidth)
            : m_standard (standard),
              m_band (band),
              m_channelWidth (channelWidth),
              m_channelFrequency (channelFrequency),
              m_channelNumber (channelNumber)
    {
        NS_LOG_FUNCTION (this << standard << band << channelNumber);
    }

    MmWaveSpectrumChannel::MmWaveSpectrumChannel (MmWaveChannelNumberStandardPair channelNumberStandardPair, MmWaveFrequencyWidthPair frequencyWidthPair)
            : m_standard (channelNumberStandardPair.second),
              m_band (channelNumberStandardPair.first.second),
              m_channelWidth (frequencyWidthPair.second),
              m_channelFrequency (frequencyWidthPair.first),
              m_channelNumber (channelNumberStandardPair.first.first)
    {
        NS_LOG_FUNCTION (this << m_standard << m_band << m_channelNumber);
    }

    bool
    MmWaveSpectrumChannel::IsMatch (MmWaveChannelNumberStandardPair channelNumberStandardPair)
    {
        MmWaveChannelNumberStandardPair c = std::make_pair(std::make_pair(m_channelNumber, m_band), m_standard);
        if (c == channelNumberStandardPair)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool
    MmWaveSpectrumChannel::IsMatch (MmWaveFrequencyWidthPair frequencyWidthPair)
    {
        MmWaveFrequencyWidthPair f = std::make_pair(m_channelFrequency, m_channelWidth);
        if (f == frequencyWidthPair)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool
    MmWaveSpectrumChannel::IsMatch (MmWavePhyStandard standard, MmWavePhyBand band, uint8_t channelNumber, uint16_t channelFrequency, uint16_t channelWidth)
    {
        if ((m_standard == standard)
            && (m_band == band)
            && (m_channelNumber == channelNumber)
            && (m_channelFrequency == channelFrequency)
            && (m_channelWidth == channelWidth))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    MmWaveChannelNumberStandardPair
    MmWaveSpectrumChannel::GetChannelNumberStandardPair ()
    {
        return std::make_pair(std::make_pair(m_channelNumber,m_band),m_standard);
    }

    MmWaveSpectrumSignalParameters::MmWaveSpectrumSignalParameters ()
    {
        NS_LOG_FUNCTION (this);
        channel = 0;
    }

    MmWaveSpectrumSignalParameters::MmWaveSpectrumSignalParameters (const MmWaveSpectrumSignalParameters& p)
            : SpectrumSignalParameters (p)
    {
        NS_LOG_FUNCTION (this << &p);
        ppdu = p.ppdu;
        channel = p.channel;
    }

    Ptr<SpectrumSignalParameters>
    MmWaveSpectrumSignalParameters::Copy ()
    {
        NS_LOG_FUNCTION (this);
        Ptr<MmWaveSpectrumSignalParameters> wssp (new MmWaveSpectrumSignalParameters (*this), false);
        return wssp;
    }

    void
    MmWaveSpectrumSignalParameters::SetMmWaveSpectrumChannel (Ptr<MmWaveSpectrumChannel> c)
    {
        channel = c;
    }

} // namespace ns3