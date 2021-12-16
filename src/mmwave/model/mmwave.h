/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_H
#define MMWAVE_H
#include <list>
#include <map>
#include "ns3/nstime.h"
#include "ns3/mac48-address.h"
#include "ns3/fatal-error.h"


namespace ns3 {
    class MmWaveMacHeader;

    const uint16_t MMWAVE_SEQNO_SPACE_SIZE = 4096;
    const uint16_t MMWAVE_SEQNO_SPACE_HALF_SIZE = MMWAVE_SEQNO_SPACE_SIZE / 2;
    const uint16_t MMWAVE_MAC_FCS_LENGTH = 4;

    enum MmWavePhyStandard
    {
        MMWAVE_PHY_STANDARD_320MHz,
        MMWAVE_PHY_STANDARD_640MHz,
        MMWAVE_PHY_STANDARD_1280MHz,
        MMWAVE_PHY_STANDARD_UNSPECIFIED
    };

    inline std::ostream& operator << (std::ostream& os, MmWavePhyStandard standard)
    {
        switch (standard)
        {
            case MMWAVE_PHY_STANDARD_320MHz:
                return (os << "MMWAVE_PHY_STANDARD_320MHz");
            case MMWAVE_PHY_STANDARD_640MHz:
                return (os << "MMWAVE_PHY_STANDARD_640MHz");
            case MMWAVE_PHY_STANDARD_1280MHz:
                return (os << "MMWAVE_PHY_STANDARD_1280MHz");
            case MMWAVE_PHY_STANDARD_UNSPECIFIED:
            default:
                return (os << "MMWAVE_PHY_STANDARD_UNSPECIFIED");
        }
    }

    enum MmWaveMacStandard
    {
        MMWAVE_MAC_STANDARD_COGNITIVE_RADIO,
        MMWAVE_MAC_STANDARD_SUB6GHz_ASSISTED,
        MMWAVE_MAC_STANDARD_JAMMER,
        MMWAVE_MAC_STANDARD_UNSPECIFIED
    };

    inline std::ostream& operator << (std::ostream& os, MmWaveMacStandard standard)
    {
        switch (standard)
        {
            case MMWAVE_MAC_STANDARD_COGNITIVE_RADIO:
                return (os << "MMWAVE_MAC_STANDARD_COGNITIVE_RADIO");
            case MMWAVE_MAC_STANDARD_SUB6GHz_ASSISTED:
                return (os << "MMWAVE_MAC_STANDARD_SUB6GHz_ASSISTED");
            case MMWAVE_MAC_STANDARD_JAMMER:
                return (os << "MMWAVE_MAC_STANDARD_JAMMER");
            case MMWAVE_MAC_STANDARD_UNSPECIFIED:
            default:
                return (os << "MMWAVE_MAC_STANDARD_UNSPECIFIED");
        }
    }

    enum MmWavePhyBand
    {
        MMWAVE_PHY_BAND_60GHZ,
        MMWAVE_PHY_BAND_56GHZ,
        MMWAVE_PHY_BAND_60GHz_56GHZ,
        MMWAVE_PHY_BAND_UNSPECIFIED
    };

    inline std::ostream& operator << (std::ostream& os, MmWavePhyBand band)
    {
        switch (band)
        {
            case MMWAVE_PHY_BAND_60GHZ:
                return (os << "MMWAVE_PHY_BAND_60GHZ");
            case MMWAVE_PHY_BAND_56GHZ:
                return (os << "MMWAVE_PHY_BAND_56GHZ");
            case MMWAVE_PHY_BAND_60GHz_56GHZ:
                return (os << "MMWAVE_PHY_BAND_60GHz_56GHZ");
            case MMWAVE_PHY_BAND_UNSPECIFIED:
            default:
                return (os << "MMWAVE_PHY_BAND_UNSPECIFIED");
        }
    }

    enum MmWaveStandard
    {
        MMWAVE_UNSPECIFIED,
        MMWAVE_COGNITIVE_RADIO_60GHz_1C_1280MHz,
        MMWAVE_COGNITIVE_RADIO_60GHz_2C_640MHz,
        MMWAVE_COGNITIVE_RADIO_60GHz_4C_320MHz,
        MMWAVE_COGNITIVE_RADIO_60GHz56GHz_1280MHz,
        MMWAVE_JAMMER_60GHz_1280MHz,
        MMWAVE_JAMMER_60GHz_640MHz,
        MMWAVE_JAMMER_60GHz_320MHz,
        MMWAVE_JAMMER_56GHz_1280MHz,
        MMWAVE_SUB6GHz_ASSISTED_60GHz_1280MHz,
    };

