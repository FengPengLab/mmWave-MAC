/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_CHANNEL_HELPER_H
#define MMWAVE_CHANNEL_HELPER_H

#include "ns3/ptr.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/spectrum-channel.h"
namespace ns3 {
    class MmWaveChannelHelper
    {
    public:
        MmWaveChannelHelper ();
        ~MmWaveChannelHelper ();
        void SetFrequency (double frequency);
        Ptr<MultiModelSpectrumChannel> CreateSpectrumChannel ();
    protected:
        double m_frequency;
    };
}

#endif //MMWAVE_CHANNEL_HELPER_H
