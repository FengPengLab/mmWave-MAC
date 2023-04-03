/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_SPECTRUM_PHY_INTERFACE_H
#define MMWAVE_SPECTRUM_PHY_INTERFACE_H
#include "ns3/spectrum-phy.h"

namespace ns3 {
    class MmWaveSpectrumPhy;

    class MmWaveSpectrumPhyInterface : public SpectrumPhy
    {
    public:
        static TypeId GetTypeId ();
        MmWaveSpectrumPhyInterface ();

        void SetPhy (const Ptr<MmWaveSpectrumPhy> phy);

        Ptr<NetDevice> GetDevice () const;
        void SetDevice (const Ptr<NetDevice> d);
        void SetMobility (const Ptr<MobilityModel> m);
        Ptr<MobilityModel> GetMobility ();
        void SetChannel (const Ptr<SpectrumChannel> c);
        Ptr<const SpectrumModel> GetRxSpectrumModel () const;
        Ptr<AntennaModel> GetRxAntenna ();
        void StartRx (Ptr<SpectrumSignalParameters> params);

    private:
        virtual void DoDispose ();
        Ptr<MmWaveSpectrumPhy> m_phy; ///< spectrum PHY
        Ptr<NetDevice> m_netDevice; ///< the device
        Ptr<SpectrumChannel> m_channel; ///< spectrum channel
    };

} // namespace ns3
#endif //MMWAVE_SPECTRUM_PHY_INTERFACE_H
