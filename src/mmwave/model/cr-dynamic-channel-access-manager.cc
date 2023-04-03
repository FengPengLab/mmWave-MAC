/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
#include <algorithm>
#include <cmath>
#include <utility>
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "mmwave-phy-listener.h"
#include "mmwave-spectrum-repository.h"
#include "mmwave-phy-state-helper.h"
#include "cr-mac.h"
#include "cr-mac-low.h"
#include "cr-dynamic-channel-access-manager.h"

#undef NS_LOG_APPEND_CONTEXT
#define NS_LOG_APPEND_CONTEXT std::clog << "[" << m_typeOfGroup << "] "

namespace ns3 {
    NS_LOG_COMPONENT_DEFINE ("CrDynamicChannelAccessManager");
    NS_OBJECT_ENSURE_REGISTERED (CrDynamicChannelAccessManager);

    class CrDynamicChannelAccessListener : public ns3::MmWavePhyListener
    {
    public:
        CrDynamicChannelAccessListener (ns3::CrDynamicChannelAccessManager *cam)
                : m_cam (cam)
        {
        }
        virtual ~CrDynamicChannelAccessListener ()
        {
        }
        void NotifyRxStart (Time duration)
        {
            m_cam->NotifyRxStartNow (duration);
        }
        void NotifyRxEndOk ()
        {
            m_cam->NotifyRxEndOkNow ();
        }
        void NotifyRxEndError ()
        {
            m_cam->NotifyRxEndErrorNow ();
        }
        void NotifyTxStart (Time duration, double txPowerDbm)
        {
            m_cam->NotifyTxStartNow (duration);
        }
        void NotifyMaybeCcaBusyStart (Time duration)
        {
            m_cam->NotifyMaybeCcaBusyStartNow (duration);
        }
        void NotifySwitchingStart (Time duration)
        {
            m_cam->NotifySwitchingStartNow (duration);
        }
        void NotifySleep ()
        {
            m_cam->NotifySleepNow ();
        }
        void NotifyOff ()
        {
            m_cam->NotifyOffNow ();
        }
        void NotifyWakeup ()
        {
            m_cam->NotifyWakeupNow ();
        }
        void NotifyOn ()
        {
            m_cam->NotifyOnNow ();
        }

    private:
        ns3::CrDynamicChannelAccessManager *m_cam;
    };

    CrDynamicChannelAccessManager::CrDynamicChannelAccessManager ()
            : m_lastAckTimeoutEnd (Seconds (0.0)),
              m_lastCtsTimeoutEnd (Seconds (0.0)),
              m_lastBulkTimeoutEnd (Seconds (0.0)),
              m_lastNavStart (Seconds (0.0)),
              m_lastNavDuration (Seconds (0.0)),
              m_lastRxReceivedOk (true),
              m_lastRxStart (Seconds (0.0)),
              m_lastRxDuration (Seconds (0.0)),
              m_lastTxStart (Seconds (0.0)),
              m_lastTxDuration (Seconds (0.0)),
              m_lastBusyStart (Seconds (0.0)),
              m_lastBusyDuration (Seconds (0.0)),
              m_lastSwitchingStart (Seconds (0.0)),
              m_lastSwitchingDuration (Seconds (0.0)),
              m_sleeping (false),
              m_off (false),
              m_channelAccessListener (0),
              m_gamma_init (0),
              m_gamma (0),
              m_k (8)
    {
        NS_LOG_FUNCTION (this);
    }

    CrDynamicChannelAccessManager::~CrDynamicChannelAccessManager ()
    {
        delete m_channelAccessListener;
        m_channelAccessListener = 0;
        m_requestAccess.Cancel ();
    }

    TypeId
    CrDynamicChannelAccessManager::GetTypeId ()
    {
        static TypeId tid = TypeId("ns3::CrDynamicChannelAccessManager")
                .SetParent<Object>()
                .SetGroupName("MmWave")
                .AddConstructor<CrDynamicChannelAccessManager> ();
        return tid;
    }

    void
    CrDynamicChannelAccessManager::DoInitialize ()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    CrDynamicChannelAccessManager::DoDispose ()
    {
        m_phy = 0;
    }

    void
    CrDynamicChannelAccessManager::SetupSpectrumRepository (Ptr<MmWaveSpectrumRepository> repos)
    {
        m_repos = repos;
    }

