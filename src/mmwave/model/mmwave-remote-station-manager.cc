/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/simulator.h"
#include "mmwave-phy.h"
#include "mmwave-mac.h"
#include "mmwave.h"
#include "mmwave-remote-station-manager.h"
namespace ns3 {

    MmWaveRemoteStationInfo::MmWaveRemoteStationInfo ()
            : m_memoryTime (Seconds (1.0)),
              m_lastUpdate (Seconds (0.0)),
              m_failAvg (0.0)
    {
    }

    MmWaveRemoteStationInfo::~MmWaveRemoteStationInfo ()
    {
    }

    double
    MmWaveRemoteStationInfo::CalculateAveragingCoefficient ()
    {
        double retval = std::exp ( ((m_lastUpdate - Now ()) / m_memoryTime).GetDouble () );
        m_lastUpdate = Simulator::Now ();
        return retval;
    }

    void
    MmWaveRemoteStationInfo::NotifyTxSuccess (uint32_t retryCounter)
    {
        double coefficient = CalculateAveragingCoefficient ();
        m_failAvg = static_cast<double> (retryCounter) / (1 + retryCounter) * (1 - coefficient) + coefficient * m_failAvg;
    }

    void
    MmWaveRemoteStationInfo::NotifyTxFailed ()
    {
        double coefficient = CalculateAveragingCoefficient ();
        m_failAvg = (1 - coefficient) + coefficient * m_failAvg;
    }

