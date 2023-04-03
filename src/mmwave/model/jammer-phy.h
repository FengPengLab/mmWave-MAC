/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
#ifndef JAMMER_PHY_H
#define JAMMER_PHY_H
#include "ns3/antenna-model.h"
#include "ns3/spectrum-channel.h"
#include "ns3/spectrum-model.h"
#include "ns3/type-id.h"
#include "ns3/traced-callback.h"
#include "ns3/spectrum-signal-parameters.h"
#include "mmwave-spectrum-value-helper.h"
#include "mmwave-spectrum-phy.h"

namespace ns3 {
    class MmWaveSpectrumPhyInterface;
    class MmWavePpdu;
    class JammerMac;

    class JammerPhy : public MmWaveSpectrumPhy
    {
    public:
        static TypeId GetTypeId ();
        typedef void (* SignalArrivalCallback) (bool signalType, uint32_t senderNodeId, double rxPower, Time duration);
        JammerPhy ();
        ~JammerPhy ();

        void SetMac (Ptr<JammerMac> mac);
        void SendSignal (Ptr<const MmWavePsdu> psdu, MmWaveTxVector txVector);
        void StartTx (Ptr<MmWavePpdu> ppdu);
        void StartRx (Ptr<SpectrumSignalParameters> rxParams);
        void SetChannelNumber (uint8_t id);
        void SetFrequency (uint16_t freq);
        void SetChannelWidth (uint16_t channelwidth);
        void ConfigureStandardAndBand (MmWavePhyStandard standard, MmWavePhyBand band);
        void CreateMmWaveSpectrumPhyInterface (Ptr<NetDevice> device);
        Ptr<const SpectrumModel> GetRxSpectrumModel ();
        uint32_t GetBandBandwidth () const;
        uint16_t GetGuardBandwidth (uint16_t currentChannelWidth) const;
        uint16_t GetCenterFrequencyForChannelWidth (MmWaveTxVector txVector) const;

    protected:
        void DoDispose ();
        void DoInitialize ();
        MmWaveSpectrumBand GetBand (uint16_t bandWidth, uint8_t bandIndex = 0);
        Ptr<SpectrumValue> GetTxPowerSpectralDensity (uint16_t centerFrequency, uint16_t channelWidth, double txPowerW, MmWaveModulationClass modulationClass) const;
        void ResetSpectrumModel ();
        void UpdateInterferenceHelperBands ();

    private:
        Ptr<JammerMac> m_mac;
        Ptr<MmWaveSpectrumPhyInterface> m_mmWaveSpectrumPhyInterface;
        mutable Ptr<const SpectrumModel> m_rxSpectrumModel;
        bool m_disableReception;
        TracedCallback<bool, uint32_t, double, Time> m_signalCb;
        EventId m_endTxSignal;
        double m_txMaskInnerBandMinimumRejection; //!< The minimum rejection (in dBr) for the inner band of the transmit spectrum mask
        double m_txMaskOuterBandMinimumRejection; //!< The minimum rejection (in dBr) for the outer band of the transmit spectrum mask
        double m_txMaskOuterBandMaximumRejection; //!< The maximum rejection (in dBr) for the outer band of the transmit spectrum mask
    };
}
#endif //JAMMER_PHY_H
