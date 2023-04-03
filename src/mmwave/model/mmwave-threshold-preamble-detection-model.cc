/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include <cmath>
#include "ns3/object.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "mmwave-threshold-preamble-detection-model.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("MmWaveThresholdPreambleDetectionModel");
    NS_OBJECT_ENSURE_REGISTERED (MmWaveThresholdPreambleDetectionModel);

    TypeId
    MmWaveThresholdPreambleDetectionModel::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::MmWaveThresholdPreambleDetectionModel")
                .SetParent<MmWavePreambleDetectionModel> ()
                .SetGroupName ("MmWave")
                .AddConstructor<MmWaveThresholdPreambleDetectionModel> ()
                .AddAttribute ("Threshold",
                               "Preamble is successfully detection if the SNR is at or above this value (expressed in dB).",
                               DoubleValue (4),
                               MakeDoubleAccessor (&MmWaveThresholdPreambleDetectionModel::m_threshold),
                               MakeDoubleChecker<double> ())
                .AddAttribute ("MinimumRssi",
                               "Preamble is dropped if the RSSI is below this value (expressed in dBm).",
                               DoubleValue (-82),
                               MakeDoubleAccessor (&MmWaveThresholdPreambleDetectionModel::m_rssiMin),
                               MakeDoubleChecker<double> ())
        ;
        return tid;
    }

    MmWaveThresholdPreambleDetectionModel::MmWaveThresholdPreambleDetectionModel ()
    {
        NS_LOG_FUNCTION (this);
    }

    MmWaveThresholdPreambleDetectionModel::~MmWaveThresholdPreambleDetectionModel ()
    {
        NS_LOG_FUNCTION (this);
    }

    bool
    MmWaveThresholdPreambleDetectionModel::IsPreambleDetected (double rssi, double snr, double channelWidth) const
    {
        NS_LOG_FUNCTION (this << WToDbm (rssi) << RatioToDb (snr) << channelWidth);
        if (WToDbm (rssi) >= m_rssiMin)
        {
            if (RatioToDb (snr) >= m_threshold)
            {
                return true;
            }
            else
            {
                NS_LOG_DEBUG ("Received RSSI is above the target RSSI but SNR is too low");
                return false;
            }
        }
        else
        {
            NS_LOG_DEBUG ("Received RSSI is below the target RSSI");
            return false;
        }

    }

    double
    MmWaveThresholdPreambleDetectionModel::DbmToW (double dBm) const
    {
        return std::pow (10.0, 0.1 * (dBm - 30.0));
    }

    double
    MmWaveThresholdPreambleDetectionModel::WToDbm (double w) const
    {
        return 10.0 * std::log10 (w) + 30.0;
    }

    double
    MmWaveThresholdPreambleDetectionModel::RatioToDb (double ratio) const
    {
        return 10.0 * std::log10 (ratio);
    }

} //namespace ns3
