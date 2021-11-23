/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_PHY_H
#define MMWAVE_PHY_H
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/traced-callback.h"
#include "ns3/error-model.h"
#include "ns3/antenna-model.h"
#include "ns3/spectrum-channel.h"
#include "ns3/spectrum-signal-parameters.h"
#include "mmwave.h"
#include "mmwave-mode.h"
#include "mmwave-ppdu.h"
#include "mmwave-phy-listener.h"
#include "mmwave-interference-helper.h"

namespace ns3 {

    class Channel;
    class NetDevice;
    class MobilityModel;
    class UniformRandomVariable;
    class MmWavePhyStateHelper;
    class MmWavePsdu;
    class MmWaveFrameCaptureModel;
    class MmWavePreambleDetectionModel;
    class MmWaveErrorRateModel;

    class MmWavePhy : public Object
    {
    public:
        static TypeId GetTypeId();
        typedef void (* MonitorSnifferRxCallback)(Ptr<const Packet> packet, uint16_t channelFreqMhz, MmWaveTxVector txVector, MmWaveMpduInfo aMpdu, MmWaveSignalNoiseDbm signalNoise);
        typedef void (* MonitorSnifferTxCallback)(const Ptr<const Packet> packet, uint16_t channelFreqMhz, MmWaveTxVector txVector, MmWaveMpduInfo aMpdu);
        typedef void (* PsduTxBeginCallback)(MmWaveConstPsduMap psduMap, MmWaveTxVector txVector, double txPowerW);
        typedef void (* EndOfMmWavePreambleCallback)(MmWavePreambleParameters params);
        typedef void (* PhyRxPayloadBeginTracedCallback)(MmWaveTxVector txVector, Time psduDuration);

        MmWavePhy();
        ~MmWavePhy();
        virtual void DoInitialize ();
        virtual void DoDispose ();
        virtual MmWaveSpectrumBand GetBand (uint16_t bandWidth, uint8_t bandIndex = 0);
        virtual void SetChannelNumber (uint8_t id);
        virtual void SetFrequency (uint16_t freq);
        virtual void SetChannelWidth (uint16_t channelWidth);
        virtual void ConfigureStandardAndBand (MmWavePhyStandard standard, MmWavePhyBand band);

        virtual void StartTx (Ptr<MmWavePpdu> ppdu) = 0;
        virtual void StartRx (Ptr<SpectrumSignalParameters> rxParams) = 0;
        virtual Ptr<const SpectrumModel> GetRxSpectrumModel () = 0;
        virtual Ptr<Channel> GetChannel () const = 0;

        static MmWaveMode GetMmWaveMcs0 ();
        static MmWaveMode GetMmWaveMcs1 ();
        static MmWaveMode GetMmWaveMcs2 ();
        static MmWaveMode GetMmWaveMcs3 ();
        static MmWaveMode GetMmWaveMcs (uint8_t mcs);
        static MmWaveMode GetPhyHeaderMcsMode ();
        static MmWaveMode GetPhyHeaderMode ();
        static Time CalculateTxDuration (uint32_t size, MmWaveTxVector txVector, MmWavePhyBand band);
        static Time CalculateTxDuration (MmWaveConstPsduMap psduMap, MmWaveTxVector txVector, MmWavePhyBand band);
        static Time CalculatePhyPreambleAndHeaderDuration (MmWaveTxVector txVector);
        static Time GetPreambleDetectionDuration ();
        static Time GetPhyHeaderDuration (MmWaveTxVector txVector);
        static Time GetPhyPreambleDuration (MmWaveTxVector txVector);
        static Time GetPayloadDuration (uint32_t size, MmWaveTxVector txVector, MmWavePhyBand band, MmWaveMpduType mpdutype = MMWAVE_NORMAL_MPDU);
        static Time GetPayloadDuration (uint32_t size, MmWaveTxVector txVector, MmWavePhyBand band, MmWaveMpduType mpdutype, bool incFlag, uint32_t &totalAmpduSize, double &totalAmpduNumSymbols);

