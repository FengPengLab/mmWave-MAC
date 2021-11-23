/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "mmwave-preamble-detection-model.h"

namespace ns3 {

    NS_OBJECT_ENSURE_REGISTERED (MmWavePreambleDetectionModel);

    TypeId MmWavePreambleDetectionModel::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::MmWavePreambleDetectionModel")
                .SetParent<Object> ()
                .SetGroupName ("MmWave")
        ;
        return tid;
    }

} //namespace ns3