/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include <algorithm>
#include <utility>
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "mmwave-psdu.h"
#include "mmwave-remote-station-manager.h"
#include "mmwave-phy-listener.h"
#include "mmwave-mac-trailer.h"
#include "mmwave-mac.h"
#include "mmwave-phy.h"
#include "mmwave-snr-tag.h"
#include "mmwave-mac-low.h"
#undef NS_LOG_APPEND_CONTEXT
#define NS_LOG_APPEND_CONTEXT std::clog << "[mac=" << m_self << "] "

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("MmWaveMacLow");
    NS_OBJECT_ENSURE_REGISTERED (MmWaveMacLow);

    class MmWaveMacLowListener : public ns3::MmWavePhyListener
    {
    public:
        MmWaveMacLowListener (ns3::MmWaveMacLow *macLow)
                : m_macLow (macLow)
        {
        }
        virtual ~MmWaveMacLowListener ()
        {
        }
        void NotifyRxStart (Time duration)
        {
        }
        void NotifyRxEndOk ()
        {
        }
        void NotifyRxEndError ()
        {
        }
        void NotifyTxStart (Time duration, double txPowerDbm)
        {
        }
        void NotifyMaybeCcaBusyStart (Time duration)
        {
        }
        void NotifySwitchingStart (Time duration)
        {
            m_macLow->NotifySwitchingStartNow (duration);
        }
        void NotifySleep ()
        {
            m_macLow->NotifySleepNow ();
        }
        void NotifyOff ()
        {
            m_macLow->NotifyOffNow ();
        }
        void NotifyWakeup ()
        {
        }
        void NotifyOn ()
        {
        }

    private:
        ns3::MmWaveMacLow *m_macLow; ///< the MAC
    };


    MmWaveMacLow::MmWaveMacLow ()
            : m_normalAckTimeoutEvent (),
              m_ctsTimeoutEvent (),
              m_sendCtsEvent (),
              m_sendAckEvent (),
              m_waitIfsEvent (),
              m_endTxNoAckEvent (),
              m_currentPacket (0),
              m_lastNavStart (Seconds (0.0)),
              m_lastNavDuration (Seconds (0.0)),
              m_promisc (false),
              m_macLowListener (0)
    {
        NS_LOG_FUNCTION (this);
        m_rng = CreateObject<UniformRandomVariable> ();

    }

    MmWaveMacLow::~MmWaveMacLow ()
    {
        NS_LOG_FUNCTION (this);
    }

    TypeId
    MmWaveMacLow::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::MmWaveMacLow")
                .SetParent<Object> ()
                .SetGroupName ("MmWave")
        ;
        return tid;
    }

    void
    MmWaveMacLow::DoInitialize ()
    {
        NS_LOG_FUNCTION (this);
    }

    void
    MmWaveMacLow::DoDispose ()
    {
        NS_LOG_FUNCTION (this);
        m_normalAckTimeoutEvent.Cancel ();
        m_ctsTimeoutEvent.Cancel ();
        m_sendCtsEvent.Cancel ();
        m_sendAckEvent.Cancel ();
        m_waitIfsEvent.Cancel ();
        m_endTxNoAckEvent.Cancel ();
        m_phy = 0;
        m_mac = 0;
        m_rng = 0;
        m_stationManager = 0;
        if (m_macLowListener != 0)
        {
            delete m_macLowListener;
            m_macLowListener = 0;
        }
    }

    void
    MmWaveMacLow::SetupMacLowListener (const Ptr<MmWavePhy> phy)
    {
        m_macLowListener = new MmWaveMacLowListener (this);
        phy->RegisterListener (m_macLowListener);
    }

    void
    MmWaveMacLow::RemoveMacLowListener (Ptr<MmWavePhy> phy)
    {
        if (m_macLowListener != 0 )
        {
            phy->UnregisterListener (m_macLowListener);
            delete m_macLowListener;
            m_macLowListener = 0;
        }
    }

    void
    MmWaveMacLow::SetPhy (const Ptr<MmWavePhy> phy)
    {
        m_phy = phy;
        m_phy->TraceConnectWithoutContext ("PhyRxPayloadBegin", MakeCallback (&MmWaveMacLow::RxStartIndication, this));
        m_phy->SetReceiveOkCallback (MakeCallback (&MmWaveMacLow::ReceiveOk, this));
        m_phy->SetReceiveErrorCallback (MakeCallback (&MmWaveMacLow::ReceiveError, this));
        m_phy->SetUnrecognizedSignalDetectedCallback (MakeCallback (&MmWaveMacLow::UnrecognizedSignalDetected, this));
        m_phy->SetRecognizedSignalDetectedCallback (MakeCallback (&MmWaveMacLow::RecognizedSignalDetected, this));
        SetupMacLowListener (phy);
    }

    void
    MmWaveMacLow::ResetPhy ()
    {
        m_phy->TraceDisconnectWithoutContext ("PhyRxPayloadBegin", MakeCallback (&MmWaveMacLow::RxStartIndication, this));
        m_phy->SetReceiveOkCallback (MakeNullCallback<void, Ptr<MmWavePsdu>, double, Time, Time, MmWaveTxVector> ());
        m_phy->SetReceiveErrorCallback (MakeNullCallback<void, Ptr<MmWavePsdu>, double, Time, Time, MmWaveTxVector> ());
        m_phy->SetUnrecognizedSignalDetectedCallback (MakeNullCallback<void, double, double, Time, Time> ());
        m_phy->SetRecognizedSignalDetectedCallback (MakeNullCallback<void, double, double, Time, Time> ());
        RemoveMacLowListener (m_phy);
        m_phy = 0;
    }

    void
    MmWaveMacLow::SetMac (const Ptr<MmWaveMac> mac)
    {
        m_mac = mac;
    }

    Ptr<MmWavePhy>
    MmWaveMacLow::GetPhy () const
    {
        return m_phy;
    }

    Ptr<MmWaveMac>
    MmWaveMacLow::GetMac () const
    {
        return m_mac;
    }

    void
    MmWaveMacLow::SetRemoteStationManager (const Ptr<MmWaveRemoteStationManager> manager)
    {
        m_stationManager = manager;
    }

    void
    MmWaveMacLow::SetAddress (Mac48Address ad)
    {
        m_self = ad;
    }

    void
    MmWaveMacLow::SetBssid (Mac48Address bssid)
    {
        m_bssid = bssid;
    }

    void
    MmWaveMacLow::SetPromisc ()
    {
        m_promisc = true;
    }

    Mac48Address
    MmWaveMacLow::GetAddress () const
    {
        return m_self;
    }

    Mac48Address
    MmWaveMacLow::GetBssid () const
    {
        return m_bssid;
    }

    Time
    MmWaveMacLow::GetSifs () const
    {
        return m_phy->GetSifs ();
    }

    Time
    MmWaveMacLow::GetSlotTime () const
    {
        return m_phy->GetSlot ();
    }
    
    bool
    MmWaveMacLow::IsPromisc () const
    {
        return m_promisc;
    }

    void
    MmWaveMacLow::SetRxCallback (Callback<void, Ptr<MmWaveMacQueueItem>> callback)
    {
        m_rxCallback = std::move(callback);
    }

    void
    MmWaveMacLow::SetTxOkCallback (Callback <void, const MmWaveMacHeader &> callback)
    {
        m_txOk = std::move(callback);
    }

    void
    MmWaveMacLow::SetTxFailedCallback (Callback <void, const MmWaveMacHeader &> callback)
    {
        m_txFailed = std::move(callback);
    }

    Time
    MmWaveMacLow::GetAckDuration (Mac48Address to, const MmWaveTxVector dataTxVector) const
    {
        MmWaveTxVector ackTxVector = GetCtrlTxVector (to);
        return GetAckDuration (ackTxVector);
    }

    Time
    MmWaveMacLow::GetAckDuration (const MmWaveTxVector ackTxVector) const
    {
        return m_phy->CalculateTxDuration (GetAckSize (), ackTxVector, m_phy->GetPhyBand ());
    }

    Time
    MmWaveMacLow::GetCtsDuration (Mac48Address to, const MmWaveTxVector rtsTxVector) const
    {
        MmWaveTxVector ctsTxVector = GetCtsTxVectorForRts (to, rtsTxVector.GetMode ());
        return GetCtsDuration (ctsTxVector);
    }

    Time
    MmWaveMacLow::GetCtsDuration (const MmWaveTxVector ctsTxVector) const
    {
        return m_phy->CalculateTxDuration (GetCtsSize (), ctsTxVector, m_phy->GetPhyBand ());
    }

    MmWaveTxVector
    MmWaveMacLow::GetCtrlTxVector (Mac48Address address) const
    {
        return m_stationManager->GetCtrlTxVector (address);
    }

    MmWaveTxVector
    MmWaveMacLow::GetDataTxVector (Mac48Address address) const
    {
        return m_stationManager->GetDataTxVector (address);
    }

    Time
    MmWaveMacLow::GetResponseDuration (const MmWaveMacLowParameters params, const MmWaveTxVector dataTxVector, Mac48Address receiver) const
    {
        Time duration = Seconds (0.0);
        if (params.MustWaitNormalAck ())
        {
            duration += GetSifs ();
            duration += GetAckDuration (receiver, dataTxVector);
        }
        return duration;
    }

    MmWaveTxVector
    MmWaveMacLow::GetCtsTxVector (Mac48Address to, MmWaveMode rtsTxMode) const
    {
        return GetCtrlTxVector (to);
    }

    MmWaveTxVector
    MmWaveMacLow::GetAckTxVector (Mac48Address to, MmWaveMode dataTxMode) const
    {
        return GetCtrlTxVector (to);
    }

    MmWaveTxVector
    MmWaveMacLow::GetCtsTxVectorForRts (Mac48Address to, MmWaveMode rtsTxMode) const
    {
        return GetCtsTxVector (to, rtsTxMode);
    }

    MmWaveTxVector
    MmWaveMacLow::GetAckTxVectorForData (Mac48Address to, MmWaveMode dataTxMode) const
    {
        return GetAckTxVector (to, dataTxMode);
    }

    Time
    MmWaveMacLow::CalculateOverheadTxTime (Ptr<const MmWaveMacQueueItem> item, const MmWaveMacLowParameters& params) const
    {
        Time txTime = Seconds (0.0);
        txTime += GetResponseDuration (params, GetDataTxVector (item->GetHeader().GetAddr1 ()), item->GetHeader ().GetAddr1 ());
        return txTime;
    }

    void
    MmWaveMacLow::ForwardDown (Ptr<const MmWavePsdu> psdu, MmWaveTxVector txVector)
    {
        NS_LOG_FUNCTION (this << psdu << txVector);
        const MmWaveMacHeader& hdr = (psdu->GetHeader ());

        NS_LOG_DEBUG ("send " << hdr.GetTypeString () <<
                              ", to=" << hdr.GetAddr1 () <<
                              ", size=" << psdu->GetSize () <<
                              ", mode=" << txVector.GetMode  () <<
                              ", preamble=" << txVector.GetPreambleType () <<
                              ", duration=" << hdr.GetDuration () <<
                              ", seq=0x" << std::hex << hdr.GetSequenceControl () << std::dec);
        
        m_phy->Send (psdu, txVector);
    }

    bool
    MmWaveMacLow::IsNavZero () const
    {
        return (m_lastNavStart + m_lastNavDuration < Simulator::Now ());
    }

    uint32_t
    MmWaveMacLow::GetAckSize () const
    {
        MmWaveMacHeader ack;
        ack.SetType (MMWAVE_MAC_CTL_ACK);
        return ack.GetSize () + 4;
    }

    uint32_t
    MmWaveMacLow::GetRtsSize () const
    {
        MmWaveMacHeader rts;
        rts.SetType (MMWAVE_MAC_CTL_RTS);
        return rts.GetSize () + 4;
    }

    uint32_t
    MmWaveMacLow::GetCtsSize () const
    {
        MmWaveMacHeader cts;
        cts.SetType (MMWAVE_MAC_CTL_CTS);
        return cts.GetSize () + 4;
    }

    uint32_t
    MmWaveMacLow::GetSize (Ptr<const Packet> packet, const MmWaveMacHeader *hdr) const
    {
        uint32_t size;
        MmWaveMacTrailer fcs;
        size = packet->GetSize () + hdr->GetSize () + fcs.GetSerializedSize ();
        return size;
    }

    int64_t
    MmWaveMacLow::AssignStreams (int64_t stream)
    {
        NS_LOG_FUNCTION (this << stream);
        m_rng->SetStream (stream);
        return 1;
    }
} //namespace ns3