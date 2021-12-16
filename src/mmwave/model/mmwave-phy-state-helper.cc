/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include <algorithm>
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "ns3/log.h"
#include "mmwave-phy-state-helper.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("MmWavePhyStateHelper");
    NS_OBJECT_ENSURE_REGISTERED (MmWavePhyStateHelper);

    TypeId
    MmWavePhyStateHelper::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::MmWavePhyStateHelper")
                .SetParent<Object> ()
                .SetGroupName ("MmWave")
                .AddConstructor<MmWavePhyStateHelper> ()
                .AddTraceSource ("State",
                                 "The state of the PHY layer",
                                 MakeTraceSourceAccessor (&MmWavePhyStateHelper::m_stateLogger),
                                 "ns3::MmWavePhyStateHelper::StateTracedCallback")
                .AddTraceSource ("RxOk",
                                 "A packet has been received successfully.",
                                 MakeTraceSourceAccessor (&MmWavePhyStateHelper::m_rxOkTrace),
                                 "ns3::MmWavePhyStateHelper::RxOkTracedCallback")
                .AddTraceSource ("RxError",
                                 "A packet has been received unsuccessfully.",
                                 MakeTraceSourceAccessor (&MmWavePhyStateHelper::m_rxErrorTrace),
                                 "ns3::MmWavePhyStateHelper::RxEndErrorTracedCallback")
                .AddTraceSource ("Tx", "Packet transmission is starting.",
                                 MakeTraceSourceAccessor (&MmWavePhyStateHelper::m_txTrace),
                                 "ns3::MmWavePhyStateHelper::TxTracedCallback")
        ;
        return tid;
    }

    MmWavePhyStateHelper::MmWavePhyStateHelper ()
            : m_sleeping (false),
              m_isOff (false),
              m_endTx (Seconds (0.0)),
              m_endRx (Seconds (0.0)),
              m_endCcaBusy (Seconds (0.0)),
              m_endSwitching (Seconds (0.0)),
              m_startTx (Seconds (0.0)),
              m_startRx (Seconds (0.0)),
              m_startCcaBusy (Seconds (0.0)),
              m_startSwitching (Seconds (0.0)),
              m_startSleep (Seconds (0.0)),
              m_previousStateChangeTime (Seconds (0.0))
    {
        NS_LOG_FUNCTION (this);
    }

    void
    MmWavePhyStateHelper::SetReceiveOkCallback (Callback<void, Ptr<MmWavePsdu>, double, Time, Time, MmWaveTxVector> callback)
    {
        m_rxOkCallback = callback;
    }

    void
    MmWavePhyStateHelper::SetReceiveErrorCallback (Callback<void, Ptr<MmWavePsdu>, double, Time, Time, MmWaveTxVector> callback)
    {
        m_rxErrorCallback = callback;
    }

    void
    MmWavePhyStateHelper::RegisterListener (MmWavePhyListener *listener)
    {
        m_listeners.push_back (listener);
    }

    void
    MmWavePhyStateHelper::UnregisterListener (MmWavePhyListener *listener)
    {
        std::vector<MmWavePhyListener *>::iterator i = find (m_listeners.begin (), m_listeners.end (), listener);
        if (i != m_listeners.end ())
        {
            m_listeners.erase (i);
        }
    }

    bool
    MmWavePhyStateHelper::IsStateIdle () const
    {
        return (GetState () == MmWavePhyState::MMWAVE_IDLE);
    }

    bool
    MmWavePhyStateHelper::IsStateCcaBusy () const
    {
        return (GetState () == MmWavePhyState::MMWAVE_CCA_BUSY);
    }

    bool
    MmWavePhyStateHelper::IsStateRx () const
    {
        return (GetState () == MmWavePhyState::MMWAVE_RX);
    }

    bool
    MmWavePhyStateHelper::IsStateTx () const
    {
        return (GetState () == MmWavePhyState::MMWAVE_TX);
    }

    bool
    MmWavePhyStateHelper::IsStateSwitching () const
    {
        return (GetState () == MmWavePhyState::MMWAVE_SWITCHING);
    }

    bool
    MmWavePhyStateHelper::IsStateSleep () const
    {
        return (GetState () == MmWavePhyState::MMWAVE_SLEEP);
    }

    bool
    MmWavePhyStateHelper::IsStateOff () const
    {
        return (GetState () == MmWavePhyState::MMWAVE_OFF);
    }

    Time
    MmWavePhyStateHelper::GetDelayUntilIdle () const
    {
        Time retval;

        switch (GetState ())
        {
            case MmWavePhyState::MMWAVE_RX:
                retval = m_endRx - Simulator::Now ();
                break;
            case MmWavePhyState::MMWAVE_TX:
                retval = m_endTx - Simulator::Now ();
                break;
            case MmWavePhyState::MMWAVE_CCA_BUSY:
                retval = m_endCcaBusy - Simulator::Now ();
                break;
            case MmWavePhyState::MMWAVE_SWITCHING:
                retval = m_endSwitching - Simulator::Now ();
                break;
            case MmWavePhyState::MMWAVE_IDLE:
            case MmWavePhyState::MMWAVE_SLEEP:
            case MmWavePhyState::MMWAVE_OFF:
                retval = Seconds (0.0);
                break;
            default:
                NS_FATAL_ERROR ("Invalid MmWavePhy state.");
                retval = Seconds (0.0);
                break;
        }
        retval = Max (retval, Seconds (0.0));
        return retval;
    }

    Time
    MmWavePhyStateHelper::GetLastRxStartTime () const
    {
        return m_startRx;
    }

    Time
    MmWavePhyStateHelper::GetLastRxEndTime () const
    {
        return m_endRx;
    }

    MmWavePhyState
    MmWavePhyStateHelper::GetState () const
    {
        NS_LOG_DEBUG ("GetState endTx:" << m_endTx << ", endRx:" << m_endRx << ", endCca:" << m_endCcaBusy);
        if (m_isOff)
        {
            return MmWavePhyState::MMWAVE_OFF;
        }
        if (m_sleeping)
        {
            return MmWavePhyState::MMWAVE_SLEEP;
        }
        else if (m_endTx > Simulator::Now ())
        {
            return MmWavePhyState::MMWAVE_TX;
        }
        else if (m_endRx > Simulator::Now ())
        {
            return MmWavePhyState::MMWAVE_RX;
        }
        else if (m_endSwitching > Simulator::Now ())
        {
            return MmWavePhyState::MMWAVE_SWITCHING;
        }
        else if (m_endCcaBusy > Simulator::Now ())
        {
            return MmWavePhyState::MMWAVE_CCA_BUSY;
        }
        else
        {
            return MmWavePhyState::MMWAVE_IDLE;
        }
    }

    void
    MmWavePhyStateHelper::NotifyTxStart (Time duration, double txPowerDbm)
    {
        NS_LOG_FUNCTION (this);
        for (std::vector<MmWavePhyListener *>::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
        {
            (*i)->NotifyTxStart (duration, txPowerDbm);
        }
    }

    void
    MmWavePhyStateHelper::NotifyRxStart (Time duration)
    {
        NS_LOG_FUNCTION (this);
        for (std::vector<MmWavePhyListener *>::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
        {
            (*i)->NotifyRxStart (duration);
        }
    }

    void
    MmWavePhyStateHelper::NotifyRxEndOk ()
    {
        NS_LOG_FUNCTION (this);
        for (std::vector<MmWavePhyListener *>::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
        {
            (*i)->NotifyRxEndOk ();
        }
    }

    void
    MmWavePhyStateHelper::NotifyRxEndError ()
    {
        NS_LOG_FUNCTION (this);
        for (std::vector<MmWavePhyListener *>::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
        {
            (*i)->NotifyRxEndError ();
        }
    }

    void
    MmWavePhyStateHelper::NotifyMaybeCcaBusyStart (Time duration)
    {
        NS_LOG_FUNCTION (this);
        for (std::vector<MmWavePhyListener *>::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
        {
            (*i)->NotifyMaybeCcaBusyStart (duration);
        }
    }

    void
    MmWavePhyStateHelper::NotifySwitchingStart (Time duration)
    {
        NS_LOG_FUNCTION (this);
        for (std::vector<MmWavePhyListener *>::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
        {
            (*i)->NotifySwitchingStart (duration);
        }
    }

    void
    MmWavePhyStateHelper::NotifySleep ()
    {
        NS_LOG_FUNCTION (this);
        for (std::vector<MmWavePhyListener *>::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
        {
            (*i)->NotifySleep ();
        }
    }

    void
    MmWavePhyStateHelper::NotifyOff ()
    {
        NS_LOG_FUNCTION (this);
        for (std::vector<MmWavePhyListener *>::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
        {
            (*i)->NotifyOff ();
        }
    }

    void
    MmWavePhyStateHelper::NotifyWakeup ()
    {
        NS_LOG_FUNCTION (this);
        for (std::vector<MmWavePhyListener *>::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
        {
            (*i)->NotifyWakeup ();
        }
    }

    void
    MmWavePhyStateHelper::NotifyOn ()
    {
        NS_LOG_FUNCTION (this);
        for (std::vector<MmWavePhyListener *>::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
        {
            (*i)->NotifyOn ();
        }
    }

    void
    MmWavePhyStateHelper::LogPreviousIdleAndCcaBusyStates ()
    {
        NS_LOG_FUNCTION (this);
        Time now = Simulator::Now ();
        Time idleStart = Max (m_endCcaBusy, m_endRx);
        idleStart = Max (idleStart, m_endTx);
        idleStart = Max (idleStart, m_endSwitching);
        NS_ASSERT (idleStart <= now);
        if (m_endCcaBusy > m_endRx
            && m_endCcaBusy > m_endSwitching
            && m_endCcaBusy > m_endTx)
        {
            Time ccaBusyStart = Max (m_endTx, m_endRx);
            ccaBusyStart = Max (ccaBusyStart, m_startCcaBusy);
            ccaBusyStart = Max (ccaBusyStart, m_endSwitching);
            Time ccaBusyDuration = idleStart - ccaBusyStart;
            if (ccaBusyDuration.IsStrictlyPositive ())
            {
                m_stateLogger (ccaBusyStart, ccaBusyDuration, MmWavePhyState::MMWAVE_CCA_BUSY);
            }
        }
        Time idleDuration = now - idleStart;
        if (idleDuration.IsStrictlyPositive ())
        {
            m_stateLogger (idleStart, idleDuration, MmWavePhyState::MMWAVE_IDLE);
        }
    }

    void
    MmWavePhyStateHelper::SwitchToTx (Time txDuration, MmWaveConstPsduMap psdus, double txPowerDbm, MmWaveTxVector txVector, uint8_t channelNum, uint16_t channelFreq, uint16_t channelWidth)
    {
        NS_LOG_FUNCTION (this << txDuration << psdus << txPowerDbm << txVector);
        for (auto const& psdu : psdus)
        {
            m_txTrace (psdu.second->GetPacket (), txVector.GetMode (), txVector.GetPreambleType (), txVector.GetTxPowerLevel (), channelNum, channelFreq, channelWidth);
        }
        Time now = Simulator::Now ();
        switch (GetState ())
        {
            case MmWavePhyState::MMWAVE_RX:
                /* The packet which is being received as well
                 * as its endRx event are cancelled by the caller.
                 */
                m_stateLogger (m_startRx, now - m_startRx, MmWavePhyState::MMWAVE_RX);
                m_endRx = now;
                break;
            case MmWavePhyState::MMWAVE_CCA_BUSY:
            {
                Time ccaStart = Max (m_endRx, m_endTx);
                ccaStart = Max (ccaStart, m_startCcaBusy);
                ccaStart = Max (ccaStart, m_endSwitching);
                m_stateLogger (ccaStart, now - ccaStart, MmWavePhyState::MMWAVE_CCA_BUSY);
            } break;
            case MmWavePhyState::MMWAVE_IDLE:
                LogPreviousIdleAndCcaBusyStates ();
                break;
            default:
                return;
//                NS_FATAL_ERROR ("Invalid MmWavePhy state.");
                break;
        }
        m_stateLogger (now, txDuration, MmWavePhyState::MMWAVE_TX);
        m_previousStateChangeTime = now;
        m_endTx = now + txDuration;
        m_startTx = now;
        NotifyTxStart (txDuration, txPowerDbm);
    }

    void
    MmWavePhyStateHelper::SwitchToRx (Time rxDuration)
    {
        NS_LOG_FUNCTION (this << rxDuration);
        NS_ASSERT (IsStateIdle () || IsStateCcaBusy ());
        Time now = Simulator::Now ();
        switch (GetState ())
        {
            case MmWavePhyState::MMWAVE_IDLE:
                LogPreviousIdleAndCcaBusyStates ();
                break;
            case MmWavePhyState::MMWAVE_CCA_BUSY:
            {
                Time ccaStart = Max (m_endRx, m_endTx);
                ccaStart = Max (ccaStart, m_startCcaBusy);
                ccaStart = Max (ccaStart, m_endSwitching);
                m_stateLogger (ccaStart, now - ccaStart, MmWavePhyState::MMWAVE_CCA_BUSY);
            } break;
            default:
                return;
//                NS_FATAL_ERROR ("Invalid MmWavePhy state " << GetState ());
                break;
        }
        m_previousStateChangeTime = now;
        m_startRx = now;
        m_endRx = now + rxDuration;
        NotifyRxStart (rxDuration);
        NS_ASSERT (IsStateRx ());
    }

    void
    MmWavePhyStateHelper::SwitchToChannelSwitching (Time switchingDuration)
    {
        NS_LOG_FUNCTION (this << switchingDuration);
        Time now = Simulator::Now ();
        switch (GetState ())
        {
            case MmWavePhyState::MMWAVE_RX:
                /* The packet which is being received as well
                 * as its endRx event are cancelled by the caller.
                 */
                m_stateLogger (m_startRx, now - m_startRx, MmWavePhyState::MMWAVE_RX);
                m_endRx = now;
                break;
            case MmWavePhyState::MMWAVE_CCA_BUSY:
            {
                Time ccaStart = Max (m_endRx, m_endTx);
                ccaStart = Max (ccaStart, m_startCcaBusy);
                ccaStart = Max (ccaStart, m_endSwitching);
                m_stateLogger (ccaStart, now - ccaStart, MmWavePhyState::MMWAVE_CCA_BUSY);
            } break;
            case MmWavePhyState::MMWAVE_IDLE:
                LogPreviousIdleAndCcaBusyStates ();
                break;
            default:
                return;
//                NS_FATAL_ERROR ("Invalid MmWavePhy state.");
                break;
        }

        if (now < m_endCcaBusy)
        {
            m_endCcaBusy = now;
        }

        m_stateLogger (now, switchingDuration, MmWavePhyState::MMWAVE_SWITCHING);
        m_previousStateChangeTime = now;
        m_startSwitching = now;
        m_endSwitching = now + switchingDuration;
        NotifySwitchingStart (switchingDuration);
        NS_ASSERT (IsStateSwitching ());
    }

    void
    MmWavePhyStateHelper::SwitchFromRxEndOk (Ptr<MmWavePsdu> psdu, double snr, Time start, Time duration, MmWaveTxVector txVector, uint8_t channelNum, uint16_t channelFreq, uint16_t channelWidth)
    {
        NS_LOG_FUNCTION (this << *psdu << snr << txVector);
        NS_ASSERT (m_endRx == Simulator::Now ());
        m_rxOkTrace (psdu->GetPacket (), snr, txVector.GetMode (), txVector.GetPreambleType (), channelNum, channelFreq, channelWidth);
        NotifyRxEndOk ();
        DoSwitchFromRx ();
        if (!m_rxOkCallback.IsNull ())
        {
            m_rxOkCallback (psdu, snr, start, duration, txVector);
        }
    }

    void
    MmWavePhyStateHelper::SwitchFromRxEndError (Ptr<MmWavePsdu> psdu, double snr, Time start, Time duration, MmWaveTxVector txVector)
    {
        NS_LOG_FUNCTION (this << *psdu << snr);
        NS_ASSERT (m_endRx == Simulator::Now ());
        m_rxErrorTrace (psdu->GetPacket (), snr);
        NotifyRxEndError ();
        DoSwitchFromRx ();
        if (!m_rxErrorCallback.IsNull ())
        {
            m_rxErrorCallback (psdu, snr, start, duration, txVector);
        }
    }

    void
    MmWavePhyStateHelper::DoSwitchFromRx ()
    {
        NS_LOG_FUNCTION (this);
        Time now = Simulator::Now ();
        m_stateLogger (m_startRx, now - m_startRx, MmWavePhyState::MMWAVE_RX);
        m_previousStateChangeTime = now;
        m_endRx = Simulator::Now ();
        NS_ASSERT (IsStateIdle () || IsStateCcaBusy ());
    }

    void
    MmWavePhyStateHelper::SwitchMaybeToCcaBusy (Time duration)
    {
        NS_LOG_FUNCTION (this << duration);
        Time now = Simulator::Now ();
        if (GetState () != MmWavePhyState::MMWAVE_RX)
        {
            NotifyMaybeCcaBusyStart (duration);
        }
        switch (GetState ())
        {
            case MmWavePhyState::MMWAVE_IDLE:
                LogPreviousIdleAndCcaBusyStates ();
                break;
            case MmWavePhyState::MMWAVE_RX:
                return;
            default:
                break;
        }
        if (GetState () != MmWavePhyState::MMWAVE_CCA_BUSY)
        {
            m_startCcaBusy = now;
        }
        m_endCcaBusy = std::max (m_endCcaBusy, now + duration);
    }

    void
    MmWavePhyStateHelper::SwitchToSleep ()
    {
        NS_LOG_FUNCTION (this);
        Time now = Simulator::Now ();
        switch (GetState ())
        {
            case MmWavePhyState::MMWAVE_IDLE:
                LogPreviousIdleAndCcaBusyStates ();
                break;
            case MmWavePhyState::MMWAVE_CCA_BUSY:
            {
                Time ccaStart = Max (m_endRx, m_endTx);
                ccaStart = Max (ccaStart, m_startCcaBusy);
                ccaStart = Max (ccaStart, m_endSwitching);
                m_stateLogger (ccaStart, now - ccaStart, MmWavePhyState::MMWAVE_CCA_BUSY);
            } break;
            default:
                NS_FATAL_ERROR ("Invalid MmWavePhy state.");
                break;
        }
        m_previousStateChangeTime = now;
        m_sleeping = true;
        m_startSleep = now;
        NotifySleep ();
        NS_ASSERT (IsStateSleep ());
    }

    void
    MmWavePhyStateHelper::SwitchFromSleep (Time duration)
    {
        NS_LOG_FUNCTION (this << duration);
        NS_ASSERT (IsStateSleep ());
        Time now = Simulator::Now ();
        m_stateLogger (m_startSleep, now - m_startSleep, MmWavePhyState::MMWAVE_SLEEP);
        m_previousStateChangeTime = now;
        m_sleeping = false;
        NotifyWakeup ();
        //update m_endCcaBusy after the sleep period
        m_endCcaBusy = std::max (m_endCcaBusy, now + duration);
        if (m_endCcaBusy > now)
        {
            NotifyMaybeCcaBusyStart (m_endCcaBusy - now);
        }
    }

    void
    MmWavePhyStateHelper::SwitchFromRxAbort ()
    {
        NS_LOG_FUNCTION (this);
        NS_ASSERT (IsStateRx ());
        NotifyRxEndOk ();
        DoSwitchFromRx ();
        m_endCcaBusy = Simulator::Now ();
        NotifyMaybeCcaBusyStart (Seconds (0.0));
        NS_ASSERT (IsStateIdle ());
    }

    void
    MmWavePhyStateHelper::SwitchToOff ()
    {
        NS_LOG_FUNCTION (this);
        Time now = Simulator::Now ();
        switch (GetState ())
        {
            case MmWavePhyState::MMWAVE_RX:
                /* The packet which is being received as well
                 * as its endRx event are cancelled by the caller.
                 */
                m_stateLogger (m_startRx, now - m_startRx, MmWavePhyState::MMWAVE_RX);
                m_endRx = now;
                break;
            case MmWavePhyState::MMWAVE_TX:
                /* The packet which is being transmitted as well
                 * as its endTx event are cancelled by the caller.
                 */
                m_stateLogger (m_startTx, now - m_startTx, MmWavePhyState::MMWAVE_TX);
                m_endTx = now;
                break;
            case MmWavePhyState::MMWAVE_IDLE:
                LogPreviousIdleAndCcaBusyStates ();
                break;
            case MmWavePhyState::MMWAVE_CCA_BUSY:
            {
                Time ccaStart = Max (m_endRx, m_endTx);
                ccaStart = Max (ccaStart, m_startCcaBusy);
                ccaStart = Max (ccaStart, m_endSwitching);
                m_stateLogger (ccaStart, now - ccaStart, MmWavePhyState::MMWAVE_CCA_BUSY);
            } break;
            default:
                NS_FATAL_ERROR ("Invalid MmWavePhy state.");
                break;
        }
        m_previousStateChangeTime = now;
        m_isOff = true;
        NotifyOff ();
        NS_ASSERT (IsStateOff ());
    }

    void
    MmWavePhyStateHelper::SwitchFromOff (Time duration)
    {
        NS_LOG_FUNCTION (this << duration);
        NS_ASSERT (IsStateOff ());
        Time now = Simulator::Now ();
        m_previousStateChangeTime = now;
        m_isOff = false;
        NotifyOn ();
        //update m_endCcaBusy after the off period
        m_endCcaBusy = std::max (m_endCcaBusy, now + duration);
        if (m_endCcaBusy > now)
        {
            NotifyMaybeCcaBusyStart (m_endCcaBusy - now);
        }
    }

} //namespace ns3