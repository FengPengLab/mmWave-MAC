/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/log.h"
#include "ns3/spectrum-value.h"
#include "ns3/mobility-model.h"
#include "ns3/net-device.h"
#include "ns3/object.h"
#include "mmwave-spectrum-phy-interface.h"
#include "mmwave-spectrum-phy.h"
namespace ns3 {
    
    NS_LOG_COMPONENT_DEFINE ("MmWaveSpectrumPhyInterface");
    NS_OBJECT_ENSURE_REGISTERED (MmWaveSpectrumPhyInterface);

    TypeId
    MmWaveSpectrumPhyInterface::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::MmWaveSpectrumPhyInterface")
                .SetParent<SpectrumPhy> ()
                .SetGroupName ("MmWave");
        return tid;
    }

    MmWaveSpectrumPhyInterface::MmWaveSpectrumPhyInterface ()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    MmWaveSpectrumPhyInterface::DoDispose ()
    {
        NS_LOG_FUNCTION (this);
        m_phy = 0;
        m_netDevice = 0;
        m_channel = 0;
    }

    void 
    MmWaveSpectrumPhyInterface::SetPhy (const Ptr<MmWaveSpectrumPhy> phy)
    {
        m_phy = phy;
    }

    Ptr<NetDevice>
    MmWaveSpectrumPhyInterface::GetDevice () const
    {
        return m_netDevice;
    }

    Ptr<MobilityModel>
    MmWaveSpectrumPhyInterface::GetMobility ()
    {
        return m_phy->GetMobility ();
    }

    void
    MmWaveSpectrumPhyInterface::SetDevice (const Ptr<NetDevice> d)
    {
        m_netDevice = d;
    }

    void
    MmWaveSpectrumPhyInterface::SetMobility (const Ptr<MobilityModel> m)
    {
        m_phy->SetMobility (m);
    }

    void
    MmWaveSpectrumPhyInterface::SetChannel (const Ptr<SpectrumChannel> c)
    {
        NS_LOG_FUNCTION (this << c);
        m_channel = c;
    }

    Ptr<const SpectrumModel>
    MmWaveSpectrumPhyInterface::GetRxSpectrumModel () const
    {
        return m_phy->GetRxSpectrumModel ();
    }

    Ptr<AntennaModel>
    MmWaveSpectrumPhyInterface::GetRxAntenna ()
    {
        NS_LOG_FUNCTION (this);
        return m_phy->GetRxAntenna ();
    }

    void
    MmWaveSpectrumPhyInterface::StartRx (Ptr<SpectrumSignalParameters> params)
    {
        m_phy->StartRx (params);
    }

} //namespace ns3
