/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include <cmath>
#include "ns3/log.h"
#include "ns3/type-id.h"
#include "ns3/vector.h"
#include "ns3/double.h"
#include "ns3/integer.h"
#include "ns3/node.h"
#include "ns3/object.h"
#include "ns3/mobility-model.h"
#include "ns3/net-device.h"
#include "ns3/simulator.h"
#include "mmwave-spectrum-repository.h"
namespace ns3 {
    NS_LOG_COMPONENT_DEFINE ("MmWaveSpectrumRepository");
    NS_OBJECT_ENSURE_REGISTERED (MmWaveSpectrumRepository);

    MmWaveSpectrumData::MmWaveSpectrumData ()
            : m_power (0),
              m_snr (0),
              m_state (UNUTILIZED),
              m_occupiedDuration (Seconds (0.0)),
              m_relativeStart (Seconds (0.0)),
              m_detectionStart (Seconds (0.0)),
              m_detectionEnd (Seconds (0.0)),
              m_updateTime (Seconds (0.0))
    {
        NS_LOG_FUNCTION (this);
    }

    MmWaveSpectrumData::~MmWaveSpectrumData ()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    MmWaveSpectrumData::SetPower (double power)
    {
        m_power = power;
    }

    double
    MmWaveSpectrumData::GetPower () const
    {
        return m_power;
    }

    void
    MmWaveSpectrumData::SetSnr (double snr)
    {
        m_snr = snr;
    }

    double
    MmWaveSpectrumData::GetSnr () const
    {
        return m_snr;
    }

    void
    MmWaveSpectrumData::SetOccupiedDuration (Time time)
    {
        m_occupiedDuration = time;
    }

    void
    MmWaveSpectrumData::SetRelativeStart (Time time)
    {
        m_relativeStart = time;
    }

    void
    MmWaveSpectrumData::SetDetectionStart (Time time)
    {
        m_detectionStart = time;
    }

    void
    MmWaveSpectrumData::SetDetectionEnd (Time time)
    {
        m_detectionEnd = time;
    }

    void
    MmWaveSpectrumData::SetChannelState (MmWaveChannelState utilization)
    {
        m_state = utilization;
    }

    void
    MmWaveSpectrumData::SetPosition (Vector position)
    {
        m_position = position;
    }

    void
    MmWaveSpectrumData::SetUpdateTime (Time updateTime)
    {
        m_updateTime = updateTime;
    }

    Time MmWaveSpectrumData::GetDetectionStart () const
    {
        return m_detectionStart;
    }

    Time MmWaveSpectrumData::GetDetectionEnd () const
    {
        return m_detectionEnd;
    }

    Time
    MmWaveSpectrumData::GetRelativeStart () const
    {
        return m_relativeStart;
    }

    Time
    MmWaveSpectrumData::GetOccupiedDuration () const
    {
        return m_occupiedDuration;
    }

    Time
    MmWaveSpectrumData::GetUpdateTime () const
    {
        return m_updateTime;
    }

    Vector
    MmWaveSpectrumData::GetPosition () const
    {
        return m_position;
    }

    MmWaveChannelState
    MmWaveSpectrumData::GetChannelState () const
    {
        return m_state;
    }

    MmWaveSpectrumStatistical::MmWaveSpectrumStatistical ()
            : m_isValid (false),
              m_isAnyPUs (false),
              m_isAnySUs (false),
              m_avgPower (0),
              m_utilizationRateForPUs (0),
              m_utilizationRateForSUs (0),
              m_vacancyRate (1)
    {
        NS_LOG_FUNCTION (this);
    }

    MmWaveSpectrumStatistical::~MmWaveSpectrumStatistical ()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    MmWaveSpectrumStatistical::SetValid (bool valid)
    {
        m_isValid = valid;
    }

    bool
    MmWaveSpectrumStatistical::IsValid () const
    {
        return m_isValid;
    }

    void
    MmWaveSpectrumStatistical::SetPUs (bool isAny)
    {
        m_isAnyPUs = isAny;
    }