    void
    CrDynamicChannelAccessManager::SetupChannelAccessListener (Ptr<MmWavePhy> phy)
    {
        NS_ASSERT (m_channelAccessListener == 0);
        m_channelAccessListener = new CrDynamicChannelAccessListener (this);
        phy->RegisterListener (m_channelAccessListener);
        m_phy = phy;
    }

    void
    CrDynamicChannelAccessManager::RemoveChannelAccessListener (Ptr<MmWavePhy> phy)
    {
        if (m_channelAccessListener != 0)
        {
            phy->UnregisterListener (m_channelAccessListener);
            delete m_channelAccessListener;
            m_channelAccessListener = 0;
            m_phy = 0;
        }
    }

    void
    CrDynamicChannelAccessManager::SetupMacLow (Ptr<CrMmWaveMacLow> low)
    {
        low->SetChannelAccessManager (this);
        m_low = low;
        m_typeOfGroup = m_low->GetTypeOfGroup ();
    }

    Time
    CrDynamicChannelAccessManager::MostRecent (std::initializer_list<Time> list)
    {
        NS_ASSERT (list.size () > 0);
        return *std::max_element (list.begin (), list.end ());
    }

    bool
    CrDynamicChannelAccessManager::IsBusy () const
    {
        // PHY busy
        Time lastRxEnd = m_lastRxStart + m_lastRxDuration;
        if (lastRxEnd > Simulator::Now ())
        {
            return true;
        }
        Time lastTxEnd = m_lastTxStart + m_lastTxDuration;
        if (lastTxEnd > Simulator::Now ())
        {
            return true;
        }
        // NAV busy
        Time lastNavEnd = m_lastNavStart + m_lastNavDuration;
        if (lastNavEnd > Simulator::Now ())
        {
            return true;
        }
        // CCA busy
        Time lastCCABusyEnd = m_lastBusyStart + m_lastBusyDuration;
        if (lastCCABusyEnd > Simulator::Now ())
        {
            return true;
        }

        Time lastAckTimeoutEnd = m_lastAckTimeoutEnd;
        if (lastAckTimeoutEnd > Simulator::Now ())
        {
            return true;
        }

        Time lastCtsTimeoutEnd = m_lastCtsTimeoutEnd;
        if (lastCtsTimeoutEnd > Simulator::Now ())
        {
            return true;
        }

        Time lastBulkTimeoutEnd = m_lastBulkTimeoutEnd;
        if (lastBulkTimeoutEnd > Simulator::Now ())
        {
            return true;
        }

        Time lastSwitchingEnd = m_lastSwitchingStart + m_lastSwitchingDuration;
        if (lastSwitchingEnd > Simulator::Now ())
        {
            return true;
        }
        return false;
    }

    Time
    CrDynamicChannelAccessManager::GetAccessGrantStart (bool ignoreNav) const
    {
        Time lastRxEnd = m_lastRxStart + m_lastRxDuration;
        Time rxAccessStart = lastRxEnd + m_phy->GetSifs ();
        if ((lastRxEnd <= Simulator::Now ()) && !m_lastRxReceivedOk)
        {
            rxAccessStart += m_phy->GetSifs ();
        }
        Time busyAccessStart = m_lastBusyStart + m_lastBusyDuration + m_phy->GetSifs ();
        Time txAccessStart = m_lastTxStart + m_lastTxDuration + m_phy->GetSifs ();
        Time navAccessStart = m_lastNavStart + m_lastNavDuration + m_phy->GetSifs ();
        Time ackTimeoutAccessStart = m_lastAckTimeoutEnd + m_phy->GetSifs ();
        Time ctsTimeoutAccessStart = m_lastCtsTimeoutEnd + m_phy->GetSifs ();
        Time bulkTimeoutAccessStart = m_lastBulkTimeoutEnd + m_phy->GetSifs ();
        Time switchingAccessStart = m_lastSwitchingStart + m_lastSwitchingDuration + m_phy->GetSifs ();
        Time accessGrantedStart;
        if (ignoreNav)
        {
            accessGrantedStart = MostRecent ({rxAccessStart,
                                              busyAccessStart,
                                              txAccessStart,
                                              ackTimeoutAccessStart,
                                              ctsTimeoutAccessStart,
                                              bulkTimeoutAccessStart,
                                              switchingAccessStart});
        }
        else
        {
            accessGrantedStart = MostRecent ({rxAccessStart,
                                              busyAccessStart,
                                              txAccessStart,
                                              navAccessStart,
                                              ackTimeoutAccessStart,
                                              ctsTimeoutAccessStart,
                                              bulkTimeoutAccessStart,
                                              switchingAccessStart});
        }
        return accessGrantedStart;
    }

