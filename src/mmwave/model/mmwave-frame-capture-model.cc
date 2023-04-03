/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "mmwave-frame-capture-model.h"
namespace ns3 {

    NS_OBJECT_ENSURE_REGISTERED (MmWaveFrameCaptureModel);

    TypeId MmWaveFrameCaptureModel::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::MmWaveFrameCaptureModel")
                .SetParent<Object> ()
                .SetGroupName ("MmWave")
                .AddAttribute ("CaptureWindow",
                               "The duration of the capture window.",
                               TimeValue (NanoSeconds (0.57 * 1152)),
                               MakeTimeAccessor (&MmWaveFrameCaptureModel::m_captureWindow),
                               MakeTimeChecker ())
        ;
        return tid;
    }

    bool
    MmWaveFrameCaptureModel::IsInCaptureWindow (Time timePreambleDetected) const
    {
        return (timePreambleDetected + m_captureWindow >= Simulator::Now ());
    }

} //namespace ns3
