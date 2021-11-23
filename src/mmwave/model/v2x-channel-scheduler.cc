/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/log.h"
#include "ns3/channel-scheduler.h"
#include "v2x-channel-scheduler.h"
#include "v2x-ctrl-mac.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("V2xChannelScheduler");
    NS_OBJECT_ENSURE_REGISTERED (V2xChannelScheduler);

    TypeId
    V2xChannelScheduler::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::V2xChannelScheduler")
                .SetParent<Object> ()
                .SetGroupName ("MmWave")
        ;
        return tid;
    }

    V2xChannelScheduler::V2xChannelScheduler ()
    {
        NS_LOG_FUNCTION (this);
    }
    V2xChannelScheduler::~V2xChannelScheduler ()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    V2xChannelScheduler::DoInitialize ()
    {
        // assign default CCH access when the device is initialized
        AssignDefaultCchAccess ();
    }

    void
    V2xChannelScheduler::SetV2xMmWaveNetDevice (Ptr<V2xMmWaveNetDevice> device)
    {
        NS_LOG_FUNCTION (this << device);
        m_device = device;
    }

    bool
    V2xChannelScheduler::IsChannelAccessAssigned (uint32_t channelNumber) const
    {
        NS_LOG_FUNCTION (this << channelNumber);
        return (GetAssignedAccessType (channelNumber) != NoAccess);
    }

    bool
    V2xChannelScheduler::IsCchAccessAssigned () const
    {
        NS_LOG_FUNCTION (this);
        return (GetAssignedAccessType (CCH) != NoAccess);
    }

    bool
    V2xChannelScheduler::IsSchAccessAssigned () const
    {
        NS_LOG_FUNCTION (this);
        return (GetAssignedAccessType (SCH1) != NoAccess) || (GetAssignedAccessType (SCH2) != NoAccess)
               || (GetAssignedAccessType (SCH3) != NoAccess) || (GetAssignedAccessType (SCH4) != NoAccess)
               || (GetAssignedAccessType (SCH5) != NoAccess) || (GetAssignedAccessType (SCH6) != NoAccess);
    }

    bool
    V2xChannelScheduler::IsContinuousAccessAssigned (uint32_t channelNumber) const
    {
        NS_LOG_FUNCTION (this << channelNumber);
        return (GetAssignedAccessType (channelNumber) == ContinuousAccess);
    }

    bool
    V2xChannelScheduler::IsAlternatingAccessAssigned (uint32_t channelNumber) const
    {
        NS_LOG_FUNCTION (this << channelNumber);
        return (GetAssignedAccessType (channelNumber) == AlternatingAccess);
    }

    bool
    V2xChannelScheduler::IsExtendedAccessAssigned (uint32_t channelNumber) const
    {
        NS_LOG_FUNCTION (this << channelNumber);
        return (GetAssignedAccessType (channelNumber) == ExtendedAccess);
    }

    bool
    V2xChannelScheduler::IsDefaultCchAccessAssigned () const
    {
        NS_LOG_FUNCTION (this);
        return (GetAssignedAccessType (CCH) == DefaultCchAccess);
    }

    bool
    V2xChannelScheduler::StartSch (const SchInfo & schInfo)
    {
        NS_LOG_FUNCTION (this << &schInfo);
        uint32_t cn = schInfo.channelNumber;

        if (ChannelManager::IsCch (schInfo.channelNumber))
        {
            NS_LOG_DEBUG ("the channel access requirement for CCH is not allowed.");
            return false;
        }
        uint32_t extends = schInfo.extendedAccess;
        bool immediate = schInfo.immediateAccess;
        Ptr<V2xCtrlMac> mac = m_device->GetCtrlMac (cn);
        for (EdcaParametersI i = schInfo.edcaParameters.begin (); i != schInfo.edcaParameters.end (); ++i)
        {
            EdcaParameter edca = i->second;
            mac->ConfigureEdca (edca.cwmin, edca.cwmax, edca.aifsn, i->first);
        }

        if (extends == EXTENDED_CONTINUOUS)
        {
            return AssignContinuousAccess (cn, immediate);
        }
        else if (extends == EXTENDED_ALTERNATING)
        {
            return AssignAlternatingAccess (cn, immediate);
        }
        else
        {
            return AssignExtendedAccess (cn, extends, immediate);
        }
    }

    bool
    V2xChannelScheduler::StopSch (uint32_t channelNumber)
    {
        NS_LOG_FUNCTION (this << channelNumber);
        if (ChannelManager::IsCch (channelNumber))
        {
            NS_LOG_DEBUG ("the channel access for CCH is not allowed to be released.");
            return false;
        }
        if (!IsChannelAccessAssigned (channelNumber))
        {
            NS_LOG_DEBUG ("the channel access for channel " << channelNumber << " has already been released.");
            return true;
        }
        return ReleaseAccess (channelNumber);
    }

} // namespace ns3
