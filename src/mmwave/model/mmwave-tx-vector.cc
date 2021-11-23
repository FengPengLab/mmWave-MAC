/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/abort.h"
#include "mmwave-tx-vector.h"
namespace ns3 {

    MmWaveTxVector::MmWaveTxVector ()
            : m_preamble (MMWAVE_PREAMBLE_DEFAULT),
              m_channelWidth (80),
              m_guardInterval (3200),
              m_nTx (1),
              m_nss (1),
              m_ness (1),
              m_stbc (false),
              m_ldpc (false),
              m_bssColor (0),
              m_modeInitialized (false)
    {
    }

    MmWaveTxVector::MmWaveTxVector (MmWaveMode mode,
                                    uint8_t powerLevel,
                                    MmWavePreamble preamble,
                                    uint16_t guardInterval,
                                    uint8_t nTx,
                                    uint8_t nss,
                                    uint8_t ness,
                                    uint16_t channelWidth,
                                    bool stbc,
                                    bool ldpc,
                                    uint8_t groupColor)
            : m_mode (mode),
              m_txPowerLevel (powerLevel),
              m_preamble (preamble),
              m_channelWidth (channelWidth),
              m_guardInterval (guardInterval),
              m_nTx (nTx),
              m_nss (nss),
              m_ness (ness),
              m_stbc (stbc),
              m_ldpc (ldpc),
              m_bssColor (groupColor),
              m_modeInitialized (true)
    {
    }

    MmWaveTxVector::MmWaveTxVector (const MmWaveTxVector& txVector)
            : m_mode (txVector.m_mode),
              m_txPowerLevel (txVector.m_txPowerLevel),
              m_preamble (txVector.m_preamble),
              m_channelWidth (txVector.m_channelWidth),
              m_guardInterval (txVector.m_guardInterval),
              m_nTx (txVector.m_nTx),
              m_nss (txVector.m_nss),
              m_ness (txVector.m_ness),
              m_stbc (txVector.m_stbc),
              m_ldpc (txVector.m_ldpc),
              m_bssColor (txVector.m_bssColor),
              m_modeInitialized (txVector.m_modeInitialized)
    {
    }

    MmWaveTxVector::~MmWaveTxVector ()
    {
    }

    bool
    MmWaveTxVector::GetModeInitialized () const
    {
        return m_modeInitialized;
    }

    MmWaveMode
    MmWaveTxVector::GetMode () const
    {
        if (!m_modeInitialized)
        {
            NS_FATAL_ERROR ("MmWaveTxVector mode must be set before using");
        }
        return m_mode;
    }

    uint8_t
    MmWaveTxVector::GetTxPowerLevel () const
    {
        return m_txPowerLevel;
    }

    MmWavePreamble
    MmWaveTxVector::GetPreambleType () const
    {
        return m_preamble;
    }

    uint16_t
    MmWaveTxVector::GetChannelWidth () const
    {
        return m_channelWidth;
    }

    uint16_t
    MmWaveTxVector::GetGuardInterval () const
    {
        return m_guardInterval;
    }

    uint8_t
    MmWaveTxVector::GetNTx () const
    {
        return m_nTx;
    }

    uint8_t
    MmWaveTxVector::GetNss () const
    {
        return m_nss;
    }

    uint8_t
    MmWaveTxVector::GetNssMax () const
    {
        return m_nss;
    }

    uint8_t
    MmWaveTxVector::GetNess () const
    {
        return m_ness;
    }

    bool
    MmWaveTxVector::IsStbc () const
    {
        return m_stbc;
    }

    bool
    MmWaveTxVector::IsLdpc () const
    {
        return m_ldpc;
    }

    void
    MmWaveTxVector::SetMode (MmWaveMode mode)
    {
        m_mode = mode;
        m_modeInitialized = true;
    }

    void
    MmWaveTxVector::SetTxPowerLevel (uint8_t powerlevel)
    {
        m_txPowerLevel = powerlevel;
    }

    void
    MmWaveTxVector::SetPreambleType (MmWavePreamble preamble)
    {
        m_preamble = preamble;
    }

    void
    MmWaveTxVector::SetChannelWidth (uint16_t channelWidth)
    {
        m_channelWidth = channelWidth;
    }

    void
    MmWaveTxVector::SetGuardInterval (uint16_t guardInterval)
    {
        m_guardInterval = guardInterval;
    }

    void
    MmWaveTxVector::SetNTx (uint8_t nTx)
    {
        m_nTx = nTx;
    }

    void
    MmWaveTxVector::SetNss (uint8_t nss)
    {
        m_nss = nss;
    }

    void
    MmWaveTxVector::SetNess (uint8_t ness)
    {
        m_ness = ness;
    }

    void
    MmWaveTxVector::SetStbc (bool stbc)
    {
        m_stbc = stbc;
    }

    void
    MmWaveTxVector::SetLdpc (bool ldpc)
    {
        m_ldpc = ldpc;
    }

    void
    MmWaveTxVector::SetBssColor (uint8_t color)
    {
        m_bssColor = color;
    }

    uint8_t
    MmWaveTxVector::GetBssColor () const
    {
        return m_bssColor;
    }

    bool
    MmWaveTxVector::IsValid () const
    {
        if (!GetModeInitialized ())
        {
            return false;
        }
        return true;
    }

    std::ostream & operator << ( std::ostream &os, const MmWaveTxVector &v)
    {
        if (!v.IsValid ())
        {
            os << "TXVECTOR not valid";
            return os;
        }
        os << "txpwrlvl: " << +v.GetTxPowerLevel ()
           << " preamble: " << v.GetPreambleType ()
           << " channel width: " << v.GetChannelWidth ()
           << " GI: " << v.GetGuardInterval ()
           << " NTx: " << +v.GetNTx ()
           << " Ness: " << +v.GetNess ()
           << " STBC: " << v.IsStbc ()
           << " FEC coding: " << (v.IsLdpc () ? "LDPC" : "BCC")
           << " mode: " << v.GetMode ()
           << " Nss: " << +v.GetNss ();
        return os;
    }

} //namespace ns3