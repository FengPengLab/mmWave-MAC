/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_REMOTE_STATION_MANAGER_H
#define MMWAVE_REMOTE_STATION_MANAGER_H
#include <array>
#include "ns3/traced-callback.h"
#include "ns3/object.h"
#include "ns3/data-rate.h"
#include "ns3/mac48-address.h"
#include "ns3/nstime.h"
#include "ns3/uinteger.h"
#include "mmwave.h"
#include "mmwave-mode.h"
#include "mmwave-mac-header.h"
#include "mmwave-tx-vector.h"

namespace ns3 {

    class Packet;
    class MmWavePhy;
    class MmWaveMac;

    class MmWaveRemoteStationInfo
    {
    public:
        MmWaveRemoteStationInfo ();
        virtual ~MmWaveRemoteStationInfo ();

        void NotifyTxSuccess (uint32_t retryCounter);
        void NotifyTxFailed ();
        double GetFrameErrorRate () const;
    private:
        double CalculateAveragingCoefficient ();

        Time m_memoryTime; ///< averaging coefficient depends on the memory time
        Time m_lastUpdate; ///< when last update has occurred

        double m_failAvg;  ///< moving percentage of failed frames
    };

    struct MmWaveRemoteStationState;

    struct MmWaveRemoteStation
    {
        virtual ~MmWaveRemoteStation () {};
        MmWaveRemoteStationState *m_state;  //!< Remote station state
    };

    struct MmWaveRemoteStationState
    {
        Mac48Address m_address;            //!< Mac48Address of the remote station
        MmWaveRemoteStationInfo m_info;      //!< remote station info
        MmWavePreamble m_preamble;
        uint16_t m_mpduBufferSize; //!< MPDU buffer size
        uint16_t m_channelWidth;    //!< Channel width (in MHz) supported by the remote station
        uint16_t m_guardInterval;   //!< Guard interval duration (in nanoseconds) supported by the remote station
        uint8_t m_ness;             //!< Number of extended spatial streams of the remote station
        uint8_t m_nss;
        uint8_t m_bssColor;        //!< Bss color
        bool m_ldpcSupported;
    };

    class MmWaveRemoteStationManager : public Object
    {
    public:
        static TypeId GetTypeId ();

        MmWaveRemoteStationManager ();
        virtual ~MmWaveRemoteStationManager ();

        enum ProtectionMode
        {
            RTS_CTS,
            CTS_TO_SELF
        };

        typedef std::vector <MmWaveRemoteStation *> Stations;
        typedef std::vector <MmWaveRemoteStationState *> StationStates;
        typedef void (*PowerChangeTracedCallback)(double oldPower, double newPower, Mac48Address remoteAddress);
        typedef void (*RateChangeTracedCallback)(DataRate oldRate, DataRate newRate, Mac48Address remoteAddress);

        virtual void SetupMac (const Ptr<MmWaveMac> mac);
        virtual void DoDispose ();

        Ptr<MmWaveMac> GetMac () const;

        MmWaveMode GetDefaultMode () const;
        MmWaveMode GetDefaultMcs () const;
        MmWaveMode GetNonUnicastMode () const;
        MmWaveTxVector GetDataTxVector (Mac48Address address);
        MmWaveTxVector GetCtrlTxVector (Mac48Address address);
        MmWaveTxVector GetCtsToSelfTxVector (Mac48Address address);
        MmWavePreamble GetPreamble () const;
        MmWaveRemoteStationInfo GetInfo (Mac48Address address);
        Mac48Address GetAddress (const MmWaveRemoteStation *station) const;
        MmWavePreamble GetPreamble (const MmWaveRemoteStation *station) const;

        void InitializePhyParams  (uint8_t numberOfAntennas, uint8_t nss, uint16_t channelWidth);
        void SetMaxSsrc (uint32_t maxSsrc);
        void SetMaxSlrc (uint32_t maxSlrc);
        void SetRtsCtsThreshold (uint32_t threshold);
        void SetFragmentationThreshold (uint32_t threshold);
        void SetBssColor (uint8_t color);
        void SetGuardInterval (uint16_t guardInterval);
        void SetMpduBufferSize (uint16_t size);
        void SetPreamble (MmWavePreamble preamble);
        void SetDefaultTxPowerLevel (uint8_t txPower);
        void SetLdpcSupported (bool supported);
        void SetLdpcSupported (Mac48Address address, bool supported);
        uint8_t GetBssColor () const;
        uint8_t GetDefaultTxPowerLevel () const;
        uint8_t GetNumberOfAntennas () const;
        uint8_t GetMaxNumberOfTransmitStreams () const;
        uint8_t GetNumberOfSupportedStreams (const MmWaveRemoteStation *station) const;
        uint8_t GetNess (const MmWaveRemoteStation *station) const;
        uint16_t GetGuardInterval () const;
        uint16_t GetMpduBufferSize () const;
        uint16_t GetChannelWidth (const MmWaveRemoteStation *station) const;
        uint16_t GetGuardInterval (const MmWaveRemoteStation *station) const;
        uint16_t GetChannelWidthSupported (Mac48Address address) const;
        uint32_t GetFragmentationThreshold () const;
        uint32_t GetFragmentSize (Mac48Address address, const MmWaveMacHeader *header, Ptr<const Packet> packet, uint32_t fragmentNumber);
        uint32_t GetFragmentOffset (Mac48Address address, const MmWaveMacHeader *header, Ptr<const Packet> packet, uint32_t fragmentNumber);

