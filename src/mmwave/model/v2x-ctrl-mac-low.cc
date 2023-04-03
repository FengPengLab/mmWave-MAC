/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/log.h"
#include "ns3/event-id.h"
#include "ns3/wifi-phy.h"
#include "ns3/higher-tx-tag.h"
#include "v2x-ctrl-mac-low.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("V2xCtrlMacLow");

    NS_OBJECT_ENSURE_REGISTERED (V2xCtrlMacLow);

    TypeId
    V2xCtrlMacLow::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::V2xCtrlMacLow")
                .SetParent<MacLow> ()
                .SetGroupName ("MmWave")
                .AddConstructor<V2xCtrlMacLow> ()
        ;
        return tid;
    }
    V2xCtrlMacLow::V2xCtrlMacLow ()
    {
        NS_LOG_FUNCTION (this);
    }
    V2xCtrlMacLow::~V2xCtrlMacLow ()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    V2xCtrlMacLow::SetWaveNetDevice (Ptr<V2xMmWaveNetDevice> device)
    {
        m_scheduler = device->GetChannelScheduler ();
        m_coordinator = device->GetChannelCoordinator ();
        NS_ASSERT (m_scheduler != 0 && m_coordinator != 0);
    }

    WifiTxVector
    V2xCtrlMacLow::GetDataTxVector (Ptr<const WifiMacQueueItem> item) const
    {
        HigherLayerTxVectorTag datatag;
        bool found;
        found = ConstCast<Packet> (item->GetPacket ())->PeekPacketTag (datatag);
        // if high layer has not controlled transmit parameters, the real transmit parameters
        // will be determined by MAC layer itself.
        if (!found)
        {
            return MacLow::GetDataTxVector (item);
        }

        // if high layer has set the transmit parameters with non-adaption mode,
        // the real transmit parameters are determined by high layer.
        if (!datatag.IsAdaptable ())
        {
            return datatag.GetTxVector ();
        }

        // if high layer has set the transmit parameters with non-adaption mode,
        // the real transmit parameters are determined by both high layer and MAC layer.
        WifiTxVector txHigher = datatag.GetTxVector ();
        WifiTxVector txMac = MacLow::GetDataTxVector (item);
        WifiTxVector txAdapter;
        txAdapter.SetChannelWidth (10);
        // the DataRate set by higher layer is the minimum data rate
        // which is the lower bound for the actual data rate.
        if (txHigher.GetMode ().GetDataRate (txHigher.GetChannelWidth ()) > txMac.GetMode ().GetDataRate (txMac.GetChannelWidth ()))
        {
            txAdapter.SetMode (txHigher.GetMode ());
            txAdapter.SetPreambleType (txHigher.GetPreambleType ());
        }
        else
        {
            txAdapter.SetMode (txMac.GetMode ());
            txAdapter.SetPreambleType (txMac.GetPreambleType ());
        }
        // the TxPwr_Level set by higher layer is the maximum transmit
        // power which is the upper bound for the actual transmit power;
        txAdapter.SetTxPowerLevel (std::min (txHigher.GetTxPowerLevel (), txMac.GetTxPowerLevel ()));

        return txAdapter;
    }

    void
    V2xCtrlMacLow::StartTransmission (Ptr<WifiMacQueueItem> mpdu,
                                      MacLowTransmissionParameters params,
                                      Ptr<Txop> dca)
    {
        NS_LOG_FUNCTION (this << *mpdu << params << dca);
        Ptr<WifiPhy> phy = MacLow::GetPhy ();
        uint32_t curChannel = phy->GetChannelNumber ();
        // if current channel access is not AlternatingAccess, just do as MacLow.
        if (!m_scheduler->IsAlternatingAccessAssigned (curChannel))
        {
            MacLow::StartTransmission (mpdu, params, dca);
            return;
        }

        Time transmissionTime = MacLow::CalculateTransmissionTime (mpdu->GetPacket (), &mpdu->GetHeader (), params);
        Time remainingTime = m_coordinator->NeedTimeToGuardInterval ();

        if (transmissionTime > remainingTime)
        {
            // The attempt for this transmission will be canceled;
            // and this packet will be pending for next transmission by QosTxop class
            NS_LOG_DEBUG ("Because the required transmission time = " << transmissionTime.As (Time::MS)
                                                                      << " exceeds the remainingTime = " << remainingTime.As (Time::MS)
                                                                      << ", currently this packet will not be transmitted.");
        }
        else
        {
            MacLow::StartTransmission (mpdu, params, dca);
        }
    }

} // namespace ns3