    bool
    MmWaveSpectrumStatistical::IsAnyPUs () const
    {
        return m_isAnyPUs;
    }

    void
    MmWaveSpectrumStatistical::SetSUs (bool isAny)
    {
        m_isAnySUs = isAny;
    }

    bool
    MmWaveSpectrumStatistical::IsAnySUs () const
    {
        return m_isAnySUs;
    }

    void
    MmWaveSpectrumStatistical::SetAvgPower (double power)
    {
        m_avgPower = power;
    }

    double
    MmWaveSpectrumStatistical::GetAvgPower () const
    {
        return m_avgPower;
    }

    void
    MmWaveSpectrumStatistical::SetUtilizationForPUs (double utilization)
    {
        m_utilizationRateForPUs = utilization;
    }

    double
    MmWaveSpectrumStatistical::GetUtilizationForPUs () const
    {
        return m_utilizationRateForPUs;
    }

    void
    MmWaveSpectrumStatistical::SetUtilizationForSUs (double utilization)
    {
        m_utilizationRateForSUs = utilization;
    }

    double
    MmWaveSpectrumStatistical::GetUtilizationForSUs () const
    {
        return m_utilizationRateForSUs;
    }

    void
    MmWaveSpectrumStatistical::SetVacancyRate (double rate)
    {
        m_vacancyRate = rate;
    }

    double
    MmWaveSpectrumStatistical::GetVacancyRate ()
    {
        return m_vacancyRate;
    }

    MmWaveSpectrumRepository::MmWaveSpectrumRepository ()
            : m_deltaNum (2),
              m_configure (false),
              m_channelToFrequencyWidthMapInitialized (false),
              m_isEachChannelDetected (false)
    {
        NS_LOG_FUNCTION (this);
    }

    MmWaveSpectrumRepository::~MmWaveSpectrumRepository ()
    {
        NS_LOG_FUNCTION (this);
    }