    void
    CrDynamicChannelAccessManager::NotifyRxStartNow (Time duration)
    {
        m_lastRxStart = Simulator::Now ();
        m_lastRxDuration = duration;
        m_lastRxReceivedOk = true;

        if (m_requestAccess.IsRunning ())
        {
            m_requestAccess.Cancel ();
            RequestAccess (m_typeOfAccessMode);
        }
    }

    void
    CrDynamicChannelAccessManager::NotifyRxEndOkNow ()
    {
        m_lastRxDuration = Simulator::Now () - m_lastRxStart;
        m_lastRxReceivedOk = true;
        if (m_requestAccess.IsRunning ())
        {
            m_requestAccess.Cancel ();
            RequestAccess (m_typeOfAccessMode);
        }
    }

    void
    CrDynamicChannelAccessManager::NotifyRxEndErrorNow ()
    {
        Time now = Simulator::Now ();
        Time lastRxEnd = m_lastRxStart + m_lastRxDuration;
        if (lastRxEnd > now)
        {
            m_lastBusyStart = now;
            m_lastBusyDuration = lastRxEnd - m_lastBusyStart;
        }
        m_lastRxDuration = now - m_lastRxStart;
        m_lastRxReceivedOk = false;

        if (m_requestAccess.IsRunning ())
        {
            m_requestAccess.Cancel ();
            RequestAccess (m_typeOfAccessMode);
        }
    }

    void
    CrDynamicChannelAccessManager::NotifyTxStartNow (Time duration)
    {
        m_lastRxReceivedOk = true;
        Time now = Simulator::Now ();
        Time lastRxEnd = m_lastRxStart + m_lastRxDuration;
        if (lastRxEnd > now)
        {
            //this may be caused only if PHY has started to receive a packet
            //inside SIFS, so, we check that lastRxStart was maximum a SIFS ago
            NS_ASSERT (now - m_lastRxStart <= m_phy->GetSifs ());
            m_lastRxDuration = now - m_lastRxStart;
        }
        m_lastTxStart = now;
        m_lastTxDuration = duration;

        if (m_requestAccess.IsRunning ())
        {
            m_requestAccess.Cancel ();
            RequestAccess (m_typeOfAccessMode);
        }
    }

    void
    CrDynamicChannelAccessManager::NotifyMaybeCcaBusyStartNow (Time duration)
    {
        m_lastBusyStart = Simulator::Now ();
        m_lastBusyDuration = duration;

        if (m_requestAccess.IsRunning ())
        {
            m_requestAccess.Cancel ();
            RequestAccess (m_typeOfAccessMode);
        }
    }

    void
    CrDynamicChannelAccessManager::NotifySwitchingStartNow (Time duration)
    {
        Time now = Simulator::Now ();
        NS_ASSERT (m_lastTxStart + m_lastTxDuration <= now);
        NS_ASSERT (m_lastSwitchingStart + m_lastSwitchingDuration <= now);

        m_lastRxReceivedOk = true;

        if (m_lastRxStart + m_lastRxDuration > now)
        {
            m_lastRxDuration = now - m_lastRxStart;
        }
        if (m_lastNavStart + m_lastNavDuration > now)
        {
            m_lastNavDuration = now - m_lastNavStart;
        }
        if (m_lastBusyStart + m_lastBusyDuration > now)
        {
            m_lastBusyDuration = now - m_lastBusyStart;
        }
        if (m_lastAckTimeoutEnd > now)
        {
            m_lastAckTimeoutEnd = now;
        }
        if (m_lastCtsTimeoutEnd > now)
        {
            m_lastCtsTimeoutEnd = now;
        }
        if (m_lastBulkTimeoutEnd > now)
        {
            m_lastBulkTimeoutEnd = now;
        }

        m_lastSwitchingStart = Simulator::Now ();
        m_lastSwitchingDuration = duration;

        CancelRequestAccess ();
    }

    void
    CrDynamicChannelAccessManager::NotifySleepNow ()
    {
        m_sleeping = true;
        CancelRequestAccess ();
    }

    void
    CrDynamicChannelAccessManager::NotifyOffNow ()
    {
        m_off = true;
        CancelRequestAccess ();
    }

    void
    CrDynamicChannelAccessManager::NotifyWakeupNow ()
    {
        m_sleeping = false;
        CancelRequestAccess ();
    }

    void
    CrDynamicChannelAccessManager::NotifyOnNow ()
    {
        m_off = false;
        CancelRequestAccess ();
    }

