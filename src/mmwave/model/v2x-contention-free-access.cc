/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include <vector>
#include <algorithm>
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/address-utils.h"
#include "v2x-net-device.h"
#include "v2x-contention-free-access.h"
namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("V2xContentionFreeAccess");
    NS_OBJECT_ENSURE_REGISTERED (V2xContentionFreeAccess);

    TxBeaconHeader::TxBeaconHeader ()
    {
    }

    TxBeaconHeader::~TxBeaconHeader ()
    {
    }

    TypeId
    TxBeaconHeader::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::TxBeaconHeader")
                .SetParent<Header> ()
                .SetGroupName ("MmWave")
                .AddConstructor<TxBeaconHeader> ()
        ;
        return tid;
    }

    TypeId
    TxBeaconHeader::GetInstanceTypeId () const
    {
        return GetTypeId ();
    }

    void
    TxBeaconHeader::Print (std::ostream &os) const
    {
        NS_LOG_FUNCTION (this << &os);
    }

    uint32_t
    TxBeaconHeader::GetSerializedSize () const
    {
        uint32_t size = 0;
        size += sizeof (m_duration);
        size += sizeof (m_num);
        size += m_num * 6;

        return size;
    }

    void
    TxBeaconHeader::Serialize (Buffer::Iterator start) const
    {
        Buffer::Iterator i = start;
        i.WriteHtolsbU64 (m_duration);
        i.WriteHtolsbU16 (m_num);
        for (auto it = m_rx.begin(); it != m_rx.end(); it++)
        {
            WriteTo (i, (*it));
        }
    }

    uint32_t
    TxBeaconHeader::Deserialize (Buffer::Iterator start)
    {
        Buffer::Iterator i = start;
        m_duration = i.ReadLsbtohU64 ();
        m_num = i.ReadLsbtohU16 ();
        m_rx.clear ();
        for (uint16_t n = 1; n <= m_num; n++)
        {
            Mac48Address addr;
            ReadFrom (i, addr);
            m_rx.push_back(addr);
        }

        return i.GetDistanceFrom (start);
    }

    void
    TxBeaconHeader::SetRx (std::vector<Mac48Address> rx)
    {
        m_num = 0;
        m_rx.clear ();
        if (rx.size() >= 1)
        {
            m_num = rx.size ();
            m_rx = rx;
        }
        else
        {
            NS_FATAL_ERROR ("rx.size is error");
        }
    }

    void
    TxBeaconHeader::SetDuration (uint64_t t)
    {
        m_duration = t;
    }

    std::vector<Mac48Address>
    TxBeaconHeader::GetRx ()
    {
        return m_rx;
    }

    Time
    TxBeaconHeader::GetDuration ()
    {
        return NanoSeconds (m_duration);
    }

    uint16_t
    TxBeaconHeader::GetNumber ()
    {
        return m_num;
    }

    RxBeaconHeader::RxBeaconHeader ()
    {
    }

    RxBeaconHeader::~RxBeaconHeader ()
    {
    }

    TypeId
    RxBeaconHeader::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::RxBeaconHeader")
                .SetParent<Header> ()
                .SetGroupName ("V2XmmWave")
                .AddConstructor<RxBeaconHeader> ()
        ;
        return tid;
    }

    TypeId
    RxBeaconHeader::GetInstanceTypeId () const
    {
        return GetTypeId ();
    }

    void
    RxBeaconHeader::Print (std::ostream &os) const
    {
        NS_LOG_FUNCTION (this << &os);
    }

    uint32_t
    RxBeaconHeader::GetSerializedSize () const
    {
        uint32_t size = 0;
        size += sizeof(m_duration);
        size += sizeof(m_delay);
        return size;
    }

    void
    RxBeaconHeader::Serialize (Buffer::Iterator start) const
    {
        Buffer::Iterator i = start;
        i.WriteHtolsbU64 (m_duration);
        i.WriteHtolsbU64 (m_delay);
    }

    uint32_t
    RxBeaconHeader::Deserialize (Buffer::Iterator start)
    {
        Buffer::Iterator i = start;
        m_duration = i.ReadLsbtohU64 ();
        m_delay = i.ReadLsbtohU64 ();
        return i.GetDistanceFrom (start);
    }

    void
    RxBeaconHeader::SetDuration (uint64_t t)
    {
        m_duration = t;
    }

    void
    RxBeaconHeader::SetDelay (uint64_t t)
    {
        m_delay = t;
    }

    Time
    RxBeaconHeader::GetDuration ()
    {
        return NanoSeconds (m_duration);
    }

    Time
    RxBeaconHeader::GetDelay ()
    {
        return NanoSeconds (m_delay);
    }
    
    TypeId
    V2xContentionFreeAccess::GetTypeId ()
    {
        static TypeId tid = TypeId("ns3::V2xContentionFreeAccess")
                .SetParent<Object>()
                .SetGroupName("MmWave")
                .AddConstructor<V2xContentionFreeAccess> ();
        return tid;
    }

    V2xContentionFreeAccess::V2xContentionFreeAccess ()
    {
        NS_LOG_FUNCTION (this);
    }

    V2xContentionFreeAccess::~V2xContentionFreeAccess ()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    V2xContentionFreeAccess::DoInitialize ()
    {
        m_requests.clear ();
        m_agreements.clear ();
    }

    void
    V2xContentionFreeAccess::DoDispose ()
    {
        m_requests.clear ();
        m_agreements.clear ();
    }

    bool
    V2xContentionFreeAccess::IsAnyRequest ()
    {
        if (m_requests.size () >= 1)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool
    V2xContentionFreeAccess::IsAnyAgreement ()
    {
        if (m_agreements.size () >= 1)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    void
    V2xContentionFreeAccess::AddRequest (Mac48Address tx, Mac48Address rx, Time duration)
    {
        NS_LOG_FUNCTION (this << tx << rx << duration);
        if (rx != m_device->GetMacAddress())
        {
            NS_FATAL_ERROR("rx is not myself");
        }

        for (auto i = m_requests.begin (); i != m_requests.end (); i++)
        {
            AgreementInfo info = (*i);
            if (info.m_tx == tx && info.m_rx == rx)
            {
                return;
            }
        }
        AgreementInfo info;
        info.m_tx = tx;
        info.m_rx = rx;
        info.m_duration = duration;
        info.m_start = NanoSeconds (0);
        m_requests.push_back (info);
    }

    void
    V2xContentionFreeAccess::RemoveRequest (Mac48Address tx, Mac48Address rx, Time duration)
    {
        NS_LOG_FUNCTION (this << tx << rx << duration);
        for (auto i = m_requests.begin (); i != m_requests.end ();)
        {
            AgreementInfo info = (*i);
            if (info.m_tx == tx
                && info.m_rx == rx
                && info.m_duration == duration)
            {
                i = m_requests.erase (i);
                break;
            }
            else
            {
                i++;
            }
        }
    }

    AgreementInfo
    V2xContentionFreeAccess::DequeueRequest ()
    {
        AgreementInfo info;
        NS_ASSERT (m_requests.size() >= 1);
        auto i = m_requests.begin ();
        info = (*i);
        i = m_requests.erase (i);
        NS_LOG_FUNCTION (this << info.m_tx << info.m_rx << info.m_duration << info.m_start);
        return info;
    }

    std::vector<AgreementInfo>
    V2xContentionFreeAccess::GetRequests ()
    {
        return m_requests;
    }

    std::vector<AgreementInfo>
    V2xContentionFreeAccess::GetAgreements ()
    {
        return m_agreements;
    }

    bool
    V2xContentionFreeAccess::FindAgreement (Mac48Address tx, Mac48Address rx)
    {
        bool find = false;
        CheckAgreementTimeout ();
        for (auto i = m_agreements.begin (); i != m_agreements.end (); i++)
        {
            if (((*i).m_tx == tx) && ((*i).m_rx == rx))
            {
                find = true;
            }
        }
        return find;
    }

    Time
    V2xContentionFreeAccess::NewAgreement (Mac48Address tx, Mac48Address rx, Time start, Time duration)
    {
        NS_LOG_FUNCTION (this << tx << rx << start << duration);
        Time idleStart = start;
        Time idleEnd = start;

        if (idleStart < Simulator::Now())
        {
            NS_FATAL_ERROR (" start time is wrong.");
        }

        CheckAgreementTimeout ();
        std::sort (m_agreements.begin (), m_agreements.end (), V2xContentionFreeAccess::SortByTime);

        if (m_agreements.size() > 0)
        {
            for (auto i = m_agreements.begin(); i != m_agreements.end(); i++)
            {
                if (Simulator::Now() <= idleStart)
                {
                    if (idleStart + duration < (*i).m_start)
                    {
                        NS_ASSERT (!CheckAgreementConfilct (idleStart, duration));
                        AddAgreement (tx, rx, idleStart, duration);
                        return idleStart;
                    }

                    if (idleStart < (*i).m_start + (*i).m_duration)
                    {
                        idleStart = (*i).m_start + (*i).m_duration;
                    }
                }
                else
                {
                    NS_FATAL_ERROR ("idleStart is error.");
                }
            }
        }

        NS_ASSERT (!CheckAgreementConfilct (idleStart, duration));
        AddAgreement (tx, rx, idleStart, duration);
        return idleStart;
    }

    void
    V2xContentionFreeAccess::AddAgreement (Mac48Address tx, Mac48Address rx, Time start, Time duration)
    {
        NS_LOG_FUNCTION (this << tx << rx << start << duration);
        AgreementInfo b;
        b.m_tx = tx;
        b.m_rx = rx;
        b.m_start = start;
        b.m_duration = duration;
        CheckAgreementTimeout ();
        if (!CheckAgreementConfilct (start, duration))
        {
            m_agreements.push_back (b);
            std::sort (m_agreements.begin (), m_agreements.end (), V2xContentionFreeAccess::SortByTime);
        }
    }

    bool
    V2xContentionFreeAccess::SortByTime (const AgreementInfo &v1, const AgreementInfo &v2)
    {
        return v1.m_start < v2.m_start;
    }

    void
    V2xContentionFreeAccess::CheckAgreementTimeout ()
    {
        for (auto i = m_agreements.begin (); i != m_agreements.end ();)
        {
            AgreementInfo info = (*i);
            if (info.m_start + info.m_duration < Simulator::Now ())
            {
                i = m_agreements.erase (i);
            }
            else
            {
                i++;
            }
        }
    }

    bool
    V2xContentionFreeAccess::CheckAgreementConfilct (Time start, Time duration)
    {
        bool confilct = false;
        for (auto i = m_agreements.begin (); i != m_agreements.end (); i++)
        {
            AgreementInfo info = (*i);
            if ((info.m_start < start + duration) && (start + duration < info.m_start + info.m_duration))
            {
                confilct = true;
            }
            else if ((info.m_start < start) && (start < info.m_start + info.m_duration))
            {
                confilct = true;
            }
        }
        return confilct;
    }

    void
    V2xContentionFreeAccess::SetDevice (Ptr<V2xMmWaveNetDevice> device)
    {
        m_device = device;
    }

    void
    V2xContentionFreeAccess::StartContentionFreeDurationIfNeed ()
    {
        CheckAgreementTimeout ();
        if (m_device->IsAnyActiveContentionFreeDuration ())
        {
            return;
        }

        for (auto i = m_agreements.begin (); i != m_agreements.end (); i++)
        {
            AgreementInfo info = (*i);
            if (info.m_tx == m_device->GetMacAddress ())
            {
                if (info.m_start <= Simulator::Now ())
                {
                    m_device->StartContentionFreeDuration (info.m_duration, info.m_rx);
                }
                else
                {
                    Time delay = info.m_start - Simulator::Now ();
                    m_device->WaitContentionFreeDuration (delay, info.m_duration, info.m_rx);
                }
                break;
            }
        }
    }
}