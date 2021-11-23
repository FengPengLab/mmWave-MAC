/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/simulator.h"
#include "mmwave-snr-tag.h"
#include "mmwave-psdu.h"
#include "mmwave-phy.h"
#include "mmwave-remote-station-manager.h"
#include "v2x-data-mac-low.h"

#undef NS_LOG_APPEND_CONTEXT
#define NS_LOG_APPEND_CONTEXT std::clog << "[mac=" << m_self << "] "

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("V2xDataMacLow");
    V2xDataMacLow::V2xDataMacLow ()
    {
        NS_LOG_FUNCTION (this);
    }

    V2xDataMacLow::~V2xDataMacLow ()
    {
        NS_LOG_FUNCTION (this);
    }

    TypeId
    V2xDataMacLow::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::V2xDataMacLow")
                .SetParent<MmWaveMacLow> ()
                .SetGroupName ("MmWave")
                .AddConstructor<V2xDataMacLow> ()
        ;
        return tid;
    }

    void
    V2xDataMacLow::DoNavResetNow (Time duration)
    {
        NS_LOG_FUNCTION (this << duration);
    }

    bool
    V2xDataMacLow::DoNavStartNow (Time duration)
    {
        return false;
    }

    void
    V2xDataMacLow::NotifyAckTimeoutStartNow (Time duration)
    {
        NS_LOG_FUNCTION (this);
    }

    void
    V2xDataMacLow::NotifyAckTimeoutResetNow ()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    V2xDataMacLow::NotifyCtsTimeoutStartNow (Time duration)
    {
        NS_LOG_FUNCTION (this);
    }

    void
    V2xDataMacLow::NotifyCtsTimeoutResetNow ()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    V2xDataMacLow::NotifyNav (Ptr<const Packet> packet,const MmWaveMacHeader &hdr)
    {
        NS_LOG_FUNCTION (this);
    }

    void
    V2xDataMacLow::NotifySwitchingStartNow (Time duration)
    {
        NS_LOG_DEBUG ("switching channel. Cancelling MAC pending events");
        m_stationManager->Reset ();
        CancelAllEvents ();
        if (m_navCounterReset.IsRunning ())
        {
            m_navCounterReset.Cancel ();
        }
        m_lastNavStart = Simulator::Now ();
        m_lastNavDuration = Seconds (0.0);
        m_currentPacket = 0;
    }

    void
    V2xDataMacLow::NotifySleepNow ()
    {
        NS_LOG_DEBUG ("Device in sleep mode. Cancelling MAC pending events");
        CancelAllEvents ();
        if (m_navCounterReset.IsRunning ())
        {
            m_navCounterReset.Cancel ();
        }
        m_lastNavStart = Simulator::Now ();
        m_lastNavDuration = Seconds (0.0);
        m_currentPacket = 0;
    }

    void
    V2xDataMacLow::NotifyOffNow ()
    {
        NS_LOG_DEBUG ("Device is switched off. Cancelling MAC pending events");
        CancelAllEvents ();
        if (m_navCounterReset.IsRunning ())
        {
            m_navCounterReset.Cancel ();
        }
        m_lastNavStart = Simulator::Now ();
        m_lastNavDuration = Seconds (0.0);
        m_currentPacket = 0;
    }

    void
    V2xDataMacLow::CtsTimeout ()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    V2xDataMacLow::NormalAckTimeout ()
    {
        NS_LOG_FUNCTION (this);
        if (!m_txFailed.IsNull ())
        {
            m_txFailed (m_currentPacket->GetHeader ());
        }
    }

    void
    V2xDataMacLow::WaitIfsAfterEndTxFragment ()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    V2xDataMacLow::WaitIfsAfterEndTxPacket ()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    V2xDataMacLow::EndTxNoAck ()
    {
        NS_LOG_FUNCTION (this);
        if (!m_txOk.IsNull ())
        {
            m_txOk (m_currentPacket->GetHeader());
        }
    }

    void
    V2xDataMacLow::StartTransmissionImmediately (Ptr<MmWaveMacQueueItem> mpdu, MmWaveMacLowParameters params)
    {
        m_currentPacket = Create<MmWavePsdu> (mpdu->GetPacket (), mpdu->GetHeader ());
        const MmWaveMacHeader& hdr = mpdu->GetHeader ();
        CancelAllEvents ();
        m_txParams = params;
        if (hdr.IsCtl ())
        {
            m_currentTxVector = GetCtrlTxVector (mpdu->GetHeader().GetAddr1());
        }
        else
        {
            m_currentTxVector = GetDataTxVector (mpdu->GetHeader().GetAddr1());
        }
        SendPacket ();

        NS_ASSERT (m_phy->IsStateTx () || m_phy->IsStateOff ());
    }

    void
    V2xDataMacLow::StartDataTxTimers (const MmWaveTxVector dataTxVector)
    {
        Time txDuration = m_phy->CalculateTxDuration (m_currentPacket->GetSize (), dataTxVector, m_phy->GetPhyBand ());
        if (m_txParams.MustWaitNormalAck ())
        {
            MmWaveTxVector ackTxVector = GetCtrlTxVector (m_currentPacket->GetAddr1 ());
            Time timerDelay = txDuration + GetAckDuration (ackTxVector) + GetSifs () + GetSlotTime () + m_phy->CalculatePhyPreambleAndHeaderDuration (ackTxVector);
            NS_ASSERT (m_normalAckTimeoutEvent.IsExpired ());
            NotifyAckTimeoutStartNow (timerDelay);
            m_normalAckTimeoutEvent = Simulator::Schedule (timerDelay, &V2xDataMacLow::NormalAckTimeout, this);
        }
        else if (m_txParams.HasNextPacket ())
        {
            NS_ASSERT (m_waitIfsEvent.IsExpired ());
            Time delay = txDuration + GetSifs ();
            m_waitIfsEvent = Simulator::Schedule (delay, &V2xDataMacLow::WaitIfsAfterEndTxFragment, this);
        }
        else
        {
            m_endTxNoAckEvent = Simulator::Schedule (txDuration, &V2xDataMacLow::EndTxNoAck, this);
        }
    }

    void
    V2xDataMacLow::SendPacket ()
    {
        NS_LOG_FUNCTION (this);
        StartDataTxTimers (m_currentTxVector);
        if (m_currentPacket->GetHeader().IsData ())
        {
            Time duration = GetResponseDuration (m_txParams, m_currentTxVector, m_currentPacket->GetAddr1 ());
            if (m_txParams.HasNextPacket ())
            {
                duration += GetSifs ();
                duration += m_phy->CalculateTxDuration (m_txParams.GetNextPacketSize (), m_currentTxVector, m_phy->GetPhyBand ());
                duration += GetResponseDuration (m_txParams, m_currentTxVector, m_currentPacket->GetAddr1 ());
            }
            m_currentPacket->SetDuration (duration);
        }
        ForwardDown (m_currentPacket, m_currentTxVector);
    }

    void
    V2xDataMacLow::SendAckAfterData (Mac48Address source, MmWaveMode dataTxMode, double dataSnr)
    {
        NS_LOG_FUNCTION (this << source << dataTxMode << dataSnr);
        MmWaveTxVector ackTxVector = GetCtrlTxVector (source);
        MmWaveMacHeader ack;
        ack.SetType (MMWAVE_MAC_CTL_ACK);
        ack.SetDsNotFrom ();
        ack.SetDsNotTo ();
        ack.SetNoRetry ();
        ack.SetNoMoreFragments ();
        ack.SetAddr1 (source);
        ack.SetDuration (Seconds (0.0));
        Ptr<Packet> packet = Create<Packet> ();
        MmWaveSnrTag tag;
        tag.Set (dataSnr);
        packet->AddPacketTag (tag);
        ForwardDown (Create<const MmWavePsdu> (packet, ack), ackTxVector);
    }

    void
    V2xDataMacLow::RxStartIndication (MmWaveTxVector txVector, Time psduDuration)
    {
        if (psduDuration.IsZero ())
        {
            return;
        }
        NS_ASSERT (psduDuration.IsStrictlyPositive ());

        if (m_normalAckTimeoutEvent.IsRunning ())
        {
            NS_LOG_DEBUG ("Rescheduling Normal Ack timeout");
            m_normalAckTimeoutEvent.Cancel ();
            NotifyAckTimeoutResetNow ();
            m_normalAckTimeoutEvent = Simulator::Schedule (psduDuration + NanoSeconds (400), &MmWaveMacLow::NormalAckTimeout, this);
        }
        else if (m_ctsTimeoutEvent.IsRunning ())
        {
            NS_LOG_DEBUG ("Rescheduling CTS timeout");
            m_ctsTimeoutEvent.Cancel ();
            NotifyCtsTimeoutResetNow ();
            m_ctsTimeoutEvent = Simulator::Schedule (psduDuration + NanoSeconds (400), &MmWaveMacLow::CtsTimeout, this);
        }
        else if (m_navCounterReset.IsRunning ())
        {
            NS_LOG_DEBUG ("Cannot reset NAV");
            m_navCounterReset.Cancel ();
        }
    }

    void
    V2xDataMacLow::UnrecognizedSignalDetected (double power, double snr, Time start, Time duration)
    {
        NS_LOG_FUNCTION (this);
    }

    void
    V2xDataMacLow::RecognizedSignalDetected (double power, double snr, Time start, Time duration)
    {
        NS_LOG_FUNCTION (this);
    }

    void
    V2xDataMacLow::CancelAllEvents ()
    {
        if (m_normalAckTimeoutEvent.IsRunning ())
        {
            m_normalAckTimeoutEvent.Cancel ();
        }
        if (m_ctsTimeoutEvent.IsRunning ())
        {
            m_ctsTimeoutEvent.Cancel ();
        }
        if (m_sendCtsEvent.IsRunning ())
        {
            m_sendCtsEvent.Cancel ();
        }
        if (m_sendAckEvent.IsRunning ())
        {
            m_sendAckEvent.Cancel ();
        }
        if (m_waitIfsEvent.IsRunning ())
        {
            m_waitIfsEvent.Cancel ();
        }
        if (m_endTxNoAckEvent.IsRunning ())
        {
            m_endTxNoAckEvent.Cancel ();
        }
    }

    void
    V2xDataMacLow::ReceiveError (Ptr<MmWavePsdu> psdu, double rxSnr, Time start, Time duration, MmWaveTxVector txVector)
    {
        NS_LOG_FUNCTION (this << *psdu << rxSnr << start << duration);
        return;
    }

    void
    V2xDataMacLow::ReceiveOk (Ptr<MmWavePsdu> psdu, double rxSnr, Time start, Time duration, MmWaveTxVector txVector)
    {
        NS_LOG_FUNCTION (this << *psdu << rxSnr << start << duration);
        Ptr<MmWaveMacQueueItem> mpdu = psdu->GetMpdu ();
        const MmWaveMacHeader& hdr = mpdu->GetHeader ();
        Ptr<Packet> packet = mpdu->GetPacket ()->Copy ();
        NotifyNav (packet, hdr);
        if (hdr.IsAck ()
                 && hdr.GetAddr1 () == m_self
                 && m_normalAckTimeoutEvent.IsRunning ()
                 && m_txParams.MustWaitNormalAck ())
        {
            NS_LOG_DEBUG ("receive ack from=" << m_currentPacket->GetAddr1 ());
            MmWaveSnrTag tag;
            packet->RemovePacketTag (tag);
            m_normalAckTimeoutEvent.Cancel ();
            NotifyAckTimeoutResetNow ();
            if (!m_txOk.IsNull ())
            {
                m_txOk (m_currentPacket->GetHeader());
            }
        }
        else if (hdr.GetAddr1 () == m_self)
        {
            m_stationManager->ReportRxOk (hdr.GetAddr2 (), rxSnr, txVector.GetMode ());
            if (hdr.IsData () || hdr.IsMgt ())
            {
                NS_ASSERT (m_sendAckEvent.IsExpired ());
                m_sendAckEvent = Simulator::Schedule (GetSifs (), &V2xDataMacLow::SendAckAfterData, this, hdr.GetAddr2 (), txVector.GetMode (), rxSnr);
                m_rxCallback (mpdu);
            }
        }
        else if (hdr.GetAddr1 ().IsGroup ())
        {
            if (hdr.IsData () || hdr.IsMgt ())
            {
                NS_LOG_DEBUG ("rx data/mgt from=" << hdr.GetAddr2 ());
                m_rxCallback (mpdu);
            }
        }
        else if (m_promisc)
        {
            NS_ASSERT (hdr.GetAddr1 () != m_self);
            if (hdr.IsData () || hdr.IsMgt ())
            {
                m_rxCallback (mpdu);
            }
        }
        else
        {
            NS_LOG_DEBUG ("rx not for me from=" << hdr.GetAddr2 ());
        }
    }

}