/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/log.h"
#include "ns3/object.h"
#include "ns3/double.h"
#include "ns3/boolean.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/spectrum-value.h"
#include "ns3/simulator.h"
#include "mmwave-spectrum-signal-parameters.h"
#include "mmwave-spectrum-phy-interface.h"
#include "mmwave-ppdu.h"
#include "mmwave-psdu.h"
#include "mmwave-phy-state-helper.h"
#include "jammer-mac.h"
#include "jammer-phy.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("JammerPhy");
    NS_OBJECT_ENSURE_REGISTERED (JammerPhy);

    TypeId
    JammerPhy::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::JammerPhy")
                .SetParent<MmWavePhy> ()
                .SetGroupName ("MmWave")
                .AddConstructor<JammerPhy> ()
                .AddAttribute ("DisableMmWaveReception",
                               "Prevent frame sync from ever happening",
                               BooleanValue (false),
                               MakeBooleanAccessor (&JammerPhy::m_disableReception),
                               MakeBooleanChecker ())
                .AddAttribute ("TxMaskInnerBandMinimumRejection",
                               "Minimum rejection (dBr) for the inner band of the transmit spectrum mask",
                               DoubleValue (-20.0),
                               MakeDoubleAccessor (&JammerPhy::m_txMaskInnerBandMinimumRejection),
                               MakeDoubleChecker<double> ())
                .AddAttribute ("TxMaskOuterBandMinimumRejection",
                               "Minimum rejection (dBr) for the outer band of the transmit spectrum mask",
                               DoubleValue (-28.0),
                               MakeDoubleAccessor (&JammerPhy::m_txMaskOuterBandMinimumRejection),
                               MakeDoubleChecker<double> ())
                .AddAttribute ("TxMaskOuterBandMaximumRejection",
                               "Maximum rejection (dBr) for the outer band of the transmit spectrum mask",
                               DoubleValue (-40.0),
                               MakeDoubleAccessor (&JammerPhy::m_txMaskOuterBandMaximumRejection),
                               MakeDoubleChecker<double> ())
                .AddTraceSource ("SignalArrival",
                                 "Signal arrival",
                                 MakeTraceSourceAccessor (&JammerPhy::m_signalCb),
                                 "ns3::JammerPhy::SignalArrivalCallback")
        ;
        return tid;
    }

    JammerPhy::JammerPhy ()
    {
        NS_LOG_FUNCTION (this);
        SetReceiveOkCallback (MakeNullCallback<void, Ptr<MmWavePsdu>, double, Time, Time, MmWaveTxVector> ());
        SetReceiveErrorCallback (MakeNullCallback<void, Ptr<MmWavePsdu>, double, Time, Time, MmWaveTxVector> ());
        SetUnrecognizedSignalDetectedCallback (MakeNullCallback<void, double, double, Time, Time> ());
        SetRecognizedSignalDetectedCallback (MakeNullCallback<void, double, double, Time, Time> ());
    }

    JammerPhy::~JammerPhy ()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    JammerPhy::DoDispose ()
    {
        NS_LOG_FUNCTION (this);
        m_channel = 0;
        m_mmWaveSpectrumPhyInterface = 0;
        MmWavePhy::DoDispose ();
    }

    void
    JammerPhy::DoInitialize ()
    {
        NS_LOG_FUNCTION (this);
        MmWavePhy::DoInitialize ();
        if (m_channel && m_mmWaveSpectrumPhyInterface)
        {
            m_channel->AddRx (m_mmWaveSpectrumPhyInterface);
        }
        else
        {
            NS_FATAL_ERROR ("JammerPhy misses channel and JammerPhyInterface objects at initialization time");
        }
    }

    Ptr<const SpectrumModel>
    JammerPhy::GetRxSpectrumModel ()
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
                NS_LOG_DEBUG ("Creating spectrum model from frequency/width pair of (" << GetFrequency () << ", " << channelWidth << ")");
                m_rxSpectrumModel = MmWaveSpectrumValueHelper::GetSpectrumModel (GetFrequency (), channelWidth, GetBandBandwidth (), GetGuardBandwidth (channelWidth));
                UpdateInterferenceHelperBands ();
            }
        }
        return m_rxSpectrumModel;
    }

    void
    JammerPhy::UpdateInterferenceHelperBands ()
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
    JammerPhy::ResetSpectrumModel ()
    {
        NS_LOG_FUNCTION (this);
        NS_ASSERT_MSG (IsInitialized (), "Executing method before run-time");
        uint16_t channelWidth = GetChannelWidth ();
        NS_LOG_DEBUG ("Run-time change of spectrum model from frequency/width pair of (" << GetFrequency () << ", " << channelWidth << ")");
        m_rxSpectrumModel = MmWaveSpectrumValueHelper::GetSpectrumModel (GetFrequency (), channelWidth, GetBandBandwidth (), GetGuardBandwidth (channelWidth));
        m_channel->AddRx (m_mmWaveSpectrumPhyInterface);
        UpdateInterferenceHelperBands ();
    }

    void
    JammerPhy::SetChannelNumber (uint8_t nch)
    {
        NS_LOG_FUNCTION (this << +nch);
        MmWavePhy::SetChannelNumber (nch);
        if (IsInitialized ())
        {
            ResetSpectrumModel ();
        }
    }

    void
    JammerPhy::SetFrequency (uint16_t freq)
    {
        NS_LOG_FUNCTION (this << freq);
        MmWavePhy::SetFrequency (freq);
        if (IsInitialized ())
        {
            ResetSpectrumModel ();
        }
    }

    void
    JammerPhy::SetChannelWidth (uint16_t channelwidth)
    {
        NS_LOG_FUNCTION (this << channelwidth);
        MmWavePhy::SetChannelWidth (channelwidth);
        if (IsInitialized ())
        {
            ResetSpectrumModel ();
        }
    }

    void
    JammerPhy::ConfigureStandardAndBand (MmWavePhyStandard standard, MmWavePhyBand band)
    {
        NS_LOG_FUNCTION (this << standard << band);
        MmWavePhy::ConfigureStandardAndBand (standard, band);
        if (IsInitialized ())
        {
            ResetSpectrumModel ();
        }
    }

    void
    JammerPhy::StartRx (Ptr<SpectrumSignalParameters> rxParams)
    {
        NS_LOG_FUNCTION (this << rxParams);
        Time rxDuration = rxParams->duration;
        Ptr<SpectrumValue> receivedSignalPsd = rxParams->psd;
        uint32_t senderNodeId = 0;
        if (rxParams->txPhy)
        {
            senderNodeId = rxParams->txPhy->GetDevice ()->GetNode ()->GetId ();
        }
        NS_LOG_DEBUG ("Received signal from " << senderNodeId << " with unfiltered power " << WToDbm (Integral (*receivedSignalPsd)) << " dBm");

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
                NS_LOG_DEBUG ("Signal power received (watts) before antenna gain for" << bw << " MHz channel band " << +i << ": " << Integral (filteredSignal));
                double rxPowerPerBandW = Integral (filteredSignal) * DbToRatio (GetRxGain ());
                rxPowerW.insert ({filteredBand, rxPowerPerBandW});
                NS_LOG_DEBUG ("Signal power received after antenna gain for" << bw << " MHz channel band " << +i << ": " << rxPowerPerBandW << " W (" << WToDbm (rxPowerPerBandW) << " dBm)");
            }
        }

        for (uint8_t i = 0; i < (channelWidth / 20); i++)
        {
            MmWaveSpectrumBand filteredBand = GetBand (20, i);
            Ptr<SpectrumValue> filter = MmWaveSpectrumValueHelper::CreateRfFilter (GetFrequency (), channelWidth, GetBandBandwidth (), GetGuardBandwidth (channelWidth), filteredBand);
            SpectrumValue filteredSignal = (*filter) * (*receivedSignalPsd);
            NS_LOG_DEBUG ("Signal power received (watts) before antenna gain for 20 MHz channel band " << +i << ": " << Integral (filteredSignal));
            double rxPowerPerBandW = Integral (filteredSignal) * DbToRatio (GetRxGain ());
            totalRxPowerW += rxPowerPerBandW;
            rxPowerW.insert ({filteredBand, rxPowerPerBandW});
            NS_LOG_DEBUG ("Signal power received after antenna gain for 20 MHz channel band " << +i << ": " << rxPowerPerBandW << " W (" << WToDbm (rxPowerPerBandW) << " dBm)");
        }

        NS_LOG_DEBUG ("Total signal power received after antenna gain: " << totalRxPowerW << " W (" << WToDbm (totalRxPowerW) << " dBm)");
        Ptr<MmWaveSpectrumSignalParameters> mmWaveRxParams = DynamicCast<MmWaveSpectrumSignalParameters> (rxParams);

        // Log the signal arrival to the trace source
        m_signalCb (mmWaveRxParams ? true : false, senderNodeId, WToDbm (totalRxPowerW), rxDuration);

        // Do no further processing if signal is too weak
        // Current implementation assumes constant RX power over the PPDU duration
        if (WToDbm (totalRxPowerW) < GetRxSensitivity ())
        {
            NS_LOG_INFO ("Received signal too weak to process: " << WToDbm (totalRxPowerW) << " dBm");
            return;
        }

        m_interference.AddForeignSignal (rxDuration, rxPowerW);
        SwitchMaybeToCcaBusy ();
    }

    void
    JammerPhy::CreateMmWaveSpectrumPhyInterface (Ptr<NetDevice> device)
    {
        NS_LOG_FUNCTION (this << device);
        m_mmWaveSpectrumPhyInterface = CreateObject<MmWaveSpectrumPhyInterface> ();
        m_mmWaveSpectrumPhyInterface->SetPhy (this);
        m_mmWaveSpectrumPhyInterface->SetDevice (device);
    }

    Ptr<SpectrumValue>
    JammerPhy::GetTxPowerSpectralDensity (uint16_t centerFrequency, uint16_t channelWidth, double txPowerW, MmWaveModulationClass modulationClass) const
    {
        NS_LOG_FUNCTION (centerFrequency << channelWidth << txPowerW);
        Ptr<SpectrumValue> v;
        switch (modulationClass)
        {
            case MMWAVE_MOD_CLASS_OFDM:
                v = MmWaveSpectrumValueHelper::CreateMmWaveOfdmTxPowerSpectralDensity (centerFrequency, channelWidth, txPowerW,
                                                                                       GetGuardBandwidth (channelWidth),
                                                                                       m_txMaskInnerBandMinimumRejection,
                                                                                       m_txMaskOuterBandMinimumRejection,
                                                                                       m_txMaskOuterBandMaximumRejection);
                break;
            default:
                NS_FATAL_ERROR ("modulation class unknown: " << modulationClass);
                break;
        }
        return v;
    }

    uint16_t
    JammerPhy::GetCenterFrequencyForChannelWidth (MmWaveTxVector txVector) const
    {
        NS_LOG_FUNCTION (this << txVector);
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
    JammerPhy::StartTx (Ptr<MmWavePpdu> ppdu)
    {
        NS_LOG_FUNCTION (this << ppdu);
        ppdu->SetUnrecognizedSignal ();
        MmWaveTxVector txVector = ppdu->GetTxVector ();
        double txPowerDbm = GetTxPowerForTransmission (txVector) + GetTxGain ();
        NS_LOG_DEBUG ("Start transmission: signal power before antenna gain=" << txPowerDbm << "dBm");
        double txPowerWatts = DbmToW (txPowerDbm);
        Ptr<SpectrumValue> txPowerSpectrum = GetTxPowerSpectralDensity (GetCenterFrequencyForChannelWidth (txVector), txVector.GetChannelWidth (), txPowerWatts, ppdu->GetModulation ());
        Ptr<MmWaveSpectrumSignalParameters> txParams = Create<MmWaveSpectrumSignalParameters> ();
        txParams->duration = ppdu->GetTxDuration ();
        txParams->psd = txPowerSpectrum;
        NS_ASSERT_MSG (m_mmWaveSpectrumPhyInterface, "SpectrumPhy() is not set; maybe forgot to call CreateMmWaveSpectrumPhyInterface?");
        txParams->txPhy = m_mmWaveSpectrumPhyInterface->GetObject<SpectrumPhy> ();
        txParams->txAntenna = m_antenna;
        txParams->ppdu = ppdu;
        NS_LOG_DEBUG ("Starting transmission with power " << WToDbm (txPowerWatts) << " dBm on channel " << +GetChannelNumber ());
        NS_LOG_DEBUG ("Starting transmission with integrated spectrum power " << WToDbm (Integral (*txPowerSpectrum)) << " dBm; spectrum model Uid: " << txPowerSpectrum->GetSpectrumModel ()->GetUid ());
        m_channel->StartTx (txParams);
    }

    uint32_t
    JammerPhy::GetBandBandwidth () const
    {
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
    JammerPhy::GetGuardBandwidth (uint16_t currentChannelWidth) const
    {
        return currentChannelWidth;
    }

    MmWaveSpectrumBand
    JammerPhy::GetBand (uint16_t bandWidth, uint8_t bandIndex)
    {
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

    void
    JammerPhy::SetMac (Ptr<JammerMac> mac)
    {
        m_mac = mac;
    }

    void
    JammerPhy::SendSignal (Ptr<const MmWavePsdu> p, MmWaveTxVector txVector)
    {
        NS_LOG_FUNCTION (this << *p << txVector);
        MmWaveConstPsduMap psdus;
        psdus.insert (std::make_pair (SU_STA_ID, p));

        NS_ASSERT (!m_state->IsStateTx () && !m_state->IsStateSwitching ());
        NS_ASSERT (m_endTxEvent.IsExpired ());

        if (txVector.GetNssMax () > GetMaxSupportedTxSpatialStreams ())
        {
            NS_FATAL_ERROR ("Unsupported number of spatial streams!");
        }

        if (m_state->IsStateSleep ())
        {
            NS_LOG_DEBUG ("Dropping packet because in sleep mode");
            for (auto const& psdu : psdus)
            {
                NotifyTxDrop (psdu.second);
            }
            return;
        }

        Time txDuration = CalculateTxDuration (psdus, txVector, GetPhyBand ());
        if ((m_currentEvent != 0) && (m_currentEvent->GetEndTime () > (Simulator::Now () + m_state->GetDelayUntilIdle ())))
        {
            //that packet will be noise _after_ the transmission.
            MaybeCcaBusyDuration ();
        }

        if (m_currentEvent != 0)
        {
            AbortCurrentReception (MMWAVE_RECEPTION_ABORTED_BY_TX);
        }

        if (m_powerRestricted)
        {
            NS_LOG_DEBUG ("Transmitting with power restriction");
        }
        else
        {
            NS_LOG_DEBUG ("Transmitting without power restriction");
        }

        if (m_state->GetState () == MmWavePhyState::MMWAVE_OFF)
        {
            NS_LOG_DEBUG ("Transmission canceled because device is OFF");
            return;
        }

        double txPowerW = DbmToW (GetTxPowerForTransmission (txVector) + GetTxGain ());
        NotifyTxBegin (psdus, txPowerW);
        m_phyTxPsduBeginTrace (psdus, txVector, txPowerW);
        for (auto const& psdu : psdus)
        {
            NotifyMonitorSniffTx (psdu.second, GetFrequency (), txVector);
        }
        m_state->SwitchToTx (txDuration, psdus, GetPowerDbm (txVector.GetTxPowerLevel ()), txVector);
        Ptr<MmWavePpdu> ppdu = Create<MmWavePpdu> (psdus, txVector, txDuration, GetPhyBand ());
        //m_endTxEvent = Simulator::Schedule (txDuration, &MmWavePhy::NotifyTxEnd, this, psdus);
        m_endTxSignal = Simulator::Schedule (txDuration, &JammerMac::NotifyTxEnd, m_mac);
        StartTx (ppdu);
        NS_ASSERT (m_endTxSignal.IsRunning ());
        NS_ASSERT (m_mac != 0);
        NS_ASSERT (txDuration.IsPositive ());
        m_channelAccessRequested = false;
        m_powerRestricted = false;
    }
}