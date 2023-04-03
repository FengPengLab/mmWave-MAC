/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef CR_REPOSITORY_H
#define CR_REPOSITORY_H
#include <map>
#include <vector>
#include "ns3/vector.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/object-factory.h"
#include "mmwave.h"

namespace ns3 {
    class NetDevice;

    class MmWaveSpectrumData : public SimpleRefCount<MmWaveSpectrumData>
    {
    public:
        MmWaveSpectrumData ();
        ~MmWaveSpectrumData ();

        void SetPower (double power);
        void SetSnr (double snr);
        void SetDetectionStart (Time time);
        void SetDetectionEnd (Time time);
        void SetOccupiedDuration (Time time);
        void SetRelativeStart (Time time);
        void SetChannelState (MmWaveChannelState state);
        void SetPosition (Vector position);
        void SetUpdateTime (Time time);

        double GetPower () const;
        double GetSnr () const;
        Time GetDetectionStart () const;
        Time GetDetectionEnd () const;
        Time GetOccupiedDuration () const;
        Time GetRelativeStart () const;
        MmWaveChannelState GetChannelState () const;
        Vector GetPosition () const;
        Time GetUpdateTime () const;

    private:
        double m_power;
        double m_snr;
        MmWaveChannelState m_state;
        Vector m_position;
        Time m_occupiedDuration;
        Time m_relativeStart;
        Time m_detectionStart;
        Time m_detectionEnd;
        Time m_updateTime;
    };

    class MmWaveSpectrumStatistical : public SimpleRefCount<MmWaveSpectrumStatistical>
    {
    public:
        MmWaveSpectrumStatistical ();
        ~MmWaveSpectrumStatistical ();

        void SetValid (bool valid);
        bool IsValid () const;

        void SetPUs (bool isAny);
        bool IsAnyPUs () const;

        void SetSUs (bool isAny);
        bool IsAnySUs () const;

        void SetAvgPower (double power);
        double GetAvgPower () const;

        void SetUtilizationForPUs (double utilization);
        double GetUtilizationForPUs () const;

        void SetUtilizationForSUs (double utilization);
        double GetUtilizationForSUs () const;

        void SetVacancyRate (double rate);
        double GetVacancyRate ();

        bool m_isValid;
        bool m_isAnyPUs;
        bool m_isAnySUs;
        double m_avgPower;
        double m_utilizationRateForPUs;
        double m_utilizationRateForSUs;
        double m_vacancyRate;
    };

    class MmWaveSpectrumRepository : public Object
    {
    public:
        MmWaveSpectrumRepository ();
        ~MmWaveSpectrumRepository ();
        static TypeId GetTypeId ();
        void DoDispose ();
        void DoInitialize ();

        struct ActivityDuration
        {
            bool isAnyPUs;
            bool isAnySUs;
            int64_t durationForPUs;
            int64_t durationForSUs;
            int64_t totalDuration;
        };

        typedef std::map<MmWaveChannelNumberStandardPair, Ptr <MmWaveSpectrumStatistical>> StatisticalInfo;
        typedef std::map<MmWaveChannelNumberStandardPair, std::vector<Ptr<MmWaveSpectrumData>>> SpectrumDataInfo;

        void CheckTtlExceeded ();
        void CheckSizeExceeded ();
        void CheckRadiusExceeded (Vector aPos);
        void SetMaxDelay (Time delay);
        void SetMaxRadius (double radius);
        void SetMaxSize (int size);
        void SetNetDevice (Ptr<NetDevice> device);
        void SetChannelToFrequencyWidth (MmWavePhyStandard standard, MmWavePhyBand band);
        void AddActivityInfoToRepository (MmWaveChannelNumberStandardPair channel, Ptr<MmWaveSpectrumData> data);
        void UpdateStatisticalInfo (Vector position);
        void CalculateStatisticalInfo ();
        void ResetStatisticalInfo ();
        bool IsEachChannelDetected (Vector position);
        double GetMaxRadius () const;
        int GetMaxSize () const;
        Time GetMaxDelay () const;
        Ptr<NetDevice> GetNetDevice () const;
        Ptr<MmWaveSpectrumStatistical> GetStatisticalInfo (Vector position, MmWaveChannelNumberStandardPair channel);
        StatisticalInfo GetAllStatisticalInfo (Vector position);
        MmWaveChannelNumberStandardPair GetRecommendedChannel (MmWaveChannelNumberStandardPair c, Vector position, MmWaveNeighborDevices neighbors);
    protected:
        MmWavePhyStandard m_standard;
        MmWavePhyBand m_band;
        SpectrumDataInfo m_spectrumDataInfo;
        StatisticalInfo m_statisticalInfo;
        MmWaveChannelToFrequencyWidthMap m_channelToFrequency;
        Ptr<NetDevice> m_device;
        Time m_maxDelay;
        int m_maxSize;
        int m_deltaNum;
        bool m_configure;
        bool m_channelToFrequencyWidthMapInitialized;
        bool m_isEachChannelDetected;
        double m_maxRadius;
        double m_thresholdForPUsActivity;
        double m_thresholdForSUsActivity;
    };

}

#endif //CR_REPOSITORY_H