    inline std::ostream& operator<< (std::ostream& os, MmWaveStandard standard)
    {
        switch (standard)
        {
            case MMWAVE_COGNITIVE_RADIO_60GHz_1C_1280MHz:
                return (os << "MMWAVE_COGNITIVE_RADIO_60GHz_1C_1280MHz");
            case MMWAVE_COGNITIVE_RADIO_60GHz_2C_640MHz:
                return (os << "MMWAVE_COGNITIVE_RADIO_60GHz_2C_640MHz");
            case MMWAVE_COGNITIVE_RADIO_60GHz_4C_320MHz:
                return (os << "MMWAVE_COGNITIVE_RADIO_60GHz_4C_320MHz");
            case MMWAVE_COGNITIVE_RADIO_60GHz56GHz_1280MHz:
                return (os << "MMWAVE_COGNITIVE_RADIO_60GHz56GHz_1280MHz");
            case MMWAVE_JAMMER_60GHz_1280MHz:
                return (os << "MMWAVE_JAMMER_60GHz_1280MHz");
            case MMWAVE_JAMMER_60GHz_640MHz:
                return (os << "MMWAVE_JAMMER_60GHz_640MHz");
            case MMWAVE_JAMMER_60GHz_320MHz:
                return (os << "MMWAVE_JAMMER_60GHz_320MHz");
            case MMWAVE_JAMMER_56GHz_1280MHz:
                return (os << "MMWAVE_JAMMER_56GHz_1280MHz");
            case MMWAVE_SUB6GHz_ASSISTED_60GHz_1280MHz:
                return (os << "MMWAVE_SUB6GHz_ASSISTED_60GHz_1280MHz");
            case MMWAVE_UNSPECIFIED:
            default:
                return (os << "MMWAVE_UNSPECIFIED");
        }
    }

    typedef std::pair<uint8_t, MmWavePhyBand> MmWaveChannelNumberBandPair;
    typedef std::pair<MmWaveChannelNumberBandPair, MmWavePhyStandard> MmWaveChannelNumberStandardPair;
    typedef std::pair<uint16_t, uint16_t> MmWaveFrequencyWidthPair;
    typedef std::map<MmWaveChannelNumberStandardPair, MmWaveFrequencyWidthPair> MmWaveChannelToFrequencyWidthMap;

    inline std::ostream& operator << (std::ostream& os, MmWaveChannelNumberStandardPair channel)
    {
        return (os << "ChannelNumber:" << +(channel.first.first));
    }

    struct MmWaveStandardInfo
    {
        MmWavePhyStandard m_phyStandard;
        MmWavePhyBand m_phyBand;
        MmWaveMacStandard m_macStandard;
    };