        int64_t AssignStreams (int64_t stream);
        void SetSifs (Time sifs);
        void SetSlot (Time slot);
        void SetSleepMode ();
        void ResumeFromSleep ();
        void SetOffMode ();
        void ResumeFromOff ();
        void SetReceiveOkCallback (Callback<void, Ptr<MmWavePsdu>, double, Time, Time, MmWaveTxVector> callback);
        void SetReceiveErrorCallback (Callback<void, Ptr<MmWavePsdu>, double, Time, Time, MmWaveTxVector> callback);
        void SetUnrecognizedSignalDetectedCallback (Callback<void, double, double, Time, Time> callback);
        void SetRecognizedSignalDetectedCallback (Callback<void, double, double, Time, Time> callback);
        void RegisterListener (MmWavePhyListener *listener);
        void UnregisterListener (MmWavePhyListener *listener);
        void SetCapabilitiesChangedCallback (Callback<void> callback);
        void StartRx (Ptr<MmWaveEvent> event);
        void StartReceivePreamble (Ptr<MmWavePpdu> ppdu, RxPowerWattPerChannelBand rxPowersW);
        void StartReceiveHeader (Ptr<MmWaveEvent> event);
        void ContinueReceiveHeader (Ptr<MmWaveEvent> event);
        void StartReceivePayload (Ptr<MmWaveEvent> event);
        void EndReceive (Ptr<MmWaveEvent> event);
        void ResetReceive (Ptr<MmWaveEvent> event);
        void EndReceiveInterBss ();
        void Send (Ptr<const MmWavePsdu> psdu, MmWaveTxVector txVector);
        void Send (MmWaveConstPsduMap psdus, MmWaveTxVector txVector);
        void NotifyTxBegin (MmWaveConstPsduMap psdus, double txPowerW);
        void NotifyTxEnd (MmWaveConstPsduMap psdus);
        void NotifyTxDrop (Ptr<const MmWavePsdu> psdu);
        void NotifyRxBegin (Ptr<const MmWavePsdu> psdu, RxPowerWattPerChannelBand rxPowersW);
        void NotifyRxEnd (Ptr<const MmWavePsdu> psdu);
        void NotifyRxDrop (Ptr<const MmWavePsdu> psdu, MmWavePhyRxfailureReason reason);
        void NotifyMonitorSniffRx (Ptr<const MmWavePsdu> psdu, uint16_t channelFreqMhz, MmWaveTxVector txVector, MmWaveSignalNoiseDbm signalNoise, std::vector<bool> statusPerMpdu);
        void NotifyMonitorSniffTx (Ptr<const MmWavePsdu> psdu, uint16_t channelFreqMhz, MmWaveTxVector txVector);
        void NotifyEndOfMmWavePreamble (MmWavePreambleParameters params);
        void NotifyChannelAccessRequested ();
        void NotifyUnrecognizedSignalDetected (Ptr<MmWaveEvent> event);
        void NotifyRecognizedSignalDetected (Ptr<MmWaveEvent> event);
        void SetRxSensitivity (double threshold);
        void SetCcaEdThreshold (double threshold);
        void SetRxNoiseFigure (double noiseFigureDb);
        void SetTxPowerStart (double start);
        void SetTxPowerEnd (double end);
        void SetNTxPower (uint8_t n);
        void SetTxGain (double gain);
        void SetRxGain (double gain);
        void SetNumberOfAntennas (uint8_t antennas);
        void SetMaxSupportedTxSpatialStreams (uint8_t streams);
        void SetMaxSupportedRxSpatialStreams (uint8_t streams);
        void AddSupportedChannelWidth (uint16_t width);
        void ResetCca (bool powerRestricted, double txPowerMaxSiso = 0, double txPowerMaxMimo = 0);
        void SetDevice (const Ptr<NetDevice> device);
        void SetMobility (const Ptr<MobilityModel> mobility);
        void SetErrorRateModel (const Ptr<MmWaveErrorRateModel> rate);
        void SetPostReceptionErrorModel (const Ptr<ErrorModel> em);
        void SetFrameCaptureModel (const Ptr<MmWaveFrameCaptureModel> frameCaptureModel);
        void SetPreambleDetectionModel (const Ptr<MmWavePreambleDetectionModel> preambleDetectionModel);
        void SwitchMaybeToCcaBusy ();
        void InitializeFrequencyChannelNumber ();
        void ConfigureDefaultsForStandard ();
        void ConfigureChannelForStandard ();
        void PushMcs (MmWaveMode mode);
        void RebuildMcsMap ();
        void AbortCurrentReception (MmWavePhyRxfailureReason reason);
        void MaybeCcaBusyDuration ();
        void SetSignalDetectionMode ();
        void ResetSignalDetectionMode ();
        std::pair<bool, MmWaveSignalNoiseDbm> GetReceptionStatus (Ptr<const MmWavePsdu> psdu, Ptr<MmWaveEvent> event, Time relativeMpduStart, Time mpduDuration);
        bool DefineChannelNumber (uint8_t channelNumber, MmWavePhyBand band, MmWavePhyStandard standard, uint16_t frequency, uint16_t channelWidth);
        bool DoChannelSwitch (uint8_t id);
        bool DoFrequencySwitch (uint16_t frequency);
        bool IsSignalDetectionMode () const;
        bool IsStateIdle () const;
        bool IsStateCcaBusy () const;
        bool IsStateRx () const;
        bool IsStateTx () const;
        bool IsStateSwitching () const;
        bool IsStateSleep () const;
        bool IsStateOff () const;
        bool IsMcsSupported (MmWaveMode mcs) const;
        bool IsMcsSupported (MmWaveModulationClass mc, uint8_t mcs) const;
        uint8_t GetNMcs () const;
        uint8_t GetChannelNumber () const;
        uint8_t GetNTxPower () const;
        uint8_t GetNumberOfAntennas () const;
        uint8_t GetMaxSupportedTxSpatialStreams () const;
        uint8_t GetMaxSupportedRxSpatialStreams () const;
        uint8_t FindChannelNumberForFrequencyWidth (uint16_t frequency, uint16_t width) const;
        uint16_t GetFrequency () const;
        uint16_t GetChannelWidth () const;
        double CalculateSnr (MmWaveTxVector txVector, double ber) const;
        double GetRxSensitivity () const;
        double GetCcaEdThreshold () const;
        double GetTxPowerStart () const;
        double GetTxPowerEnd () const;
        double GetTxGain () const;
        double GetRxGain () const;
        double GetTxPowerForTransmission (MmWaveTxVector txVector) const;
        double GetPowerDbm (uint8_t power) const;
        double DbmToW (double dbm) const;
        double DbToRatio (double db) const;
        double WToDbm (double w) const;
        double RatioToDb (double ratio) const;
        Ptr<MmWavePhyStateHelper> GetState () const;
        Ptr<NetDevice> GetDevice () const;
        Ptr<MobilityModel> GetMobility () const;
        Ptr<const MmWavePsdu> GetAddressedPsduInPpdu (Ptr<const MmWavePpdu> ppdu);
        Time GetDelayUntilIdle () const;
        Time GetLastRxStartTime () const;
        Time GetLastRxEndTime () const;
        Time GetSifs () const;
        Time GetSlot () const;
        Time GetChannelSwitchDelay () const;
        MmWavePhyStandard GetPhyStandard () const;
        MmWavePhyBand GetPhyBand () const;
        MmWaveMode GetMcs (uint8_t mcs) const;
        MmWaveMode GetMcs (MmWaveModulationClass modulation, uint8_t mcs) const;
        MmWaveChannelToFrequencyWidthMap GetChannelToFrequency () const;
        MmWaveChannelNumberStandardPair GetCurrentChannel () const;
        MmWaveChannelNumberStandardPair GetChannelFromChannelNumber (uint8_t num);
        MmWaveFrequencyWidthPair GetFrequencyWidthForChannelNumberStandard (uint8_t channelNumber, MmWavePhyBand band, MmWavePhyStandard standard) const;
        std::vector<uint16_t> GetSupportedChannelWidthSet () const;

