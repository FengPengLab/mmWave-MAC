/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_PREAMBLE_DETECTION_MODEL_H
#define MMWAVE_PREAMBLE_DETECTION_MODEL_H
#include "ns3/object.h"
namespace ns3 {

    class MmWavePreambleDetectionModel : public Object
    {
    public:
        static TypeId GetTypeId (void);
        virtual bool IsPreambleDetected (double rssi, double snr, double channelWidth) const = 0;
    };

} //namespace ns3
#endif //MMWAVE_PREAMBLE_DETECTION_MODEL_H