    const std::map<MmWaveStandard, MmWaveStandardInfo> mmWaveStandards =
            {
                    {MMWAVE_UNSPECIFIED, {MMWAVE_PHY_STANDARD_UNSPECIFIED, MMWAVE_PHY_BAND_UNSPECIFIED, MMWAVE_MAC_STANDARD_UNSPECIFIED}},
                    {MMWAVE_COGNITIVE_RADIO_60GHz_1C_1280MHz, {MMWAVE_PHY_STANDARD_1280MHz, MMWAVE_PHY_BAND_60GHZ, MMWAVE_MAC_STANDARD_COGNITIVE_RADIO}},
                    {MMWAVE_COGNITIVE_RADIO_60GHz_2C_640MHz, {MMWAVE_PHY_STANDARD_640MHz, MMWAVE_PHY_BAND_60GHZ, MMWAVE_MAC_STANDARD_COGNITIVE_RADIO}},
                    {MMWAVE_COGNITIVE_RADIO_60GHz_4C_320MHz, {MMWAVE_PHY_STANDARD_320MHz, MMWAVE_PHY_BAND_60GHZ, MMWAVE_MAC_STANDARD_COGNITIVE_RADIO}},
                    {MMWAVE_COGNITIVE_RADIO_60GHz56GHz_1280MHz, {MMWAVE_PHY_STANDARD_1280MHz, MMWAVE_PHY_BAND_60GHz_56GHZ, MMWAVE_MAC_STANDARD_COGNITIVE_RADIO}},
                    {MMWAVE_SUB6GHz_ASSISTED_60GHz_1280MHz, {MMWAVE_PHY_STANDARD_1280MHz, MMWAVE_PHY_BAND_60GHZ, MMWAVE_MAC_STANDARD_SUB6GHz_ASSISTED}},
                    {MMWAVE_JAMMER_60GHz_1280MHz, {MMWAVE_PHY_STANDARD_1280MHz, MMWAVE_PHY_BAND_60GHZ, MMWAVE_MAC_STANDARD_JAMMER}},
                    {MMWAVE_JAMMER_60GHz_640MHz, {MMWAVE_PHY_STANDARD_640MHz, MMWAVE_PHY_BAND_60GHZ, MMWAVE_MAC_STANDARD_JAMMER}},
                    {MMWAVE_JAMMER_60GHz_320MHz, {MMWAVE_PHY_STANDARD_320MHz, MMWAVE_PHY_BAND_60GHZ, MMWAVE_MAC_STANDARD_JAMMER}},
                    {MMWAVE_JAMMER_56GHz_1280MHz, {MMWAVE_PHY_STANDARD_1280MHz, MMWAVE_PHY_BAND_56GHZ, MMWAVE_MAC_STANDARD_JAMMER}}
            };

    const MmWaveChannelToFrequencyWidthMap mmWaveChannelToFrequency = {
            // 60GHz 1280MHz 1Channel
            { { {1, MMWAVE_PHY_BAND_60GHZ}, MMWAVE_PHY_STANDARD_1280MHz}, {60000, 1280} },
            // 56GHz 1280MHz 1Channel
            { { {2, MMWAVE_PHY_BAND_56GHZ}, MMWAVE_PHY_STANDARD_1280MHz}, {56000, 1280} },
            // 60GHz 640MHz 2Channel
            { { {3, MMWAVE_PHY_BAND_60GHZ}, MMWAVE_PHY_STANDARD_640MHz}, {59680, 640} },
            { { {4, MMWAVE_PHY_BAND_60GHZ}, MMWAVE_PHY_STANDARD_640MHz}, {60320, 640} },
            // 60GHz 320MHz 4Channel
            { { {5, MMWAVE_PHY_BAND_60GHZ}, MMWAVE_PHY_STANDARD_320MHz}, {59520, 320} },
            { { {6, MMWAVE_PHY_BAND_60GHZ}, MMWAVE_PHY_STANDARD_320MHz}, {59840, 320} },
            { { {7, MMWAVE_PHY_BAND_60GHZ}, MMWAVE_PHY_STANDARD_320MHz}, {60160, 320} },
            { { {8, MMWAVE_PHY_BAND_60GHZ}, MMWAVE_PHY_STANDARD_320MHz}, {60480, 320} },
            // 60GHz-56GHz 1280MHz
            { { {9, MMWAVE_PHY_BAND_60GHz_56GHZ}, MMWAVE_PHY_STANDARD_1280MHz}, {60000, 1280} },
            { { {10, MMWAVE_PHY_BAND_60GHz_56GHZ}, MMWAVE_PHY_STANDARD_1280MHz}, {56000, 1280} },
    };

    enum MmWavePreamble
    {
        MMWAVE_PREAMBLE_DEFAULT = 0,
        MMWAVE_PREAMBLE_UNSPECIFIED = 100
    };

    inline std::ostream& operator<< (std::ostream& os, MmWavePreamble preamble)
    {
        switch (preamble)
        {
            case MMWAVE_PREAMBLE_DEFAULT:
                return (os << "MMWAVE_PREAMBLE_DEFAULT");
            case MMWAVE_PREAMBLE_UNSPECIFIED:
                return (os << "MMWAVE_PREAMBLE_UNSPECIFIED");
            default:
                NS_FATAL_ERROR ("Invalid preamble");
                return (os << "INVALID");
        }
    }

    enum MmWaveMpduType
    {
        MMWAVE_NORMAL_MPDU
    };

