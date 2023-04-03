/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include <cmath>
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/nstime.h"
#include "mmwave-phy.h"
#include "mmwave-mac-low.h"
#include "mmwave-mac-header.h"
#include "v2x-data-mac.h"
#include "v2x-net-device.h"
namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("V2xDataMac");
    NS_OBJECT_ENSURE_REGISTERED (V2xDataMac);

    TypeId
    V2xDataMac::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::V2xDataMac")
                .SetParent<MmWaveMac> ()
                .SetGroupName ("MmWave")
                .AddConstructor<V2xDataMac> ()
        ;
        return tid;
    }

    V2xDataMac::V2xDataMac ()
    {
        m_rxMiddle = Create<MmWaveMacRxMiddle> ();
        m_rxMiddle->SetForwardCallback (MakeCallback (&V2xDataMac::Receive, this));

        m_txMiddle = Create<MmWaveMacTxMiddle> ();

        m_low = CreateObject<V2xDataMacLow> ();
        m_low->SetRxCallback (MakeCallback (&MmWaveMacRxMiddle::Receive, m_rxMiddle));
        m_low->SetMac (this);
        m_low->SetTxOkCallback (MakeCallback (&MmWaveMac::TxOk, this));
        m_low->SetTxFailedCallback (MakeCallback (&MmWaveMac::TxFailed, this));

    }

    V2xDataMac::~V2xDataMac ()
    {
    }

    void
    V2xDataMac::DoInitialize ()
    {
        MmWaveMac::DoInitialize ();
    }

    void
    V2xDataMac::DoDispose ()
    {
        MmWaveMac::DoDispose ();
    }

    void
    V2xDataMac::SetPhy (const Ptr<MmWavePhy> phy)
    {
        m_phy = phy;
        m_low->SetPhy (phy);
    }

    void
    V2xDataMac::ResetPhy ()
    {
        m_low->ResetPhy ();
        m_phy = 0;
    }

    void
    V2xDataMac::SetRemoteStationManager (Ptr<MmWaveRemoteStationManager> stationManager)
    {
        m_stationManager = stationManager;
        m_low->SetRemoteStationManager (stationManager);
    }

    void
    V2xDataMac::SetAddress (Mac48Address address)
    {
        m_low->SetAddress (address);
    }

    void
    V2xDataMac::SetBssid (Mac48Address address)
    {
        m_low->SetBssid (address);
    }

    void
    V2xDataMac::SetLinkUpCallback (Callback<void> linkUp)
    {
        MmWaveMac::SetLinkUpCallback (linkUp);
        linkUp ();
    }

    void
    V2xDataMac::SetPromisc ()
    {
        m_low->SetPromisc ();
    }

    void
    V2xDataMac::SetTxOkCallback (Callback <void, const MmWaveMacHeader &> callback)
    {
        m_txOk = callback;
    }

    void
    V2xDataMac::SetTxFailedCallback (Callback <void, const MmWaveMacHeader &> callback)
    {
        m_txFailed = callback;
    }

    Mac48Address
    V2xDataMac::GetAddress () const
    {
        return m_low->GetAddress ();
    }

    Mac48Address V2xDataMac::GetBssid () const
    {
        return m_low->GetBssid ();
    }

    Ptr<V2xDataMacLow>
    V2xDataMac::GetMacLow () const
    {
        return m_low;
    }

    Ptr<MmWavePhy>
    V2xDataMac::GetPhy () const
    {
        return m_phy;
    }

    void
    V2xDataMac::TransmitImmediately (Ptr<Packet> packet, Mac48Address to)
    {
        NS_LOG_FUNCTION (this << packet << to);
        if (m_phy->IsStateOff ())
        {
            NS_LOG_DEBUG ("Cannot start TX because device is OFF");
            return;
        }

        if (IsStateIdle ()) {
            MmWaveMacHeader hdr;
            hdr.SetType(MMWAVE_MAC_DATA);
            hdr.SetNoOrder();
            hdr.SetAddr1(to);
            hdr.SetAddr2(m_low->GetAddress());
            hdr.SetAddr3(GetBssid());
            hdr.SetDsNotFrom();
            hdr.SetDsNotTo();
            hdr.SetDuration(MicroSeconds(0));
            uint16_t sequence = m_txMiddle->GetNextSequenceNumberFor(&hdr);
            hdr.SetSequenceNumber(sequence);
            hdr.SetFragmentNumber(0);
            hdr.SetNoMoreFragments();
            hdr.SetNoRetry();
            MmWaveMacLowParameters params;
            if (to != Mac48Address::GetBroadcast()) {
                params.EnableAck();
            } else {
                params.DisableAck();
            }
            m_low->StartTransmissionImmediately(Create<MmWaveMacQueueItem>(packet, hdr), params);
        }
        else
        {
            Ptr<V2xMmWaveNetDevice> device = DynamicCast<V2xMmWaveNetDevice> (m_device);
            NS_ASSERT (m_device != 0);
            if (!m_tryAgain.IsRunning ())
            {
                m_tryAgain = Simulator::Schedule (m_phy->GetDelayUntilIdle (), &V2xMmWaveNetDevice::TryToStartContentionFreeDuration, device);
            }
        }
    }

    void
    V2xDataMac::Enqueue (Ptr<Packet> packet, Mac48Address to)
    {
        TransmitImmediately (packet, to);
    }

    void
    V2xDataMac::Enqueue (Ptr<Packet> packet, Mac48Address to, Time delay)
    {
        NS_ASSERT (delay.IsStrictlyPositive ());
        m_waitToSend = Simulator::Schedule (delay, &V2xDataMac::TransmitImmediately, this, packet, to);
    }

    Time
    V2xDataMac::GetAckDuration (Mac48Address to)
    {
        return m_low->GetAckDuration (m_low->GetCtrlTxVector (to));
    }

    void
    V2xDataMac::Enqueue (Ptr<Packet> packet, Mac48Address to, Mac48Address from)
    {
        NS_FATAL_ERROR ("This MAC entity (" << this << ", " << GetAddress () << ") does not support Enqueue() with from address");
    }

    void
    V2xDataMac::Receive (Ptr<MmWaveMacQueueItem> mpdu)
    {
        NS_LOG_FUNCTION (this << *mpdu);
        const MmWaveMacHeader* hdr = &mpdu->GetHeader ();
        NS_ASSERT (!hdr->IsCtl ());
        Mac48Address from = hdr->GetAddr2 ();
        Mac48Address to = hdr->GetAddr1 ();

        if (hdr->IsData ())
        {
            ForwardUp (mpdu->GetPacket ()->Copy (), from, to);
            return;
        }

        MmWaveMac::Receive (mpdu);
    }

    Time
    V2xDataMac::GetResponseDuration (const MmWaveMacLowParameters& params, MmWaveTxVector dataTxVector, Mac48Address receiver) const
    {
        return m_low->GetResponseDuration (params, dataTxVector, receiver);
    }

    bool
    V2xDataMac::IsWaitToTransmit ()
    {
        return m_waitToSend.IsRunning ();
    }

    bool
    V2xDataMac::IsStateIdle ()
    {
        return m_low->GetPhy ()->IsStateIdle ();
    }

    void
    V2xDataMac::TxOk (const MmWaveMacHeader &hdr)
    {
        NS_LOG_FUNCTION (this << hdr);
        MmWaveMac::TxOk (hdr);
        m_txOk (hdr);
    }

    void
    V2xDataMac::TxFailed (const MmWaveMacHeader &hdr)
    {
        NS_LOG_FUNCTION (this << hdr);
        MmWaveMac::TxFailed (hdr);
        m_txFailed (hdr);
    }
} // namespace ns3