    void
    CrDynamicChannelAccessManager::NotifyNavResetNow (Time duration)
    {
        m_lastNavStart = Simulator::Now ();
        m_lastNavDuration = duration;

        if (m_requestAccess.IsRunning ())
        {
            m_requestAccess.Cancel ();
            RequestAccess (m_typeOfAccessMode);
        }
    }

    void
    CrDynamicChannelAccessManager::NotifyNavStartNow (Time duration)
    {
        NS_ASSERT (m_lastNavStart <= Simulator::Now ());
        Time newNavEnd = Simulator::Now () + duration;
        Time lastNavEnd = m_lastNavStart + m_lastNavDuration;
        if (newNavEnd > lastNavEnd)
        {
            m_lastNavStart = Simulator::Now ();
            m_lastNavDuration = duration;

            if (m_requestAccess.IsRunning ())
            {
                m_requestAccess.Cancel ();
                RequestAccess (m_typeOfAccessMode);
            }
        }
    }

    void
    CrDynamicChannelAccessManager::NotifyAckTimeoutStartNow (Time duration)
    {
        NS_ASSERT (m_lastAckTimeoutEnd < Simulator::Now ());
        m_lastAckTimeoutEnd = Simulator::Now () + duration;

        if (m_requestAccess.IsRunning ())
        {
            m_requestAccess.Cancel ();
            RequestAccess (m_typeOfAccessMode);
        }
    }

    void
    CrDynamicChannelAccessManager::NotifyAckTimeoutResetNow ()
    {
        m_lastAckTimeoutEnd = Simulator::Now ();
    }

    void
    CrDynamicChannelAccessManager::NotifyCtsTimeoutStartNow (Time duration)
    {
        m_lastCtsTimeoutEnd = Simulator::Now () + duration;

        if (m_requestAccess.IsRunning ())
        {
            m_requestAccess.Cancel ();
            RequestAccess (m_typeOfAccessMode);
        }
    }

    void
    CrDynamicChannelAccessManager::NotifyCtsTimeoutResetNow ()
    {
        m_lastCtsTimeoutEnd = Simulator::Now ();
    }

    void
    CrDynamicChannelAccessManager::NotifyBulkTimeoutStartNow (Time duration)
    {
        m_lastBulkTimeoutEnd = Simulator::Now () + duration;

        if (m_requestAccess.IsRunning ())
        {
            m_requestAccess.Cancel ();
            RequestAccess (m_typeOfAccessMode);
        }
    }

    void
    CrDynamicChannelAccessManager::NotifyBulkTimeoutResetNow ()
    {
        m_lastBulkTimeoutEnd = Simulator::Now ();
    }

    bool
    CrDynamicChannelAccessManager::IsRequestAccess ()
    {
        return m_requestAccess.IsRunning ();
    }

    void
    CrDynamicChannelAccessManager::CancelRequestAccess ()
    {
        if (m_requestAccess.IsRunning ())
        {
            m_requestAccess.Cancel ();
        }
    }

    void
    CrDynamicChannelAccessManager::StartRequestAccess (TypeOfAccessMode typeOfAccess)
    {
        if (!m_requestAccess.IsRunning ())
        {
            ResetRotationFactor ();
        }
        RequestAccess (typeOfAccess);
    }

    void
    CrDynamicChannelAccessManager::RequestAccess (TypeOfAccessMode typeOfAccess)
    {
        Time delay = Seconds (0.0);
        Time duration = GetAccessGrantStart () - Simulator::Now ();

        if (m_requestAccess.IsRunning ())
        {
            switch (m_typeOfAccessMode)
            {
                case BULK_ACCESS:
                    if (typeOfAccess == BEACON_ACCESS || typeOfAccess == DETECTION_ACCESS)
                    {
                        m_typeOfAccessMode = typeOfAccess;
                        delay = Simulator::GetDelayLeft (m_requestAccess);
                        m_requestAccess.Cancel ();
                        m_requestAccess = Simulator::Schedule (delay, &CrDynamicChannelAccessManager::RequestAccessCallback, this, m_typeOfAccessMode);
                    }
                    break;
                case BEACON_ACCESS:
                    if (typeOfAccess == DETECTION_ACCESS)
                    {
                        m_typeOfAccessMode = typeOfAccess;
                        delay = Simulator::GetDelayLeft (m_requestAccess);
                        m_requestAccess.Cancel ();
                        m_requestAccess = Simulator::Schedule (delay, &CrDynamicChannelAccessManager::RequestAccessCallback, this, m_typeOfAccessMode);
                    }
                    break;
                case DETECTION_ACCESS:
                default:
                    break;
            }
        }
        else
        {
            if (duration.IsStrictlyPositive ())
            {
                delay += duration;
            }
            switch (typeOfAccess)
            {
                case DETECTION_ACCESS:
                case BEACON_ACCESS:
                    delay += GetBeifs ();
                    break;
                case BULK_ACCESS:
                    delay += GetBuifs ();
                    break;
                default:
                    NS_FATAL_ERROR("TypeOfAccess is error");
                    break;
            }
            m_typeOfAccessMode = typeOfAccess;
            m_requestAccess = Simulator::Schedule (delay, &CrDynamicChannelAccessManager::RequestAccessCallback, this, m_typeOfAccessMode);
        }
    }

