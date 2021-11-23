/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_DYNAMIC_CHANNEL_ACCESS_MANAGER_H
#define MMWAVE_DYNAMIC_CHANNEL_ACCESS_MANAGER_H
#include "ns3/object.h"
#include "ns3/vector.h"
#include "ns3/simulator.h"
#include "mmwave.h"
#include "cr-txop.h"
#include "mmwave-spectrum-repository.h"
namespace ns3 {
    class MmWavePhy;
    class CrMmWaveMac;
    class CrMmWaveMacLow;
    class CrDynamicChannelAccessListener;

    class CrDynamicChannelAccessManager : public Object
    {
    public:
        static TypeId GetTypeId ();
        CrDynamicChannelAccessManager ();
        ~CrDynamicChannelAccessManager ();
        virtual void DoDispose ();
        virtual void DoInitialize ();
        void SetupMacLow (Ptr<CrMmWaveMacLow> low);
        void SetupSpectrumRepository (Ptr<MmWaveSpectrumRepository> repos);
        void SetupChannelAccessListener (Ptr<MmWavePhy> phy);
        void RemoveChannelAccessListener (Ptr<MmWavePhy> phy);
        void NotifyRxStartNow (Time duration);
        void NotifyRxEndOkNow ();
        void NotifyRxEndErrorNow ();
        void NotifyTxStartNow (Time duration);
        void NotifyMaybeCcaBusyStartNow (Time duration);
        void NotifySwitchingStartNow (Time duration);
        void NotifySleepNow ();
        void NotifyOffNow ();
        void NotifyWakeupNow ();
        void NotifyOnNow ();
        void NotifyNavResetNow (Time duration);
        void NotifyNavStartNow (Time duration);
        void NotifyAckTimeoutStartNow (Time duration);
        void NotifyAckTimeoutResetNow ();
        void NotifyCtsTimeoutStartNow (Time duration);
        void NotifyCtsTimeoutResetNow ();
        void NotifyBulkTimeoutStartNow (Time duration);
        void NotifyBulkTimeoutResetNow ();
        void CancelRequestAccess ();
        void StartRequestAccess (TypeOfAccessMode typeOfAccess);
        void RequestAccess (TypeOfAccessMode typeOfAccess);
        void RequestAccessCallback (TypeOfAccessMode typeOfAccess);
        void ResetRotationFactor ();
        void UpdateRotationFactor ();
        bool IsBusy () const;
        bool IsRequestAccess ();
        static Time MostRecent (std::initializer_list<Time> list);
        Time GetAccessGrantStart (bool ignoreNav = false) const;
        Time GetBeifs ();
        Time GetBuifs ();

        EventId m_requestAccess;
        Time m_lastAckTimeoutEnd;     //!< the last Ack timeout end time
        Time m_lastCtsTimeoutEnd;     //!< the last CTS timeout end time
        Time m_lastBulkTimeoutEnd;    //!< the last bulk access request timeout end time
        Time m_lastNavStart;          //!< the last NAV start time
        Time m_lastNavDuration;       //!< the last NAV duration time
        bool m_lastRxReceivedOk;      //!< the last receive OK
        Time m_lastRxStart;           //!< the last receive start time
        Time m_lastRxDuration;        //!< the last receive duration time
        Time m_lastTxStart;           //!< the last transmit start time
        Time m_lastTxDuration;        //!< the last transmit duration time
        Time m_lastBusyStart;         //!< the last busy start time
        Time m_lastBusyDuration;      //!< the last busy duration time
        Time m_lastSwitchingStart;    //!< the last switching start time
        Time m_lastSwitchingDuration; //!< the last switching duration time
        bool m_sleeping;              //!< flag whether it is in sleeping state
        bool m_off;                   //!< flag whether it is in off state
        CrDynamicChannelAccessListener* m_channelAccessListener;
        TypeOfAccessMode m_typeOfAccessMode;
        TypeOfGroup m_typeOfGroup;
        Ptr<MmWaveSpectrumRepository> m_repos;
        Ptr<MmWavePhy> m_phy;
        Ptr<CrMmWaveMacLow> m_low;
        uint32_t m_gamma_init;
        uint32_t m_gamma;
        uint32_t m_k;
    };
}
#endif //CR_CHANNEL_ACCESS_MANAGER_H