    TypeId
    MmWaveSpectrumRepository::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::MmWaveSpectrumRepository")
                .SetParent<Object> ()
                .SetGroupName ("MmWave")
                .AddConstructor<MmWaveSpectrumRepository> ()
                .AddAttribute ("MaxRadius", "The max radius size",
                               DoubleValue (100),
                               MakeDoubleAccessor (&MmWaveSpectrumRepository::SetMaxRadius,
                                                   &MmWaveSpectrumRepository::GetMaxRadius),
                               MakeDoubleChecker<double> ())
                .AddAttribute ("MaxSize", "The max buffer size",
                               IntegerValue (5000),
                               MakeIntegerAccessor (&MmWaveSpectrumRepository::SetMaxSize,
                                                    &MmWaveSpectrumRepository::GetMaxSize),
                               MakeIntegerChecker<int> ())
                .AddAttribute ("MaxDelay", "If a data stays longer than this delay in the buffer, it is dropped.",
                               TimeValue (Seconds (10.0)),
                               MakeTimeAccessor (&MmWaveSpectrumRepository::SetMaxDelay,
                                                 &MmWaveSpectrumRepository::GetMaxDelay),
                               MakeTimeChecker ())
                .AddAttribute ("ThresholdForPUsActivity",
                               "If the occupancy rate of the unrecognized signal larger than this value, PU is considered to be active.",
                               DoubleValue (0.1),
                               MakeDoubleAccessor (&MmWaveSpectrumRepository::m_thresholdForPUsActivity),
                               MakeDoubleChecker<double> (0, 1))
                .AddAttribute ("ThresholdForSUsActivity",
                               "If the occupancy rate of the recognizable signal larger than this value, SU is considered to be active.",
                               DoubleValue (0.1),
                               MakeDoubleAccessor (&MmWaveSpectrumRepository::m_thresholdForSUsActivity),
                               MakeDoubleChecker<double> (0, 1));
        return tid;
    }

    void
    MmWaveSpectrumRepository::DoDispose ()
    {
        NS_LOG_FUNCTION (this);
        m_spectrumDataInfo.clear ();
        m_statisticalInfo.clear ();
    }

    void
    MmWaveSpectrumRepository::DoInitialize ()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    MmWaveSpectrumRepository::SetMaxDelay (Time delay)
    {
        m_maxDelay = delay;
    }

    Time
    MmWaveSpectrumRepository::GetMaxDelay () const
    {
        return m_maxDelay;
    }

    void
    MmWaveSpectrumRepository::SetMaxRadius (double radius)
    {
        m_maxRadius = radius;
    }

    double
    MmWaveSpectrumRepository::GetMaxRadius () const
    {
        return m_maxRadius;
    }

    void
    MmWaveSpectrumRepository::SetMaxSize (int size)
    {
        m_maxSize = size;
    }

    int
    MmWaveSpectrumRepository::GetMaxSize () const
    {
        return m_maxSize;
    }

    void MmWaveSpectrumRepository::SetNetDevice (Ptr<NetDevice> device)
    {
        m_device = device;
    }

    Ptr<NetDevice>
    MmWaveSpectrumRepository::GetNetDevice () const
    {
        return m_device;
    }

    bool
    MmWaveSpectrumRepository::IsEachChannelDetected (Vector position)
    {
        if (!m_isEachChannelDetected)
        {
            UpdateStatisticalInfo (position);
            for (auto & i : m_channelToFrequency)
            {
                if (!m_statisticalInfo[i.first]->IsValid ())
                {
                    return false;
                }
            }
            m_isEachChannelDetected = true;
        }
        return true;
    }

    void
    MmWaveSpectrumRepository::AddActivityInfoToRepository (MmWaveChannelNumberStandardPair channel, Ptr<MmWaveSpectrumData> data)
    {
        m_spectrumDataInfo[channel].push_back (data);
        CheckSizeExceeded ();
    }

    void
    MmWaveSpectrumRepository::CheckSizeExceeded ()
    {
        std::vector<Ptr<MmWaveSpectrumData>>::iterator j;
        for (auto & i : m_spectrumDataInfo)
        {
            j = i.second.begin ();
            while (static_cast<int> (i.second.size ()) > GetMaxSize ())
            {
                i.second.erase (j++);
            }
        }
    }

    void
    MmWaveSpectrumRepository::CheckTtlExceeded ()
    {
        std::vector<Ptr<MmWaveSpectrumData>>::iterator j;
        for (auto & i : m_spectrumDataInfo)
        {
            j = i.second.begin ();
            while (j != i.second.end ())
            {
                if (Simulator::Now () > (*j)->GetUpdateTime () + GetMaxDelay ())
                {
                    i.second.erase (j++);
                }
                else
                {
                    j++;
                }
            }
        }
    }

    void
    MmWaveSpectrumRepository::CheckRadiusExceeded (Vector aPos)
    {
        std::vector<Ptr<MmWaveSpectrumData>>::iterator j;
        Vector bPos;
        double MaxRadius = std::pow (GetMaxRadius (), 2);
        double d;
        for (auto & i : m_spectrumDataInfo)
        {
            j = i.second.begin ();
            while (j != i.second.end ())
            {
                bPos = (*j)->GetPosition ();
                d = std::pow (aPos.x - bPos.x, 2)
                    + std::pow (aPos.y - bPos.y, 2)
                    + std::pow (aPos.z - bPos.z, 2);

                if (d > MaxRadius)
                {
                    i.second.erase (j++);
                }
                else
                {
                    j++;
                }
            }
        }
    }

    void
    MmWaveSpectrumRepository::SetChannelToFrequencyWidth (MmWavePhyStandard standard, MmWavePhyBand band)
    {
        m_standard = standard;
        m_band = band;
        for (const auto & i : mmWaveChannelToFrequency)
        {
            if ((i.first.second == m_standard) && (i.first.first.second == m_band))
            {
                m_channelToFrequency[i.first] = i.second;
            }
        }
        m_channelToFrequencyWidthMapInitialized = true;
    }

    void
    MmWaveSpectrumRepository::ResetStatisticalInfo ()
    {
        if (!m_channelToFrequencyWidthMapInitialized)
        {
            NS_FATAL_ERROR ("channel to frequency has not been initialized");
        }
        m_statisticalInfo.clear ();
        Ptr<MmWaveSpectrumStatistical> s;
        for (auto & it : m_channelToFrequency)
        {
            s = Create<MmWaveSpectrumStatistical> ();
            m_statisticalInfo[it.first]= s;
        }
    }

    Ptr<MmWaveSpectrumStatistical>
    MmWaveSpectrumRepository::GetStatisticalInfo (Vector position, MmWaveChannelNumberStandardPair channel)
    {
        UpdateStatisticalInfo (position);
        auto find = m_statisticalInfo.find (channel);
        if (find != m_statisticalInfo.end ())
        {
            return m_statisticalInfo[channel];
        }
        NS_FATAL_ERROR ("Cannot find channel in spectrum repository");
        return nullptr;
    }

    MmWaveSpectrumRepository::StatisticalInfo
    MmWaveSpectrumRepository::GetAllStatisticalInfo (Vector position)
    {
        UpdateStatisticalInfo (position);
        return m_statisticalInfo;
    }

    void
    MmWaveSpectrumRepository::UpdateStatisticalInfo (Vector position)
    {
//        CheckTtlExceeded ();
//        CheckRadiusExceeded (position);
        CalculateStatisticalInfo ();
    }

    void
    MmWaveSpectrumRepository::CalculateStatisticalInfo ()
    {
        if (!m_channelToFrequencyWidthMapInitialized)
        {
            NS_FATAL_ERROR ("channel to frequency has not been initialized");
        }
        std::map<std::pair<Time, Time>, ActivityDuration> recordList;
        std::pair<Time, Time> startAndEnd;
        bool valid;
        bool isAnyPUs;
        bool isAnySUs;
        double avgPower;
        double durationForPUs;
        double durationForSUs;
        double totalDuration;
        ResetStatisticalInfo ();
        for (auto & i : m_channelToFrequency)
        {
            valid = false;
            isAnyPUs = false;
            isAnySUs = false;
            avgPower = 0;
            durationForPUs = 0;
            durationForSUs = 0;
            totalDuration = 0;
            recordList.clear ();
            for (auto & k : m_spectrumDataInfo[i.first])
            {
                valid = true;
                startAndEnd = std::make_pair (k->GetDetectionStart (), k->GetDetectionEnd ());
                auto g = recordList.find (startAndEnd);
                if (g == recordList.end ())
                {
                    ActivityDuration info{};
                    info.isAnyPUs = false;
                    info.isAnySUs = false;
                    info.durationForPUs = 0;
                    info.durationForSUs = 0;
                    info.totalDuration = k->GetDetectionEnd ().GetMicroSeconds () - k->GetDetectionStart ().GetMicroSeconds ();
                    recordList[startAndEnd]= info;
                }

                switch (k->GetChannelState ())
                {
                    case UTILIZED_BY_PUs:
                        recordList[startAndEnd].isAnyPUs = true;
                        recordList[startAndEnd].durationForPUs += k->GetOccupiedDuration ().GetMicroSeconds ();
                        break;
                    case UTILIZED_BY_SUs:
                        recordList[startAndEnd].isAnySUs = true;
                        recordList[startAndEnd].durationForSUs += k->GetOccupiedDuration ().GetMicroSeconds ();
                        break;
                    case UNUTILIZED:
                    default:
                        break;
                }
            }

            for (auto & j : recordList)
            {
                if (j.second.isAnyPUs)
                {
                    isAnyPUs = true;
                }
                if (j.second.isAnySUs)
                {
                    isAnySUs = true;
                }
                durationForPUs += j.second.durationForPUs;
                durationForSUs += j.second.durationForSUs;
                totalDuration += j.second.totalDuration;
            }

            for (auto & u : m_spectrumDataInfo[i.first])
            {
                avgPower += (u->GetPower() * u->GetOccupiedDuration ().GetMicroSeconds () / (durationForPUs + durationForSUs));
            }

            Ptr<MmWaveSpectrumStatistical> statistic = Create<MmWaveSpectrumStatistical> ();
            statistic->SetValid (valid);
            statistic->SetAvgPower (avgPower);
            statistic->SetPUs (isAnyPUs);
            statistic->SetSUs (isAnySUs);
            if (totalDuration == 0)
            {
                statistic->SetUtilizationForPUs (0);
                statistic->SetUtilizationForSUs (0);
                statistic->SetVacancyRate (1);
            }
            else
            {
                statistic->SetUtilizationForPUs (durationForPUs / totalDuration);
                statistic->SetUtilizationForSUs (durationForSUs / totalDuration);
                statistic->SetVacancyRate ((totalDuration - durationForPUs - durationForSUs) / totalDuration);
            }

            m_statisticalInfo[i.first] = statistic;

            NS_LOG_DEBUG ("-------------------------------------");
            NS_LOG_DEBUG ("channel:" << i.first
                                     << ", m_isValid:"<< statistic->m_isValid
                                     << ", m_isAnyPUs:" << statistic->m_isAnyPUs
                                     << ", m_isAnySUs:" << statistic->m_isAnySUs
                                     << ", m_avgPower:" << statistic->m_avgPower
                                     << ", m_utilizationRateForPUs:" << statistic->m_utilizationRateForPUs
                                     << ", m_utilizationRateForSUs:" << statistic->m_utilizationRateForSUs
                                     << ", m_vacancyRate:" << statistic->m_vacancyRate);
            NS_LOG_DEBUG ("-------------------------------------");
        }
    }

    MmWaveChannelNumberStandardPair
    MmWaveSpectrumRepository::GetRecommendedChannel (MmWaveChannelNumberStandardPair c, Vector position, MmWaveNeighborDevices neighbors)
    {
        NS_LOG_FUNCTION (this);
        if (!m_channelToFrequencyWidthMapInitialized)
        {
            NS_FATAL_ERROR ("channel to frequency has not been initialized");
        }
        bool first;
        MmWaveChannelNumberStandardPair h;
        UpdateStatisticalInfo (position);

        std::map<MmWaveChannelNumberStandardPair, int> noPUs;
        int currentNeighborNum;
        int minNum;
        if (m_statisticalInfo[c]->IsValid ())
        {
            for (auto i : m_statisticalInfo)
            {
                if (i.second->IsValid ())
                {
                    if (!i.second->IsAnyPUs ())
                    {
                        noPUs[i.first] = 0;
                    }
                }
            }

            currentNeighborNum = 0;
            for (auto & n : neighbors)
            {
                auto f = noPUs.find (n.second->m_channel);
                if (f != noPUs.end ())
                {
                    f->second += 1;
                }
                if (c == n.second->m_channel)
                {
                    currentNeighborNum += 1;
                }
            }

            minNum = 0;
            first = true;
            if (m_statisticalInfo[c]->IsAnyPUs ())
            {
                if (noPUs.size() == 0)
                {
                    return c;
                }
                else
                {
                    for (auto & k : noPUs)
                    {
                        if (first)
                        {
                            h = k.first;
                            minNum = k.second;
                            first = false;
                        }

                        if (minNum > k.second)
                        {
                            h = k.first;
                            minNum = k.second;
                        }
                    }
                    return h;
                }
            }
            else
            {
                NS_ASSERT (noPUs.size() != 0);
                for (auto & k : noPUs)
                {
                    if (first)
                    {
                        h = k.first;
                        minNum = k.second;
                        first = false;
                    }

                    if (minNum > k.second)
                    {
                        h = k.first;
                        minNum = k.second;
                    }
                }

                if (currentNeighborNum - minNum > m_deltaNum)
                {
                    return h;
                }
                else if (currentNeighborNum - minNum == m_deltaNum)
                {
                    if ((m_statisticalInfo[c]->GetVacancyRate () < m_statisticalInfo[h]->GetVacancyRate ())
                        && (m_statisticalInfo[c]->GetAvgPower () > m_statisticalInfo[h]->GetAvgPower ()))
                    {
                        return h;
                    }
                }
                return c;
            }
        }
        else
        {
            return c;
        }
    }

} //namespace ns3