    inline std::ostream& operator<< (std::ostream& os, MmWaveMpduType mpduType)
    {
        switch (mpduType)
        {
            case MMWAVE_NORMAL_MPDU:
                return (os << "NORMAL_MPDU");
            default:
                NS_FATAL_ERROR ("Invalid mpdu");
                return (os << "INVALID");
        }
    }

    enum TypeOfGroup
    {
        PROBE_GROUP = 0,
        INTRA_GROUP = 1,
        INTER_GROUP = 2
    };

    inline std::ostream& operator<< (std::ostream& os, TypeOfGroup type)
    {
        switch (type)
        {
            case INTRA_GROUP:
                return (os << "INTRA_GROUP");
            case INTER_GROUP:
                return (os << "INTER_GROUP");
            case PROBE_GROUP:
                return (os << "PROBE_GROUP");
            default:
                return (os << "INVALID");
        }
    }

    enum MmWaveChannelState
    {
        UNUTILIZED = 0,
        UTILIZED_BY_PUs = 1,
        UTILIZED_BY_SUs = 2
    };

    inline std::ostream& operator<< (std::ostream& os, MmWaveChannelState state)
    {
        switch (state)
        {
            case UNUTILIZED:
                return (os << "UNUTILIZED");
            case UTILIZED_BY_PUs:
                return (os << "UTILIZED_BY_PUs");
            case UTILIZED_BY_SUs:
                return (os << "UTILIZED_BY_SUs");
            default:
                NS_FATAL_ERROR ("Invalid State");
                return (os << "INVALID");
        }
    }

    enum MmWaveMacState
    {
        INTRA_SUSPEND,
        INTRA_SWITCH,
        INTRA_TRANSMISSION,
        INTRA_DETECTION,

        INTER_SUSPEND,
        INTER_SWITCH,
        INTER_TRANSMISSION,

        PROBE_SUSPEND,
        PROBE_SWITCH,
        PROBE_DETECTION
    };

    inline std::ostream& operator<< (std::ostream& os, MmWaveMacState state)
    {
        switch (state)
        {
            case INTRA_SUSPEND:
                return (os << "INTRA_SUSPEND");
            case INTRA_SWITCH:
                return (os << "INTRA_SWITCH");
            case INTRA_TRANSMISSION:
                return (os << "INTRA_TRANSMISSION");
            case INTRA_DETECTION:
                return (os << "INTRA_DETECTION");
            case INTER_SUSPEND:
                return (os << "INTER_SUSPEND");
            case INTER_SWITCH:
                return (os << "INTER_SWITCH");
            case INTER_TRANSMISSION:
                return (os << "INTER_TRANSMISSION");
            case PROBE_SWITCH:
                return (os << "PROBE_SWITCH");
            case PROBE_DETECTION:
                return (os << "PROBE_DETECTION");
            default:
                NS_FATAL_ERROR ("Invalid state");
                return (os << "INVALID");
        }
    }

    enum MmWavePhyRxfailureReason
    {
        MMWAVE_UNKNOWN = 0,
        MMWAVE_UNSUPPORTED_SETTINGS,
        MMWAVE_CHANNEL_SWITCHING,
        MMWAVE_RXING,
        MMWAVE_TXING,
        MMWAVE_SLEEPING,
        MMWAVE_BUSY_DECODING_PREAMBLE,
        MMWAVE_PREAMBLE_DETECT_FAILURE,
        MMWAVE_RECEPTION_ABORTED_BY_TX,
        MMWAVE_L_SIG_FAILURE,
        MMWAVE_SIG_A_FAILURE,
        MMWAVE_PREAMBLE_DETECTION_PACKET_SWITCH,
        MMWAVE_FRAME_CAPTURE_PACKET_SWITCH,
        MMWAVE_OBSS_PD_CCA_RESET,
        MMWAVE_FILTERED
    };

