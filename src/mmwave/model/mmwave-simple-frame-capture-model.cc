/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "mmwave-simple-frame-capture-model.h"
#include "mmwave-phy.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("MmWaveSimpleFrameCaptureModel");
    NS_OBJECT_ENSURE_REGISTERED (MmWaveSimpleFrameCaptureModel);

    TypeId
    MmWaveSimpleFrameCaptureModel::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::MmWaveSimpleFrameCaptureModel")
                .SetParent<MmWaveFrameCaptureModel> ()
                .SetGroupName ("MmWave")
                .AddConstructor<MmWaveSimpleFrameCaptureModel> ()
                .AddAttribute ("Margin",
                               "Reception is switched if the newly arrived frame has a power higher than "
                               "this value above the frame currently being received (expressed in dB).",
                               DoubleValue (5),
                               MakeDoubleAccessor (&MmWaveSimpleFrameCaptureModel::GetMargin,
                                                   &MmWaveSimpleFrameCaptureModel::SetMargin),
                               MakeDoubleChecker<double> ())
        ;
        return tid;
    }

    MmWaveSimpleFrameCaptureModel::MmWaveSimpleFrameCaptureModel ()
    {
        NS_LOG_FUNCTION (this);
    }

    MmWaveSimpleFrameCaptureModel::~MmWaveSimpleFrameCaptureModel ()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    MmWaveSimpleFrameCaptureModel::SetMargin (double margin)
    {
        NS_LOG_FUNCTION (this << margin);
        m_margin = margin;
    }

    double
    MmWaveSimpleFrameCaptureModel::GetMargin () const
    {
        return m_margin;
    }

    bool
    MmWaveSimpleFrameCaptureModel::CaptureNewFrame (Ptr<MmWaveEvent> currentEvent, Ptr<MmWaveEvent> newEvent) const
    {
        NS_LOG_FUNCTION (this);
        if ((WToDbm (currentEvent->GetRxPowerW ()) + GetMargin ()) < WToDbm (newEvent->GetRxPowerW ())
            && (IsInCaptureWindow (currentEvent->GetStartTime ())))
        {
            return true;
        }
        return false;
    }

    double
    MmWaveSimpleFrameCaptureModel::WToDbm (double w) const
    {
        return 10.0 * std::log10 (w) + 30.0;
    }
} //namespace ns3