    double
    MmWaveRemoteStationInfo::GetFrameErrorRate () const
    {
        return m_failAvg;
    }
} //namespace ns3

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("MmWaveRemoteStationManager");
    NS_OBJECT_ENSURE_REGISTERED (MmWaveRemoteStationManager);

    TypeId
    MmWaveRemoteStationManager::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::MmWaveRemoteStationManager")
                .SetParent<Object> ()
                .SetGroupName ("MmWave")
                .AddAttribute ("MaxSsrc",
                               "The maximum number of retransmission attempts for any packet with size <= RtsCtsThreshold. "
                               "This value will not have any effect on some rate control algorithms.",
                               UintegerValue (7),
                               MakeUintegerAccessor (&MmWaveRemoteStationManager::SetMaxSsrc),
                               MakeUintegerChecker<uint32_t> ())
                .AddAttribute ("MaxSlrc",
                               "The maximum number of retransmission attempts for any packet with size > RtsCtsThreshold. "
                               "This value will not have any effect on some rate control algorithms.",
                               UintegerValue (4),
                               MakeUintegerAccessor (&MmWaveRemoteStationManager::SetMaxSlrc),
                               MakeUintegerChecker<uint32_t> ())
                .AddAttribute ("RtsCtsThreshold",
                               "If the size of the PSDU is bigger than this value, we use an RTS/CTS handshake before sending the data frame."
                               "This value will not have any effect on some rate control algorithms.",
                               UintegerValue (65535),
                               MakeUintegerAccessor (&MmWaveRemoteStationManager::SetRtsCtsThreshold),
                               MakeUintegerChecker<uint32_t> ())
                .AddAttribute ("FragmentationThreshold",
                               "If the size of the PSDU is bigger than this value, we fragment it such that the size of the fragments are equal or smaller. "
                               "This value does not apply when it is carried in an A-MPDU. "
                               "This value will not have any effect on some rate control algorithms.",
                               UintegerValue (65535),
                               MakeUintegerAccessor (&MmWaveRemoteStationManager::DoSetFragmentationThreshold,
                                                     &MmWaveRemoteStationManager::DoGetFragmentationThreshold),
                               MakeUintegerChecker<uint32_t> ())
                .AddAttribute ("NonUnicastMode",
                               "MmWave mode used for non-unicast transmissions.",
                               MmWaveModeValue (),
                               MakeMmWaveModeAccessor (&MmWaveRemoteStationManager::m_nonUnicastMode),
                               MakeMmWaveModeChecker ())
                .AddAttribute ("DefaultTxPowerLevel",
                               "Default power level to be used for transmissions. "
                               "This is the power level that is used by all those MmWaveManagers that do not implement TX power control.",
                               UintegerValue (0),
                               MakeUintegerAccessor (&MmWaveRemoteStationManager::m_defaultTxPowerLevel),
                               MakeUintegerChecker<uint8_t> ())
                .AddTraceSource ("MacTxRtsFailed",
                                 "The transmission of a RTS by the MAC layer has failed",
                                 MakeTraceSourceAccessor (&MmWaveRemoteStationManager::m_macTxRtsFailed),
                                 "ns3::Mac48Address::TracedCallback")
                .AddTraceSource ("MacTxDataFailed",
                                 "The transmission of a data packet by the MAC layer has failed",
                                 MakeTraceSourceAccessor (&MmWaveRemoteStationManager::m_macTxDataFailed),
                                 "ns3::Mac48Address::TracedCallback")
                .AddTraceSource ("MacTxFinalRtsFailed",
                                 "The transmission of a RTS has exceeded the maximum number of attempts",
                                 MakeTraceSourceAccessor (&MmWaveRemoteStationManager::m_macTxFinalRtsFailed),
                                 "ns3::Mac48Address::TracedCallback")
                .AddTraceSource ("MacTxFinalDataFailed",
                                 "The transmission of a data packet has exceeded the maximum number of attempts",
                                 MakeTraceSourceAccessor (&MmWaveRemoteStationManager::m_macTxFinalDataFailed),
                                 "ns3::Mac48Address::TracedCallback")
        ;
        return tid;
    }

    MmWaveRemoteStationManager::MmWaveRemoteStationManager ()
    {
        NS_LOG_FUNCTION (this);
        Time guardInterval = NanoSeconds (3200);
        m_guardInterval = guardInterval.GetNanoSeconds ();
        m_mpduBufferSize = 256;
        m_ldpcSupported = false;
        m_bssColor = 0;
        m_ness = 0;
        m_preamble = MMWAVE_PREAMBLE_DEFAULT;
    }

    MmWaveRemoteStationManager::~MmWaveRemoteStationManager ()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    MmWaveRemoteStationManager::DoDispose ()
    {
        NS_LOG_FUNCTION (this);
        Reset ();
    }

    void
    MmWaveRemoteStationManager::InitializePhyParams (uint8_t numberOfAntennas, uint8_t nss, uint16_t channelWidth)
    {
        NS_LOG_FUNCTION (this);
        m_numberOfAntennas = numberOfAntennas;
        m_maxSupportedTxSpatialStreams = nss;
        m_channelWidth = channelWidth;
        m_defaultTxMode = MmWavePhy::GetMmWaveMcs0 ();
        m_defaultTxMcs = MmWavePhy::GetMmWaveMcs0 ();
        m_nonUnicastMode = MmWavePhy::GetMmWaveMcs0 ();
        Reset ();
    }

    void
    MmWaveRemoteStationManager::SetupMac (const Ptr<MmWaveMac> mac)
    {
        NS_LOG_FUNCTION (this << mac);
        m_mac = mac;
        Reset ();
    }

    void
    MmWaveRemoteStationManager::Reset ()
    {
        NS_LOG_FUNCTION (this);
        for (StationStates::const_iterator i = m_states.begin (); i != m_states.end (); i++)
        {
            delete (*i);
        }
        m_states.clear ();
        for (Stations::const_iterator i = m_stations.begin (); i != m_stations.end (); i++)
        {
            delete (*i);
        }
        m_stations.clear ();
        m_ssrc = 0;
        m_slrc = 0;
    }

    void
    MmWaveRemoteStationManager::SetMaxSsrc (uint32_t maxSsrc)
    {
        NS_LOG_FUNCTION (this << maxSsrc);
        m_maxSsrc = maxSsrc;
    }

    void
    MmWaveRemoteStationManager::SetMaxSlrc (uint32_t maxSlrc)
    {
        NS_LOG_FUNCTION (this << maxSlrc);
        m_maxSlrc = maxSlrc;
    }

    void
    MmWaveRemoteStationManager::SetRtsCtsThreshold (uint32_t threshold)
    {
        NS_LOG_FUNCTION (this << threshold);
        m_rtsCtsThreshold = threshold;
    }

    void
    MmWaveRemoteStationManager::SetFragmentationThreshold (uint32_t threshold)
    {
        NS_LOG_FUNCTION (this << threshold);
        DoSetFragmentationThreshold (threshold);
    }

    void
    MmWaveRemoteStationManager::SetBssColor (uint8_t color)
    {
        m_bssColor = color;
    }

    void
    MmWaveRemoteStationManager::SetGuardInterval (uint16_t guardInterval)
    {
        m_guardInterval = guardInterval;
    }
    void
    MmWaveRemoteStationManager::SetMpduBufferSize (uint16_t size)
    {
        m_mpduBufferSize = size;
    }

    void
    MmWaveRemoteStationManager::SetPreamble (MmWavePreamble preamble)
    {
        m_preamble = preamble;
    }

    uint16_t
    MmWaveRemoteStationManager::GetGuardInterval () const
    {
        return m_guardInterval;
    }

    uint8_t
    MmWaveRemoteStationManager::GetBssColor () const
    {
        return m_bssColor;
    }

    uint16_t
    MmWaveRemoteStationManager::GetMpduBufferSize () const
    {
        return m_mpduBufferSize;
    }

    Mac48Address
    MmWaveRemoteStationManager::GetAddress (const MmWaveRemoteStation *station) const
    {
        return station->m_state->m_address;
    }

    uint16_t
    MmWaveRemoteStationManager::GetChannelWidth (const MmWaveRemoteStation *station) const
    {
        return station->m_state->m_channelWidth;
    }

    uint16_t
    MmWaveRemoteStationManager::GetGuardInterval (const MmWaveRemoteStation *station) const
    {
        return station->m_state->m_guardInterval;
    }

    uint8_t
    MmWaveRemoteStationManager::GetNess (const MmWaveRemoteStation *station) const
    {
        return station->m_state->m_ness;
    }

    MmWavePreamble
    MmWaveRemoteStationManager::GetPreamble (const MmWaveRemoteStation *station) const
    {
        return station->m_state->m_preamble;
    }

    uint8_t
    MmWaveRemoteStationManager::GetNumberOfSupportedStreams (const MmWaveRemoteStation *station) const
    {
        return station->m_state->m_nss;
    }

    Ptr<MmWaveMac>
    MmWaveRemoteStationManager::GetMac () const
    {
        return m_mac;
    }

    uint16_t
    MmWaveRemoteStationManager::GetChannelWidthSupported (Mac48Address address) const
    {
        return LookupState (address)->m_channelWidth;
    }

    void
    MmWaveRemoteStationManager::SetDefaultTxPowerLevel (uint8_t txPower)
    {
        m_defaultTxPowerLevel = txPower;
    }

    uint8_t
    MmWaveRemoteStationManager::GetNumberOfAntennas () const
    {
        return m_numberOfAntennas;
    }

    uint8_t
    MmWaveRemoteStationManager::GetMaxNumberOfTransmitStreams () const
    {
        return m_maxSupportedTxSpatialStreams;
    }

    void
    MmWaveRemoteStationManager::SetLdpcSupported (bool supported)
    {
        m_ldpcSupported = supported;
    }

    void
    MmWaveRemoteStationManager::SetLdpcSupported (Mac48Address address, bool supported)
    {
        MmWaveRemoteStationState *state;
        state = LookupState (address);
        state->m_ldpcSupported = supported;
    }

    bool
    MmWaveRemoteStationManager::GetLdpcSupported () const
    {
        return m_ldpcSupported;
    }

    bool
    MmWaveRemoteStationManager::GetLdpcSupported (Mac48Address address) const
    {
        return LookupState (address)->m_ldpcSupported;
    }
    bool
    MmWaveRemoteStationManager::UseLdpcForDestination (Mac48Address dest) const
    {
        return (GetLdpcSupported () && GetLdpcSupported (dest));
    }

    uint32_t
    MmWaveRemoteStationManager::GetFragmentationThreshold () const
    {
        return DoGetFragmentationThreshold ();
    }

    uint8_t
    MmWaveRemoteStationManager::GetDefaultTxPowerLevel () const
    {
        return m_defaultTxPowerLevel;
    }

    MmWaveRemoteStationInfo
    MmWaveRemoteStationManager::GetInfo (Mac48Address address)
    {
        MmWaveRemoteStationState *state = LookupState (address);
        return state->m_info;
    }

    MmWavePreamble
    MmWaveRemoteStationManager::GetPreamble () const
    {
        return m_preamble;
    }

    MmWaveMode
    MmWaveRemoteStationManager::GetDefaultMode () const
    {
        return m_defaultTxMode;
    }

    MmWaveMode
    MmWaveRemoteStationManager::GetDefaultMcs () const
    {
        return m_defaultTxMcs;
    }

    MmWaveMode
    MmWaveRemoteStationManager::GetNonUnicastMode () const
    {
        if (m_nonUnicastMode == MmWaveMode ())
        {
            return m_defaultTxMcs;
        }
        else
        {
            return m_nonUnicastMode;
        }
    }

    bool
    MmWaveRemoteStationManager::DoNeedRts (MmWaveRemoteStation *station, uint32_t size, bool normally)
    {
        return normally;
    }

    bool
    MmWaveRemoteStationManager::DoNeedRetransmission (MmWaveRemoteStation *station, Ptr<const Packet> packet, bool normally)
    {
        return normally;
    }

    bool
    MmWaveRemoteStationManager::DoNeedFragmentation (MmWaveRemoteStation *station, Ptr<const Packet> packet, bool normally)
    {
        return normally;
    }

    void
    MmWaveRemoteStationManager::DoReportAmpduTxStatus (MmWaveRemoteStation *station, uint8_t nSuccessfulMpdus, uint8_t nFailedMpdus, double rxSnr, double dataSnr, uint16_t dataChannelWidth, uint8_t dataNss)
    {
        NS_LOG_DEBUG ("DoReportAmpduTxStatus received but the manager does not handle A-MPDUs!");
    }

    void
    MmWaveRemoteStationManager::UpdateFragmentationThreshold ()
    {
        m_fragmentationThreshold = m_nextFragmentationThreshold;
    }

    uint32_t
    MmWaveRemoteStationManager::DoGetFragmentationThreshold () const
    {
        return m_fragmentationThreshold;
    }

    MmWaveTxVector
    MmWaveRemoteStationManager::GetDataTxVector (Mac48Address address)
    {
        NS_LOG_FUNCTION (this << address);
        if (address.IsGroup ())
        {
            MmWaveMode mode = GetNonUnicastMode ();
            MmWaveTxVector v;
            v.SetMode (mode);
            v.SetPreambleType (m_preamble);
            v.SetTxPowerLevel (m_defaultTxPowerLevel);
            v.SetChannelWidth (m_channelWidth);
            v.SetGuardInterval (m_guardInterval);
            v.SetNTx (m_numberOfAntennas);
            v.SetNss (m_numberOfAntennas);
            v.SetNess (1);
            return v;
        }
        MmWaveTxVector txVector;
        txVector = DoGetDataTxVector (Lookup (address));
        txVector.SetLdpc (UseLdpcForDestination (address));
        txVector.SetBssColor (m_bssColor);
        return txVector;
    }

    MmWaveTxVector
    MmWaveRemoteStationManager::GetCtsToSelfTxVector (Mac48Address address)
    {
        NS_LOG_FUNCTION (this << address);
        if (address.IsGroup ())
        {
            MmWaveMode mode = GetNonUnicastMode ();
            MmWaveTxVector v;
            v.SetMode (mode);
            v.SetPreambleType (m_preamble);
            v.SetTxPowerLevel (m_defaultTxPowerLevel);
            v.SetChannelWidth (m_channelWidth);
            v.SetGuardInterval (m_guardInterval);
            v.SetNTx (1);
            v.SetNss (1);
            v.SetNess (1);
            return v;
        }
        return DoGetCtrlTxVector (Lookup (address));
    }

    MmWaveTxVector
    MmWaveRemoteStationManager::GetCtrlTxVector (Mac48Address address)
    {
        NS_LOG_FUNCTION (this << address);
        if (address.IsGroup ())
        {
            MmWaveMode mode = GetNonUnicastMode ();
            MmWaveTxVector v;
            v.SetMode (mode);
            v.SetPreambleType (m_preamble);
            v.SetTxPowerLevel (m_defaultTxPowerLevel);
            v.SetChannelWidth (m_channelWidth);
            v.SetGuardInterval (m_guardInterval);
            v.SetNTx (1);
            v.SetNss (1);
            v.SetNess (1);
            return v;
        }
        return DoGetCtrlTxVector (Lookup (address));
    }

    void
    MmWaveRemoteStationManager::ReportRtsFailed (Mac48Address address, const MmWaveMacHeader *header)
    {
        NS_LOG_FUNCTION (this << address << *header);
        NS_ASSERT (!address.IsGroup ());
        m_ssrc++;
        m_macTxRtsFailed (address);
        DoReportRtsFailed (Lookup (address));
    }

    void
    MmWaveRemoteStationManager::ReportDataFailed (Mac48Address address, const MmWaveMacHeader *header, uint32_t packetSize)
    {
        NS_LOG_FUNCTION (this << address << *header);
        NS_ASSERT (!address.IsGroup ());
        bool longMpdu = (packetSize + header->GetSize () + MMWAVE_MAC_FCS_LENGTH) > m_rtsCtsThreshold;
        if (longMpdu)
        {
            m_slrc++;
        }
        else
        {
            m_ssrc++;
        }
        m_macTxDataFailed (address);
        DoReportDataFailed (Lookup (address));
    }

    void
    MmWaveRemoteStationManager::ReportRtsOk (Mac48Address address, const MmWaveMacHeader *header, double ctsSnr, MmWaveMode ctsMode, double rtsSnr)
    {
        NS_LOG_FUNCTION (this << address << *header << ctsSnr << ctsMode << rtsSnr);
        NS_ASSERT (!address.IsGroup ());
        MmWaveRemoteStation *station = Lookup (address);
        station->m_state->m_info.NotifyTxSuccess (m_ssrc);
        m_ssrc = 0;
        DoReportRtsOk (station, ctsSnr, ctsMode, rtsSnr);
    }

    void
    MmWaveRemoteStationManager::ReportDataOk (Mac48Address address, const MmWaveMacHeader *header, double ackSnr, MmWaveMode ackMode, double dataSnr, MmWaveTxVector dataTxVector, uint32_t packetSize)
    {
        NS_LOG_FUNCTION (this << address << *header << ackSnr << ackMode << dataSnr << dataTxVector << packetSize);
        NS_ASSERT (!address.IsGroup ());
        MmWaveRemoteStation *station = Lookup (address);
        bool longMpdu = (packetSize + header->GetSize () + MMWAVE_MAC_FCS_LENGTH) > m_rtsCtsThreshold;
        if (longMpdu)
        {
            station->m_state->m_info.NotifyTxSuccess (m_slrc);
            m_slrc = 0;
        }
        else
        {
            station->m_state->m_info.NotifyTxSuccess (m_ssrc);
            m_ssrc = 0;
        }
        DoReportDataOk (station, ackSnr, ackMode, dataSnr, dataTxVector.GetChannelWidth (), dataTxVector.GetNss ());
    }

    void
    MmWaveRemoteStationManager::ReportFinalRtsFailed (Mac48Address address, const MmWaveMacHeader *header)
    {
        NS_LOG_FUNCTION (this << address << *header);
        NS_ASSERT (!address.IsGroup ());
        MmWaveRemoteStation *station = Lookup (address);
        station->m_state->m_info.NotifyTxFailed ();
        m_ssrc = 0;
        m_macTxFinalRtsFailed (address);
        DoReportFinalRtsFailed (station);
    }

    void
    MmWaveRemoteStationManager::ReportFinalDataFailed (Mac48Address address, const MmWaveMacHeader *header, uint32_t packetSize)
    {
        NS_LOG_FUNCTION (this << address << *header);
        NS_ASSERT (!address.IsGroup ());
        MmWaveRemoteStation *station = Lookup (address);
        station->m_state->m_info.NotifyTxFailed ();
        bool longMpdu = (packetSize + header->GetSize () + MMWAVE_MAC_FCS_LENGTH) > m_rtsCtsThreshold;
        if (longMpdu)
        {
            m_slrc = 0;
        }
        else
        {
            m_ssrc = 0;
        }
        m_macTxFinalDataFailed (address);
        DoReportFinalDataFailed (station);
    }

    void
    MmWaveRemoteStationManager::ReportRxOk (Mac48Address address, double rxSnr, MmWaveMode txMode)
    {
        NS_LOG_FUNCTION (this << address << rxSnr << txMode);
        if (address.IsGroup ())
        {
            return;
        }
        DoReportRxOk (Lookup (address), rxSnr, txMode);
    }

    void
    MmWaveRemoteStationManager::ReportAmpduTxStatus (Mac48Address address, uint8_t nSuccessfulMpdus, uint8_t nFailedMpdus, double rxSnr, double dataSnr, MmWaveTxVector dataTxVector)
    {
        NS_LOG_FUNCTION (this << address << +nSuccessfulMpdus << +nFailedMpdus << rxSnr << dataSnr << dataTxVector);
        NS_ASSERT (!address.IsGroup ());
        for (uint8_t i = 0; i < nFailedMpdus; i++)
        {
            m_macTxDataFailed (address);
        }
        DoReportAmpduTxStatus (Lookup (address), nSuccessfulMpdus, nFailedMpdus, rxSnr, dataSnr, dataTxVector.GetChannelWidth (), dataTxVector.GetNss ());
    }

    bool
    MmWaveRemoteStationManager::NeedRts (const MmWaveMacHeader &header, uint32_t size)
    {
        NS_LOG_FUNCTION (this << header << size);
        Mac48Address address = header.GetAddr1 ();
        if (address.IsGroup ())
        {
            return false;
        }
        bool normally = (size > m_rtsCtsThreshold);
        return DoNeedRts (Lookup (address), size, normally);
    }

    bool
    MmWaveRemoteStationManager::NeedRetransmission (Mac48Address address, const MmWaveMacHeader *header, Ptr<const Packet> packet)
    {
        NS_LOG_FUNCTION (this << address << packet << *header);
        NS_ASSERT (!address.IsGroup ());
        bool longMpdu = (packet->GetSize () + header->GetSize () + MMWAVE_MAC_FCS_LENGTH) > m_rtsCtsThreshold;
        uint32_t retryCount, maxRetryCount;
        if (longMpdu)
        {
            retryCount = m_slrc;
            maxRetryCount = m_maxSlrc;
        }
        else
        {
            retryCount = m_ssrc;
            maxRetryCount = m_maxSsrc;
        }
        bool normally = retryCount < maxRetryCount;
        NS_LOG_DEBUG ("MmWaveRemoteStationManager::NeedRetransmission count: " << retryCount << " result: " << std::boolalpha << normally);
        return DoNeedRetransmission (Lookup (address), packet, normally);
    }

    bool
    MmWaveRemoteStationManager::NeedFragmentation (Mac48Address address, const MmWaveMacHeader *header, Ptr<const Packet> packet)
    {
        NS_LOG_FUNCTION (this << address << packet << *header);
        if (address.IsGroup ())
        {
            return false;
        }
        bool normally = (packet->GetSize () + header->GetSize () + MMWAVE_MAC_FCS_LENGTH) > GetFragmentationThreshold ();
        NS_LOG_DEBUG ("MmWaveRemoteStationManager::NeedFragmentation result: " << std::boolalpha << normally);
        return DoNeedFragmentation (Lookup (address), packet, normally);
    }

    void
    MmWaveRemoteStationManager::DoSetFragmentationThreshold (uint32_t threshold)
    {
        NS_LOG_FUNCTION (this << threshold);
        if (threshold < 256)
        {
            NS_LOG_WARN ("Fragmentation threshold should be larger than 256. Setting to 256.");
            m_nextFragmentationThreshold = 256;
        }
        else
        {
            if (threshold % 2 != 0)
            {
                NS_LOG_WARN ("Fragmentation threshold should be an even number. Setting to " << threshold - 1);
                m_nextFragmentationThreshold = threshold - 1;
            }
            else
            {
                m_nextFragmentationThreshold = threshold;
            }
        }
    }

    uint32_t
    MmWaveRemoteStationManager::GetNFragments (const MmWaveMacHeader *header, Ptr<const Packet> packet)
    {
        NS_LOG_FUNCTION (this << *header << packet);
        //The number of bytes a fragment can support is (Threshold - MMWAVE_HEADER_SIZE - MMWAVE_FCS).
        uint32_t nFragments = (packet->GetSize () / (GetFragmentationThreshold () - header->GetSize () - MMWAVE_MAC_FCS_LENGTH));

        //If the size of the last fragment is not 0.
        if ((packet->GetSize () % (GetFragmentationThreshold () - header->GetSize () - MMWAVE_MAC_FCS_LENGTH)) > 0)
        {
            nFragments++;
        }
        NS_LOG_DEBUG ("MmWaveRemoteStationManager::GetNFragments returning " << nFragments);
        return nFragments;
    }

    uint32_t
    MmWaveRemoteStationManager::GetFragmentSize (Mac48Address address, const MmWaveMacHeader *header, Ptr<const Packet> packet, uint32_t fragmentNumber)
    {
        NS_LOG_FUNCTION (this << address << *header << packet << fragmentNumber);
        NS_ASSERT (!address.IsGroup ());
        uint32_t nFragment = GetNFragments (header, packet);
        if (fragmentNumber >= nFragment)
        {
            NS_LOG_DEBUG ("MmWaveRemoteStationManager::GetFragmentSize returning 0");
            return 0;
        }
        //Last fragment
        if (fragmentNumber == nFragment - 1)
        {
            uint32_t lastFragmentSize = packet->GetSize () - (fragmentNumber * (GetFragmentationThreshold () - header->GetSize () - MMWAVE_MAC_FCS_LENGTH));
            NS_LOG_DEBUG ("MmWaveRemoteStationManager::GetFragmentSize returning " << lastFragmentSize);
            return lastFragmentSize;
        }
            //All fragments but the last, the number of bytes is (Threshold - MMWAVE_HEADER_SIZE - MMWAVE_FCS).
        else
        {
            uint32_t fragmentSize = GetFragmentationThreshold () - header->GetSize () - MMWAVE_MAC_FCS_LENGTH;
            NS_LOG_DEBUG ("MmWaveRemoteStationManager::GetFragmentSize returning " << fragmentSize);
            return fragmentSize;
        }
    }

    uint32_t
    MmWaveRemoteStationManager::GetFragmentOffset (Mac48Address address, const MmWaveMacHeader *header, Ptr<const Packet> packet, uint32_t fragmentNumber)
    {
        NS_LOG_FUNCTION (this << address << *header << packet << fragmentNumber);
        NS_ASSERT (!address.IsGroup ());
        NS_ASSERT (fragmentNumber < GetNFragments (header, packet));
        uint32_t fragmentOffset = fragmentNumber * (GetFragmentationThreshold () - header->GetSize () - MMWAVE_MAC_FCS_LENGTH);
        NS_LOG_DEBUG ("MmWaveRemoteStationManager::GetFragmentOffset returning " << fragmentOffset);
        return fragmentOffset;
    }

    bool
    MmWaveRemoteStationManager::IsLastFragment (Mac48Address address, const MmWaveMacHeader *header, Ptr<const Packet> packet, uint32_t fragmentNumber)
    {
        NS_LOG_FUNCTION (this << address << *header << packet << fragmentNumber);
        NS_ASSERT (!address.IsGroup ());
        bool isLast = fragmentNumber == (GetNFragments (header, packet) - 1);
        NS_LOG_DEBUG ("MmWaveRemoteStationManager::IsLastFragment returning " << std::boolalpha << isLast);
        return isLast;
    }

    MmWaveRemoteStationState *
    MmWaveRemoteStationManager::LookupState (Mac48Address address) const
    {
        NS_LOG_FUNCTION (this << address);
        for (StationStates::const_iterator i = m_states.begin (); i != m_states.end (); i++)
        {
            if ((*i)->m_address == address)
            {
                NS_LOG_DEBUG ("MmWaveRemoteStationManager::LookupState returning existing state");
                return (*i);
            }
        }
        MmWaveRemoteStationState *state = new MmWaveRemoteStationState ();
        state->m_address = address;
        state->m_channelWidth = m_channelWidth;
        state->m_guardInterval = GetGuardInterval ();
        state->m_ness = 0;
        state->m_nss = GetMaxNumberOfTransmitStreams ();
        state->m_mpduBufferSize = GetMpduBufferSize ();
        state->m_preamble = GetPreamble ();
        state->m_bssColor = GetBssColor ();
        state->m_ldpcSupported = GetLdpcSupported();

        const_cast<MmWaveRemoteStationManager *> (this)->m_states.push_back (state);
        NS_LOG_DEBUG ("MmWaveRemoteStationManager::LookupState returning new state");
        return state;
    }

    MmWaveRemoteStation *
    MmWaveRemoteStationManager::Lookup (Mac48Address address) const
    {
        NS_LOG_FUNCTION (this << address);
        for (Stations::const_iterator i = m_stations.begin (); i != m_stations.end (); i++)
        {
            if ((*i)->m_state->m_address == address)
            {
                return (*i);
            }
        }
        MmWaveRemoteStationState *state = LookupState (address);
        MmWaveRemoteStation *station = DoCreateStation ();
        station->m_state = state;
        const_cast<MmWaveRemoteStationManager *> (this)->m_stations.push_back (station);
        return station;
    }

} //namespace ns3