/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_PPDU_H
#define MMWAVE_PPDU_H
#include <list>
#include <unordered_map>
#include "ns3/simple-ref-count.h"
#include "ns3/ptr.h"
#include "mmwave.h"
#include "mmwave-tx-vector.h"
#include "mmwave-phy-header.h"

namespace ns3 {
    class MmWavePsdu;

    typedef std::unordered_map <uint16_t /* staId */, Ptr<const MmWavePsdu> /* PSDU */> MmWaveConstPsduMap;

    class MmWavePpdu : public SimpleRefCount<MmWavePpdu>
    {
    public:
        MmWavePpdu (Ptr<const MmWavePsdu> psdu, MmWaveTxVector txVector, Time ppduDuration, MmWavePhyBand band);
        MmWavePpdu (const MmWaveConstPsduMap & psdus, MmWaveTxVector txVector, Time ppduDuration, MmWavePhyBand band);

        virtual ~MmWavePpdu ();
        MmWaveTxVector GetTxVector () const;
        Ptr<const MmWavePsdu> GetPsdu (uint8_t bssColor = 64, uint16_t staId = SU_STA_ID) const;
        bool IsTruncatedTx () const;
        void SetTruncatedTx ();
        bool IsUnrecognizedSignal () const;
        void SetUnrecognizedSignal ();
        Time GetTxDuration () const;
        bool IsMu () const;
        MmWaveModulationClass GetModulation () const;
        void Print (std::ostream &os) const;

    private:
        void SetPhyHeaders (MmWaveTxVector txVector, Time ppduDuration, MmWavePhyBand band);
        MmWaveSigHeader m_sig;                         //!< the SIG PHY header
        MmWavePreamble m_preamble;                     //!< the PHY preamble
        MmWaveModulationClass m_modulation;            //!< the modulation used for the transmission of this PPDU
        MmWaveConstPsduMap m_psdus;                    //!< the PSDUs contained in this PPDU
        bool m_truncatedTx;                          //!< flag indicating whether the frame's transmission was aborted due to transmitter switch off
        bool m_unrecognizedSignal;
        MmWavePhyBand m_band;                          //!< the MmWavePhyBand used to transmit that PPDU
        uint16_t m_channelWidth;                     //!< the channel width used to transmit that PPDU in MHz
        uint8_t m_txPowerLevel;                      //!< the transmission power level (used only for TX and initializing the returned MmWaveTxVector)

    };

    std::ostream & operator << (std::ostream &os, const MmWavePpdu &ppdu);
    std::ostream & operator << (std::ostream &os, const MmWaveConstPsduMap &psdus);

} //namespace ns3
#endif //MMWAVE_PPDU_H
