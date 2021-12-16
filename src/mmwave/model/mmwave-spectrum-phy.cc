/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/log.h"
#include "ns3/object.h"
#include "ns3/double.h"
#include "ns3/boolean.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/spectrum-value.h"
#include "mmwave-spectrum-phy.h"
#include "mmwave-spectrum-signal-parameters.h"
#include "mmwave-spectrum-phy-interface.h"
#include "mmwave-ppdu.h"
#include "mmwave-psdu.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("MmWaveSpectrumPhy");
    NS_OBJECT_ENSURE_REGISTERED (MmWaveSpectrumPhy);

    TypeId
    MmWaveSpectrumPhy::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::MmWaveSpectrumPhy")
                .SetParent<MmWavePhy> ()
                .SetGroupName ("MmWave")
                .AddConstructor<MmWaveSpectrumPhy> ()
                .AddAttribute ("DisableMmWaveReception",
                               "Prevent frame sync from ever happening",
                               BooleanValue (false),
                               MakeBooleanAccessor (&MmWaveSpectrumPhy::m_disableReception),
                               MakeBooleanChecker ())
                .AddAttribute ("TxMaskInnerBandMinimumRejection",
                               "Minimum rejection (dBr) for the inner band of the transmit spectrum mask",
                               DoubleValue (-20.0),
                               MakeDoubleAccessor (&MmWaveSpectrumPhy::m_txMaskInnerBandMinimumRejection),
                               MakeDoubleChecker<double> ())
                .AddAttribute ("TxMaskOuterBandMinimumRejection",
                               "Minimum rejection (dBr) for the outer band of the transmit spectrum mask",
                               DoubleValue (-28.0),
                               MakeDoubleAccessor (&MmWaveSpectrumPhy::m_txMaskOuterBandMinimumRejection),
                               MakeDoubleChecker<double> ())
                .AddAttribute ("TxMaskOuterBandMaximumRejection",
                               "Maximum rejection (dBr) for the outer band of the transmit spectrum mask",
                               DoubleValue (-40.0),
                               MakeDoubleAccessor (&MmWaveSpectrumPhy::m_txMaskOuterBandMaximumRejection),
                               MakeDoubleChecker<double> ())
                .AddTraceSource ("SignalArrival",
                                 "Signal arrival",
                                 MakeTraceSourceAccessor (&MmWaveSpectrumPhy::m_signalCb),
                                 "ns3::MmWaveSpectrumPhy::SignalArrivalCallback")
        ;
        return tid;
    }

    MmWaveSpectrumPhy::MmWaveSpectrumPhy ()
    {
        NS_LOG_FUNCTION (this);
    }

    MmWaveSpectrumPhy::~MmWaveSpectrumPhy ()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    MmWaveSpectrumPhy::DoDispose ()
    {
        NS_LOG_FUNCTION (this);
        m_channel = 0;
        m_mmWaveSpectrumPhyInterface = 0;
        MmWavePhy::DoDispose ();
    }

    void
    MmWaveSpectrumPhy::DoInitialize ()
    {
        NS_LOG_FUNCTION (this);
        MmWavePhy::DoInitialize ();
        if (m_channel && m_mmWaveSpectrumPhyInterface)
        {
            m_channel->AddRx (m_mmWaveSpectrumPhyInterface);
        }
        else
        {
            NS_FATAL_ERROR ("MmWaveSpectrumPhy misses channel and MmWaveSpectrumPhyInterface objects at initialization time");
        }
    }

    Ptr<const SpectrumModel>
    MmWaveSpectrumPhy::GetRxSpectrumModel ()
    {
        NS_LOG_FUNCTION (this);
        if (m_rxSpectrumModel)
        {
            return m_rxSpectrumModel;
        }
        else
        {
            if (GetFrequency () == 0)
            {
                NS_LOG_DEBUG ("Frequency is not set; returning 0");
                return 0;
            }
            else
            {
                uint16_t channelWidth = GetChannelWidth ();
//                NS_LOG_DEBUG ("Creating spectrum model from frequency/width pair of (" << GetFrequency () << ", " << channelWidth << ")");
                m_rxSpectrumModel = MmWaveSpectrumValueHelper::GetSpectrumModel (GetFrequency (), channelWidth, GetBandBandwidth (), GetGuardBandwidth (channelWidth));
                UpdateInterferenceHelperBands ();
            }
        }
        return m_rxSpectrumModel;
    }

    void
    MmWaveSpectrumPhy::UpdateInterferenceHelperBands ()
    {
        NS_LOG_FUNCTION (this);
        uint16_t channelWidth = GetChannelWidth ();
        m_interference.RemoveBands ();
        if (channelWidth < 20)
        {
            MmWaveSpectrumBand band = GetBand (channelWidth);
            m_interference.AddBand (band);
        }
        else
        {
            for (uint16_t bw = 1280; bw >= 20; bw = bw / 2)
            {
                for (uint8_t i = 0; i < (channelWidth / bw); ++i)
                {
                    m_interference.AddBand (GetBand (bw, i));
                }
            }
        }
    }

    void
    MmWaveSpectrumPhy::ResetSpectrumModel ()
    {
        NS_LOG_FUNCTION (this);
        NS_ASSERT_MSG (IsInitialized (), "Executing method before run-time");
        uint16_t channelWidth = GetChannelWidth ();
//        NS_LOG_DEBUG ("Run-time change of spectrum model from frequency/width pair of (" << GetFrequency () << ", " << channelWidth << ")");
        m_rxSpectrumModel = MmWaveSpectrumValueHelper::GetSpectrumModel (GetFrequency (), channelWidth, GetBandBandwidth (), GetGuardBandwidth (channelWidth));
        m_channel->AddRx (m_mmWaveSpectrumPhyInterface);
        UpdateInterferenceHelperBands ();
    }

    Ptr<AntennaModel>
    MmWaveSpectrumPhy::GetRxAntenna () const
    {
        return m_antenna;
    }

    void
    MmWaveSpectrumPhy::SetAntenna (const Ptr<AntennaModel> a)
    {
        m_antenna = a;
    }

    Ptr<Channel>
    MmWaveSpectrumPhy::GetChannel () const
    {
        return m_channel;
    }

    void
    MmWaveSpectrumPhy::SetChannel (const Ptr<SpectrumChannel> channel)
    {
        m_channel = channel;
    }

    void
    MmWaveSpectrumPhy::SetChannelNumber (uint8_t nch)
    {
        NS_LOG_FUNCTION (this);
        MmWavePhy::SetChannelNumber (nch);
        if (IsInitialized ())
        {
            ResetSpectrumModel ();
        }
    }

    void
    MmWaveSpectrumPhy::SetFrequency (uint16_t freq)
    {
        NS_LOG_FUNCTION (this << freq);
        MmWavePhy::SetFrequency (freq);
        if (IsInitialized ())
        {
            ResetSpectrumModel ();
        }
    }

    void
    MmWaveSpectrumPhy::SetChannelWidth (uint16_t channelwidth)
    {
        NS_LOG_FUNCTION (this);
        MmWavePhy::SetChannelWidth (channelwidth);
        if (IsInitialized ())
        {
            ResetSpectrumModel ();
        }
    }

    void
    MmWaveSpectrumPhy::ConfigureStandardAndBand (MmWavePhyStandard standard, MmWavePhyBand band)
    {
        NS_LOG_FUNCTION (this);
        MmWavePhy::ConfigureStandardAndBand (standard, band);
        if (IsInitialized ())
        {
            ResetSpectrumModel ();
        }
    }

    void
    MmWaveSpectrumPhy::StartRx (Ptr<SpectrumSignalParameters> rxParams)
    {
        NS_LOG_FUNCTION (this);
        Ptr<MmWaveSpectrumSignalParameters> mmWaveRxParams = DynamicCast<MmWaveSpectrumSignalParameters> (rxParams);
        bool isMatch = mmWaveRxParams->channel->IsMatch (GetPhyStandard(), GetPhyBand(), GetChannelNumber(),GetFrequency(), GetChannelWidth());
        if (!isMatch)
        {
            return;
        }
        Time rxDuration = rxParams->duration;
        Ptr<SpectrumValue> receivedSignalPsd = rxParams->psd;
        uint32_t senderNodeId = 0;
        if (rxParams->txPhy)
        {
            senderNodeId = rxParams->txPhy->GetDevice ()->GetNode ()->GetId ();
        }
        uint16_t channelWidth = GetChannelWidth ();
        double totalRxPowerW = 0;
        RxPowerWattPerChannelBand rxPowerW;

        for (uint16_t bw = 1280; bw > 20; bw = bw / 2)
        {
            for (uint8_t i = 0; i < (channelWidth / bw); i++)
            {
                NS_ASSERT (channelWidth >= bw);
                MmWaveSpectrumBand filteredBand = GetBand (bw, i);
                Ptr<SpectrumValue> filter = MmWaveSpectrumValueHelper::CreateRfFilter (GetFrequency (), channelWidth, GetBandBandwidth (), GetGuardBandwidth (channelWidth), filteredBand);
                SpectrumValue filteredSignal = (*filter) * (*receivedSignalPsd);
//                NS_LOG_DEBUG ("Signal power received (watts) before antenna gain for" << bw << " MHz channel band " << +i << ": " << Integral (filteredSignal));
                double rxPowerPerBandW = Integral (filteredSignal) * DbToRatio (GetRxGain ());
                rxPowerW.insert ({filteredBand, rxPowerPerBandW});
//                NS_LOG_DEBUG ("Signal power received after antenna gain for" << bw << " MHz channel band " << +i << ": " << rxPowerPerBandW << " W (" << WToDbm (rxPowerPerBandW) << " dBm)");
            }
        }
        
        for (uint8_t i = 0; i < (channelWidth / 20); i++)
        {
            MmWaveSpectrumBand filteredBand = GetBand (20, i);
            Ptr<SpectrumValue> filter = MmWaveSpectrumValueHelper::CreateRfFilter (GetFrequency (), channelWidth, GetBandBandwidth (), GetGuardBandwidth (channelWidth), filteredBand);
            SpectrumValue filteredSignal = (*filter) * (*receivedSignalPsd);
//            NS_LOG_DEBUG ("Signal power received (watts) before antenna gain for 20 MHz channel band " << +i << ": " << Integral (filteredSignal));
            double rxPowerPerBandW = Integral (filteredSignal) * DbToRatio (GetRxGain ());
            totalRxPowerW += rxPowerPerBandW;
            rxPowerW.insert ({filteredBand, rxPowerPerBandW});
//            NS_LOG_DEBUG ("Signal power received after antenna gain for 20 MHz channel band " << +i << ": " << rxPowerPerBandW << " W (" << WToDbm (rxPowerPerBandW) << " dBm)");
        }

//        NS_LOG_DEBUG ("Total signal power received after antenna gain: " << totalRxPowerW << " W (" << WToDbm (totalRxPowerW) << " dBm)");
//        Ptr<MmWaveSpectrumSignalParameters> mmWaveRxParams = DynamicCast<MmWaveSpectrumSignalParameters> (rxParams);
        // Log the signal arrival to the trace source
        m_signalCb (mmWaveRxParams ? true : false, senderNodeId, WToDbm (totalRxPowerW), rxDuration);
        // Do no further processing if signal is too weak
        // Current implementation assumes constant RX power over the PPDU duration
        if (WToDbm (totalRxPowerW) < GetRxSensitivity ())
        {
//            NS_LOG_INFO ("Received signal too weak to process: " << WToDbm (totalRxPowerW) << " dBm");
            return;
        }
        if (mmWaveRxParams == 0)
        {
//            NS_LOG_INFO ("Received non mmWave signal");
            m_interference.AddForeignSignal (rxDuration, rxPowerW);
            SwitchMaybeToCcaBusy ();
            return;
        }
        if (mmWaveRxParams && m_disableReception)
        {
//            NS_LOG_INFO ("Received signal but blocked from syncing");
            m_interference.AddForeignSignal (rxDuration, rxPowerW);
            SwitchMaybeToCcaBusy ();
            return;
        }

        NS_LOG_INFO ("Received signal");
        Ptr<MmWavePpdu> ppdu = Copy (mmWaveRxParams->ppdu);
        StartReceivePreamble (ppdu, rxPowerW);
    }

    void
    MmWaveSpectrumPhy::CreateMmWaveSpectrumPhyInterface (Ptr<NetDevice> device)
    {
        NS_LOG_FUNCTION (this);
        m_mmWaveSpectrumPhyInterface = CreateObject<MmWaveSpectrumPhyInterface> ();
        m_mmWaveSpectrumPhyInterface->SetPhy (this);
        m_mmWaveSpectrumPhyInterface->SetDevice (device);
    }

    Ptr<SpectrumValue>
    MmWaveSpectrumPhy::GetTxPowerSpectralDensity (uint16_t centerFrequency, uint16_t channelWidth, double txPowerW, MmWaveModulationClass modulationClass) const
    {
        NS_LOG_FUNCTION (this);
        Ptr<SpectrumValue> v;
        switch (modulationClass)
        {
            case MMWAVE_MOD_CLASS_OFDM:
                v = MmWaveSpectrumValueHelper::CreateMmWaveOfdmTxPowerSpectralDensity (centerFrequency, channelWidth, txPowerW, GetGuardBandwidth (channelWidth),
                                                                                       m_txMaskInnerBandMinimumRejection, m_txMaskOuterBandMinimumRejection,
                                                                                       m_txMaskOuterBandMaximumRejection);
                break;
            default:
                NS_FATAL_ERROR ("modulation class unknown: " << modulationClass);
                break;
        }
        return v;
    }

    uint16_t
    MmWaveSpectrumPhy::GetCenterFrequencyForChannelWidth (MmWaveTxVector txVector) const
    {
        NS_LOG_FUNCTION (this);
        uint16_t centerFrequencyForSupportedWidth = GetFrequency ();
        uint16_t supportedWidth = GetChannelWidth ();
        uint16_t currentWidth = txVector.GetChannelWidth ();
        if (currentWidth != supportedWidth)
        {
            uint16_t startingFrequency = centerFrequencyForSupportedWidth - (supportedWidth / 2);
            return startingFrequency + (currentWidth / 2); // primary channel is in the lower part (for the time being)
        }
        return centerFrequencyForSupportedWidth;
    }

    void
    MmWaveSpectrumPhy::StartTx (Ptr<MmWavePpdu> ppdu)
    {
        NS_LOG_FUNCTION (this << ppdu);
        MmWaveTxVector txVector = ppdu->GetTxVector ();
        double txPowerDbm = GetTxPowerForTransmission (txVector) + GetTxGain ();
//        NS_LOG_DEBUG ("Start transmission: signal power before antenna gain=" << txPowerDbm << "dBm");
        double txPowerWatts = DbmToW (txPowerDbm);
        Ptr<SpectrumValue> txPowerSpectrum = GetTxPowerSpectralDensity (GetCenterFrequencyForChannelWidth (txVector), txVector.GetChannelWidth (), txPowerWatts, ppdu->GetModulation ());
        Ptr<MmWaveSpectrumSignalParameters> txParams = Create<MmWaveSpectrumSignalParameters> ();
        txParams->duration = ppdu->GetTxDuration ();
        txParams->psd = txPowerSpectrum;
        NS_ASSERT_MSG (m_mmWaveSpectrumPhyInterface, "SpectrumPhy() is not set; maybe forgot to call CreateMmWaveSpectrumPhyInterface?");
        txParams->txPhy = m_mmWaveSpectrumPhyInterface->GetObject<SpectrumPhy> ();
        txParams->txAntenna = m_antenna;
        txParams->ppdu = ppdu;
//        NS_LOG_DEBUG ("Starting transmission with power " << WToDbm (txPowerWatts) << " dBm on channel " << +GetChannelNumber ());
//        NS_LOG_DEBUG ("Starting transmission with integrated spectrum power " << WToDbm (Integral (*txPowerSpectrum)) << " dBm; spectrum model Uid: " << txPowerSpectrum->GetSpectrumModel ()->GetUid ());

        Ptr<MmWaveSpectrumChannel> c = Create<MmWaveSpectrumChannel>(GetPhyStandard(), GetPhyBand(), GetChannelNumber(),GetFrequency(), GetChannelWidth());
        txParams->SetMmWaveSpectrumChannel(c);
        m_channel->StartTx (txParams);
    }

    uint32_t
    MmWaveSpectrumPhy::GetBandBandwidth () const
    {
        NS_LOG_FUNCTION (this);
        uint32_t bandBandwidth = 0;
        switch (GetPhyStandard ())
        {
            case MMWAVE_PHY_STANDARD_320MHz:
            case MMWAVE_PHY_STANDARD_640MHz:
            case MMWAVE_PHY_STANDARD_1280MHz:
                // Use OFDM subcarrier width of 78.125 KHz as band granularity
                bandBandwidth = 78125;
                break;
            default:
                NS_FATAL_ERROR ("Standard unknown: " << GetPhyStandard ());
                break;
        }
        return bandBandwidth;
    }

    uint16_t
    MmWaveSpectrumPhy::GetGuardBandwidth (uint16_t currentChannelWidth) const
    {
        return currentChannelWidth;
    }

    MmWaveSpectrumBand
    MmWaveSpectrumPhy::GetBand (uint16_t bandWidth, uint8_t bandIndex)
    {
        NS_LOG_FUNCTION (this);
        uint16_t channelWidth = GetChannelWidth ();
        uint32_t bandBandwidth = GetBandBandwidth ();
        size_t numBandsInChannel = static_cast<size_t> (channelWidth * 1e6 / bandBandwidth);
        size_t numBandsInBand = static_cast<size_t> (bandWidth * 1e6 / bandBandwidth);
        if (numBandsInBand % 2 == 0)
        {
            numBandsInChannel += 1; // symmetry around center frequency
        }
        size_t totalNumBands = GetRxSpectrumModel ()->GetNumBands ();
        NS_ASSERT_MSG ((numBandsInChannel % 2 == 1) && (totalNumBands % 2 == 1), "Should have odd number of bands");
        NS_ASSERT_MSG ((bandIndex * bandWidth) < channelWidth, "Band index is out of bound");
        MmWaveSpectrumBand band;
        band.first = ((totalNumBands - numBandsInChannel) / 2) + (bandIndex * numBandsInBand);
        if (band.first >= totalNumBands / 2)
        {
            //step past DC
            band.first += 1;
        }
        band.second = band.first + numBandsInBand - 1;
        return band;
    }

} //namespace ns3