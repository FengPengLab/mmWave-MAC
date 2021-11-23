/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Junling Bu <linlinjavaer@gmail.com>
 */
#ifndef DEFAULT_CHANNEL_SCHEDULER_H
#define DEFAULT_CHANNEL_SCHEDULER_H
#include "v2x-channel-scheduler.h"
#include "v2x-net-device.h"
namespace ns3 {
    class V2xMmWaveNetDevice;
    
    class V2xDefaultChannelScheduler : public V2xChannelScheduler
    {
    public:
        static TypeId GetTypeId ();
        V2xDefaultChannelScheduler ();
        virtual ~V2xDefaultChannelScheduler ();
        
        virtual void SetV2xMmWaveNetDevice (Ptr<V2xMmWaveNetDevice> device);
        virtual enum ChannelAccess GetAssignedAccessType (uint32_t channelNumber) const;
        void NotifyCchSlotStart (Time duration);
        void NotifySchSlotStart (Time duration);
        void NotifyGuardSlotStart (Time duration, bool cchi);
    private:
        virtual void DoInitialize ();
        virtual void DoDispose (void);
        virtual bool AssignAlternatingAccess (uint32_t channelNumber, bool immediate);
        virtual bool AssignContinuousAccess (uint32_t channelNumber, bool immediate);
        virtual bool AssignExtendedAccess (uint32_t channelNumber, uint32_t extends, bool immediate);
        virtual bool AssignDefaultCchAccess ();
        virtual bool ReleaseAccess (uint32_t channelNumber);
        void SwitchToNextChannel (uint32_t curChannelNumber, uint32_t nextChannelNumber);

        Ptr<ChannelManager> m_manager; ///< channel manager
        Ptr<ChannelCoordinator> m_coordinator; ///< channel coordinator
        Ptr<WifiPhy> m_phy; ///< Phy

        uint32_t m_channelNumber; ///< channel number
        uint32_t m_extend; ///< extend
        EventId m_extendEvent; ///< extend event
        enum ChannelAccess m_channelAccess; ///< channel access

        EventId m_waitEvent; ///< wait event
        uint32_t m_waitChannelNumber; ///< wait channel number
        uint32_t m_waitExtend; ///< wait extend

        Ptr<ChannelCoordinationListener> m_coordinationListener; ///< coordination listener
    };

}
#endif /* DEFAULT_CHANNEL_SCHEDULER_H */
