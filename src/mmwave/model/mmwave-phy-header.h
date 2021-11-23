/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_PHY_HEADER_H
#define MMWAVE_PHY_HEADER_H
#include "ns3/header.h"
#include "ns3/type-id.h"

namespace ns3 {

    class MmWaveSigHeader : public Header
    {
    public:
        MmWaveSigHeader ();
        virtual ~MmWaveSigHeader ();

        static TypeId GetTypeId ();
        TypeId GetInstanceTypeId () const;
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize () const;
        void Serialize (Buffer::Iterator start) const;
        uint32_t Deserialize (Buffer::Iterator start);

        void SetMcs (uint8_t mcs);
        uint8_t GetMcs () const;
        void SetBssColor (uint8_t bssColor);
        uint8_t GetBssColor () const;
        void SetChannelWidth (uint16_t channelWidth);
        uint16_t GetChannelWidth () const;
        void SetGuardIntervalAndLtfSize (uint16_t gi, uint8_t ltf);
        uint16_t GetGuardInterval () const;
        void SetNStreams (uint8_t nStreams);
        uint8_t GetNStreams () const;
        void SetCoding (bool ldpc);
        bool IsLdpcCoding () const;
        void SetLength (uint16_t length);
        uint16_t GetLength () const;
    private:
        //HE-SIG-A1 fields
        uint8_t m_format;       ///< Format bit
        uint8_t m_bssColor;     ///< Group color field
        uint8_t m_ul_dl;        ///< UL/DL bit
        uint8_t m_mcs;          ///< MCS field
        uint8_t m_spatialReuse; ///< Spatial Reuse field
        uint8_t m_bandwidth;    ///< Bandwidth field
        uint8_t m_gi_ltf_size;  ///< GI+LTF Size field
        uint8_t m_nsts;         ///< NSTS
        uint8_t m_coding; ///< Coding (0 for BCC, 1 for LDPC)
        uint16_t m_length; ///< LENGTH field
    };

} //namespace ns3
#endif //MMWAVE_PHY_HEADER_H
