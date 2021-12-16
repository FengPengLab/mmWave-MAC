/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/string.h"
#include "ns3/log.h"
#include "ns3/object.h"
#include "mmwave-constant-rate-manager.h"
#include "mmwave-tx-vector.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("MmWaveConstantRateManager");
    NS_OBJECT_ENSURE_REGISTERED (MmWaveConstantRateManager);

    TypeId
    MmWaveConstantRateManager::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::MmWaveConstantRateManager")
                .SetParent<MmWaveRemoteStationManager> ()
                .SetGroupName ("MmWave")
                .AddConstructor<MmWaveConstantRateManager> ()
                .AddAttribute ("DataMode", "The transmission mode to use for every data packet transmission",
                               StringValue ("MmWaveMcs0"),
                               MakeMmWaveModeAccessor (&MmWaveConstantRateManager::m_dataMode),
                               MakeMmWaveModeChecker ())
                .AddAttribute ("ControlMode", "The transmission mode to use for every RTS packet transmission.",
                               StringValue ("MmWaveMcs0"),
                               MakeMmWaveModeAccessor (&MmWaveConstantRateManager::m_ctlMode),
                               MakeMmWaveModeChecker ())
        ;
        return tid;
    }

    MmWaveConstantRateManager::MmWaveConstantRateManager ()
    {
        NS_LOG_FUNCTION (this);
    }

    MmWaveConstantRateManager::~MmWaveConstantRateManager ()
    {
        NS_LOG_FUNCTION (this);
    }

    MmWaveRemoteStation *
    MmWaveConstantRateManager::DoCreateStation () const
    {
        NS_LOG_FUNCTION (this);
        MmWaveRemoteStation *station = new MmWaveRemoteStation ();
        return station;
    }

    void
    MmWaveConstantRateManager::DoReportRxOk (MmWaveRemoteStation *station, double rxSnr, MmWaveMode txMode)
    {
        NS_LOG_FUNCTION (this << station << rxSnr << txMode);
    }

    void
    MmWaveConstantRateManager::DoReportRtsFailed (MmWaveRemoteStation *station)
    {
        NS_LOG_FUNCTION (this << station);
    }

    void
    MmWaveConstantRateManager::DoReportDataFailed (MmWaveRemoteStation *station)
    {
        NS_LOG_FUNCTION (this << station);
    }

    void
    MmWaveConstantRateManager::DoReportRtsOk (MmWaveRemoteStation *st, double ctsSnr, MmWaveMode ctsMode, double rtsSnr)
    {
        NS_LOG_FUNCTION (this << st << ctsSnr << ctsMode << rtsSnr);
    }

    void
    MmWaveConstantRateManager::DoReportDataOk (MmWaveRemoteStation *st, double ackSnr, MmWaveMode ackMode, double dataSnr, uint16_t dataChannelWidth, uint8_t dataNss)
    {
        NS_LOG_FUNCTION (this << st << ackSnr << ackMode << dataSnr << dataChannelWidth << +dataNss);
    }

    void
    MmWaveConstantRateManager::DoReportFinalRtsFailed (MmWaveRemoteStation *station)
    {
        NS_LOG_FUNCTION (this << station);
    }

    void
    MmWaveConstantRateManager::DoReportFinalDataFailed (MmWaveRemoteStation *station)
    {
        NS_LOG_FUNCTION (this << station);
    }

    MmWaveTxVector
    MmWaveConstantRateManager::DoGetDataTxVector (MmWaveRemoteStation *st)
    {
        NS_LOG_FUNCTION (this << st);
        uint8_t powerLevel = GetDefaultTxPowerLevel ();
        MmWavePreamble preamble = GetPreamble (st);
        uint16_t guardInterval = GetGuardInterval (st);
        uint8_t nTx = GetNumberOfAntennas ();
        uint8_t nss = ((GetMaxNumberOfTransmitStreams () < GetNumberOfSupportedStreams (st)) ? GetMaxNumberOfTransmitStreams () : GetNumberOfSupportedStreams (st));
        uint8_t ness = 0;
        uint16_t channelWidth = GetChannelWidth (st);
        return MmWaveTxVector (m_dataMode, powerLevel, preamble, guardInterval, nTx, nss, ness, channelWidth);
    }

    MmWaveTxVector
    MmWaveConstantRateManager::DoGetCtrlTxVector (MmWaveRemoteStation *st)
    {
        NS_LOG_FUNCTION (this << st);
        uint8_t powerLevel = GetDefaultTxPowerLevel ();
        MmWavePreamble preamble = GetPreamble (st);
        uint16_t guardInterval = GetGuardInterval (st);
        uint8_t nTx = 1;
        uint8_t nss = 1;
        uint8_t ness = 0;
        uint16_t channelWidth = GetChannelWidth (st);
        return MmWaveTxVector (m_ctlMode, powerLevel, preamble, guardInterval, nTx, nss, ness, channelWidth);
    }

} //namespace ns3