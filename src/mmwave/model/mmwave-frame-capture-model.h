/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_FRAME_CAPTURE_MODEL_H
#define MMWAVE_FRAME_CAPTURE_MODEL_H
#include "ns3/object.h"

namespace ns3 {
    class MmWaveEvent;
    class Time;

    class MmWaveFrameCaptureModel : public Object
    {
    public:
        static TypeId GetTypeId (void);
        virtual bool CaptureNewFrame (Ptr<MmWaveEvent> currentEvent, Ptr<MmWaveEvent> newEvent) const = 0;
        virtual bool IsInCaptureWindow (Time timePreambleDetected) const;

    private:
        Time m_captureWindow; //!< Capture window duration
    };

} //namespace ns3

#endif //MMWAVE_FRAME_CAPTURE_MODEL_H
