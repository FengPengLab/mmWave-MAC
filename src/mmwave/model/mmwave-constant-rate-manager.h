/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_CONSTANT_RATE_MANAGER_H
#define MMWAVE_CONSTANT_RATE_MANAGER_H
#include "mmwave-remote-station-manager.h"

namespace ns3 {
 
    class MmWaveConstantRateManager : public MmWaveRemoteStationManager
    {
    public:
        static TypeId GetTypeId ();
        MmWaveConstantRateManager ();
        virtual ~MmWaveConstantRateManager ();
        
    private:
        MmWaveRemoteStation* DoCreateStation () const;
        void DoReportRxOk (MmWaveRemoteStation *station, double rxSnr, MmWaveMode txMode);
        void DoReportRtsFailed (MmWaveRemoteStation *station);
        void DoReportDataFailed (MmWaveRemoteStation *station);
        void DoReportRtsOk (MmWaveRemoteStation *station, double ctsSnr, MmWaveMode ctsMode, double rtsSnr);
        void DoReportDataOk (MmWaveRemoteStation *station, double ackSnr, MmWaveMode ackMode, double dataSnr, uint16_t dataChannelWidth, uint8_t dataNss);
        void DoReportFinalRtsFailed (MmWaveRemoteStation *station);
        void DoReportFinalDataFailed (MmWaveRemoteStation *station);
        MmWaveTxVector DoGetDataTxVector (MmWaveRemoteStation *station);
        MmWaveTxVector DoGetCtrlTxVector (MmWaveRemoteStation *station);

        MmWaveMode m_dataMode; //!< MmWave mode for unicast Data frames
        MmWaveMode m_ctlMode;  //!< MmWave mode for RTS frames
    };

} //namespace ns3
#endif //MMWAVE_CONSTANT_RATE_MANAGER_H
