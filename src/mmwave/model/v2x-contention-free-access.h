/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef CONTENTION_FREE_ACCESS_H
#define CONTENTION_FREE_ACCESS_H
#include <vector>
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/header.h"
#include "ns3/mac48-address.h"

namespace ns3 {
    class V2xMmWaveNetDevice;

    class AgreementInfo
    {
    public:
        Mac48Address m_tx;
        Mac48Address m_rx;
        Time m_start;
        Time m_duration;
    };

    class TxBeaconHeader : public Header
    {
    public:
        TxBeaconHeader ();
        virtual ~TxBeaconHeader ();
        static TypeId GetTypeId (void);
        virtual TypeId GetInstanceTypeId (void) const;
        virtual void Print (std::ostream &os) const;
        virtual uint32_t GetSerializedSize (void) const;
        virtual void Serialize (Buffer::Iterator start) const;
        virtual uint32_t Deserialize (Buffer::Iterator start);

        void SetRx (std::vector<Mac48Address> rx);
        void SetDuration (uint64_t t);

        Time GetDuration ();
        uint16_t GetNumber ();
        std::vector<Mac48Address> GetRx ();

    private:
        uint64_t m_duration;
        uint16_t m_num;
        std::vector<Mac48Address> m_rx;
    };

    class RxBeaconHeader : public Header
    {
    public:
        RxBeaconHeader ();
        virtual ~RxBeaconHeader ();
        static TypeId GetTypeId (void);
        virtual TypeId GetInstanceTypeId (void) const;
        virtual void Print (std::ostream &os) const;
        virtual uint32_t GetSerializedSize (void) const;
        virtual void Serialize (Buffer::Iterator start) const;
        virtual uint32_t Deserialize (Buffer::Iterator start);

        void SetDelay (uint64_t t);
        void SetDuration (uint64_t t);

        Time GetDelay ();
        Time GetDuration ();
    private:
        uint64_t m_delay;
        uint64_t m_duration;
    };

    class V2xContentionFreeAccess : public Object
    {
    public:
        static TypeId GetTypeId ();
        V2xContentionFreeAccess ();
        ~V2xContentionFreeAccess ();

        static bool SortByTime (const AgreementInfo &v1, const AgreementInfo &v2);

        bool IsAnyRequest ();
        bool IsAnyAgreement ();
        bool FindAgreement (Mac48Address tx, Mac48Address rx);
        void AddRequest (Mac48Address tx, Mac48Address rx, Time duration);
        void RemoveRequest (Mac48Address tx, Mac48Address rx, Time duration);
        AgreementInfo DequeueRequest ();
        std::vector<AgreementInfo> GetRequests ();
        std::vector<AgreementInfo> GetAgreements ();

        Time NewAgreement (Mac48Address tx, Mac48Address rx, Time start, Time duration);
        void AddAgreement (Mac48Address tx, Mac48Address rx, Time start, Time duration);
        void CheckAgreementTimeout ();
        bool CheckAgreementConfilct (Time start, Time duration);
        void SetDevice (Ptr<V2xMmWaveNetDevice> device);
        void StartContentionFreeDurationIfNeed ();

    private:
        void DoDispose ();
        void DoInitialize ();

        Ptr<V2xMmWaveNetDevice> m_device;
        std::vector<AgreementInfo> m_agreements;
        std::vector<AgreementInfo> m_requests;

        Time m_sifs;
    };
}
#endif //CONTENTION_FREE_ACCESS_H
