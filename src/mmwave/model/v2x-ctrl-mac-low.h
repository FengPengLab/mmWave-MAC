/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
 *
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *         Junling Bu <linlinjavaer@gmail.com>
 */
#ifndef V2X_CTRL_MAC_LOW_H
#define V2X_CTRL_MAC_LOW_H
#include <map>
#include "ns3/mac-low.h"
#include "ns3/msdu-aggregator.h"
#include "ns3/mpdu-aggregator.h"
#include "ns3/wifi-psdu.h"
#include "ns3/channel-coordinator.h"
#include "v2x-channel-scheduler.h"
#include "v2x-net-device.h"
namespace ns3 {
    class V2xMmWaveNetDevice;
    class WifiMacQueueItem;

    class V2xCtrlMacLow : public MacLow
    {
    public:
        static TypeId GetTypeId (void);
        V2xCtrlMacLow ();
        virtual ~V2xCtrlMacLow ();

        void SetWaveNetDevice (Ptr<V2xMmWaveNetDevice> device);
        void StartTransmission (Ptr<WifiMacQueueItem> mpdu, MacLowTransmissionParameters parameters, Ptr<Txop> txop);
    private:
        WifiTxVector GetDataTxVector (Ptr<const WifiMacQueueItem> item) const;

        Ptr<V2xChannelScheduler> m_scheduler; ///< the channel scheduler
        Ptr<ChannelCoordinator> m_coordinator; ///< the channel coordinator
    };

} // namespace ns3

#endif /* V2X_CTRL_MAC_LOW_H*/