    void
    CrDynamicChannelAccessManager::RequestAccessCallback (TypeOfAccessMode typeOfAccess)
    {
        if (IsBusy ())
        {
            RequestAccess (m_typeOfAccessMode);
        }
        else
        {
            NS_ASSERT (m_phy->IsStateIdle ());
            switch (typeOfAccess)
            {
                case DETECTION_ACCESS:
                    m_low->SendDetectionRequest ();
                    break;
                case BEACON_ACCESS:
                    m_low->SendBeacon ();
                    break;
                case BULK_ACCESS:
                    m_low->SendBulkAccessRequest ();
                    break;
                default:
                    NS_FATAL_ERROR("TypeOfAccess is error");
                    break;
            }
        }
    }

    void
    CrDynamicChannelAccessManager::ResetRotationFactor ()
    {
        Ptr<CrMmWaveMac> mac = DynamicCast<CrMmWaveMac> (m_low->GetMac ());
        MmWaveNeighborDevices devices = mac->GetAllNeighborDevices ();
        uint32_t neighor = 0;
        for (auto & i : devices)
        {
            if (i.second->m_channel == m_low->GetCurrentChannel ())
            {
                neighor++;
            }
        }

        m_gamma_init = neighor;
        m_gamma = neighor;
    }

    void
    CrDynamicChannelAccessManager::UpdateRotationFactor ()
    {
        if (IsRequestAccess ())
        {
            Ptr<CrMmWaveMac> mac = DynamicCast<CrMmWaveMac> (m_low->GetMac ());
            MmWaveNeighborDevices devices = mac->GetAllNeighborDevices ();
            uint32_t neighor = 0;
            for (auto & i : devices)
            {
                if (i.second->m_channel == m_low->GetCurrentChannel ())
                {
                    neighor++;
                }
            }

            if (m_gamma_init == neighor)
            {
                if (m_gamma > 1)
                {
                    m_gamma--;
                }
            }
            else
            {
                m_gamma_init = neighor;
                m_gamma = neighor;
            }
        }
    }

    Time
    CrDynamicChannelAccessManager::GetBuifs ()
    {
        uint32_t minCw = std::ceil((std::pow(2, 7) * m_gamma) / m_k);
        uint32_t maxCw = std::ceil((std::pow(2, 7) * (m_gamma + 1)) / m_k);
        uint32_t rng = m_low->GetRandomInteger (minCw, maxCw);
        Time duration = m_phy->GetSifs () + (rng * m_phy->GetSlot ());
        NS_LOG_DEBUG ("total=" << std::pow(2, 7) << ", minCw=" << minCw << ", maxCw=" << maxCw << ", rng=" << rng << ", m_gamma=" << m_gamma << ", BUIFS="<< duration.ToDouble (Time::NS) << "ns");
        return duration;
    }

    Time
    CrDynamicChannelAccessManager::GetBeifs ()
    {
        uint32_t minCw = std::ceil((std::pow(2, 6) * m_gamma) / m_k);
        uint32_t maxCw = std::ceil((std::pow(2, 6) * (m_gamma + 1)) / m_k);
        uint32_t rng = m_low->GetRandomInteger (minCw, maxCw);
        Time duration = m_phy->GetSifs () + (rng * m_phy->GetSlot ());
        NS_LOG_DEBUG ("total=" << std::pow(2, 6) << ", minCw=" << minCw << ", maxCw=" << maxCw << ", rng=" << rng << ", m_gamma=" << m_gamma << ", BEIFS="<< duration.ToDouble (Time::NS) << "ns");
        return duration;
    }
} //namespace ns3