        MmWaveInterferenceHelper m_interference;   //!< Pointer to InterferenceHelper
        Ptr<UniformRandomVariable> m_rng; //!< Provides uniform random variables.
        Ptr<MmWavePhyStateHelper> m_state;     //!< Pointer to MmWavePhyStateHelper

        uint32_t m_txMpduReferenceNumber;    //!< A-MPDU reference number to identify all transmitted subframes belonging to the same received A-MPDU
        uint32_t m_rxMpduReferenceNumber;    //!< A-MPDU reference number to identify all received subframes belonging to the same received A-MPDU

        EventId m_endRxEvent;                //!< the end of receive event
        EventId m_endPhyRxEvent;             //!< the end of PHY receive event
        EventId m_endPreambleDetectionEvent; //!< the end of preamble detection event
        EventId m_endTxEvent;                //!< the end of transmit event

        TracedCallback<Ptr<const Packet>, double> m_phyTxBeginTrace;
        TracedCallback<MmWaveConstPsduMap, MmWaveTxVector, double /* TX power (W) */> m_phyTxPsduBeginTrace;
        TracedCallback<Ptr<const Packet>> m_phyTxEndTrace;
        TracedCallback<Ptr<const Packet>> m_phyTxDropTrace;
        TracedCallback<Ptr<const Packet>, RxPowerWattPerChannelBand> m_phyRxBeginTrace;
        TracedCallback<MmWaveTxVector, Time> m_phyRxPayloadBeginTrace;
        TracedCallback<Ptr<const Packet>> m_phyRxEndTrace;
        TracedCallback<Ptr<const Packet>, MmWavePhyRxfailureReason> m_phyRxDropTrace;
        TracedCallback<Ptr<const Packet>, uint16_t /* frequency (MHz) */, MmWaveTxVector, MmWaveMpduInfo, MmWaveSignalNoiseDbm> m_phyMonitorSniffRxTrace;
        TracedCallback<Ptr<const Packet>, uint16_t /* frequency (MHz) */, MmWaveTxVector, MmWaveMpduInfo> m_phyMonitorSniffTxTrace;
        TracedCallback<MmWavePreambleParameters> m_phyEndOfMmWavePreambleTrace;
        Callback<void> m_capabilitiesChangedCallback;         //!< Callback when PHY capabilities changed
        Callback <void, double, double, Time, Time> m_unrecognizedSignalDetected;
        Callback <void, double, double, Time, Time> m_recognizedSignalDetected;
        MmWaveModeList m_deviceMcsSet; //!< the device MCS set

