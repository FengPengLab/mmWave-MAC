/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include <algorithm>
#include "ns3/simulator.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/pointer.h"
#include "ns3/nstime.h"
#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/mobility-model.h"
#include "mmwave-phy.h"
#include "mmwave-psdu.h"
#include "mmwave-error-rate-model.h"
#include "mmwave-frame-capture-model.h"
#include "mmwave-preamble-detection-model.h"
#include "mmwave-phy-state-helper.h"

#undef NS_LOG_APPEND_CONTEXT
#define NS_LOG_APPEND_CONTEXT std::clog << "[" << m_typeOfGroup << "] "
namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("MmWavePhy");
    NS_OBJECT_ENSURE_REGISTERED (MmWavePhy);

    TypeId
    MmWavePhy::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::MmWavePhy")
                .SetParent<Object> ()
                .SetGroupName ("MmWave")
                .AddAttribute ("Frequency",
                               "The operating center frequency (MHz)",
                               UintegerValue (0),
                               MakeUintegerAccessor (&MmWavePhy::GetFrequency,
                                                     &MmWavePhy::SetFrequency),
                               MakeUintegerChecker<uint16_t> ())
                .AddAttribute ("ChannelWidth",
                               "Whether 80MHz, 160MHz, 320MHz, 640MHz.",
                               UintegerValue (80),
                               MakeUintegerAccessor (&MmWavePhy::GetChannelWidth,
                                                     &MmWavePhy::SetChannelWidth),
                               MakeUintegerChecker<uint16_t> (80, 640))
                .AddAttribute ("ChannelNumber",
                               "If set to non-zero defined value, will control Frequency and ChannelWidth assignment",
                               UintegerValue (0),
                               MakeUintegerAccessor (&MmWavePhy::SetChannelNumber,
                                                     &MmWavePhy::GetChannelNumber),
                               MakeUintegerChecker<uint8_t> (0, 100))
                .AddAttribute ("RxSensitivity",
                               "The energy of a received signal should be higher than "
                               "this threshold (dBm) for the PHY to detect the signal.",
                               DoubleValue (-101.0),
                               MakeDoubleAccessor (&MmWavePhy::SetRxSensitivity,
                                                   &MmWavePhy::GetRxSensitivity),
                               MakeDoubleChecker<double> ())
                .AddAttribute ("CcaEdThreshold",
                               "The energy of a non Wi-Fi received signal should be higher than "
                               "this threshold (dBm) to allow the PHY layer to declare CCA BUSY state. "
                               "This check is performed on the 20 MHz primary channel only.",
                               DoubleValue (-62.0),
                               MakeDoubleAccessor (&MmWavePhy::SetCcaEdThreshold,
                                                   &MmWavePhy::GetCcaEdThreshold),
                               MakeDoubleChecker<double> ())
                .AddAttribute ("TxGain",
                               "Transmission gain (dB).",
                               DoubleValue (0.0),
                               MakeDoubleAccessor (&MmWavePhy::SetTxGain,
                                                   &MmWavePhy::GetTxGain),
                               MakeDoubleChecker<double> ())
                .AddAttribute ("RxGain",
                               "Reception gain (dB).",
                               DoubleValue (0.0),
                               MakeDoubleAccessor (&MmWavePhy::SetRxGain,
                                                   &MmWavePhy::GetRxGain),
                               MakeDoubleChecker<double> ())
                .AddAttribute ("TxPowerLevels",
                               "Number of transmission power levels available between "
                               "TxPowerStart and TxPowerEnd included.",
                               UintegerValue (1),
                               MakeUintegerAccessor (&MmWavePhy::m_nTxPower),
                               MakeUintegerChecker<uint8_t> ())
                .AddAttribute ("TxPowerEnd",
                               "Maximum available transmission level (dBm).",
                               DoubleValue (26),
                               MakeDoubleAccessor (&MmWavePhy::SetTxPowerEnd,
                                                   &MmWavePhy::GetTxPowerEnd),
                               MakeDoubleChecker<double> ())
                .AddAttribute ("TxPowerStart",
                               "Minimum available transmission level (dBm).",
                               DoubleValue (26),
                               MakeDoubleAccessor (&MmWavePhy::SetTxPowerStart,
                                                   &MmWavePhy::GetTxPowerStart),
                               MakeDoubleChecker<double> ())
                .AddAttribute ("RxNoiseFigure",
                               "Loss (dB) in the Signal-to-Noise-Ratio due to non-idealities in the receiver."
                               " According to Wikipedia (http://en.wikipedia.org/wiki/Noise_figure), this is "
                               "\"the difference in decibels (dB) between"
                               " the noise output of the actual receiver to the noise output of an "
                               " ideal receiver with the same overall gain and bandwidth when the receivers "
                               " are connected to sources at the standard noise temperature T0 (usually 290 K)\".",
                               DoubleValue (7),
                               MakeDoubleAccessor (&MmWavePhy::SetRxNoiseFigure),
                               MakeDoubleChecker<double> ())
                .AddAttribute ("State",
                               "The state of the PHY layer.",
                               PointerValue (),
                               MakePointerAccessor (&MmWavePhy::m_state),
                               MakePointerChecker<MmWavePhyStateHelper> ())
                .AddAttribute ("ChannelSwitchDelay",
                               "Delay between two short frames transmitted on different frequencies.",
                               TimeValue (MicroSeconds (250)),
                               MakeTimeAccessor (&MmWavePhy::m_channelSwitchDelay),
                               MakeTimeChecker ())
                .AddAttribute ("Antennas",
                               "The number of antennas on the device.",
                               UintegerValue (8),
                               MakeUintegerAccessor (&MmWavePhy::GetNumberOfAntennas,
                                                     &MmWavePhy::SetNumberOfAntennas),
                               MakeUintegerChecker<uint8_t> (1, 8))
                .AddAttribute ("MaxSupportedTxSpatialStreams",
                               "The maximum number of supported TX spatial streams.",
                               UintegerValue (8),
                               MakeUintegerAccessor (&MmWavePhy::GetMaxSupportedTxSpatialStreams,
                                                     &MmWavePhy::SetMaxSupportedTxSpatialStreams),
                               MakeUintegerChecker<uint8_t> (1, 8))
                .AddAttribute ("MaxSupportedRxSpatialStreams",
                               "The maximum number of supported RX spatial streams.",
                               UintegerValue (8),
                               MakeUintegerAccessor (&MmWavePhy::GetMaxSupportedRxSpatialStreams,
                                                     &MmWavePhy::SetMaxSupportedRxSpatialStreams),
                               MakeUintegerChecker<uint8_t> (1, 8))
                .AddAttribute ("FrameCaptureModel",
                               "Ptr to an object that implements the frame capture model",
                               PointerValue (),
                               MakePointerAccessor (&MmWavePhy::m_frameCaptureModel),
                               MakePointerChecker <MmWaveFrameCaptureModel> ())
                .AddAttribute ("PreambleDetectionModel",
                               "Ptr to an object that implements the preamble detection model",
                               PointerValue (),
                               MakePointerAccessor (&MmWavePhy::m_preambleDetectionModel),
                               MakePointerChecker <MmWavePreambleDetectionModel> ())
                .AddAttribute ("PostReceptionErrorModel",
                               "An optional packet error model can be added to the receive "
                               "packet process after any propagation-based (SNR-based) error "
                               "models have been applied. Typically this is used to force "
                               "specific packet drops, for testing purposes.",
                               PointerValue (),
                               MakePointerAccessor (&MmWavePhy::m_postReceptionErrorModel),
                               MakePointerChecker<ErrorModel> ())
                .AddAttribute ("Sifs",
                               "The duration of the Short Interframe Space. "
                               "NOTE that the default value is overwritten by the value defined "
                               "by the standard; if you want to set this attribute, you have to "
                               "do it after that the PHY object is initialized.",
                               TimeValue (MicroSeconds (0)),
                               MakeTimeAccessor (&MmWavePhy::m_sifs),
                               MakeTimeChecker ())
                .AddAttribute ("Slot",
                               "The duration of a slot. "
                               "NOTE that the default value is overwritten by the value defined "
                               "by the standard; if you want to set this attribute, you have to "
                               "do it after that the PHY object is initialized.",
                               TimeValue (MicroSeconds (0)),
                               MakeTimeAccessor (&MmWavePhy::m_slot),
                               MakeTimeChecker ())
                .AddTraceSource ("PhyTxBegin",
                                 "Trace source indicating a packet "
                                 "has begun transmitting over the channel medium",
                                 MakeTraceSourceAccessor (&MmWavePhy::m_phyTxBeginTrace),
                                 "ns3::Packet::TracedCallback")
                .AddTraceSource ("PhyTxPsduBegin",
                                 "Trace source indicating a PSDU "
                                 "has begun transmitting over the channel medium",
                                 MakeTraceSourceAccessor (&MmWavePhy::m_phyTxPsduBeginTrace),
                                 "ns3::MmWavePhy::PsduTxBeginCallback")
                .AddTraceSource ("PhyTxEnd",
                                 "Trace source indicating a packet "
                                 "has been completely transmitted over the channel.",
                                 MakeTraceSourceAccessor (&MmWavePhy::m_phyTxEndTrace),
                                 "ns3::Packet::TracedCallback")
                .AddTraceSource ("PhyTxDrop",
                                 "Trace source indicating a packet "
                                 "has been dropped by the device during transmission",
                                 MakeTraceSourceAccessor (&MmWavePhy::m_phyTxDropTrace),
                                 "ns3::Packet::TracedCallback")
                .AddTraceSource ("PhyRxBegin",
                                 "Trace source indicating a packet "
                                 "has begun being received from the channel medium "
                                 "by the device",
                                 MakeTraceSourceAccessor (&MmWavePhy::m_phyRxBeginTrace),
                                 "ns3::Packet::TracedCallback")
                .AddTraceSource ("PhyRxPayloadBegin",
                                 "Trace source indicating the reception of the "
                                 "payload of a PPDU has begun",
                                 MakeTraceSourceAccessor (&MmWavePhy::m_phyRxPayloadBeginTrace),
                                 "ns3::MmWavePhy::PhyRxPayloadBeginTracedCallback")
                .AddTraceSource ("PhyRxEnd",
                                 "Trace source indicating a packet "
                                 "has been completely received from the channel medium "
                                 "by the device",
                                 MakeTraceSourceAccessor (&MmWavePhy::m_phyRxEndTrace),
                                 "ns3::Packet::TracedCallback")
                .AddTraceSource ("PhyRxDrop",
                                 "Trace source indicating a packet "
                                 "has been dropped by the device during reception",
                                 MakeTraceSourceAccessor (&MmWavePhy::m_phyRxDropTrace),
                                 "ns3::Packet::TracedCallback")
                .AddTraceSource ("MonitorSnifferRx",
                                 "Trace source simulating a MmWave device in monitor mode "
                                 "sniffing all received frames",
                                 MakeTraceSourceAccessor (&MmWavePhy::m_phyMonitorSniffRxTrace),
                                 "ns3::MmWavePhy::MonitorSnifferRxTracedCallback")
                .AddTraceSource ("MonitorSnifferTx",
                                 "Trace source simulating the capability of a MmWave device "
                                 "in monitor mode to sniff all frames being transmitted",
                                 MakeTraceSourceAccessor (&MmWavePhy::m_phyMonitorSniffTxTrace),
                                 "ns3::MmWavePhy::MonitorSnifferTxTracedCallback")
                .AddTraceSource ("EndOfMmWavePreamble",
                                 "Trace source indicating the end of the preamble (after training fields)",
                                 MakeTraceSourceAccessor (&MmWavePhy::m_phyEndOfMmWavePreambleTrace),
                                 "ns3::MmWavePhy::EndOfMmWavePreambleTracedCallback")
        ;
        return tid;
    }

    MmWavePhy::MmWavePhy ()
            : m_txMpduReferenceNumber (0xffffffff),
              m_rxMpduReferenceNumber (0xffffffff),
              m_endRxEvent (),
              m_endPhyRxEvent (),
              m_endPreambleDetectionEvent (),
              m_endTxEvent (),
              m_standard (MMWAVE_PHY_STANDARD_UNSPECIFIED),
              m_band (MMWAVE_PHY_BAND_UNSPECIFIED),
              m_isConstructed (false),
              m_channelCenterFrequency (0),
              m_initialFrequency (0),
              m_frequencyChannelNumberInitialized (false),
              m_channelWidth (0),
              m_sifs (Seconds (0.0)),
              m_slot (Seconds (0.0)),
              m_timeLastPreambleDetected (Seconds (0.0)),
              m_powerRestricted (false),
              m_channelAccessRequested (false),
              m_signalDetectionMode (false),
              m_txSpatialStreams (0),
              m_rxSpatialStreams (0),
              m_channelNumber (0),
              m_initialChannelNumber (0),
              m_currentEvent (0)
    {
        NS_LOG_FUNCTION (this);
        m_rng = CreateObject<UniformRandomVariable> ();
        m_state = CreateObject<MmWavePhyStateHelper> ();
    }

    MmWavePhy::~MmWavePhy ()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    MmWavePhy::DoDispose ()
    {
        NS_LOG_FUNCTION (this);
        m_endTxEvent.Cancel ();
        m_endRxEvent.Cancel ();
        m_endPhyRxEvent.Cancel ();
        m_endPreambleDetectionEvent.Cancel ();
        m_device = 0;
        m_mobility = 0;
        m_state = 0;
        m_postReceptionErrorModel = 0;
        m_deviceMcsSet.clear ();
        m_mcsIndexMap.clear ();
    }

    void
    MmWavePhy::DoInitialize ()
    {
        NS_LOG_FUNCTION (this);
    }

    MmWaveMode
    MmWavePhy::GetMmWaveMcs (uint8_t mcs)
    {
        MmWaveMode mode;
        switch (mcs)
        {
            case 0:
                mode = MmWavePhy::GetMmWaveMcs0 ();
                break;
            case 1:
                mode = MmWavePhy::GetMmWaveMcs1 ();
                break;
            case 2:
                mode = MmWavePhy::GetMmWaveMcs2 ();
                break;
            case 3:
                mode = MmWavePhy::GetMmWaveMcs3 ();
                break;
            default:
                NS_ABORT_MSG ("Invalid MmWave MCS");
                break;
        }
        return mode;
    }

    MmWaveMode
    MmWavePhy::GetPhyHeaderMcsMode ()
    {
        return MmWavePhy::GetMmWaveMcs0 ();
    }

    MmWaveMode
    MmWavePhy::GetPhyHeaderMode ()
    {
        return MmWavePhy::GetMmWaveMcs0 ();
    }

    MmWaveMode
    MmWavePhy::GetMcs (uint8_t mcs) const
    {
        return m_deviceMcsSet[mcs];
    }

    MmWaveMode
    MmWavePhy::GetMcs (MmWaveModulationClass modulation, uint8_t mcs) const
    {
        NS_ASSERT_MSG (IsMcsSupported (modulation, mcs), "Unsupported MCS");
        uint8_t index = m_mcsIndexMap.at (modulation).at (mcs);
        NS_ASSERT (index < m_deviceMcsSet.size ());
        MmWaveMode mode = m_deviceMcsSet[index];
        NS_ASSERT (mode.GetModulationClass () == modulation);
        NS_ASSERT (mode.GetMcsValue () == mcs);
        return mode;
    }

    void
    MmWavePhy::SetReceiveOkCallback (Callback<void, Ptr<MmWavePsdu>, double, Time, Time, MmWaveTxVector> callback)
    {
        m_state->SetReceiveOkCallback (callback);
    }

    void
    MmWavePhy::SetReceiveErrorCallback (Callback<void, Ptr<MmWavePsdu>, double, Time, Time, MmWaveTxVector> callback)
    {
        m_state->SetReceiveErrorCallback (callback);
    }

    void
    MmWavePhy::SetUnrecognizedSignalDetectedCallback (Callback<void, double, double, Time, Time> callback)
    {
        m_unrecognizedSignalDetected = callback;
    }

    void
    MmWavePhy::SetRecognizedSignalDetectedCallback (Callback<void, double, double, Time, Time> callback)
    {
        m_recognizedSignalDetected = callback;
    }

    void
    MmWavePhy::RegisterListener (MmWavePhyListener *listener)
    {
        m_state->RegisterListener (listener);
    }

    void
    MmWavePhy::UnregisterListener (MmWavePhyListener *listener)
    {
        m_state->UnregisterListener (listener);
    }

    void
    MmWavePhy::SetCapabilitiesChangedCallback (Callback<void> callback)
    {
        m_capabilitiesChangedCallback = callback;
    }

    Time
    MmWavePhy::CalculateTxDuration (uint32_t size, MmWaveTxVector txVector, MmWavePhyBand band)
    {
        Time duration = CalculatePhyPreambleAndHeaderDuration (txVector) + GetPayloadDuration (size, txVector, band, MMWAVE_NORMAL_MPDU);
        NS_ASSERT (duration.IsStrictlyPositive ());
        return duration;
    }

    Time
    MmWavePhy::CalculateTxDuration (MmWaveConstPsduMap psduMap, MmWaveTxVector txVector, MmWavePhyBand band)
    {
        Time maxDuration = Seconds (0.0);
        for (auto & staIdPsdu : psduMap)
        {
            Time current = CalculateTxDuration (staIdPsdu.second->GetSize (), txVector, band);
            if (current > maxDuration)
            {
                maxDuration = current;
            }
        }
        NS_ASSERT (maxDuration.IsStrictlyPositive ());
        return maxDuration;
    }

    Time
    MmWavePhy::CalculatePhyPreambleAndHeaderDuration (MmWaveTxVector txVector)
    {
        Time duration = GetPhyPreambleDuration (txVector) + GetPhyHeaderDuration (txVector);
        return duration;
    }

    Time
    MmWavePhy::GetPreambleDetectionDuration ()
    {
        // short training field (STF)
        return NanoSeconds (1000);
    }

    Time
    MmWavePhy::GetPhyPreambleDuration (MmWaveTxVector txVector)
    {
        switch (txVector.GetPreambleType ())
        {
            case MMWAVE_PREAMBLE_DEFAULT:
                // The preamble contains a short training field (STF) and channel estimation field (CEF).
                return NanoSeconds (1000);
            case MMWAVE_PREAMBLE_UNSPECIFIED:
                return NanoSeconds (0);
            default:
                NS_FATAL_ERROR ("unsupported preamble type");
                return NanoSeconds (0);
        }
    }

    Time
    MmWavePhy::GetPhyHeaderDuration (MmWaveTxVector txVector)
    {
        switch (txVector.GetPreambleType ())
        {
            case MMWAVE_PREAMBLE_DEFAULT:
                return NanoSeconds (400);
            case MMWAVE_PREAMBLE_UNSPECIFIED:
                return NanoSeconds (0);
            default:
                NS_FATAL_ERROR ("unsupported preamble type");
                return NanoSeconds (0);
        }
    }

    Time
    MmWavePhy::GetPayloadDuration (uint32_t size, MmWaveTxVector txVector, MmWavePhyBand band, MmWaveMpduType mpdutype)
    {
        uint32_t totalAmpduSize;
        double totalAmpduNumSymbols;
        return GetPayloadDuration (size, txVector, band, mpdutype, false, totalAmpduSize, totalAmpduNumSymbols);
    }

    Time
    MmWavePhy::GetPayloadDuration (uint32_t size, MmWaveTxVector txVector, MmWavePhyBand band, MmWaveMpduType mpdutype,
                                   bool incFlag, uint32_t &totalAmpduSize, double &totalAmpduNumSymbols)
    {
        NS_ASSERT (mpdutype == MMWAVE_NORMAL_MPDU);
        MmWaveMode payloadMode = txVector.GetMode ();
        double stbc = 1;
        double Nes = 1;
        uint16_t gi = txVector.GetGuardInterval ();
        NS_ASSERT (gi == 800 || gi == 1600 || gi == 3200);
        Time symbolDuration = NanoSeconds (12800 + gi);
        double numDataBitsPerSymbol = payloadMode.GetDataRate (txVector) * symbolDuration.GetNanoSeconds () / 1e9;
        double numSymbols = lrint (stbc * ceil ((16 + size * 8.0 + 6.0 * Nes) / (stbc * numDataBitsPerSymbol)));

        switch (payloadMode.GetModulationClass ())
        {
            case MMWAVE_MOD_CLASS_OFDM:
                return FemtoSeconds (static_cast<uint64_t> (numSymbols * symbolDuration.GetFemtoSeconds ()));
            default:
                NS_FATAL_ERROR ("unsupported modulation class");
                return MicroSeconds (0);
        }
    }

    Ptr<const MmWavePsdu>
    MmWavePhy::GetAddressedPsduInPpdu (Ptr<const MmWavePpdu> ppdu)
    {
        NS_LOG_FUNCTION (this);
        Ptr<const MmWavePsdu> psdu;
        psdu = ppdu->GetPsdu ();
        return psdu;
    }

    void
    MmWavePhy::SetChannelNumber (uint8_t nch)
    {
        NS_LOG_FUNCTION (this << +nch);
        if (m_isConstructed == false)
        {
            NS_LOG_DEBUG ("Saving channel number configuration for initialization");
            m_initialChannelNumber = nch;
            return;
        }
        if (GetChannelNumber () == nch)
        {
            NS_LOG_DEBUG ("No channel change requested");
            return;
        }
        if (nch == 0)
        { 
            NS_LOG_DEBUG ("Setting channel number to zero");
            m_channelNumber = 0;
            return;
        }
 
        MmWaveFrequencyWidthPair f = GetFrequencyWidthForChannelNumberStandard (nch, GetPhyBand (), GetPhyStandard ());
        if (f.first == 0)
        {
            f = GetFrequencyWidthForChannelNumberStandard (nch, GetPhyBand (), MMWAVE_PHY_STANDARD_UNSPECIFIED);
        }
        if (f.first != 0)
        {
            if (DoChannelSwitch (nch))
            {
                NS_LOG_DEBUG ("Setting frequency to " << f.first << "; width to " << +f.second);
                m_channelCenterFrequency = f.first;
                SetChannelWidth (f.second);
                m_channelNumber = nch;
            }
            else
            {
                // Subclass may have suppressed (e.g. waiting for state change)
                NS_LOG_DEBUG ("Channel switch suppressed");
            }
        }
        else
        {
            NS_FATAL_ERROR ("Frequency not found for channel number " << +nch);
        }
    }
    
    void
    MmWavePhy::SetFrequency (uint16_t frequency)
    {
        NS_LOG_FUNCTION (this << frequency);
        if (m_isConstructed == false)
        {
            NS_LOG_DEBUG ("Saving frequency configuration for initialization");
            m_initialFrequency = frequency;
            return;
        }
        if (GetFrequency () == frequency)
        {
            NS_LOG_DEBUG ("No frequency change requested");
            return;
        }
        if (frequency == 0)
        {
            DoFrequencySwitch (0);
            NS_LOG_DEBUG ("Setting frequency and channel number to zero");
            m_channelCenterFrequency = 0;
            m_channelNumber = 0;
            return;
        }
        
        uint8_t nch = FindChannelNumberForFrequencyWidth (frequency, GetChannelWidth ());
        if (nch != 0)
        {
            NS_LOG_DEBUG ("Setting frequency " << frequency << " corresponds to channel " << +nch);
            if (DoFrequencySwitch (frequency))
            {
                NS_LOG_DEBUG ("Channel frequency switched to " << frequency << "; channel number to " << +nch);
                m_channelCenterFrequency = frequency;
                m_channelNumber = nch;
            }
            else
            {
                NS_LOG_DEBUG ("Suppressing reassignment of frequency");
            }
        }
        else
        {
            NS_LOG_DEBUG ("Channel number is unknown for frequency " << frequency);
            if (DoFrequencySwitch (frequency))
            {
                NS_LOG_DEBUG ("Channel frequency switched to " << frequency << "; channel number to " << 0);
                m_channelCenterFrequency = frequency;
                m_channelNumber = 0;
            }
            else
            {
                NS_LOG_DEBUG ("Suppressing reassignment of frequency");
            }
        }
    }

    void
    MmWavePhy::SetChannelWidth (uint16_t channelWidth)
    {
        NS_LOG_FUNCTION (this << channelWidth);
        bool changed = (m_channelWidth != channelWidth);
        m_channelWidth = channelWidth;
        AddSupportedChannelWidth (channelWidth);
        if (changed && !m_capabilitiesChangedCallback.IsNull ())
        {
            m_capabilitiesChangedCallback ();
        }
    }

    void
    MmWavePhy::ConfigureStandardAndBand (MmWavePhyStandard standard, MmWavePhyBand band)
    {
        NS_LOG_FUNCTION (this << standard << band);
        m_standard = standard;
        m_band = band;
        NS_ASSERT (m_standard != MMWAVE_PHY_STANDARD_UNSPECIFIED);
        NS_ASSERT (m_band != MMWAVE_PHY_BAND_UNSPECIFIED);
        m_isConstructed = true;
        for (auto i = mmWaveChannelToFrequency.begin (); i != mmWaveChannelToFrequency.end (); i++)
        {
            if (((*i).first.second == m_standard) && ((*i).first.first.second == m_band))
            {
                m_channelToFrequency[(*i).first] = (*i).second;
            }
        }

        if (m_frequencyChannelNumberInitialized == false)
        {
            InitializeFrequencyChannelNumber ();
        }

        if (GetFrequency () == 0 && GetChannelNumber () == 0)
        {
            ConfigureDefaultsForStandard ();
        }
        else
        {
            ConfigureChannelForStandard ();
        }

        SetSifs (NanoSeconds (3000));
        SetSlot (NanoSeconds (6000));

        PushMcs (MmWavePhy::GetMmWaveMcs0 ());
        PushMcs (MmWavePhy::GetMmWaveMcs1 ());
        PushMcs (MmWavePhy::GetMmWaveMcs2 ());
        PushMcs (MmWavePhy::GetMmWaveMcs3 ());
    }

    MmWaveSpectrumBand
    MmWavePhy::GetBand (uint16_t /*bandWidth*/, uint8_t /*bandIndex*/)
    {
        MmWaveSpectrumBand band;
        band.first = 0;
        band.second = 0;
        return band;
    }

    int64_t
    MmWavePhy::AssignStreams (int64_t stream)
    {
        NS_LOG_FUNCTION (this << stream);
        m_rng->SetStream (stream);
        return 1;
    }

    void
    MmWavePhy::SetSifs (Time sifs)
    {
        m_sifs = sifs;
    }

    Time
    MmWavePhy::GetSifs () const
    {
        return m_sifs;
    }

    void
    MmWavePhy::SetSlot (Time slot)
    {
        m_slot = slot;
    }

    Time
    MmWavePhy::GetSlot () const
    {
        return m_slot;
    }

    Time
    MmWavePhy::GetChannelSwitchDelay () const
    {
        return m_channelSwitchDelay;
    }

    Time
    MmWavePhy::GetDelayUntilIdle () const
    {
        return m_state->GetDelayUntilIdle ();
    }

    Time
    MmWavePhy::GetLastRxStartTime () const
    {
        return m_state->GetLastRxStartTime ();
    }

    Time
    MmWavePhy::GetLastRxEndTime () const
    {
        return m_state->GetLastRxEndTime ();
    }

    MmWavePhyStandard
    MmWavePhy::GetPhyStandard () const
    {
        return m_standard;
    }

    MmWavePhyBand
    MmWavePhy::GetPhyBand () const
    {
        return m_band;
    }

    void
    MmWavePhy::SetRxSensitivity (double threshold)
    {
        NS_LOG_FUNCTION (this << threshold);
        m_rxSensitivityW = DbmToW (threshold);
    }

    void
    MmWavePhy::SetCcaEdThreshold (double threshold)
    {
        NS_LOG_FUNCTION (this << threshold);
        m_ccaEdThresholdW = DbmToW (threshold);
    }

    void
    MmWavePhy::SetRxNoiseFigure (double noiseFigureDb)
    {
        NS_LOG_FUNCTION (this << noiseFigureDb);
        m_interference.SetNoiseFigure (DbToRatio (noiseFigureDb));
        m_interference.SetNumberOfReceiveAntennas (GetNumberOfAntennas ());
    }

    void
    MmWavePhy::SetTxPowerStart (double start)
    {
        NS_LOG_FUNCTION (this << start);
        m_txPowerBaseDbm = start;
    }

    void
    MmWavePhy::SetTxPowerEnd (double end)
    {
        NS_LOG_FUNCTION (this << end);
        m_txPowerEndDbm = end;
    }

    void
    MmWavePhy::SetNTxPower (uint8_t n)
    {
        NS_LOG_FUNCTION (this << +n);
        m_nTxPower = n;
    }

    void
    MmWavePhy::SetTxGain (double gain)
    {
        NS_LOG_FUNCTION (this << gain);
        m_txGainDb = gain;
    }

    void
    MmWavePhy::SetRxGain (double gain)
    {
        NS_LOG_FUNCTION (this << gain);
        m_rxGainDb = gain;
    }

    void
    MmWavePhy::SetDevice (const Ptr<NetDevice> device)
    {
        NS_LOG_FUNCTION (this);
        m_device = device;
    }

    void
    MmWavePhy::SetMobility (const Ptr<MobilityModel> mobility)
    {
        NS_LOG_FUNCTION (this);
        m_mobility = mobility;
    }

    void
    MmWavePhy::SetErrorRateModel (const Ptr<MmWaveErrorRateModel> rate)
    {
        NS_LOG_FUNCTION (this);
        m_interference.SetErrorRateModel (rate);
        m_interference.SetNumberOfReceiveAntennas (GetNumberOfAntennas ());
    }

    void
    MmWavePhy::SetPostReceptionErrorModel (const Ptr<ErrorModel> em)
    {
        NS_LOG_FUNCTION (this << em);
        m_postReceptionErrorModel = em;
    }

    void
    MmWavePhy::SetFrameCaptureModel (const Ptr<MmWaveFrameCaptureModel> model)
    {
        NS_LOG_FUNCTION (this);
        m_frameCaptureModel = model;
    }

    void
    MmWavePhy::SetPreambleDetectionModel (const Ptr<MmWavePreambleDetectionModel> model)
    {
        NS_LOG_FUNCTION (this);
        m_preambleDetectionModel = model;
    }

    void
    MmWavePhy::SetSignalDetectionMode ()
    {
        NS_LOG_FUNCTION (this);
        m_signalDetectionMode = true;
    }
    void
    MmWavePhy::ResetSignalDetectionMode ()
    {
        NS_LOG_FUNCTION (this);
        m_signalDetectionMode = false;
    }

    void
    MmWavePhy::SetNumberOfAntennas (uint8_t antennas)
    {
        NS_LOG_FUNCTION (this);
        m_numberOfAntennas = antennas;
        m_interference.SetNumberOfReceiveAntennas (antennas);
    }

    void
    MmWavePhy::SetMaxSupportedTxSpatialStreams (uint8_t streams)
    {
        NS_LOG_FUNCTION (this);
        NS_ASSERT (streams <= GetNumberOfAntennas ());
        bool changed = (m_txSpatialStreams != streams);
        m_txSpatialStreams = streams;
        if (changed && !m_capabilitiesChangedCallback.IsNull ())
        {
            m_capabilitiesChangedCallback ();
        }
    }

    void
    MmWavePhy::SetMaxSupportedRxSpatialStreams (uint8_t streams)
    {
        NS_LOG_FUNCTION (this);
        NS_ASSERT (streams <= GetNumberOfAntennas ());
        bool changed = (m_rxSpatialStreams != streams);
        m_rxSpatialStreams = streams;
        if (changed && !m_capabilitiesChangedCallback.IsNull ())
        {
            m_capabilitiesChangedCallback ();
        }
    }

    void
    MmWavePhy::AddSupportedChannelWidth (uint16_t width)
    {
        NS_LOG_FUNCTION (this << width);
        for (std::vector<uint32_t>::size_type i = 0; i != m_supportedChannelWidthSet.size (); i++)
        {
            if (m_supportedChannelWidthSet[i] == width)
            {
                return;
            }
        }
        NS_LOG_FUNCTION ("Adding " << width << " to supported channel width set");
        m_supportedChannelWidthSet.push_back (width);
    }

    void
    MmWavePhy::ResetCca (bool powerRestricted, double txPowerMaxSiso, double txPowerMaxMimo)
    {
        NS_LOG_FUNCTION (this << powerRestricted << txPowerMaxSiso << txPowerMaxMimo);
        m_powerRestricted = powerRestricted;
        m_txPowerMaxSiso = txPowerMaxSiso;
        m_txPowerMaxMimo = txPowerMaxMimo;
        NS_ASSERT ((m_currentEvent->GetEndTime () - Simulator::Now ()).IsPositive ());
        Simulator::Schedule (m_currentEvent->GetEndTime () - Simulator::Now (), &MmWavePhy::EndReceiveInterBss, this);
        AbortCurrentReception (MMWAVE_OBSS_PD_CCA_RESET);
    }

    void
    MmWavePhy::SwitchMaybeToCcaBusy ()
    {
        NS_LOG_FUNCTION (this);
        uint16_t primaryChannelWidth = MMWAVE_MIN_BANDWIDTH;
        Time delayUntilCcaEnd = m_interference.GetEnergyDuration (m_ccaEdThresholdW, GetBand (primaryChannelWidth));
        if (!delayUntilCcaEnd.IsZero ())
        {
            NS_LOG_DEBUG ("Calling SwitchMaybeToCcaBusy for " << delayUntilCcaEnd.As (Time::S));
            m_state->SwitchMaybeToCcaBusy (delayUntilCcaEnd);
        }
    }

    void
    MmWavePhy::InitializeFrequencyChannelNumber ()
    {
        NS_LOG_FUNCTION (this);
        NS_ASSERT_MSG (m_frequencyChannelNumberInitialized == false, "Initialization called twice");
        if (m_initialChannelNumber != 0)
        {
            SetChannelNumber (m_initialChannelNumber);
        }
        else if (m_initialFrequency != 0)
        {
            SetFrequency (m_initialFrequency);
        }
        m_frequencyChannelNumberInitialized = true;
    }

    void
    MmWavePhy::SetSleepMode ()
    {
        NS_LOG_FUNCTION (this);
        m_powerRestricted = false;
        m_channelAccessRequested = false;
        switch (m_state->GetState ())
        {
            case MmWavePhyState::MMWAVE_TX:
                NS_LOG_DEBUG ("setting sleep mode postponed until end of current transmission");
                Simulator::Schedule (GetDelayUntilIdle (), &MmWavePhy::SetSleepMode, this);
                break;
            case MmWavePhyState::MMWAVE_RX:
                NS_LOG_DEBUG ("setting sleep mode postponed until end of current reception");
                Simulator::Schedule (GetDelayUntilIdle (), &MmWavePhy::SetSleepMode, this);
                break;
            case MmWavePhyState::MMWAVE_SWITCHING:
                NS_LOG_DEBUG ("setting sleep mode postponed until end of channel switching");
                Simulator::Schedule (GetDelayUntilIdle (), &MmWavePhy::SetSleepMode, this);
                break;
            case MmWavePhyState::MMWAVE_CCA_BUSY:
            case MmWavePhyState::MMWAVE_IDLE:
                NS_LOG_DEBUG ("setting sleep mode");
                m_state->SwitchToSleep ();
                break;
            case MmWavePhyState::MMWAVE_SLEEP:
                NS_LOG_DEBUG ("already in sleep mode");
                break;
            default:
                NS_ASSERT (false);
                break;
        }
    }

    void
    MmWavePhy::SetOffMode ()
    {
        NS_LOG_FUNCTION (this);
        m_powerRestricted = false;
        m_channelAccessRequested = false;
        m_endPhyRxEvent.Cancel ();
        m_endRxEvent.Cancel ();
        m_endPreambleDetectionEvent.Cancel ();
        m_endTxEvent.Cancel ();
        m_state->SwitchToOff ();
    }

    void
    MmWavePhy::ResumeFromSleep ()
    {
        NS_LOG_FUNCTION (this);
        switch (m_state->GetState ())
        {
            case MmWavePhyState::MMWAVE_TX:
            case MmWavePhyState::MMWAVE_RX:
            case MmWavePhyState::MMWAVE_IDLE:
            case MmWavePhyState::MMWAVE_CCA_BUSY:
            case MmWavePhyState::MMWAVE_SWITCHING:
            {
                NS_LOG_DEBUG ("not in sleep mode, there is nothing to resume");
                break;
            }
            case MmWavePhyState::MMWAVE_SLEEP:
            {
                NS_LOG_DEBUG ("resuming from sleep mode");
                uint16_t primaryChannelWidth = MMWAVE_MIN_BANDWIDTH;
                Time delayUntilCcaEnd = m_interference.GetEnergyDuration (m_ccaEdThresholdW, GetBand (primaryChannelWidth));
                m_state->SwitchFromSleep (delayUntilCcaEnd);
                break;
            }
            default:
            {
                NS_ASSERT (false);
                break;
            }
        }
    }

    void
    MmWavePhy::ResumeFromOff ()
    {
        NS_LOG_FUNCTION (this);
        switch (m_state->GetState ())
        {
            case MmWavePhyState::MMWAVE_TX:
            case MmWavePhyState::MMWAVE_RX:
            case MmWavePhyState::MMWAVE_IDLE:
            case MmWavePhyState::MMWAVE_CCA_BUSY:
            case MmWavePhyState::MMWAVE_SWITCHING:
            case MmWavePhyState::MMWAVE_SLEEP:
            {
                NS_LOG_DEBUG ("not in off mode, there is nothing to resume");
                break;
            }
            case MmWavePhyState::MMWAVE_OFF:
            {
                NS_LOG_DEBUG ("resuming from off mode");
                uint16_t primaryChannelWidth = MMWAVE_MIN_BANDWIDTH;
                Time delayUntilCcaEnd = m_interference.GetEnergyDuration (m_ccaEdThresholdW, GetBand (primaryChannelWidth));
                m_state->SwitchFromOff (delayUntilCcaEnd);
                break;
            }
            default:
            {
                NS_ASSERT (false);
                break;
            }
        }
    }

    bool
    MmWavePhy::IsSignalDetectionMode () const
    {
        NS_LOG_FUNCTION (this);
        return m_signalDetectionMode;
    }

    bool
    MmWavePhy::IsStateCcaBusy () const
    {
        NS_LOG_FUNCTION (this);
        return m_state->IsStateCcaBusy ();
    }

    bool
    MmWavePhy::IsStateIdle () const
    {
        NS_LOG_FUNCTION (this);
        return m_state->IsStateIdle ();
    }

    bool
    MmWavePhy::IsStateRx () const
    {
        return m_state->IsStateRx ();
    }

    bool
    MmWavePhy::IsStateTx () const
    {
        NS_LOG_FUNCTION (this);
        return m_state->IsStateTx ();
    }

    bool
    MmWavePhy::IsStateSwitching () const
    {
        NS_LOG_FUNCTION (this);
        return m_state->IsStateSwitching ();
    }

    bool
    MmWavePhy::IsStateSleep () const
    {
        NS_LOG_FUNCTION (this);
        return m_state->IsStateSleep ();
    }

    bool
    MmWavePhy::IsStateOff () const
    {
        NS_LOG_FUNCTION (this);
        return m_state->IsStateOff ();
    }

    bool
    MmWavePhy::IsMcsSupported (MmWaveMode mcs) const
    {
        NS_LOG_FUNCTION (this);
        MmWaveModulationClass modulation = mcs.GetModulationClass ();
        if (modulation == MMWAVE_MOD_CLASS_OFDM)
        {
            return IsMcsSupported (modulation, mcs.GetMcsValue ());
        }
        return false;
    }

    bool
    MmWavePhy::IsMcsSupported (MmWaveModulationClass mc, uint8_t mcs) const
    {
        NS_LOG_FUNCTION (this);
        if (m_mcsIndexMap.find (mc) == m_mcsIndexMap.end ())
        {
            return false;
        }
        if (m_mcsIndexMap.at (mc).find (mcs) == m_mcsIndexMap.at (mc).end ())
        {
            return false;
        }
        return true;
    }

    bool
    MmWavePhy::DefineChannelNumber (uint8_t channelNumber, MmWavePhyBand band, MmWavePhyStandard standard, uint16_t frequency, uint16_t channelWidth)
    {
        NS_LOG_FUNCTION (this << +channelNumber << band << standard << frequency << channelWidth);
        MmWaveChannelNumberStandardPair p = std::make_pair (std::make_pair (channelNumber, band), standard);
        MmWaveChannelToFrequencyWidthMap::const_iterator it;
        it = m_channelToFrequency.find (p);
        if (it != m_channelToFrequency.end ())
        {
            NS_LOG_DEBUG ("channel number/standard already defined; returning false");
            return false;
        }
        MmWaveFrequencyWidthPair f = std::make_pair (frequency, channelWidth);
        m_channelToFrequency[p] = f;
        return true;
    }

    bool
    MmWavePhy::DoChannelSwitch (uint8_t nch)
    {
        NS_LOG_FUNCTION (this);
        m_powerRestricted = false;
        m_channelAccessRequested = false;
        if (!IsInitialized ())
        {
            //this is not channel switch, this is initialization
            NS_LOG_DEBUG ("initialize to channel " << +nch);
            return true;
        }

        NS_ASSERT (!IsStateSwitching ());
        switch (m_state->GetState ())
        {
            case MmWavePhyState::MMWAVE_RX:
                NS_LOG_DEBUG ("drop packet because of channel switching while reception");
                if (m_endPhyRxEvent.IsRunning ())
                {
                    m_endPhyRxEvent.Cancel ();
                }
                if (m_endRxEvent.IsRunning ())
                {
                    m_endRxEvent.Cancel ();
                }
                if (m_endPreambleDetectionEvent.IsRunning ())
                {
                    m_endPreambleDetectionEvent.Cancel ();
                }
                NS_ASSERT (m_endPhyRxEvent.IsExpired ());
                NS_ASSERT (m_endRxEvent.IsExpired ());
                NS_ASSERT (m_endPreambleDetectionEvent.IsExpired ());
                goto switchChannel;
                break;
            case MmWavePhyState::MMWAVE_TX:
                NS_LOG_DEBUG ("channel switching postponed until end of current transmission");
                Simulator::Schedule (GetDelayUntilIdle (), &MmWavePhy::SetChannelNumber, this, nch);
                break;
            case MmWavePhyState::MMWAVE_CCA_BUSY:
            case MmWavePhyState::MMWAVE_IDLE:
                NS_LOG_DEBUG ("cancel the timer because of channel switching while timer is running");
                if (m_endPhyRxEvent.IsRunning ())
                {
                    m_endPhyRxEvent.Cancel ();
                }
                if (m_endRxEvent.IsRunning ())
                {
                    m_endRxEvent.Cancel ();
                }
                if (m_endPreambleDetectionEvent.IsRunning ())
                {
                    m_endPreambleDetectionEvent.Cancel ();
                }
                NS_ASSERT (m_endPhyRxEvent.IsExpired ());
                NS_ASSERT (m_endRxEvent.IsExpired ());
                NS_ASSERT (m_endPreambleDetectionEvent.IsExpired ());
                goto switchChannel;
                break;
            case MmWavePhyState::MMWAVE_SLEEP:
                NS_LOG_DEBUG ("channel switching ignored in sleep mode");
                break;
            default:
                NS_ASSERT (false);
                break;
        }

        return false;

        switchChannel:

        NS_LOG_DEBUG ("switching channel " << +GetChannelNumber () << " -> " << +nch);
        m_state->SwitchToChannelSwitching (GetChannelSwitchDelay ());
        m_interference.EraseEvents ();

        return true;
    }

    bool
    MmWavePhy::DoFrequencySwitch (uint16_t frequency)
    {
        NS_LOG_FUNCTION (this);
        m_powerRestricted = false;
        m_channelAccessRequested = false;
        if (!IsInitialized ())
        {
            //this is not channel switch, this is initialization
            NS_LOG_DEBUG ("start at frequency " << frequency);
            return true;
        }

        NS_ASSERT (!IsStateSwitching ());
        switch (m_state->GetState ())
        {
            case MmWavePhyState::MMWAVE_RX:
                NS_LOG_DEBUG ("drop packet because of channel/frequency switching while reception");
                m_endPhyRxEvent.Cancel ();
                m_endRxEvent.Cancel ();
                m_endPreambleDetectionEvent.Cancel ();
                goto switchFrequency;
                break;
            case MmWavePhyState::MMWAVE_TX:
                NS_LOG_DEBUG ("channel/frequency switching postponed until end of current transmission");
                printf ("channel/frequency switching postponed until end of current transmission\n");
                Simulator::Schedule (GetDelayUntilIdle (), &MmWavePhy::SetFrequency, this, frequency);
                break;
            case MmWavePhyState::MMWAVE_CCA_BUSY:
            case MmWavePhyState::MMWAVE_IDLE:
                if (m_endPreambleDetectionEvent.IsRunning ())
                {
                    m_endPreambleDetectionEvent.Cancel ();
                    m_endRxEvent.Cancel ();
                }
                goto switchFrequency;
                break;
            case MmWavePhyState::MMWAVE_SLEEP:
                NS_LOG_DEBUG ("frequency switching ignored in sleep mode");
                break;
            default:
                NS_ASSERT (false);
                break;
        }

        return false;

        switchFrequency:

        NS_LOG_DEBUG ("switching frequency " << GetFrequency () << " -> " << frequency);
        m_state->SwitchToChannelSwitching (GetChannelSwitchDelay ());
        m_interference.EraseEvents ();
        return true;
    }

    uint8_t
    MmWavePhy::GetNMcs () const
    {
        return static_cast<uint8_t> (m_deviceMcsSet.size ());
    }

    uint8_t
    MmWavePhy::GetChannelNumber () const
    {
        return m_channelNumber;
    }

    uint8_t
    MmWavePhy::GetNTxPower () const
    {
        return m_nTxPower;
    }

    uint8_t
    MmWavePhy::GetNumberOfAntennas () const
    {
        return m_numberOfAntennas;
    }

    uint8_t
    MmWavePhy::GetMaxSupportedTxSpatialStreams () const
    {
        return m_txSpatialStreams;
    }

    uint8_t
    MmWavePhy::GetMaxSupportedRxSpatialStreams () const
    {
        return m_rxSpatialStreams;
    }

    uint8_t
    MmWavePhy::FindChannelNumberForFrequencyWidth (uint16_t frequency, uint16_t width) const
    {
        bool found = false;
        MmWaveFrequencyWidthPair f = std::make_pair (frequency, width);
        MmWaveChannelToFrequencyWidthMap::const_iterator it = m_channelToFrequency.begin ();
        while (it != m_channelToFrequency.end ())
        {
            if (it->second == f)
            {
                found = true;
                break;
            }
            ++it;
        }
        if (found)
        {
            NS_LOG_DEBUG ("Found, returning " << +it->first.first.first);
            return (it->first.first.first);
        }
        else
        {
            NS_LOG_DEBUG ("Not found, returning 0");
            return 0;
        }
    }

    uint16_t
    MmWavePhy::GetFrequency () const
    {
        return m_channelCenterFrequency;
    }

    uint16_t
    MmWavePhy::GetChannelWidth () const
    {
        return m_channelWidth;
    }


    double
    MmWavePhy::CalculateSnr (MmWaveTxVector txVector, double ber) const
    {
        return m_interference.GetErrorRateModel ()->CalculateSnr (txVector, ber);
    }

    double
    MmWavePhy::GetRxSensitivity () const
    {
        return WToDbm (m_rxSensitivityW);
    }

    double
    MmWavePhy::GetCcaEdThreshold () const
    {
        return WToDbm (m_ccaEdThresholdW);
    }

    double
    MmWavePhy::GetTxPowerStart () const
    {
        return m_txPowerBaseDbm;
    }

    double
    MmWavePhy::GetTxPowerEnd () const
    {
        return m_txPowerEndDbm;
    }

    double
    MmWavePhy::GetTxGain () const
    {
        return m_txGainDb;
    }

    double
    MmWavePhy::GetRxGain () const
    {
        return m_rxGainDb;
    }

    double
    MmWavePhy::GetTxPowerForTransmission (MmWaveTxVector txVector) const
    {
        if (!m_powerRestricted)
        {
            return GetPowerDbm (txVector.GetTxPowerLevel ());
        }
        else
        {
            if (txVector.GetNssMax () > 1)
            {
                return std::min (m_txPowerMaxMimo, GetPowerDbm (txVector.GetTxPowerLevel ()));
            }
            else
            {
                return std::min (m_txPowerMaxSiso, GetPowerDbm (txVector.GetTxPowerLevel ()));
            }
        }
    }

    double
    MmWavePhy::GetPowerDbm (uint8_t power) const
    {
        NS_ASSERT (m_txPowerBaseDbm <= m_txPowerEndDbm);
        NS_ASSERT (m_nTxPower > 0);
        double dbm;
        if (m_nTxPower > 1)
        {
            dbm = m_txPowerBaseDbm + power * (m_txPowerEndDbm - m_txPowerBaseDbm) / (m_nTxPower - 1);
        }
        else
        {
            NS_ASSERT_MSG (m_txPowerBaseDbm == m_txPowerEndDbm, "cannot have TxPowerEnd != TxPowerStart with TxPowerLevels == 1");
            dbm = m_txPowerBaseDbm;
        }
        return dbm;
    }

    double
    MmWavePhy::DbToRatio (double dB) const
    {
        return std::pow (10.0, 0.1 * dB);
    }

    double
    MmWavePhy::DbmToW (double dBm) const
    {
        return std::pow (10.0, 0.1 * (dBm - 30.0));
    }

    double
    MmWavePhy::WToDbm (double w) const
    {
        return 10.0 * std::log10 (w) + 30.0;
    }
    
    double
    MmWavePhy::RatioToDb (double ratio) const
    {
        return 10.0 * std::log10 (ratio);
    }
    
    Ptr<MmWavePhyStateHelper>
    MmWavePhy::GetState () const
    {
        return m_state;
    }

    Ptr<NetDevice>
    MmWavePhy::GetDevice () const
    {
        return m_device;
    }

    Ptr<MobilityModel>
    MmWavePhy::GetMobility () const
    {
        if (m_mobility != 0)
        {
            return m_mobility;
        }
        else
        {
            return m_device->GetNode ()->GetObject<MobilityModel> ();
        }
    }

    MmWaveChannelNumberStandardPair
    MmWavePhy::GetChannelFromChannelNumber (uint8_t num)
    {
        for (auto i = m_channelToFrequency.begin(); i != m_channelToFrequency.end(); i++)
        {
            if (num == i->first.first.first)
            {
                return i->first;
            }
        }
        return GetCurrentChannel ();
    }

    MmWaveChannelNumberStandardPair
    MmWavePhy::GetCurrentChannel () const
    {
        MmWaveChannelNumberStandardPair c = std::make_pair (std::make_pair(m_channelNumber, m_band), m_standard);
        return c;
    }

    MmWaveChannelToFrequencyWidthMap
    MmWavePhy::GetChannelToFrequency () const
    {
        return m_channelToFrequency;
    }

    MmWaveFrequencyWidthPair
    MmWavePhy::GetFrequencyWidthForChannelNumberStandard (uint8_t channelNumber, MmWavePhyBand band, MmWavePhyStandard standard) const
    {
        MmWaveChannelNumberStandardPair p = std::make_pair (std::make_pair (channelNumber, band), standard);
        MmWaveFrequencyWidthPair f = m_channelToFrequency.at(p);
        return f;
    }

    std::vector<uint16_t>
    MmWavePhy::GetSupportedChannelWidthSet () const
    {
        return m_supportedChannelWidthSet;
    }

    std::pair<bool, MmWaveSignalNoiseDbm>
    MmWavePhy::GetReceptionStatus (Ptr<const MmWavePsdu> psdu, Ptr<MmWaveEvent> event, Time relativeMpduStart, Time mpduDuration)
    {
        NS_LOG_FUNCTION (this << *psdu << *event << relativeMpduStart << mpduDuration);
        uint16_t channelWidth = std::min (GetChannelWidth (), event->GetTxVector ().GetChannelWidth ());
        MmWaveTxVector txVector = event->GetTxVector ();
        MmWaveSpectrumBand band = GetBand (channelWidth);
        MmWaveInterferenceHelper::SnrPer snrPer = m_interference.CalculatePayloadSnrPer (event, channelWidth, band, std::make_pair (relativeMpduStart, relativeMpduStart + mpduDuration));
        MmWaveMode mode = event->GetTxVector ().GetMode ();
        NS_LOG_DEBUG ("mode=" << (mode.GetDataRate (event->GetTxVector ())) <<
                              ", snr(dB)=" << RatioToDb (snrPer.snr) << ", per=" << snrPer.per << ", size=" << psdu->GetSize () <<
                              ", relativeStart = " << relativeMpduStart.As (Time::NS) << ", duration = " << mpduDuration.As (Time::NS));

        MmWaveSignalNoiseDbm signalNoise;
        signalNoise.signal = WToDbm (event->GetRxPowerW (band));
        signalNoise.noise = WToDbm (event->GetRxPowerW (band) / snrPer.snr);
        if (m_rng->GetValue () > snrPer.per &&
            !(m_postReceptionErrorModel && m_postReceptionErrorModel->IsCorrupt (psdu->GetPacket ()->Copy ())))
        {
            NS_LOG_DEBUG ("Reception succeeded: " << psdu);
            return std::make_pair (true, signalNoise);
        }
        else
        {
            NS_LOG_DEBUG ("Reception failed: " << psdu);
            return std::make_pair (false, signalNoise);
        }
    }

    void
    MmWavePhy::NotifyTxBegin (MmWaveConstPsduMap psdus, double txPowerW)
    {
        for (auto const& psdu : psdus)
        {
            Ptr<MmWaveMacQueueItem> mpdu = psdu.second->GetMpdu ();
            m_phyTxBeginTrace (mpdu->GetProtocolDataUnit (), txPowerW);
        }
    }

    void
    MmWavePhy::NotifyTxEnd (MmWaveConstPsduMap psdus)
    {
        for (auto const& psdu : psdus)
        {
            Ptr<MmWaveMacQueueItem> mpdu = psdu.second->GetMpdu ();
            m_phyTxEndTrace (mpdu->GetProtocolDataUnit ());
        }
    }

    void
    MmWavePhy::NotifyTxDrop (Ptr<const MmWavePsdu> psdu)
    {
        Ptr<MmWaveMacQueueItem> mpdu = psdu->GetMpdu ();
        m_phyTxDropTrace (mpdu->GetProtocolDataUnit ());
    }

    void
    MmWavePhy::NotifyRxBegin (Ptr<const MmWavePsdu> psdu, RxPowerWattPerChannelBand rxPowersW)
    {
        if (psdu)
        {
            Ptr<MmWaveMacQueueItem> mpdu = psdu->GetMpdu ();
            m_phyRxBeginTrace (mpdu->GetProtocolDataUnit (), rxPowersW);
        }
    }

    void
    MmWavePhy::NotifyRxEnd (Ptr<const MmWavePsdu> psdu)
    {
        if (psdu)
        {
            Ptr<MmWaveMacQueueItem> mpdu = psdu->GetMpdu ();
            m_phyRxEndTrace (mpdu->GetProtocolDataUnit ());
        }
    }

    void
    MmWavePhy::NotifyRxDrop (Ptr<const MmWavePsdu> psdu, MmWavePhyRxfailureReason reason)
    {
        if (psdu)
        {
            Ptr<MmWaveMacQueueItem> mpdu = psdu->GetMpdu ();
            m_phyRxDropTrace (mpdu->GetProtocolDataUnit (), reason);
        }
    }

    void
    MmWavePhy::NotifyMonitorSniffRx (Ptr<const MmWavePsdu> psdu, uint16_t channelFreqMhz, MmWaveTxVector txVector,
                                     MmWaveSignalNoiseDbm signalNoise, std::vector<bool> statusPerMpdu)
    {
        MmWaveMpduInfo aMpdu;
        aMpdu.type = MMWAVE_NORMAL_MPDU;
        NS_ASSERT_MSG (statusPerMpdu.size () == 1, "Should have one reception status for normal MPDU");
        m_phyMonitorSniffRxTrace (psdu->GetPacket (), channelFreqMhz, txVector, aMpdu, signalNoise);

    }

    void
    MmWavePhy::NotifyMonitorSniffTx (Ptr<const MmWavePsdu> psdu, uint16_t channelFreqMhz, MmWaveTxVector txVector)
    {
        MmWaveMpduInfo aMpdu;
        aMpdu.type = MMWAVE_NORMAL_MPDU;
        m_phyMonitorSniffTxTrace (psdu->GetPacket (), channelFreqMhz, txVector, aMpdu);
    }

    void
    MmWavePhy::NotifyEndOfMmWavePreamble (MmWavePreambleParameters params)
    {
        m_phyEndOfMmWavePreambleTrace (params);
    }

    void
    MmWavePhy::NotifyChannelAccessRequested ()
    {
        m_channelAccessRequested = true;
    }

    void
    MmWavePhy::NotifyUnrecognizedSignalDetected (Ptr<MmWaveEvent> event)
    {
        NS_ASSERT (IsSignalDetectionMode ());
        auto band = GetBand (GetChannelWidth ());
        double rxPowerW = event->GetRxPowerW (band);
        MmWaveInterferenceHelper::SnrPer snrPer = m_interference.CalculatePhyHeaderSnrPer (event, band);
        double snr = snrPer.snr;
        if (!m_unrecognizedSignalDetected.IsNull ())
        {
            m_unrecognizedSignalDetected (rxPowerW, snr, event->GetStartTime (), Simulator::Now ()- event->GetStartTime ());
        }
    }

    void
    MmWavePhy::NotifyRecognizedSignalDetected (Ptr<MmWaveEvent> event)
    {
        NS_ASSERT (IsSignalDetectionMode ());
        auto band = GetBand (GetChannelWidth ());
        double rxPowerW = event->GetRxPowerW (band);
        MmWaveInterferenceHelper::SnrPer snrPer = m_interference.CalculatePhyHeaderSnrPer (event, band);
        double snr = snrPer.snr;
        if (!m_recognizedSignalDetected.IsNull ())
        {
            m_recognizedSignalDetected (rxPowerW, snr, event->GetStartTime (), event->GetDuration ());
        }

    }

    void
    MmWavePhy::Send (Ptr<const MmWavePsdu> psdu, MmWaveTxVector txVector)
    {
        MmWaveConstPsduMap psdus;
        psdus.insert (std::make_pair (SU_STA_ID, psdu));
        Send (psdus, txVector);
    }

    void
    MmWavePhy::Send (MmWaveConstPsduMap psdus, MmWaveTxVector txVector)
    {
        NS_LOG_FUNCTION (this << psdus << txVector);
        NS_ASSERT (!m_state->IsStateTx ());
        NS_ASSERT (!m_state->IsStateSwitching ());
        if (!m_endTxEvent.IsExpired ())
        {
            m_endTxEvent.Cancel ();
        }
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
        m_state->SwitchToTx (txDuration, psdus, GetPowerDbm (txVector.GetTxPowerLevel ()), txVector, GetChannelNumber(), GetFrequency(), GetChannelWidth());
        m_endTxEvent = Simulator::Schedule (txDuration, &MmWavePhy::NotifyTxEnd, this, psdus);
        
        Ptr<MmWavePpdu> ppdu = Create<MmWavePpdu> (psdus, txVector, txDuration, GetPhyBand ());
        StartTx (ppdu);
        m_channelAccessRequested = false;
        m_powerRestricted = false;
    }

    void
    MmWavePhy::StartReceivePreamble (Ptr<MmWavePpdu> ppdu, RxPowerWattPerChannelBand rxPowersW)
    {
        NS_LOG_FUNCTION (this);
        auto it = std::max_element (rxPowersW.begin (), rxPowersW.end (),
                                    [] (const std::pair<MmWaveSpectrumBand, double>& p1, const std::pair<MmWaveSpectrumBand, double>& p2)
                                    {return p1.second < p2.second;});
        NS_LOG_FUNCTION (this << *ppdu << it->second);
        MmWaveTxVector txVector = ppdu->GetTxVector ();
        Time rxDuration = ppdu->GetTxDuration ();
        Ptr<MmWaveEvent> event = m_interference.Add (ppdu, txVector, rxDuration, rxPowersW);
        Time endRx = Simulator::Now () + rxDuration;
        if (m_state->GetState () == MmWavePhyState::MMWAVE_OFF)
        {
            NS_LOG_DEBUG ("Cannot start RX because device is OFF");
            if (endRx > (Simulator::Now () + m_state->GetDelayUntilIdle ()))
            {
                MaybeCcaBusyDuration ();
            }
            return;
        }

        if (ppdu->IsUnrecognizedSignal ())
        {
            NS_LOG_DEBUG ("signal is a unrecognized signal");
            if (IsSignalDetectionMode ())
            {
                NotifyUnrecognizedSignalDetected (event);
            }

            if (endRx > (Simulator::Now () + m_state->GetDelayUntilIdle ()))
            {
                MaybeCcaBusyDuration ();
            }
            return;
        }

        if (!ppdu->IsUnrecognizedSignal ())
        {
            NS_LOG_DEBUG ("signal is a recognized signal");
            if (IsSignalDetectionMode ())
            {
                NotifyRecognizedSignalDetected (event);
            }
        }

        if (ppdu->IsTruncatedTx ())
        {
            NS_LOG_DEBUG ("Packet reception stopped because transmitter has been switched off");
            if (endRx > (Simulator::Now () + m_state->GetDelayUntilIdle ()))
            {
                MaybeCcaBusyDuration ();
            }
            return;
        }

        if (!txVector.GetModeInitialized ())
        {
            NS_LOG_DEBUG ("drop packet because of unsupported RX mode");
            NotifyRxDrop (GetAddressedPsduInPpdu (ppdu), MMWAVE_UNSUPPORTED_SETTINGS);
            if (endRx > (Simulator::Now () + m_state->GetDelayUntilIdle ()))
            {
                MaybeCcaBusyDuration ();
            }
            return;
        }

        switch (m_state->GetState ())
        {
            case MmWavePhyState::MMWAVE_SWITCHING:
                NS_LOG_DEBUG ("drop packet because of channel switching");
                NotifyRxDrop (GetAddressedPsduInPpdu (ppdu), MMWAVE_CHANNEL_SWITCHING);
                if (endRx > (Simulator::Now () + m_state->GetDelayUntilIdle ()))
                {
                    MaybeCcaBusyDuration ();
                }
                break;
            case MmWavePhyState::MMWAVE_RX:
                NS_ASSERT (m_currentEvent != 0);
                if (m_frameCaptureModel != 0
                    && m_frameCaptureModel->IsInCaptureWindow (m_timeLastPreambleDetected)
                    && m_frameCaptureModel->CaptureNewFrame (m_currentEvent, event))
                {
                    NS_LOG_DEBUG ("Switch to new packet");
                    AbortCurrentReception (MMWAVE_FRAME_CAPTURE_PACKET_SWITCH);
                    StartRx (event);
                }
                else
                {
                    NS_LOG_DEBUG ("Drop packet because already in Rx");
                    NotifyRxDrop (GetAddressedPsduInPpdu (ppdu), MMWAVE_RXING);
                    if (endRx > (Simulator::Now () + m_state->GetDelayUntilIdle ()))
                    {
                        MaybeCcaBusyDuration ();
                    }
                }
                break;
            case MmWavePhyState::MMWAVE_TX:
                NS_LOG_DEBUG ("Drop packet because already in Tx");
                NotifyRxDrop (GetAddressedPsduInPpdu (ppdu), MMWAVE_TXING);
                if (endRx > (Simulator::Now () + m_state->GetDelayUntilIdle ()))
                {
                    MaybeCcaBusyDuration ();
                }
                break;
            case MmWavePhyState::MMWAVE_CCA_BUSY:
                if (m_currentEvent != 0)
                {
                    if (m_frameCaptureModel != 0
                        && m_frameCaptureModel->IsInCaptureWindow (m_timeLastPreambleDetected)
                        && m_frameCaptureModel->CaptureNewFrame (m_currentEvent, event))
                    {
                        AbortCurrentReception (MMWAVE_FRAME_CAPTURE_PACKET_SWITCH);
                        NS_LOG_DEBUG ("Switch to new packet");
                        StartRx (event);
                    }
                    else
                    {
                        NS_LOG_DEBUG ("Drop packet because already in Rx");
                        NotifyRxDrop (ppdu->GetPsdu (), MMWAVE_RXING);
                        if (endRx > (Simulator::Now () + m_state->GetDelayUntilIdle ()))
                        {
                            MaybeCcaBusyDuration ();
                        }
                    }
                }
                else
                {
                    StartRx (event);
                }
                break;
            case MmWavePhyState::MMWAVE_IDLE:
                StartRx (event);
                break;
            case MmWavePhyState::MMWAVE_SLEEP:
                NS_LOG_DEBUG ("Drop packet because in sleep mode");
                NotifyRxDrop (GetAddressedPsduInPpdu (ppdu), MMWAVE_SLEEPING);
                if (endRx > (Simulator::Now () + m_state->GetDelayUntilIdle ()))
                {
                    MaybeCcaBusyDuration ();
                }
                break;
            default:
                NS_FATAL_ERROR ("Invalid MmWavePhy state.");
                break;
        }
    }

    void
    MmWavePhy::StartRx (Ptr<MmWaveEvent> event)
    {
        NS_LOG_FUNCTION (this << *event);
        uint16_t primaryChannelWidth = MMWAVE_MIN_BANDWIDTH;
        auto primaryBand = GetBand (primaryChannelWidth);
        double rxPowerW = event->GetRxPowerW (primaryBand);
        NS_LOG_FUNCTION (this << *event << rxPowerW);
        NS_LOG_DEBUG ("sync to signal (power=" << rxPowerW << "W)");
        m_interference.NotifyRxStart (); //We need to notify it now so that it starts recording events
        if (!m_endPreambleDetectionEvent.IsRunning ())
        {
            Time startOfPreambleDuration = GetPreambleDetectionDuration ();
            Time remainingRxDuration = event->GetDuration () - startOfPreambleDuration;
            m_endPreambleDetectionEvent = Simulator::Schedule (startOfPreambleDuration, &MmWavePhy::StartReceiveHeader, this, event);
        }
        else if ((m_frameCaptureModel != 0) && (rxPowerW > m_currentEvent->GetRxPowerW (primaryBand)))
        {
            NS_LOG_DEBUG ("Received a stronger signal during preamble detection: drop current packet and switch to new packet");
            NotifyRxDrop (GetAddressedPsduInPpdu (m_currentEvent->GetPpdu ()), MMWAVE_PREAMBLE_DETECTION_PACKET_SWITCH);
            m_interference.NotifyRxEnd ();
            m_endPreambleDetectionEvent.Cancel ();
            m_interference.NotifyRxStart ();
            Time startOfPreambleDuration = GetPreambleDetectionDuration ();
            Time remainingRxDuration = event->GetDuration () - startOfPreambleDuration;
            m_endPreambleDetectionEvent = Simulator::Schedule (startOfPreambleDuration, &MmWavePhy::StartReceiveHeader, this, event);
        }
        else
        {
            NS_LOG_DEBUG ("Drop packet because RX is already decoding preamble");
            NotifyRxDrop (GetAddressedPsduInPpdu (event->GetPpdu ()), MMWAVE_BUSY_DECODING_PREAMBLE);
            return;
        }
        m_currentEvent = event;
    }

    void
    MmWavePhy::StartReceiveHeader (Ptr<MmWaveEvent> event)
    {
        NS_LOG_FUNCTION (this << *event);
        if (IsStateRx ())
        {
            Time rxDuration = event->GetDuration ();
            Time endRx = Simulator::Now () + rxDuration;
            NS_ASSERT (m_currentEvent != 0);
            if (m_frameCaptureModel != 0
                && m_frameCaptureModel->IsInCaptureWindow (m_timeLastPreambleDetected)
                && m_frameCaptureModel->CaptureNewFrame (m_currentEvent, event))
            {
                NS_LOG_DEBUG ("Switch to new packet");
                AbortCurrentReception (MMWAVE_FRAME_CAPTURE_PACKET_SWITCH);
                StartRx (event);
            }
            else
            {
                NS_LOG_DEBUG ("Drop packet because already in Rx");
                NotifyRxDrop (GetAddressedPsduInPpdu (event->GetPpdu ()), MMWAVE_RXING);
                if (endRx > (Simulator::Now () + m_state->GetDelayUntilIdle ()))
                {
                    MaybeCcaBusyDuration ();
                }
            }
            return;
        }
        NS_ASSERT (!IsStateRx ());
        NS_ASSERT (m_endPhyRxEvent.IsExpired ());
        NS_ASSERT (m_currentEvent != 0);
        NS_ASSERT (event->GetStartTime () == m_currentEvent->GetStartTime ());
        NS_ASSERT (event->GetEndTime () == m_currentEvent->GetEndTime ());

        uint16_t channelWidth;
        channelWidth = event->GetTxVector ().GetChannelWidth ();
        auto band = GetBand (channelWidth);
        MmWaveInterferenceHelper::SnrPer snrPer = m_interference.CalculatePhyHeaderSnrPer (event, band);
        double snr = snrPer.snr;
        NS_LOG_DEBUG ("snr(dB)=" << RatioToDb (snrPer.snr) << ", per=" << snrPer.per);

        if (!m_preambleDetectionModel || (m_preambleDetectionModel->IsPreambleDetected (event->GetRxPowerW (band), snr, m_channelWidth)))
        {
            NotifyRxBegin (GetAddressedPsduInPpdu (event->GetPpdu ()), event->GetRxPowerWPerBand ());
            m_timeLastPreambleDetected = Simulator::Now ();
            MmWaveTxVector txVector = event->GetTxVector ();
            Time remainingPreambleAndHeaderDuration = GetPhyPreambleDuration (txVector) - GetPreambleDetectionDuration ();
            m_state->SwitchMaybeToCcaBusy (remainingPreambleAndHeaderDuration);
            m_endPhyRxEvent = Simulator::Schedule (remainingPreambleAndHeaderDuration, &MmWavePhy::ContinueReceiveHeader, this, event);

        }
        else
        {
            NS_LOG_DEBUG ("Drop packet because PHY preamble detection failed");
            NotifyRxDrop (GetAddressedPsduInPpdu (event->GetPpdu ()), MMWAVE_PREAMBLE_DETECT_FAILURE);
            m_interference.NotifyRxEnd ();
            m_currentEvent = 0;
            if (event->GetEndTime () > (Simulator::Now () + m_state->GetDelayUntilIdle ()))
            {
                MaybeCcaBusyDuration ();
            }
        }
    }

    void
    MmWavePhy::ContinueReceiveHeader (Ptr<MmWaveEvent> event)
    {
        NS_LOG_FUNCTION (this << *event);
        NS_ASSERT (m_endPhyRxEvent.IsExpired ());
        uint16_t channelWidth;
        channelWidth = event->GetTxVector ().GetChannelWidth ();
        MmWaveInterferenceHelper::SnrPer snrPer = m_interference.CalculatePhyHeaderSnrPer (event, GetBand (channelWidth));

        NS_LOG_DEBUG ("snr(dB)=" << RatioToDb (snrPer.snr) << ", per=" << snrPer.per);
        if (m_rng->GetValue () > snrPer.per)
        {
            NS_LOG_DEBUG ("Received PHY header");
            MmWaveTxVector txVector = event->GetTxVector ();
            Time remainingRxDuration = event->GetEndTime () - Simulator::Now ();
            m_state->SwitchMaybeToCcaBusy (remainingRxDuration);
            Time remainingPreambleHeaderDuration = CalculatePhyPreambleAndHeaderDuration (txVector) - GetPhyPreambleDuration (txVector);
            m_endPhyRxEvent = Simulator::Schedule (remainingPreambleHeaderDuration, &MmWavePhy::StartReceivePayload, this, event);
        }
        else
        {
            NS_LOG_DEBUG ("Abort reception because PHY header reception failed");
            AbortCurrentReception (MMWAVE_L_SIG_FAILURE);
            if (event->GetEndTime () > (Simulator::Now () + m_state->GetDelayUntilIdle ()))
            {
                MaybeCcaBusyDuration ();
            }
        }
    }

    void
    MmWavePhy::StartReceivePayload (Ptr<MmWaveEvent> event)
    {
        NS_LOG_FUNCTION (this << *event);
        NS_ASSERT (m_endPhyRxEvent.IsExpired ());
        NS_ASSERT (m_endRxEvent.IsExpired ());
        bool canReceivePayload = false;
        Ptr<const MmWavePpdu> ppdu = event->GetPpdu ();
        MmWaveModulationClass modulation = ppdu->GetModulation ();
        uint16_t primaryChannelWidth = MMWAVE_MIN_BANDWIDTH;
        auto primaryBand = GetBand (primaryChannelWidth);
        if (modulation == MMWAVE_MOD_CLASS_OFDM)
        {
            MmWaveInterferenceHelper::SnrPer snrPer = m_interference.CalculatePhyHeaderSnrPer (event, primaryBand);
            NS_LOG_DEBUG ("snr(dB)=" << RatioToDb (snrPer.snr) << ", per=" << snrPer.per);
            canReceivePayload = (m_rng->GetValue () > snrPer.per);
        }
        else
        {
            canReceivePayload = true;
        }
        MmWaveTxVector txVector = event->GetTxVector ();
        Time payloadDuration = event->GetEndTime () - event->GetStartTime () - CalculatePhyPreambleAndHeaderDuration (txVector);
        bool success = false;

        if (canReceivePayload) //PHY reception succeeded
        {
            Ptr<const MmWavePsdu> psdu = GetAddressedPsduInPpdu (ppdu);
            if (psdu)
            {
                MmWaveMode txMode = txVector.GetMode ();
                uint8_t nss = txVector.GetNssMax ();
                if (nss > GetMaxSupportedRxSpatialStreams ())
                {
                    NS_LOG_DEBUG ("Packet reception could not be started because not enough RX antennas");
                    NotifyRxDrop (psdu, MMWAVE_UNSUPPORTED_SETTINGS);
                }
                else if (txVector.GetChannelWidth () > GetChannelWidth ())
                {
                    NS_LOG_DEBUG ("Packet reception could not be started because not enough channel width");
                    NotifyRxDrop (psdu, MMWAVE_UNSUPPORTED_SETTINGS);
                }
                else if (IsMcsSupported (txMode))
                {
                    m_statusPerMpdu.clear ();
                    m_state->SwitchToRx (payloadDuration);
                    m_phyRxPayloadBeginTrace (txVector, payloadDuration); //this callback (equivalent to PHY-RXSTART primitive) is triggered only if headers have been correctly decoded and that the mode within is supported
                    m_endRxEvent = Simulator::Schedule (payloadDuration, &MmWavePhy::EndReceive, this, event);
                    success = true;
                    NS_LOG_DEBUG ("Receiving PSDU");
                }
                else //mode is not allowed
                {
                    NS_LOG_DEBUG ("Drop packet because it was sent using an unsupported mode (" << txMode << ")");
                    NotifyRxDrop (psdu, MMWAVE_UNSUPPORTED_SETTINGS);
                }
            }
            else
            {
                NS_ASSERT (ppdu->IsMu ());
                NS_LOG_DEBUG ("No PSDU addressed to that PHY in the received MU PPDU. The PPDU is filtered.");
                payloadDuration = NanoSeconds (0);
                m_phyRxPayloadBeginTrace (txVector, payloadDuration); //this callback (equivalent to PHY-RXSTART primitive) is also triggered for filtered PPDUs
            }

            if (modulation == MMWAVE_MOD_CLASS_OFDM)
            {
                MmWavePreambleParameters params;
                params.rssiW = event->GetRxPowerW (primaryBand);
                params.bssColor = event->GetTxVector ().GetBssColor ();
                NotifyEndOfMmWavePreamble (params);
            }
        }
        else //PHY reception failed
        {
            NS_LOG_DEBUG ("Drop packet because HT PHY header reception failed");
            NotifyRxDrop (GetAddressedPsduInPpdu (ppdu), MMWAVE_SIG_A_FAILURE);
        }
        if (!success)
        {
            if (payloadDuration.IsStrictlyPositive ())
            {
                m_endRxEvent = Simulator::Schedule (payloadDuration, &MmWavePhy::ResetReceive, this, event);
            }
            else
            {
                AbortCurrentReception (MMWAVE_FILTERED);
                if (event->GetEndTime () > (Simulator::Now () + m_state->GetDelayUntilIdle ()))
                {
                    MaybeCcaBusyDuration ();
                }
            }
        }
    }

    void
    MmWavePhy::AbortCurrentReception (MmWavePhyRxfailureReason reason)
    {
        NS_LOG_FUNCTION (this << reason);
        if (m_endPreambleDetectionEvent.IsRunning ())
        {
            m_endPreambleDetectionEvent.Cancel ();
        }
        if (m_endPhyRxEvent.IsRunning ())
        {
            m_endPhyRxEvent.Cancel ();
        }
        if (m_endRxEvent.IsRunning ())
        {
            m_endRxEvent.Cancel ();
        }
        NotifyRxDrop (GetAddressedPsduInPpdu (m_currentEvent->GetPpdu ()), reason);
        m_interference.NotifyRxEnd ();
        if (reason == MMWAVE_OBSS_PD_CCA_RESET)
        {
            m_state->SwitchFromRxAbort ();
        }
        m_currentEvent = 0;
    }

    void
    MmWavePhy::MaybeCcaBusyDuration ()
    {
        NS_LOG_FUNCTION (this);
        uint16_t primaryChannelWidth = MMWAVE_MIN_BANDWIDTH;
        Time delayUntilCcaEnd = m_interference.GetEnergyDuration (m_ccaEdThresholdW, GetBand (primaryChannelWidth));
        if (!delayUntilCcaEnd.IsZero ())
        {
            m_state->SwitchMaybeToCcaBusy (delayUntilCcaEnd);
        }
    }

    void
    MmWavePhy::EndReceive (Ptr<MmWaveEvent> event)
    {
        NS_LOG_FUNCTION (this << *event);
        Time psduDuration = event->GetEndTime () - event->GetStartTime ();
        NS_LOG_FUNCTION (this << *event << psduDuration);
        NS_ASSERT (GetLastRxEndTime () == Simulator::Now ());
        NS_ASSERT (event->GetEndTime () == Simulator::Now ());

        Ptr<const MmWavePsdu> psdu = GetAddressedPsduInPpdu (event->GetPpdu ());
        std::pair<bool, MmWaveSignalNoiseDbm> rxInfo = GetReceptionStatus (psdu, event, NanoSeconds (0), psduDuration);
        m_signalNoise = rxInfo.second;
        m_statusPerMpdu.push_back (rxInfo.first);
        NotifyRxEnd (psdu);
        MmWaveTxVector txVector = event->GetTxVector ();
        uint16_t channelWidth = std::min (GetChannelWidth (), txVector.GetChannelWidth ());
        MmWaveSpectrumBand band;
        band = GetBand (channelWidth);
        double snr = m_interference.CalculateSnr (event, channelWidth, txVector.GetNss (), band);
        if (std::count (m_statusPerMpdu.begin (), m_statusPerMpdu.end (), true))
        {
            //At least one MPDU has been successfully received
            MmWaveTxVector txVector = event->GetTxVector ();
            NotifyMonitorSniffRx (psdu, GetFrequency (), txVector, m_signalNoise, m_statusPerMpdu);
            m_state->SwitchFromRxEndOk (Copy (psdu), snr, event->GetStartTime (), psduDuration, txVector, GetChannelNumber(), GetFrequency(), GetChannelWidth());
        }
        else
        {
            m_state->SwitchFromRxEndError (Copy (psdu), snr, event->GetStartTime (), psduDuration, txVector);
        }

        m_interference.NotifyRxEnd ();
        m_currentEvent = 0;
        MaybeCcaBusyDuration ();
    }
    
    void
    MmWavePhy::ResetReceive (Ptr<MmWaveEvent> event)
    {
        NS_LOG_FUNCTION (this << *event);
        NS_ASSERT (event->GetEndTime () == Simulator::Now ());
        NS_ASSERT (!IsStateRx ());
        m_interference.NotifyRxEnd ();
        m_currentEvent = 0;
        MaybeCcaBusyDuration ();
    }

    void
    MmWavePhy::EndReceiveInterBss ()
    {
        NS_LOG_FUNCTION (this);
        if (!m_channelAccessRequested)
        {
            m_powerRestricted = false;
        }
    }

    void
    MmWavePhy::PushMcs (MmWaveMode mode)
    {
        NS_LOG_FUNCTION (this << mode);

        MmWaveModulationClass modulation = mode.GetModulationClass ();
        NS_ASSERT (modulation == MMWAVE_MOD_CLASS_OFDM);

        m_mcsIndexMap[modulation][mode.GetMcsValue ()] = m_deviceMcsSet.size ();
        m_deviceMcsSet.push_back (mode);
    }

    void
    MmWavePhy::RebuildMcsMap ()
    {
        NS_LOG_FUNCTION (this);
        m_mcsIndexMap.clear ();
        uint8_t index = 0;
        for (auto& mode : m_deviceMcsSet)
        {
            m_mcsIndexMap[mode.GetModulationClass ()][mode.GetMcsValue ()] = index++;
        }
    }

    void
    MmWavePhy::ConfigureDefaultsForStandard ()
    {
        NS_LOG_FUNCTION (this);
        NS_ASSERT (m_channelToFrequency.size() > 0);
        auto i = m_channelToFrequency.begin();
        SetChannelNumber (i->first.first.first);
        ConfigureChannelForStandard ();
    }

    void
    MmWavePhy::ConfigureChannelForStandard ()
    {
        NS_LOG_FUNCTION (this);
        if (GetChannelNumber () != 0)
        {
            NS_LOG_DEBUG ("Configuring for channel number " << +GetChannelNumber ());
            MmWaveFrequencyWidthPair f = GetFrequencyWidthForChannelNumberStandard (GetChannelNumber (), GetPhyBand (), GetPhyStandard ());
            if (f.first == 0)
            {
                // the specific pair of number/standard is not known
                NS_LOG_DEBUG ("Falling back to check MMWAVE_PHY_STANDARD_UNSPECIFIED");
                f = GetFrequencyWidthForChannelNumberStandard (GetChannelNumber (), GetPhyBand (), MMWAVE_PHY_STANDARD_UNSPECIFIED);
            }
            if (f.first == 0)
            {
                NS_FATAL_ERROR ("Error, ChannelNumber " << +GetChannelNumber () << " is unknown for this standard");
            }
            else
            {
                NS_LOG_DEBUG ("Setting frequency to " << f.first << "; width to " << +f.second);
                SetFrequency (f.first);
                SetChannelWidth (f.second);
            }
        }
        else if (GetFrequency () != 0)
        {
            NS_LOG_DEBUG ("Frequency set; checking whether a channel number corresponds");
            uint8_t channelNumberSearched = FindChannelNumberForFrequencyWidth (GetFrequency (), GetChannelWidth ());
            if (channelNumberSearched)
            {
                NS_LOG_DEBUG ("Channel number found; setting to " << +channelNumberSearched);
                SetChannelNumber (channelNumberSearched);
            }
            else
            {
                NS_LOG_DEBUG ("Channel number not found; setting to zero");
                SetChannelNumber (0);
            }
        }
        else
        {
            NS_FATAL_ERROR ("Channel configuration is error");
        }
    }

    MmWaveMode
    MmWavePhy::GetMmWaveMcs0 ()
    {
        static MmWaveMode mcs = MmWaveModeFactory::CreateMmWaveMcs ("MmWaveMcs0", 0, MMWAVE_MOD_CLASS_OFDM);
        return mcs;
    }

    MmWaveMode
    MmWavePhy::GetMmWaveMcs1 ()
    {
        static MmWaveMode mcs = MmWaveModeFactory::CreateMmWaveMcs ("MmWaveMcs1", 1, MMWAVE_MOD_CLASS_OFDM);
        return mcs;
    }

    MmWaveMode
    MmWavePhy::GetMmWaveMcs2 ()
    {
        static MmWaveMode mcs = MmWaveModeFactory::CreateMmWaveMcs ("MmWaveMcs2", 2, MMWAVE_MOD_CLASS_OFDM);
        return mcs;
    }

    MmWaveMode
    MmWavePhy::GetMmWaveMcs3 ()
    {
        static MmWaveMode mcs = MmWaveModeFactory::CreateMmWaveMcs ("MmWaveMcs3", 3, MMWAVE_MOD_CLASS_OFDM);
        return mcs;
    }
}

namespace {
    /**
     * Constructor class
     */
    static class Constructor
    {
    public:
        Constructor()
        {
            ns3::MmWavePhy::GetMmWaveMcs0();
            ns3::MmWavePhy::GetMmWaveMcs1();
            ns3::MmWavePhy::GetMmWaveMcs2();
            ns3::MmWavePhy::GetMmWaveMcs3();
        }
    } g_constructor; ///< the constructor
}