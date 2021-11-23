/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_TX_VECTOR_H
#define MMWAVE_TX_VECTOR_H
#include <list>
#include "mmwave.h"
#include "mmwave-mode.h"
namespace ns3 {
    
    class MmWaveTxVector
    {
    public:

        MmWaveTxVector ();
        ~MmWaveTxVector ();
        MmWaveTxVector (MmWaveMode mode,
                        uint8_t powerLevel,
                        MmWavePreamble preamble,
                        uint16_t guardInterval,
                        uint8_t nTx,
                        uint8_t nss,
                        uint8_t ness,
                        uint16_t channelWidth,
                        bool stbc = false,
                        bool ldpc = false,
                        uint8_t bssColor = 0);

        MmWaveTxVector (const MmWaveTxVector& txVector);
        bool GetModeInitialized () const;
        MmWaveMode GetMode () const;
        void SetMode (MmWaveMode mode);
        uint8_t GetTxPowerLevel () const;
        void SetTxPowerLevel (uint8_t powerlevel);
        MmWavePreamble GetPreambleType () const;
        void SetPreambleType (MmWavePreamble preamble);
        uint16_t GetChannelWidth () const;
        void SetChannelWidth (uint16_t channelWidth);
        uint16_t GetGuardInterval () const;
        void SetGuardInterval (uint16_t guardInterval);
        uint8_t GetNTx () const;
        void SetNTx (uint8_t nTx);
        uint8_t GetNss () const;
        uint8_t GetNssMax () const;
        void SetNss (uint8_t nss);
        uint8_t GetNess () const;
        void SetNess (uint8_t ness);
        bool IsStbc () const;
        void SetStbc (bool stbc);
        bool IsLdpc () const;
        void SetLdpc (bool ldpc);
        void SetBssColor (uint8_t color);
        uint8_t GetBssColor () const;
        bool IsValid () const;

    private:
        MmWaveMode m_mode;
        uint8_t  m_txPowerLevel;
        MmWavePreamble m_preamble;       /**< preamble */
        uint16_t m_channelWidth;       /**< channel width in MHz */
        uint16_t m_guardInterval;      /**< guard interval duration in nanoseconds */
        uint8_t  m_nTx;                /**< number of TX antennas */
        uint8_t  m_nss;                /**< number of spatial streams */
        uint8_t  m_ness;               /**< number of spatial streams in beamforming */
        bool     m_stbc;               /**< STBC used or not */
        bool     m_ldpc;               /**< LDPC FEC coding if true, BCC otherwise*/
        uint8_t  m_bssColor;           /**< Bss color */
        bool     m_modeInitialized;         /**< Internal initialization flag */
    };

    std::ostream & operator << (std::ostream & os,const MmWaveTxVector &v);

} //namespace ns3
#endif //MMWAVE_TX_VECTOR_H