        std::map<MmWaveModulationClass, std::map<uint8_t /* MCS value */, uint8_t /* index */>> m_mcsIndexMap;
        std::vector<bool> m_statusPerMpdu; //!<  current reception status per MPDU that is filled in as long as MPDUs are being processed by the PHY in case of an A-MPDU
        std::vector<uint16_t> m_supportedChannelWidthSet; //!< Supported channel width set (MHz)

        MmWavePhyStandard m_standard;               //!< MmWavePhyStandard
        MmWavePhyBand m_band;                       //!< MmWavePhyBand
        bool m_isConstructed;                     //!< true when ready to set frequency
        uint16_t m_channelCenterFrequency;        //!< Center frequency in MHz
        uint16_t m_initialFrequency;              //!< Store frequency until initialization (MHz)
        bool m_frequencyChannelNumberInitialized; //!< Store initialization state
        uint16_t m_channelWidth;                  //!< Channel width (MHz)

        Time m_sifs;                              //!< Short Interframe Space (SIFS) duration
        Time m_slot;                              //!< Slot duration
        Time m_channelSwitchDelay;     //!< Time required to switch between channel
        Time m_timeLastPreambleDetected;                      //!< Record the time the last preamble was detected

        double   m_rxSensitivityW;      //!< Receive sensitivity threshold in watts
        double   m_ccaEdThresholdW;     //!< Clear channel assessment (CCA) threshold in watts
        double   m_txGainDb;            //!< Transmission gain (dB)
        double   m_rxGainDb;            //!< Reception gain (dB)
        double   m_txPowerBaseDbm;      //!< Minimum transmission power (dBm)
        double   m_txPowerEndDbm;       //!< Maximum transmission power (dBm)
        uint8_t  m_nTxPower;            //!< Number of available transmission power levels

        bool m_powerRestricted;        //!< Flag whether transmit power is restricted by OBSS PD SR
        double m_txPowerMaxSiso;       //!< SISO maximum transmit power due to OBSS PD SR power restriction (dBm)
        double m_txPowerMaxMimo;       //!< MIMO maximum transmit power due to OBSS PD SR power restriction (dBm)
        bool m_channelAccessRequested; //!< Flag if channels access has been requested (used for OBSS_PD SR)
        bool m_signalDetectionMode;

        uint8_t m_numberOfAntennas;  //!< Number of transmitters
        uint8_t m_txSpatialStreams;  //!< Number of supported TX spatial streams
        uint8_t m_rxSpatialStreams;  //!< Number of supported RX spatial streams

        MmWaveChannelToFrequencyWidthMap m_channelToFrequency;                               //!< the channel to frequency width map
        TypeOfGroup m_typeOfGroup;
        uint8_t m_channelNumber;            //!< Operating channel number
        uint8_t m_initialChannelNumber;     //!< Initial channel number

        Ptr<NetDevice>     m_device;   //!< Pointer to the device
        Ptr<MobilityModel> m_mobility; //!< Pointer to the mobility model
        Ptr<MmWaveEvent> m_currentEvent;                            //!< Hold the current event
        Ptr<MmWaveFrameCaptureModel> m_frameCaptureModel;           //!< Frame capture model
        Ptr<MmWavePreambleDetectionModel> m_preambleDetectionModel; //!< Preamble detection model
        Ptr<ErrorModel> m_postReceptionErrorModel;            //!< Error model for receive packet events

        MmWaveSignalNoiseDbm m_signalNoise;      //!< latest signal power and noise power in dBm (noise power includes the noise figure)
    };
}
#endif //MMWAVE_PHY_H
