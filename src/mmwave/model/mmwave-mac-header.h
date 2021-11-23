/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_MAC_HEADER_H
#define MMWAVE_MAC_HEADER_H
#include <map>
#include "ns3/nstime.h"
#include "ns3/header.h"
#include "ns3/type-id.h"
#include "ns3/address-utils.h"
#include "ns3/object.h"
#include "ns3/mac48-address.h"
#include "mmwave.h"

namespace ns3 {
    class Time;
    enum MmWaveMacType
    {
        MMWAVE_MAC_CTL_CTLWRAPPER = 0,
        MMWAVE_MAC_CTL_RTS,
        MMWAVE_MAC_CTL_CTS,
        MMWAVE_MAC_CTL_ACK,
        MMWAVE_MAC_MGT_BEACON,
        MMWAVE_MAC_MGT_DETECTION,
        MMWAVE_MAC_MGT_BULK_REQUEST,
        MMWAVE_MAC_MGT_BULK_RESPONSE,
        MMWAVE_MAC_MGT_BULK_ACK,
        MMWAVE_MAC_DATA
    };

    inline std::ostream& operator<< (std::ostream& os, MmWaveMacType type)
    {
        switch (type)
        {
            case MMWAVE_MAC_CTL_CTLWRAPPER:
                return (os << "MMWAVE_MAC_CTL_CTLWRAPPER");
            case MMWAVE_MAC_CTL_RTS:
                return (os << "MMWAVE_MAC_CTL_RTS");
            case MMWAVE_MAC_CTL_CTS:
                return (os << "MMWAVE_MAC_CTL_CTS");
            case MMWAVE_MAC_CTL_ACK:
                return (os << "MMWAVE_MAC_CTL_ACK");
            case MMWAVE_MAC_MGT_BEACON:
                return (os << "MMWAVE_MAC_MGT_BEACON");
            case MMWAVE_MAC_MGT_DETECTION:
                return (os << "MMWAVE_MAC_MGT_DETECTION");
            case MMWAVE_MAC_MGT_BULK_REQUEST:
                return (os << "MMWAVE_MAC_MGT_BULK_REQUEST");
            case MMWAVE_MAC_MGT_BULK_RESPONSE:
                return (os << "MMWAVE_MAC_MGT_BULK_RESPONSE");
            case MMWAVE_MAC_MGT_BULK_ACK:
                return (os << "MMWAVE_MAC_MGT_BULK_ACK");
            case MMWAVE_MAC_DATA:
                return (os << "MMWAVE_MAC_DATA");
            default:
                NS_FATAL_ERROR ("MMWAVE_UNSPECIFIED");
                return (os << "MMWAVE_UNSPECIFIED");
        }
    }

    enum
    {
        MMWAVE_TYPE_MGT = 0,
        MMWAVE_TYPE_CTL = 1,
        MMWAVE_TYPE_DATA = 2
    };

    enum
    {
        MMWAVE_SUBTYPE_CTL_CTLWRAPPER = 0,
        MMWAVE_SUBTYPE_CTL_RTS = 1,
        MMWAVE_SUBTYPE_CTL_CTS = 2,
        MMWAVE_SUBTYPE_CTL_ACK = 3,
    };

    enum
    {
        MMWAVE_SUBTYPE_MGT_BEACON = 0,
        MMWAVE_SUBTYPE_MGT_DETECTION = 1,
        MMWAVE_SUBTYPE_MGT_BULK_REQUEST = 2,
        MMWAVE_SUBTYPE_MGT_BULK_RESPONSE = 3,
        MMWAVE_SUBTYPE_MGT_BULK_ACK = 4
    };

    class MmWaveMacHeader : public Header
    {
    public:
        enum QosAckPolicy
        {
            NORMAL_ACK = 0,
            BULK_ACK = 1,
            NO_ACK = 2
        };

        enum AddressType
        {
            ADDR1,
            ADDR2,
            ADDR3,
            ADDR4
        };

        MmWaveMacHeader ();
        virtual ~MmWaveMacHeader ();

        static TypeId GetTypeId ();
        TypeId GetInstanceTypeId () const;
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize () const;
        void Serialize (Buffer::Iterator start) const;
        uint32_t Deserialize (Buffer::Iterator start);

        void SetDsFrom ();
        void SetDsNotFrom ();
        void SetDsTo ();
        void SetDsNotTo ();
        void SetAddr1 (Mac48Address address);
        void SetAddr2 (Mac48Address address);
        void SetAddr3 (Mac48Address address);
        void SetAddr4 (Mac48Address address);
        void SetType (MmWaveMacType type, bool resetToDsFromDs = true);
        void SetRawDuration (uint16_t duration);
        void SetDuration (Time duration);
        void SetId (uint16_t id);
        void SetSequenceNumber (uint16_t seq);
        void SetFragmentNumber (uint8_t frag);
        void SetNoMoreFragments ();
        void SetMoreFragments ();
        void SetRetry ();
        void SetNoRetry ();
        void SetOrder ();
        void SetNoOrder ();

