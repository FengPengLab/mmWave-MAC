/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/propagation-delay-model.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/object.h"
#include "ns3/mmwave-channel-helper.h"
namespace ns3 {
    NS_LOG_COMPONENT_DEFINE ("MmWaveChannelHelper");

    MmWaveChannelHelper::MmWaveChannelHelper ()
    {
        m_frequency = 60e9;
    }

    MmWaveChannelHelper::~MmWaveChannelHelper ()
    {
    }

    void
    MmWaveChannelHelper::SetFrequency (double frequency)
    {
        m_frequency = frequency;
    }

    Ptr<MultiModelSpectrumChannel>
    MmWaveChannelHelper::CreateSpectrumChannel ()
    {
        Ptr<FriisPropagationLossModel> lossModel = CreateObject<FriisPropagationLossModel> ();
        Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
        Ptr<MultiModelSpectrumChannel> spectrumChannel = CreateObject<MultiModelSpectrumChannel> ();
        lossModel->SetFrequency (m_frequency);
        spectrumChannel->AddPropagationLossModel (lossModel);
        spectrumChannel->SetPropagationDelayModel (delayModel);
        return spectrumChannel;
    }
}