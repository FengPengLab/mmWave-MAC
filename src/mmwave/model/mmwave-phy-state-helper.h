/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef V2X_PHY_STATE_HELPER_H
#define V2X_PHY_STATE_HELPER_H
#include <vector>
#include "ns3/fatal-error.h"
#include "ns3/ptr.h"
#include "ns3/callback.h"
#include "ns3/traced-callback.h"
#include "ns3/object.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "mmwave.h"
#include "mmwave-ppdu.h"
#include "mmwave-psdu.h"
#include "mmwave-phy-listener.h"
#include "mmwave-mode.h"
#include "mmwave-tx-vector.h"

namespace ns3 {

    enum MmWavePhyState
    {
        MMWAVE_IDLE,
        MMWAVE_CCA_BUSY,
        MMWAVE_TX,
        MMWAVE_RX,
        MMWAVE_SWITCHING,
        MMWAVE_SLEEP,
        MMWAVE_OFF
    };

    inline std::ostream& operator<< (std::ostream& os, MmWavePhyState state)
    {
        switch (state)
        {
            case MMWAVE_IDLE:
                return (os << "IDLE");
            case MMWAVE_CCA_BUSY:
                return (os << "CCA_BUSY");
            case MMWAVE_TX:
                return (os << "TX");
            case MMWAVE_RX:
                return (os << "RX");
            case MMWAVE_SWITCHING:
                return (os << "SWITCHING");
            case MMWAVE_SLEEP:
                return (os << "SLEEP");
            case MMWAVE_OFF:
                return (os << "OFF");
            default:
                NS_FATAL_ERROR ("Invalid state");
                return (os << "INVALID");
        }
    }

    class MmWavePhyStateHelper : public Object
    {
    public:
        static TypeId GetTypeId ();
        MmWavePhyStateHelper ();

        void SetReceiveOkCallback (Callback<void, Ptr<MmWavePsdu>, double, Time, Time, MmWaveTxVector> callback);
        void SetReceiveErrorCallback (Callback<void, Ptr<MmWavePsdu>, double, Time, Time, MmWaveTxVector> callback);
        void RegisterListener (MmWavePhyListener *listener);
        void UnregisterListener (MmWavePhyListener *listener);
        MmWavePhyState GetState () const;
        bool IsStateCcaBusy () const;
        bool IsStateIdle () const;
        bool IsStateRx () const;
        bool IsStateTx () const;
        bool IsStateSwitching () const;
        bool IsStateSleep () const;
        bool IsStateOff () const;
        Time GetDelayUntilIdle () const;
        Time GetLastRxStartTime () const;
        Time GetLastRxEndTime () const;
        void SwitchToTx (Time txDuration, MmWaveConstPsduMap psdus, double txPowerDbm, MmWaveTxVector txVector, uint8_t channelNum, uint16_t channelFreq, uint16_t channelWidth);
        void SwitchToRx (Time rxDuration);
        void SwitchToChannelSwitching (Time switchingDuration);
        void SwitchFromRxEndOk (Ptr<MmWavePsdu> psdu, double snr, Time start, Time duration, MmWaveTxVector txVector, uint8_t channelNum, uint16_t channelFreq, uint16_t channelWidth);
        void SwitchFromRxEndError (Ptr<MmWavePsdu> psdu, double snr, Time start, Time duration, MmWaveTxVector txVector);
        void SwitchMaybeToCcaBusy (Time duration);
        void SwitchToSleep ();
        void SwitchFromSleep (Time duration);
        void SwitchFromRxAbort ();
        void SwitchToOff ();
        void SwitchFromOff (Time duration);
        typedef void (* StateTracedCallback)(Time start, Time duration, MmWavePhyState state);
        typedef void (* RxOkTracedCallback)(Ptr<const Packet> packet, double snr, MmWaveMode mode, MmWavePhyState preamble);
        typedef void (* RxEndErrorTracedCallback)(Ptr<const Packet> packet, double snr);
        typedef void (* TxTracedCallback)(Ptr<const Packet> packet, MmWaveMode mode, MmWavePhyState preamble, uint8_t power);
        typedef std::vector<MmWavePhyListener *> Listeners;
        typedef std::vector<MmWavePhyListener *>::iterator ListenersI;
    private:

        void LogPreviousIdleAndCcaBusyStates ();
        void NotifyTxStart (Time duration, double txPowerDbm);
        void NotifyRxStart (Time duration);
        void NotifyRxEndOk ();
        void NotifyRxEndError ();
        void NotifyMaybeCcaBusyStart (Time duration);
        void NotifySwitchingStart (Time duration);
        void NotifySleep ();
        void NotifyOff ();
        void NotifyWakeup ();
        void DoSwitchFromRx ();
        void NotifyOn ();

        bool m_sleeping; ///< sleeping
        bool m_isOff; ///< switched off
        Time m_endTx; ///< end transmit
        Time m_endRx; ///< end receive
        Time m_endCcaBusy; ///< end CCA busy
        Time m_endSwitching; ///< end switching
        Time m_startTx; ///< start transmit
        Time m_startRx; ///< start receive
        Time m_startCcaBusy; ///< start CCA busy
        Time m_startSwitching; ///< start switching
        Time m_startSleep; ///< start sleep
        Time m_previousStateChangeTime; ///< previous state change time

        Listeners m_listeners; ///< listeners
        TracedCallback<Time, Time, MmWavePhyState> m_stateLogger;
        TracedCallback<Ptr<const Packet>, double, MmWaveMode, MmWavePreamble, uint8_t, uint16_t, uint16_t> m_rxOkTrace; ///< receive OK trace callback
        TracedCallback<Ptr<const Packet>, double> m_rxErrorTrace; ///< receive error trace callback
        TracedCallback<Ptr<const Packet>, MmWaveMode,MmWavePreamble, uint8_t, uint8_t, uint16_t, uint16_t> m_txTrace; ///< transmit trace callback
        Callback<void, Ptr<MmWavePsdu>, double, Time, Time, MmWaveTxVector> m_rxOkCallback; ///< receive OK callback
        Callback<void, Ptr<MmWavePsdu>, double, Time, Time, MmWaveTxVector> m_rxErrorCallback; ///< receive error callback
    };

} //namespace ns3
#endif //V2X_PHY_STATE_HELPER_H
