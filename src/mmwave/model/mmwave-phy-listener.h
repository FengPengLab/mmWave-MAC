/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_PHY_LISTENER_H
#define MMWAVE_PHY_LISTENER_H
namespace ns3 {
    class Time;

    class MmWavePhyListener
    {
    public:
        virtual ~MmWavePhyListener ()
        {
        }

        virtual void NotifyRxStart (Time duration) = 0;
        virtual void NotifyRxEndOk (void) = 0;
        virtual void NotifyRxEndError (void) = 0;
        virtual void NotifyTxStart (Time duration, double txPowerDbm) = 0;
        virtual void NotifyMaybeCcaBusyStart (Time duration) = 0;
        virtual void NotifySwitchingStart (Time duration) = 0;
        virtual void NotifySleep (void) = 0;
        virtual void NotifyOff (void) = 0;
        virtual void NotifyWakeup (void) = 0;
        virtual void NotifyOn (void) = 0;
    };

} //namespace ns3
#endif //MMWAVE_PHY_LISTENER_H