        Mac48Address GetAddr1 () const;
        Mac48Address GetAddr2 () const;
        Mac48Address GetAddr3 () const;
        Mac48Address GetAddr4 () const;
        MmWaveMacType GetType () const;
        bool IsFromDs () const;
        bool IsToDs () const;
        bool IsData () const;
        bool HasData () const;
        bool IsCtl () const;
        bool IsMgt () const;
        bool IsRts () const;
        bool IsCts () const;
        bool IsAck () const;
        bool IsBeacon () const;
        bool IsBulkRequest () const;
        bool IsBulkResponse () const;
        bool IsBulkAck () const;
        bool IsDetectRequest () const;
        uint16_t GetRawDuration () const;
        Time GetDuration () const;
        uint16_t GetSequenceControl () const;
        uint16_t GetSequenceNumber () const;
        uint8_t GetFragmentNumber () const;
        bool IsRetry () const;
        bool IsMoreFragments () const;
        uint32_t GetSize () const;
        const char * GetTypeString () const;
        typedef void (* TracedCallback)(const MmWaveMacHeader &header);

    private:
        uint16_t GetFrameControl () const;
        void SetFrameControl (uint16_t control);
        void SetSequenceControl (uint16_t seq);
        void PrintFrameControl (std::ostream &os) const;

        uint8_t m_ctrlType;     ///< control type
        uint8_t m_ctrlSubtype;  ///< control subtype
        uint8_t m_ctrlToDs;     ///< control to DS
        uint8_t m_ctrlFromDs;   ///< control from DS
        uint8_t m_ctrlMoreFrag; ///< control more fragments
        uint8_t m_ctrlRetry;    ///< control retry
        uint8_t m_ctrlMoreData; ///< control more data
        uint8_t m_ctrlWep;      ///< control WEP
        uint8_t m_ctrlOrder;    ///< control order
        uint16_t m_duration;    ///< duration
        Mac48Address m_addr1;   ///< address 1
        Mac48Address m_addr2;   ///< address 2
        Mac48Address m_addr3;   ///< address 3
        uint8_t m_seqFrag;      ///< sequence fragment
        uint16_t m_seqSeq;      ///< sequence sequence
        Mac48Address m_addr4;   ///< address 4
    };

    class BeaconHeader : public Header
    {
    public:
        BeaconHeader ();
        ~BeaconHeader ();
        static TypeId GetTypeId ();
        TypeId GetInstanceTypeId () const;
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize () const;
        void Serialize (Buffer::Iterator start) const;
        uint32_t Deserialize (Buffer::Iterator start);
        void SetChannelNumber (uint8_t number);
        uint8_t GetChannelNumber () const;
    private:
        uint8_t m_num;
    };

    class BulkRequestHeader : public Header
    {
    public:
        BulkRequestHeader ();
        ~BulkRequestHeader ();
        static TypeId GetTypeId ();
        TypeId GetInstanceTypeId () const;
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize () const;
        void Serialize (Buffer::Iterator start) const;
        uint32_t Deserialize (Buffer::Iterator start);
        void SetTxDuration (Time txDuration);
        Time GetTxDuration () const;
        void SetStartingSequenceControl (uint16_t seqControl);
        uint16_t GetStartingSequenceControl () const;
        void SetNum (uint8_t num);
        uint8_t GetNum () const;
    private:
        uint16_t m_txDuration;
        uint16_t m_startingSeq;
        uint8_t m_num;
    };

    class BulkResponseHeader : public Header
    {
    public:
        BulkResponseHeader ();
        ~BulkResponseHeader ();
        static TypeId GetTypeId ();
        TypeId GetInstanceTypeId () const;
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize () const;
        void Serialize (Buffer::Iterator start) const;
        uint32_t Deserialize (Buffer::Iterator start);
        void SetTxDuration (Time txDuration);
        Time GetTxDuration () const;
    private:
        uint16_t m_txDuration;
    };

    class BulkAckHeader : public Header
    {
    public:
        BulkAckHeader ();
        ~BulkAckHeader ();
        static TypeId GetTypeId ();
        TypeId GetInstanceTypeId () const;
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize () const;
        void Serialize (Buffer::Iterator start) const;
        uint32_t Deserialize (Buffer::Iterator start);
        void SetStartingSequenceControl (uint16_t seqControl);
        uint16_t GetStartingSequenceControl () const;
        void SetBitmap (uint64_t bitmap);
        uint64_t GetBitmap () const;
    private:
        uint16_t m_startingSeq;
        uint64_t m_bitmap;
    };

} //namespace ns3

#endif //SRC_MMWAVE_MAC_HEADER_H
