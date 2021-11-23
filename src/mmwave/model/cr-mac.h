/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
#ifndef CR_MAC_H
#define CR_MAC_H
#include <vector>
#include <algorithm>
#include <map>
#include "ns3/ptr.h"
#include "mmwave.h"
#include "mmwave-mac.h"
#include "mmwave-spectrum-repository.h"
#include "mmwave-phy.h"
#include "mmwave-mac-tx-middle.h"
#include "mmwave-mac-rx-middle.h"
#include "mmwave-remote-station-manager.h"
#include "mmwave-mac-header.h"
#include "mmwave-mac-low.h"
#include "mmwave-mac-queue-item.h"
#include "cr-txop.h"
#include "cr-mac-low.h"
#include "cr-dynamic-channel-access-manager.h"
namespace ns3 {

    class CrMmWaveMac : public MmWaveMac
    {
    public:
        static TypeId GetTypeId ();
        CrMmWaveMac ();
        ~CrMmWaveMac ();
        void DoDispose ();
        void DoInitialize ();
        void SetAddress (Mac48Address address);
        void SetBssid (Mac48Address bssid);
        void SetRemoteStationManager (Ptr<MmWaveRemoteStationManager> stationManager);
        void SetPromisc ();
        void SetLinkUpCallback (Callback<void> linkUp);
        void SetAccessMode (MmWaveAccessMode mode);
        void Enqueue (Ptr<Packet> packet, Mac48Address to, Mac48Address from);
        void Enqueue (Ptr<Packet> packet, Mac48Address to);
        void Receive (Ptr<MmWaveMacQueueItem> mpdu);
        Mac48Address GetAddress () const;
        Mac48Address GetBssid () const;
        void SetPhy (TypeOfGroup type, Ptr<MmWavePhy> phy);
        void ResetPhy (TypeOfGroup type);
        void SetSpectrumInfoRepository (Ptr<MmWaveSpectrumRepository> info);
        void CheckNeighborDevice ();
        void UpdateNeighborDevice (Mac48Address addr, MmWaveChannelNumberStandardPair channel, Time stamp);
        bool IsAnyConflictBetweenGroup ();
        int64_t AssignStreams (int64_t stream);
        Time GetDetectionInterval () const;
        Time GetBeaconInterval () const;
        Time GetFineDetectionDuration () const;
        Time GetFastDetectionDuration () const;
        MmWaveAccessMode GetAccessMode () const;
        MmWaveNeighborDevices GetAllNeighborDevices ();
        MmWaveChannelNumberStandardPair GetChannelNeedToAccessForIntraGroup ();
        MmWaveChannelNumberStandardPair GetChannelNeedToAccessForInterGroup ();
        MmWaveChannelNumberStandardPair GetCurrentChannel (TypeOfGroup type);
        MmWaveChannelToFrequencyWidthMap GetChannelToFrequency ();
        void AddSpectrumActivityToRepository (MmWaveChannelNumberStandardPair channel, MmWaveChannelState state, double rxSnr,
                                              Time start, Time duration, Time detectionStart, Time detectionDuration);
        Ptr<MmWaveSpectrumStatistical> GetStatisticalInfo (MmWaveChannelNumberStandardPair channel);
        Ptr<MmWavePhy> GetPhy (TypeOfGroup type) const;
        Ptr<MmWaveSpectrumRepository> GetSpectrumRepository () const;
        Ptr<CrMmWaveMacLow> GetMacLow (TypeOfGroup type) const;
        Ptr<CrDynamicChannelAccessManager> GetChannelAccessManager (TypeOfGroup type) const;
        Ptr<CrMmWaveTxop> GetTxop () const;
        Ptr<UniformRandomVariable> GetRandomVariable ();
    protected:
        Ptr<MmWavePhy> m_phyOfIntraGroup;
        Ptr<MmWavePhy> m_phyOfInterGroup;
        Ptr<MmWavePhy> m_phyOfProbeGroup;
        Ptr<MmWaveMacTxMiddle> m_txMiddleOfIntraGroup;
        Ptr<MmWaveMacTxMiddle> m_txMiddleOfInterGroup;
        Ptr<MmWaveMacRxMiddle> m_rxMiddle;
        Ptr<MmWaveSpectrumRepository> m_repos;
        Ptr<CrMmWaveMacLow> m_lowOfIntraGroup;
        Ptr<CrMmWaveMacLow> m_lowOfInterGroup;
        Ptr<CrMmWaveMacLow> m_lowOfProbeGroup;
        Ptr<CrDynamicChannelAccessManager> m_channelManagerOfIntraGroup;
        Ptr<CrDynamicChannelAccessManager> m_channelManagerOfInterGroup;
        Ptr<CrDynamicChannelAccessManager> m_channelManagerOfProbeGroup;
        Ptr<CrMmWaveTxop> m_txop;
        Ptr<UniformRandomVariable> m_rng;
        std::map<Mac48Address, Ptr<MmWaveNeighborDevice>> m_neighborDevices;
        Mac48Address m_self;
        Mac48Address m_bssid;
        Time m_beaconInterval;
        Time m_detectionInterval;
        Time m_fineDetectionDuration;
        Time m_fastDetectionDuration;
        MmWaveAccessMode m_accessMode;
    };
}

#endif //CR_MAC_H
