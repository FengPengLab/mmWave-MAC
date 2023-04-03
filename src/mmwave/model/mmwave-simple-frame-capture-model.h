/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_SIMPLE_FRAME_CAPTURE_MODEL_H
#define MMWAVE_SIMPLE_FRAME_CAPTURE_MODEL_H
#include "mmwave-frame-capture-model.h"

namespace ns3 {

    class MmWaveSimpleFrameCaptureModel : public MmWaveFrameCaptureModel
    {
    public:
        static TypeId GetTypeId (void);
        MmWaveSimpleFrameCaptureModel ();
        ~MmWaveSimpleFrameCaptureModel ();
        void SetMargin (double margin);
        double GetMargin () const;
        bool CaptureNewFrame (Ptr<MmWaveEvent> currentEvent, Ptr<MmWaveEvent> newEvent) const;
        double WToDbm (double w) const;
    private:
        double m_margin;
    };

} //namespace ns3

#endif //MMWAVE_SIMPLE_FRAME_CAPTURE_MODEL_H
