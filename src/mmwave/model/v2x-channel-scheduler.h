/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef V2X_CHANNEL_SCHEDULER_H
#define V2X_CHANNEL_SCHEDULER_H
#include <map>
#include "v2x-net-device.h"
namespace ns3 {
    class V2xMmWaveNetDevice;

    class V2xChannelScheduler : public Object
    {
    public:
        static TypeId GetTypeId (void);
        V2xChannelScheduler ();
        virtual ~V2xChannelScheduler ();

        virtual void SetV2xMmWaveNetDevice (Ptr<V2xMmWaveNetDevice> device);

        bool IsCchAccessAssigned (void) const;
        bool IsSchAccessAssigned (void) const;
        bool IsChannelAccessAssigned (uint32_t channelNumber) const;
        bool IsContinuousAccessAssigned (uint32_t channelNumber) const;
        bool IsAlternatingAccessAssigned (uint32_t channelNumber) const;
        bool IsExtendedAccessAssigned (uint32_t channelNumber) const;
        bool IsDefaultCchAccessAssigned (void) const;
        virtual enum ChannelAccess GetAssignedAccessType (uint32_t channelNumber) const = 0;
        bool StartSch (const SchInfo & schInfo);
        bool StopSch (uint32_t channelNumber);

    protected:
        virtual void DoInitialize (void);
        virtual bool AssignAlternatingAccess (uint32_t channelNumber, bool immediate) = 0;
        virtual bool AssignContinuousAccess (uint32_t channelNumber, bool immediate) = 0;
        virtual bool AssignExtendedAccess (uint32_t channelNumber, uint32_t extends, bool immediate) = 0;
        virtual bool AssignDefaultCchAccess (void) = 0;
        virtual bool ReleaseAccess (uint32_t channelNumber) = 0;

        Ptr<V2xMmWaveNetDevice> m_device; ///< the device
    };

}
#endif /* V2X_CHANNEL_SCHEDULER_H */
