/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_SPECTRUM_PHY_H
#define MMWAVE_SPECTRUM_PHY_H
#include "ns3/type-id.h"
#include "ns3/traced-callback.h"
#include "ns3/spectrum-model.h"
#include "ns3/spectrum-signal-parameters.h"
#include "ns3/spectrum-channel.h"
#include "mmwave-spectrum-value-helper.h"
#include "mmwave-phy.h"

namespace ns3 {

    class MmWaveSpectrumPhyInterface;
    class MmWavePpdu;

    class MmWaveSpectrumPhy : public MmWavePhy
    {
    public:
        static TypeId GetTypeId ();

        MmWaveSpectrumPhy ();
        virtual ~MmWaveSpectrumPhy ();

        void StartTx (Ptr<MmWavePpdu> ppdu);
        void StartRx (Ptr<SpectrumSignalParameters> rxParams);
        uint16_t GetCenterFrequencyForChannelWidth (MmWaveTxVector txVector) const;
        void CreateMmWaveSpectrumPhyInterface (Ptr<NetDevice> device);
        Ptr<const SpectrumModel> GetRxSpectrumModel ();
        uint32_t GetBandBandwidth () const;
        uint16_t GetGuardBandwidth (uint16_t currentChannelWidth) const;
        typedef void (* SignalArrivalCallback) (bool signalType, uint32_t senderNodeId, double rxPower, Time duration);

        Ptr<Channel> GetChannel () const;
        Ptr<AntennaModel> GetRxAntenna () const;

        void SetChannel (const Ptr<SpectrumChannel> channel);
        void SetAntenna (const Ptr<AntennaModel> antenna);
        virtual void SetChannelNumber (uint8_t id);
        virtual void SetFrequency (uint16_t freq);
        virtual void SetChannelWidth (uint16_t channelwidth);
        virtual void ConfigureStandardAndBand (MmWavePhyStandard standard, MmWavePhyBand band);

    protected:
        void DoDispose ();
        void DoInitialize ();
        MmWaveSpectrumBand GetBand (uint16_t bandWidth, uint8_t bandIndex = 0);
        Ptr<SpectrumValue> GetTxPowerSpectralDensity (uint16_t centerFrequency, uint16_t channelWidth, double txPowerW, MmWaveModulationClass modulationClass) const;
        void ResetSpectrumModel ();
        void UpdateInterferenceHelperBands ();

        Ptr<MmWaveSpectrumPhyInterface> m_mmWaveSpectrumPhyInterface;
        Ptr<AntennaModel> m_antenna;
        Ptr<SpectrumChannel> m_channel;
        mutable Ptr<const SpectrumModel> m_rxSpectrumModel;
        bool m_disableReception;
        TracedCallback<bool, uint32_t, double, Time> m_signalCb;
        double m_txMaskInnerBandMinimumRejection; //!< The minimum rejection (in dBr) for the inner band of the transmit spectrum mask
        double m_txMaskOuterBandMinimumRejection; //!< The minimum rejection (in dBr) for the outer band of the transmit spectrum mask
        double m_txMaskOuterBandMaximumRejection; //!< The maximum rejection (in dBr) for the outer band of the transmit spectrum mask
    };

} //namespace ns3
#endif //MMWAVE_SPECTRUM_PHY_H
