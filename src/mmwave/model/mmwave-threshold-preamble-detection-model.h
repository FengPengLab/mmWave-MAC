/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_THRESHOLD_PREAMBLE_DETECTION_MODEL_H
#define MMWAVE_THRESHOLD_PREAMBLE_DETECTION_MODEL_H
#include "mmwave-preamble-detection-model.h"

namespace ns3 {

    class MmWaveThresholdPreambleDetectionModel : public MmWavePreambleDetectionModel
    {
    public:
        static TypeId GetTypeId ();
        MmWaveThresholdPreambleDetectionModel ();
        ~MmWaveThresholdPreambleDetectionModel ();
        bool IsPreambleDetected (double rssi, double snr, double channelWidth) const;
        double DbmToW (double dbm) const;
        double WToDbm (double w) const;
        double RatioToDb (double ratio) const;
    private:
        double m_threshold; ///< SNR threshold in dB used to decide whether a preamble is successfully received
        double m_rssiMin;   ///< Minimum RSSI in dBm that shall be received to start the decision
    };

} //namespace ns3
#endif //MMWAVE_THRESHOLD_PREAMBLE_DETECTION_MODEL_H