    inline std::ostream &operator<<(std::ostream &os, MmWavePhyRxfailureReason reason)
    {
        switch (reason) {
            case MMWAVE_UNSUPPORTED_SETTINGS:
                return (os << "UNSUPPORTED_SETTINGS");
            case MMWAVE_CHANNEL_SWITCHING:
                return (os << "CHANNEL_SWITCHING");
            case MMWAVE_RXING:
                return (os << "RXING");
            case MMWAVE_TXING:
                return (os << "TXING");
            case MMWAVE_SLEEPING:
                return (os << "SLEEPING");
            case MMWAVE_BUSY_DECODING_PREAMBLE:
                return (os << "BUSY_DECODING_PREAMBLE");
            case MMWAVE_PREAMBLE_DETECT_FAILURE:
                return (os << "PREAMBLE_DETECT_FAILURE");
            case MMWAVE_RECEPTION_ABORTED_BY_TX:
                return (os << "RECEPTION_ABORTED_BY_TX");
            case MMWAVE_L_SIG_FAILURE:
                return (os << "L_SIG_FAILURE");
            case MMWAVE_SIG_A_FAILURE:
                return (os << "SIG_A_FAILURE");
            case MMWAVE_PREAMBLE_DETECTION_PACKET_SWITCH:
                return (os << "PREAMBLE_DETECTION_PACKET_SWITCH");
            case MMWAVE_FRAME_CAPTURE_PACKET_SWITCH:
                return (os << "FRAME_CAPTURE_PACKET_SWITCH");
            case MMWAVE_OBSS_PD_CCA_RESET:
                return (os << "OBSS_PD_CCA_RESET");
            case MMWAVE_FILTERED:
                return (os << "FILTERED");
            case MMWAVE_UNKNOWN:
            default:
                NS_FATAL_ERROR("Unknown reason");
                return (os << "UNKNOWN");
        }
    }

    enum MmWaveAccessMode
    {
        MMWAVE_SINGLE_CHANNE = 0,
        MMWAVE_MULTI_CHANNEL
    };

    inline std::ostream &operator<<(std::ostream &os, MmWaveAccessMode mode)
    {
        switch (mode) {
            case MMWAVE_SINGLE_CHANNE:
                return (os << "MMWAVE_SINGLE_CHANNE");
            case MMWAVE_MULTI_CHANNEL:
                return (os << "MMWAVE_MULTI_CHANNEL");
            default:
                NS_FATAL_ERROR("Unknown");
                return (os << "UNKNOWN");
        }
    }

    enum TypeOfAccessMode
    {
        DETECTION_ACCESS,
        BEACON_ACCESS,
        BULK_ACCESS,
    };

    inline std::ostream &operator<<(std::ostream &os, TypeOfAccessMode mode)
    {
        switch (mode) {
            case DETECTION_ACCESS:
                return (os << "DETECTION_ACCESS");
            case BEACON_ACCESS:
                return (os << "BEACON_ACCESS");
            case BULK_ACCESS:
                return (os << "BULK_ACCESS");
            default:
                NS_FATAL_ERROR("Unknown");
                return (os << "UNKNOWN");
        }
    }

    struct MmWaveSignalNoiseDbm
    {
        double signal; ///< in dBm
        double noise; ///< in dBm
    };

    struct MmWaveMpduInfo
    {
        MmWaveMpduType type; ///< type
        uint32_t mpduRefNumber; ///< MPDU ref number
    };

    struct MmWavePreambleParameters
    {
        double rssiW; ///< RSSI in W
        uint8_t bssColor; ///< Bss color
    };

    class MmWaveDevice : public SimpleRefCount<MmWaveDevice>
    {
    public:
        Mac48Address m_address;
        Mac48Address m_group;
        bool m_isNeighbor;
        Time m_stampTime;
        MmWaveChannelNumberStandardPair m_channel;
    };

    class CrGroupInfo : public SimpleRefCount<CrGroupInfo>
    {
    public:
        std::map<uint16_t, Mac48Address> m_address;
        Time m_stampTime;
    };

    class CrChannelAccessInfo : public SimpleRefCount<CrChannelAccessInfo>
    {
    public:
        MmWaveChannelNumberStandardPair m_channel;
        Time m_stampTime;
    };

    class MmWaveNeighborDevice : public SimpleRefCount<MmWaveNeighborDevice>
    {
    public:
        Mac48Address m_address;
        MmWaveChannelNumberStandardPair m_channel;
        Time m_stamp;
    };

    typedef std::map<Mac48Address, Ptr<MmWaveNeighborDevice>> MmWaveNeighborDevices;

    class BulkAccessInfo : public SimpleRefCount<BulkAccessInfo>
    {
    public:
        Mac48Address m_address;
        Time m_txDuration;
        uint8_t m_numOfPackets;
        uint8_t m_numOfReceived;
        uint16_t m_startingSeq;
        uint64_t m_bitmap;
    };
}

#endif /* MMWAVE_H */

