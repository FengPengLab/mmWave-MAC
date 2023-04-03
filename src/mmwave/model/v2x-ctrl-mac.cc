/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/pointer.h"
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/mac-low.h"
#include "ns3/mac-rx-middle.h"
#include "ns3/mgt-headers.h"
#include "ns3/regular-wifi-mac.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/wifi-utils.h"
#include "ns3/qos-utils.h"
#include "ns3/txop.h"
#include "ns3/simulator.h"
#include "ns3/vendor-specific-action.h"
#include "ns3/higher-tx-tag.h"
#include "v2x-ctrl-mac-low.h"
#include "v2x-ctrl-mac.h"
#include "v2x-net-device.h"
#include "v2x-contention-free-access.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("V2xCtrlMac");
    NS_OBJECT_ENSURE_REGISTERED (V2xCtrlMac);

    TypeId
    V2xCtrlMac::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::V2xCtrlMac")
                .SetParent<RegularWifiMac> ()
                .SetGroupName ("MmWave")
                .AddConstructor<V2xCtrlMac> ()
        ;
        return tid;
    }

    V2xCtrlMac::V2xCtrlMac ()
    {
        NS_LOG_FUNCTION (this);
        SetTypeOfStation (OCB);
        RegularWifiMac::SetBssid (Mac48Address::GetBroadcast ());
        m_txop->SetRepalceBeaconCallback(MakeCallback (&V2xCtrlMac::ReplaceBeacon, this));
    }

    V2xCtrlMac::~V2xCtrlMac ()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    V2xCtrlMac::SendVsc (Ptr<Packet> vsc, Mac48Address peer, OrganizationIdentifier oi)
    {
        NS_LOG_FUNCTION (this << vsc << peer << oi);
        WifiMacHeader hdr;
        hdr.SetType (WIFI_MAC_MGT_ACTION);
        hdr.SetAddr1 (peer);
        hdr.SetAddr2 (GetAddress ());
        hdr.SetAddr3 (Mac48Address::GetBroadcast ());
        hdr.SetDsNotFrom ();
        hdr.SetDsNotTo ();
        VendorSpecificActionHeader vsa;
        vsa.SetOrganizationIdentifier (oi);
        vsc->AddHeader (vsa);

        if (GetQosSupported ())
        {
            uint8_t tid = QosUtilsGetTidForPacket (vsc);
            tid = tid > 7 ? 0 : tid;
            m_edca[QosUtilsMapTidToAc (tid)]->Queue (vsc, hdr);
        }
        else
        {
            m_txop->Queue (vsc, hdr);
        }
    }

    void
    V2xCtrlMac::AddReceiveVscCallback (OrganizationIdentifier oi, VscCallback cb)
    {
        m_vscManager.RegisterVscCallback (oi, cb);
    }

    void
    V2xCtrlMac::RemoveReceiveVscCallback (OrganizationIdentifier oi)
    {
        m_vscManager.DeregisterVscCallback (oi);
    }

    void
    V2xCtrlMac::SetSsid (Ssid ssid)
    {
        NS_LOG_WARN ("in OCB mode we should not call SetSsid");
    }

    Ssid
    V2xCtrlMac::GetSsid () const
    {
        NS_LOG_WARN ("in OCB mode we should not call GetSsid");
        // we really do not want to return ssid, however we have to provide
        return RegularWifiMac::GetSsid ();
    }

    void
    V2xCtrlMac::SetBssid (Mac48Address bssid)
    {
        NS_LOG_WARN ("in OCB mode we should not call SetBsid");
    }

    Mac48Address
    V2xCtrlMac::GetBssid () const
    {
        NS_LOG_WARN ("in OCB mode we should not call GetBssid");
        return Mac48Address::GetBroadcast ();
    }

    void
    V2xCtrlMac::SetLinkUpCallback (Callback<void> linkUp)
    {
        NS_LOG_FUNCTION (this << &linkUp);
        RegularWifiMac::SetLinkUpCallback (linkUp);
        linkUp ();
    }

    void
    V2xCtrlMac::SetLinkDownCallback (Callback<void> linkDown)
    {
        NS_LOG_FUNCTION (this << &linkDown);
        RegularWifiMac::SetLinkDownCallback (linkDown);
        NS_LOG_WARN ("in OCB mode the like will never down, so linkDown will never be called");
    }

    void
    V2xCtrlMac::Enqueue (Ptr<Packet> packet, Mac48Address to)
    {
        NS_LOG_FUNCTION (this << packet << to);
        if (m_stationManager->IsBrandNew (to))
        {
            //In ad hoc mode, we assume that every destination supports all
            //the rates we support.
            if (GetHtSupported () || GetVhtSupported ())
            {
                m_stationManager->AddAllSupportedMcs (to);
                m_stationManager->AddStationHtCapabilities (to, GetHtCapabilities());
            }
            if (GetVhtSupported ())
            {
                m_stationManager->AddStationVhtCapabilities (to, GetVhtCapabilities());
            }
            m_stationManager->AddAllSupportedModes (to);
            m_stationManager->RecordDisassociated (to);
        }

        WifiMacHeader hdr;

        // If we are not a QoS STA then we definitely want to use AC_BE to
        // transmit the packet. A TID of zero will map to AC_BE (through \c
        // QosUtilsMapTidToAc()), so we use that as our default here.
        uint8_t tid = 0;

        if (GetQosSupported ())
        {
            hdr.SetType (WIFI_MAC_QOSDATA);
            hdr.SetQosAckPolicy (WifiMacHeader::NORMAL_ACK);
            hdr.SetQosNoEosp ();
            hdr.SetQosNoAmsdu ();
            hdr.SetQosTxopLimit (0);
            tid = QosUtilsGetTidForPacket (packet);
            if (tid > 7)
            {
                tid = 0;
            }
            hdr.SetQosTid (tid);
        }
        else
        {
            hdr.SetType (WIFI_MAC_DATA);
        }

        if (GetHtSupported () || GetVhtSupported ())
        {
            hdr.SetNoOrder ();
        }
        hdr.SetAddr1 (to);
        hdr.SetAddr2 (GetAddress ());
        hdr.SetAddr3 (Mac48Address::GetBroadcast ());
        hdr.SetDsNotFrom ();
        hdr.SetDsNotTo ();

        if (GetQosSupported ())
        {
            // Sanity check that the TID is valid
            NS_ASSERT (tid < 8);
            m_edca[QosUtilsMapTidToAc (tid)]->Queue (packet, hdr);
        }
        else
        {
            m_txop->Queue (packet, hdr);
        }
    }

    void
    V2xCtrlMac::Receive (Ptr<WifiMacQueueItem> mpdu)
    {
        NS_LOG_FUNCTION (this << *mpdu);
        const WifiMacHeader* hdr = &mpdu->GetHeader ();
        Ptr<Packet> packet = mpdu->GetPacket ()->Copy ();
        NS_ASSERT (!hdr->IsCtl ());
        NS_ASSERT (hdr->GetAddr3 () == Mac48Address::GetBroadcast ());

        Mac48Address from = hdr->GetAddr2 ();
        Mac48Address to = hdr->GetAddr1 ();

        if (m_stationManager->IsBrandNew (from))
        {
            //In ad hoc mode, we assume that every destination supports all
            //the rates we support.
            if (GetHtSupported () || GetVhtSupported ())
            {
                m_stationManager->AddAllSupportedMcs (from);
                m_stationManager->AddStationHtCapabilities (from, GetHtCapabilities());
            }
            if (GetVhtSupported ())
            {
                m_stationManager->AddStationVhtCapabilities (from, GetVhtCapabilities());
            }
            m_stationManager->AddAllSupportedModes (from);
            m_stationManager->RecordDisassociated (from);
        }

        if (hdr->IsData ())
        {
            if (hdr->IsQosData () && hdr->IsQosAmsdu ())
            {
                NS_LOG_DEBUG ("Received A-MSDU from" << from);
                DeaggregateAmsduAndForward (mpdu);
            }
            else
            {
                ForwardUp (packet, from, to);
            }
            return;
        }

        if (to != GetAddress () && !to.IsGroup ())
        {
            NS_LOG_LOGIC ("the management frame is not for us");
            return;
        }

        if (hdr->IsMgt ())
        {
            if (hdr->IsAction ())
            {
                VendorSpecificActionHeader vsaHdr;
                packet->PeekHeader (vsaHdr);
                if (vsaHdr.GetCategory () == CATEGORY_OF_VSA)
                {
                    VendorSpecificActionHeader vsa;
                    packet->RemoveHeader (vsa);
                    OrganizationIdentifier oi = vsa.GetOrganizationIdentifier ();
                    VscCallback cb = m_vscManager.FindVscCallback (oi);

                    if (cb.IsNull ())
                    {
                        NS_LOG_DEBUG ("cannot find VscCallback for OrganizationIdentifier=" << oi);
                        return;
                    }
                    bool succeed = cb (this, oi,packet, from);

                    if (!succeed)
                    {
                        NS_LOG_DEBUG ("vsc callback could not handle the packet successfully");
                    }

                    return;
                }
            }
            else if (hdr->GetType () == WIFI_MAC_MGT_TX_BEACON)
            {
                TxBeaconHeader txBeaconHeader;
                packet->RemoveHeader (txBeaconHeader);
                Time duration = txBeaconHeader.GetDuration ();
                std::vector<Mac48Address> rx = txBeaconHeader.GetRx ();
                auto rIt = rx.begin ();
                int size = rx.size ();
                for (int n = 0; n < size; n++)
                {
                    if ((*rIt) == GetAddress ())
                    {
                        NS_ASSERT (!m_addRequest.IsNull ());
                        m_addRequest (from, (*rIt), duration);
                    }
                    rIt++;
                }
                return;
            }
            else if (hdr->GetType () == WIFI_MAC_MGT_RX_BEACON)
            {
                RxBeaconHeader rxBeaconHeader;
                packet->RemoveHeader (rxBeaconHeader);
                Time a = rxBeaconHeader.GetDuration ();
                Time b = rxBeaconHeader.GetDelay ();
                Time start = Simulator::Now () + b;

                NS_ASSERT (!m_addAgreement.IsNull ());
                if (to == Mac48Address::GetBroadcast ())
                {
                    m_addAgreement (from, to, start, a);
                }
                else
                {
                    m_addAgreement (to, from, start, a);
                }

                NS_ASSERT (!m_rxBeacon.IsNull ());
                m_rxBeacon (*hdr);
                return;
            }
            else if (hdr->GetType () == WIFI_MAC_MGT_NULL_BEACON)
            {
                return;
            }
        }

        RegularWifiMac::Receive (Create<WifiMacQueueItem> (packet, *hdr));
    }

    void
    V2xCtrlMac::ConfigureEdca (uint32_t cwmin, uint32_t cwmax, uint32_t aifsn, enum AcIndex ac)
    {
        NS_LOG_FUNCTION (this << cwmin << cwmax << aifsn << ac);
        Ptr<Txop> dcf;
        switch (ac)
        {
            case AC_VO:
                dcf = RegularWifiMac::GetVOQueue ();
                dcf->SetMinCw ((cwmin + 1) / 4 - 1);
                dcf->SetMaxCw ((cwmin + 1) / 2 - 1);
                dcf->SetAifsn (aifsn);
                break;
            case AC_VI:
                dcf = RegularWifiMac::GetVIQueue ();
                dcf->SetMinCw ((cwmin + 1) / 2 - 1);
                dcf->SetMaxCw (cwmin);
                dcf->SetAifsn (aifsn);
                break;
            case AC_BE:
                dcf = RegularWifiMac::GetBEQueue ();
                dcf->SetMinCw (cwmin);
                dcf->SetMaxCw (cwmax);
                dcf->SetAifsn (aifsn);
                break;
            case AC_BK:
                dcf = RegularWifiMac::GetBKQueue ();
                dcf->SetMinCw (cwmin);
                dcf->SetMaxCw (cwmax);
                dcf->SetAifsn (aifsn);
                break;
            case AC_BE_NQOS:
                dcf = RegularWifiMac::GetTxop ();
                dcf->SetMinCw (cwmin);
                dcf->SetMaxCw (cwmax);
                dcf->SetAifsn (aifsn);
                break;
            case AC_UNDEF:
                NS_FATAL_ERROR ("I don't know what to do with this");
                break;
        }
    }

    void
    V2xCtrlMac::ConfigureStandard (enum WifiStandard standard)
    {
        NS_LOG_FUNCTION (this << standard);
        NS_ASSERT (standard == WIFI_STANDARD_80211p);

        uint32_t cwmin = 15;
        uint32_t cwmax = 1023;

        ConfigureEdca (cwmin, cwmax, 2, AC_BE_NQOS);
        ConfigureEdca (cwmin, cwmax, 2, AC_VO);
        ConfigureEdca (cwmin, cwmax, 3, AC_VI);
        ConfigureEdca (cwmin, cwmax, 6, AC_BE);
        ConfigureEdca (cwmin, cwmax, 9, AC_BK);
    }


    void
    V2xCtrlMac::Suspend ()
    {
        NS_LOG_FUNCTION (this);
        m_channelAccessManager->NotifySleepNow ();
        m_low->NotifySleepNow ();
    }

    void
    V2xCtrlMac::Resume ()
    {
        NS_LOG_FUNCTION (this);
        m_channelAccessManager->NotifyWakeupNow ();
    }

    void
    V2xCtrlMac::MakeVirtualBusy (Time duration)
    {
        NS_LOG_FUNCTION (this << duration);
        m_channelAccessManager->NotifyMaybeCcaBusyStartNow (duration);
    }

    void
    V2xCtrlMac::CancleTx (enum AcIndex ac)
    {
        NS_LOG_FUNCTION (this << ac);
        Ptr<QosTxop> queue = m_edca.find (ac)->second;
        NS_ASSERT (queue != 0);
        queue->NotifyChannelSwitching ();
    }

    void
    V2xCtrlMac::Reset ()
    {
        NS_LOG_FUNCTION (this);
        m_channelAccessManager->NotifySwitchingStartNow (Time (0));
        m_low->NotifySwitchingStartNow (Time (0));
    }

    void
    V2xCtrlMac::EnableForWave (Ptr<V2xMmWaveNetDevice> device)
    {
        NS_LOG_FUNCTION (this << device);
        m_low = CreateObject<V2xCtrlMacLow> ();
        (DynamicCast<V2xCtrlMacLow> (m_low))->SetWaveNetDevice (device);
        m_low->SetEndTxNoAckCallback (MakeCallback (&V2xCtrlMac::NotifyEndTxNoAck, this));
        m_low->SetRxCallback (MakeCallback (&MacRxMiddle::Receive, m_rxMiddle));
        m_channelAccessManager->SetupLow (m_low);
        m_txop->SetMacLow (m_low);
        for (EdcaQueues::iterator i = m_edca.begin (); i != m_edca.end (); ++i)
        {
            i->second->SetMacLow (m_low);
            i->second->CompleteConfig ();
        }
    }

    void
    V2xCtrlMac::SendBeacon (Ptr<Packet> packet, Mac48Address to)
    {
        if (m_txop->IsEmpty ())
        {
            if (m_stationManager->IsBrandNew (to))
            {
                if (GetHtSupported () || GetVhtSupported ())
                {
                    m_stationManager->AddAllSupportedMcs (to);
                    m_stationManager->AddStationHtCapabilities (to, GetHtCapabilities());
                }
                if (GetVhtSupported ())
                {
                    m_stationManager->AddStationVhtCapabilities (to, GetVhtCapabilities());
                }
                m_stationManager->AddAllSupportedModes (to);
                m_stationManager->RecordDisassociated (to);
            }

            WifiMacHeader hdr;
            hdr.SetType (WIFI_MAC_MGT_NULL_BEACON);
            hdr.SetAddr1 (to);
            hdr.SetAddr2 (GetAddress ());
            hdr.SetAddr3 (Mac48Address::GetBroadcast ());
            hdr.SetDsNotFrom ();
            hdr.SetDsNotTo ();
            if (GetHtSupported () || GetVhtSupported ())
            {
                hdr.SetNoOrder ();
            }
            m_txop->Queue (packet, hdr);
        }
    }

    Ptr<WifiMacQueueItem>
    V2xCtrlMac::ReplaceBeacon (Ptr<WifiMacQueueItem> item)
    {
        if (!m_replaceBeacon.IsNull ())
        {
            item = m_replaceBeacon (item);
        }
        else
        {
            NS_FATAL_ERROR("V2xCtrlMac ReplaceBeacon - Callback is null");
        }
        return item;
    }

    void
    V2xCtrlMac::NotifyEndTxNoAck (WifiMacHeader & hdr)
    {
        NS_LOG_FUNCTION (this << hdr);
        if (!m_endTxNoAck.IsNull ())
        {
            m_endTxNoAck (hdr);
        }
        else
        {
            NS_FATAL_ERROR("V2xCtrlMac NotifyTxNoAck - Callback is null");
        }
    }

    void
    V2xCtrlMac::SetReplaceBeaconCallback (Callback <Ptr<WifiMacQueueItem>, Ptr<WifiMacQueueItem>> callback)
    {
        NS_LOG_FUNCTION (this << &callback);
        m_replaceBeacon = callback;
    }

    void
    V2xCtrlMac::SetAddRequestCallback (Callback <void, Mac48Address, Mac48Address, Time> callback)
    {
        NS_LOG_FUNCTION (this << &callback);
        m_addRequest = callback;
    }
    void
    V2xCtrlMac::SetAddAgreementCallback (Callback <void, Mac48Address, Mac48Address, Time, Time> callback)
    {
        NS_LOG_FUNCTION (this << &callback);
        m_addAgreement = callback;
    }

    void
    V2xCtrlMac::SetRxBeaconCallback (Callback <void, const WifiMacHeader &> callback)
    {
        NS_LOG_FUNCTION (this << &callback);
        m_rxBeacon = callback;
    }

    void
    V2xCtrlMac::SetTxNoAckCallback (Callback <void, WifiMacHeader &> callback)
    {
        NS_LOG_FUNCTION (this << &callback);
        m_endTxNoAck = callback;
    }

} // namespace ns3
