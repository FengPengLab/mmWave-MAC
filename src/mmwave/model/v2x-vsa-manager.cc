/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/socket.h"
#include "ns3/wifi-phy.h"
#include "ns3/higher-tx-tag.h"
#include "v2x-net-device.h"
#include "v2x-vsa-manager.h"
#include "v2x-ctrl-mac.h"
#include "v2x-channel-scheduler.h"
namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("V2xVsaManager");
    NS_OBJECT_ENSURE_REGISTERED (V2xVsaManager);

    const static uint8_t oi_bytes_1609[5] = {0x00, 0x50, 0xC2, 0x4A, 0x40};

    const static OrganizationIdentifier oi_1609 = OrganizationIdentifier (oi_bytes_1609, 5);

    TypeId
    V2xVsaManager::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::V2xVsaManager")
                .SetParent<Object> ()
                .SetGroupName ("MmWave")
                .AddConstructor<V2xVsaManager> ()
        ;
        return tid;
    }

    V2xVsaManager::V2xVsaManager ()
            : m_device (0)
    {
        m_vsaReceived = MakeNullCallback<bool, Ptr<const Packet>,const Address &, uint32_t, uint32_t> ();
    }

    V2xVsaManager::~V2xVsaManager ()
    {

    }

    void
    V2xVsaManager::DoDispose ()
    {
        NS_LOG_FUNCTION (this);
        RemoveAll ();
        m_device = 0;
    }

    void
    V2xVsaManager::DoInitialize ()
    {
        std::map<uint32_t, Ptr<V2xCtrlMac> > macs = m_device->GetCtrlMacs ();
        for (std::map<uint32_t, Ptr<V2xCtrlMac> >::iterator i = macs.begin (); i != macs.end (); ++i)
        {
            i->second->AddReceiveVscCallback (oi_1609, MakeCallback (&V2xVsaManager::ReceiveVsc, this));
        }
    }

    void
    V2xVsaManager::SetV2xMmWaveNetDevice (Ptr<V2xMmWaveNetDevice> device)
    {
        NS_LOG_FUNCTION (this << device);
        m_device = device;
    }

    void
    V2xVsaManager::SendVsa (const VsaInfo & vsaInfo)
    {
        NS_LOG_FUNCTION (this << &vsaInfo);
        OrganizationIdentifier oi;
        if (vsaInfo.oi.IsNull ())
        {
            // refer to 1609.4-2010 chapter 6.4.1.1
            uint8_t oibytes[5] = {0x00, 0x50, 0xC2, 0x4A, 0x40};
            oibytes[4] |= (vsaInfo.managementId & 0x0f);
            oi = OrganizationIdentifier (oibytes, 5);
        }
        else
        {
            oi = vsaInfo.oi;
        }

        if (vsaInfo.peer.IsGroup () && (vsaInfo.repeatRate != 0))
        {
            VsaWork *vsa = new VsaWork ();
            vsa->sentInterval = vsaInfo.sendInterval;
            vsa->channelNumber = vsaInfo.channelNumber;
            vsa->peer = vsaInfo.peer;
            vsa->repeatPeriod = MilliSeconds (VSA_REPEAT_PERIOD * 1000 / vsaInfo.repeatRate);
            vsa->vsc = vsaInfo.vsc;
            vsa->oi = oi;
            vsa->repeat =  Simulator::Schedule (vsa->repeatPeriod, &V2xVsaManager::DoRepeat, this, vsa);
            m_vsas.push_back (vsa);
        }
        DoSendVsa (vsaInfo.sendInterval, vsaInfo.channelNumber, vsaInfo.vsc->Copy (), oi, vsaInfo.peer);
    }

    void
    V2xVsaManager::DoRepeat (VsaWork *vsa)
    {
        NS_LOG_FUNCTION (this << vsa);
        vsa->repeat =  Simulator::Schedule (vsa->repeatPeriod, &V2xVsaManager::DoRepeat, this, vsa);
        DoSendVsa (vsa->sentInterval, vsa->channelNumber, vsa->vsc->Copy (), vsa->oi, vsa->peer);
    }

    void
    V2xVsaManager::DoSendVsa (enum VsaTransmitInterval  interval, uint32_t channel,
                           Ptr<Packet> vsc, OrganizationIdentifier oi, Mac48Address peer)
    {
        NS_LOG_FUNCTION (this << interval << channel << vsc << oi << peer);
        NS_ASSERT (m_device != 0);
        Ptr<ChannelCoordinator> coordinator = m_device->GetChannelCoordinator ();
        Ptr<V2xChannelScheduler> scheduler = m_device->GetChannelScheduler ();
        Ptr<ChannelManager> manager = m_device->GetChannelManager ();

        if (interval == VSA_TRANSMIT_IN_SCHI)
        {
            Time wait = coordinator->NeedTimeToSchInterval ();
            if (wait != Seconds (0))
            {
                Simulator::Schedule (wait, &V2xVsaManager::DoSendVsa, this,
                                     interval, channel, vsc, oi, peer);
                return;
            }
        }
        else if (interval == VSA_TRANSMIT_IN_CCHI)
        {
            Time wait = coordinator->NeedTimeToCchInterval ();
            if (wait != Seconds (0))
            {
                Simulator::Schedule (wait, &V2xVsaManager::DoSendVsa, this,
                                     interval, channel, vsc, oi, peer);
                return;
            }
        }
        else
        {
            NS_ASSERT (interval == VSA_TRANSMIT_IN_BOTHI);
            // do nothing here, since VSA_IN_BOTHI allows to sent VSA frames in any interval.
        }

        if (!scheduler->IsChannelAccessAssigned (channel))
        {
            NS_LOG_DEBUG ("there is no channel access assigned for channel " << channel);
            return;
        }

        // refer to 1609.4-2010 chapter 5.4.1
        // Management frames are assigned the highest AC (AC_VO).
        SocketPriorityTag priorityTag;
        priorityTag.SetPriority (7);
        vsc->AddPacketTag (priorityTag);

        WifiTxVector txVector;
        txVector.SetChannelWidth (10);
        txVector.SetTxPowerLevel (manager->GetManagementPowerLevel (channel));
        txVector.SetMode (manager->GetManagementDataRate (channel));
        txVector.SetPreambleType (manager->GetManagementPreamble (channel));
        HigherLayerTxVectorTag tag = HigherLayerTxVectorTag (txVector, manager->GetManagementAdaptable (channel));
        vsc->AddPacketTag (tag);

        Ptr<V2xCtrlMac> mac = m_device->GetCtrlMac (channel);
        mac->SendVsc (vsc, peer, oi);
    }

    void
    V2xVsaManager::RemoveAll ()
    {
        NS_LOG_FUNCTION (this);
        for (std::vector<VsaWork *>::iterator i = m_vsas.begin ();
             i != m_vsas.end (); ++i)
        {
            if (!(*i)->repeat.IsExpired ())
            {
                (*i)->repeat.Cancel ();
            }
            (*i)->vsc = 0;
            delete (*i);
        }
        m_vsas.clear ();
    }

    void
    V2xVsaManager::RemoveByChannel (uint32_t channelNumber)
    {
        NS_LOG_FUNCTION (this << channelNumber);
        for (std::vector<VsaWork *>::iterator i = m_vsas.begin ();
             i != m_vsas.end (); )
        {
            if ((*i)->channelNumber == channelNumber)
            {
                if (!(*i)->repeat.IsExpired ())
                {
                    (*i)->repeat.Cancel ();
                }
                (*i)->vsc = 0;
                delete (*i);
                i = m_vsas.erase (i);
            }
            else
            {
                ++i;
            }
        }
    }


    void
    V2xVsaManager::RemoveByOrganizationIdentifier (const OrganizationIdentifier &oi)
    {
        NS_LOG_FUNCTION (this << oi);
        for (std::vector<VsaWork *>::iterator i = m_vsas.begin ();
             i != m_vsas.end (); )
        {
            if ((*i)->oi == oi)
            {
                if (!(*i)->repeat.IsExpired ())
                {
                    (*i)->repeat.Cancel ();
                }
                (*i)->vsc = 0;
                delete (*i);
                i = m_vsas.erase (i);
            }
            else
            {
                ++i;
            }
        }
    }

    void
    V2xVsaManager::SetWaveVsaCallback (Callback<bool, Ptr<const Packet>,const Address &, uint32_t, uint32_t> vsaCallback)
    {
        NS_LOG_FUNCTION (this);
        m_vsaReceived = vsaCallback;
    }

    bool
    V2xVsaManager::ReceiveVsc (Ptr<WifiMac> mac, const OrganizationIdentifier &oi, Ptr<const Packet> vsc, const Address &src)
    {
        NS_LOG_FUNCTION (this << mac << oi << vsc << src);
        NS_ASSERT (oi == oi_1609);
        if (m_vsaReceived.IsNull ())
        {
            return true;
        }
        uint32_t channelNumber = mac->GetWifiPhy ()->GetChannelNumber ();
        uint32_t managementId = oi.GetManagementId ();
        return m_vsaReceived (vsc, src, managementId, channelNumber);
    }
} // namespace ns3