        void UpdateFragmentationThreshold ();
        void Reset ();
        void ReportRtsFailed (Mac48Address address, const MmWaveMacHeader *header);
        void ReportDataFailed (Mac48Address address, const MmWaveMacHeader *header, uint32_t packetSize);
        void ReportRtsOk (Mac48Address address, const MmWaveMacHeader *header, double ctsSnr, MmWaveMode ctsMode, double rtsSnr);
        void ReportDataOk (Mac48Address address, const MmWaveMacHeader *header, double ackSnr, MmWaveMode ackMode, double dataSnr, MmWaveTxVector dataTxVector, uint32_t packetSize);
        void ReportFinalRtsFailed (Mac48Address address, const MmWaveMacHeader *header);
        void ReportFinalDataFailed (Mac48Address address, const MmWaveMacHeader *header, uint32_t packetSize);
        void ReportAmpduTxStatus (Mac48Address address, uint8_t nSuccessfulMpdus, uint8_t nFailedMpdus, double rxSnr, double dataSnr, MmWaveTxVector dataTxVector);
        void ReportRxOk (Mac48Address address, double rxSnr, MmWaveMode txMode);

        bool NeedRts (const MmWaveMacHeader &header, uint32_t size);
        bool NeedRetransmission (Mac48Address address, const MmWaveMacHeader *header, Ptr<const Packet> packet);
        bool NeedFragmentation (Mac48Address address, const MmWaveMacHeader *header, Ptr<const Packet> packet);
        bool IsLastFragment (Mac48Address address, const MmWaveMacHeader *header, Ptr<const Packet> packet, uint32_t fragmentNumber);
        bool GetLdpcSupported () const;
        bool GetLdpcSupported (Mac48Address address) const;
        bool UseLdpcForDestination (Mac48Address dest) const;

    private:
        virtual bool DoNeedRts (MmWaveRemoteStation *station, uint32_t size, bool normally);
        virtual bool DoNeedRetransmission (MmWaveRemoteStation *station, Ptr<const Packet> packet, bool normally);
        virtual bool DoNeedFragmentation (MmWaveRemoteStation *station, Ptr<const Packet> packet, bool normally);
        virtual MmWaveRemoteStation* DoCreateStation () const = 0;
        virtual MmWaveTxVector DoGetDataTxVector (MmWaveRemoteStation *station) = 0;
        virtual MmWaveTxVector DoGetCtrlTxVector (MmWaveRemoteStation *station) = 0;
        virtual void DoReportRtsFailed (MmWaveRemoteStation *station) = 0;
        virtual void DoReportDataFailed (MmWaveRemoteStation *station) = 0;
        virtual void DoReportRtsOk (MmWaveRemoteStation *station, double ctsSnr, MmWaveMode ctsMode, double rtsSnr) = 0;
        virtual void DoReportDataOk (MmWaveRemoteStation *station, double ackSnr, MmWaveMode ackMode, double dataSnr, uint16_t dataChannelWidth, uint8_t dataNss) = 0;
        virtual void DoReportFinalRtsFailed (MmWaveRemoteStation *station) = 0;
        virtual void DoReportFinalDataFailed (MmWaveRemoteStation *station) = 0;
        virtual void DoReportRxOk (MmWaveRemoteStation *station, double rxSnr, MmWaveMode txMode) = 0;
        virtual void DoReportAmpduTxStatus (MmWaveRemoteStation *station, uint8_t nSuccessfulMpdus, uint8_t nFailedMpdus, double rxSnr, double dataSnr, uint16_t dataChannelWidth, uint8_t dataNss);
        MmWaveRemoteStationState* LookupState (Mac48Address address) const;
        MmWaveRemoteStation* Lookup (Mac48Address address) const;
        void DoSetFragmentationThreshold (uint32_t threshold);
        uint32_t DoGetFragmentationThreshold () const;
        uint32_t GetNFragments (const MmWaveMacHeader *header, Ptr<const Packet> packet);

        Ptr<MmWaveMac> m_mac;

        StationStates m_states;  //!< States of known stations
        Stations m_stations;     //!< Information for each known stations

        MmWaveMode m_defaultTxMode; //!< The default transmission mode
        MmWaveMode m_defaultTxMcs;  //!< The default transmission modulation-coding scheme (MCS)
        MmWaveMode m_nonUnicastMode;      //!< Transmission mode for non-unicast Data frames
        MmWavePreamble m_preamble;
        bool m_ldpcSupported;

        uint8_t m_ness;             //!< Number of extended spatial streams of the remote station
        uint8_t m_bssColor;        //!< Bss color
        uint8_t m_defaultTxPowerLevel;          //!< Default transmission power level
        uint8_t m_numberOfAntennas;
        uint8_t m_maxSupportedTxSpatialStreams;
        uint16_t m_mpduBufferSize; //!< MPDU buffer size
        uint16_t m_guardInterval;   //!< Guard interval duration (in nanoseconds) supported by the remote station
        uint16_t m_channelWidth;
        uint32_t m_maxSsrc;  //!< Maximum STA short retry count (SSRC)
        uint32_t m_maxSlrc;  //!< Maximum STA long retry count (SLRC)
        uint32_t m_ssrc; //!< short retry count
        uint32_t m_slrc; //!< long retry count
        uint32_t m_rtsCtsThreshold;             //!< Threshold for RTS/CTS
        uint32_t m_fragmentationThreshold;      //!< Current threshold for fragmentation
        uint32_t m_nextFragmentationThreshold;  //!< Threshold for fragmentation that will be used for the next transmission

        TracedCallback<Mac48Address> m_macTxRtsFailed;
        TracedCallback<Mac48Address> m_macTxDataFailed;
        TracedCallback<Mac48Address> m_macTxFinalRtsFailed;
        TracedCallback<Mac48Address> m_macTxFinalDataFailed;
    };

} //namespace ns3
#endif //MMWAVE_REMOTE_STATION_MANAGER_H
