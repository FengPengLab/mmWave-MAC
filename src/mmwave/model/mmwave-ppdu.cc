/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/log.h"
#include "ns3/packet.h"
#include "mmwave-ppdu.h"
#include "mmwave-psdu.h"
#include "mmwave-phy.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("MmWavePpdu");

    MmWavePpdu::MmWavePpdu (Ptr<const MmWavePsdu> psdu, MmWaveTxVector txVector, Time ppduDuration, MmWavePhyBand band)
            : m_preamble (txVector.GetPreambleType ()),
              m_modulation (txVector.IsValid () ? txVector.GetMode ().GetModulationClass () : MMWAVE_MOD_CLASS_UNKNOWN),
              m_truncatedTx (false),
              m_unrecognizedSignal (false),
              m_band (band),
              m_channelWidth (txVector.GetChannelWidth ()),
              m_txPowerLevel (txVector.GetTxPowerLevel ())
    {
        NS_LOG_FUNCTION (this << *psdu << txVector << ppduDuration << band);
        m_psdus.insert (std::make_pair (SU_STA_ID, psdu));
        SetPhyHeaders (txVector, ppduDuration, band);
    }

    MmWavePpdu::MmWavePpdu (const MmWaveConstPsduMap & psdus, MmWaveTxVector txVector, Time ppduDuration, MmWavePhyBand band)
            : m_preamble (txVector.GetPreambleType ()),
              m_modulation (txVector.IsValid () ? txVector.GetMode ().GetModulationClass () : MMWAVE_MOD_CLASS_UNKNOWN),
              m_psdus (psdus),
              m_truncatedTx (false),
              m_unrecognizedSignal (false),
              m_band (band),
              m_channelWidth (txVector.GetChannelWidth ()),
              m_txPowerLevel (txVector.GetTxPowerLevel ())
    {
        NS_LOG_FUNCTION (this << psdus << txVector << ppduDuration << band);
        SetPhyHeaders (txVector, ppduDuration, band);
    }

    MmWavePpdu::~MmWavePpdu ()
    {
    }

    void
    MmWavePpdu::SetPhyHeaders (MmWaveTxVector txVector, Time ppduDuration, MmWavePhyBand band)
    {
        if (!txVector.IsValid ())
        {
            return;
        }
        NS_LOG_FUNCTION (this << txVector << ppduDuration << band);

        switch (m_modulation)
        {
            case MMWAVE_MOD_CLASS_OFDM:
            {
                m_sig.SetLength (m_psdus.at (SU_STA_ID)->GetSize ());
                m_sig.SetMcs (txVector.GetMode ().GetMcsValue ());
                m_sig.SetNStreams (txVector.GetNss ());
                m_sig.SetBssColor (txVector.GetBssColor ());
                m_sig.SetChannelWidth (m_channelWidth);
                m_sig.SetGuardIntervalAndLtfSize (txVector.GetGuardInterval (), 2/*NLTF currently unused*/);
                m_sig.SetCoding (txVector.IsLdpc ());
                break;
            }
            default:
                NS_FATAL_ERROR ("unsupported modulation class");
                break;
        }
    }

    MmWaveTxVector
    MmWavePpdu::GetTxVector () const
    {
        MmWaveTxVector txVector;
        txVector.SetPreambleType (m_preamble);
        switch (m_modulation)
        {
            case MMWAVE_MOD_CLASS_OFDM:
            {
                txVector.SetMode (MmWavePhy::GetMmWaveMcs (m_sig.GetMcs ()));
                txVector.SetChannelWidth (m_sig.GetChannelWidth ());
                txVector.SetNss (m_sig.GetNStreams ());
                txVector.SetGuardInterval (m_sig.GetGuardInterval ());
                txVector.SetBssColor (m_sig.GetBssColor ());
                txVector.SetLdpc (m_sig.IsLdpcCoding ());
                break;
            }
            default:
                NS_FATAL_ERROR ("unsupported modulation class");
                break;
        }
        txVector.SetTxPowerLevel (m_txPowerLevel);
        return txVector;
    }

    Ptr<const MmWavePsdu>
    MmWavePpdu::GetPsdu (uint8_t bssColor, uint16_t staId) const
    {
        if (!IsMu ())
        {
            NS_ASSERT (m_psdus.size () == 1);
            return m_psdus.at (SU_STA_ID);
        }
        else
        {
            if (bssColor == m_sig.GetBssColor ())
            {
                auto it = m_psdus.find (staId);
                if (it != m_psdus.end ())
                {
                    return it->second;
                }
            }
        }
        return nullptr;
    }

    bool
    MmWavePpdu::IsTruncatedTx () const
    {
        return m_truncatedTx;
    }

    void
    MmWavePpdu::SetTruncatedTx ()
    {
        NS_LOG_FUNCTION (this);
        m_truncatedTx = true;
    }

    bool
    MmWavePpdu::IsUnrecognizedSignal () const
    {
        return m_unrecognizedSignal;
    }

    void
    MmWavePpdu::SetUnrecognizedSignal ()
    {
        NS_LOG_FUNCTION (this);
        m_unrecognizedSignal = true;
    }

    Time
    MmWavePpdu::GetTxDuration () const
    {
        Time ppduDuration = Seconds (0.0);
        MmWaveTxVector txVector = GetTxVector ();
        uint32_t size = m_sig.GetLength ();
        MmWavePhyBand band = m_band;
        switch (m_modulation)
        {
            case MMWAVE_MOD_CLASS_OFDM:
            {
                ppduDuration = MmWavePhy::CalculatePhyPreambleAndHeaderDuration (txVector)
                               + MmWavePhy::GetPayloadDuration (size, txVector, band, MMWAVE_NORMAL_MPDU);
                NS_ASSERT (ppduDuration.IsStrictlyPositive ());
                break;
            }
            default:
                NS_FATAL_ERROR ("unsupported modulation class");
                break;
        }
        return ppduDuration;
    }

    bool
    MmWavePpdu::IsMu () const
    {
        return false;
    }

    MmWaveModulationClass
    MmWavePpdu::GetModulation () const
    {
        return m_modulation;
    }

    void
    MmWavePpdu::Print (std::ostream& os) const
    {
        os << "preamble=" << m_preamble
           << ", modulation=" << m_modulation
           << ", truncatedTx=" << (m_truncatedTx ? "Y" : "N")
           << ", unrecognizedSignal=" << (m_unrecognizedSignal ? "Y" : "N");
        IsMu () ? (os << ", " << m_psdus) : (os << ", PSDU=" << m_psdus.at (SU_STA_ID));
    }

    std::ostream & operator << (std::ostream &os, const MmWavePpdu &ppdu)
    {
        ppdu.Print (os);
        return os;
    }

    std::ostream & operator << (std::ostream &os, const MmWaveConstPsduMap &psdus)
    {
        for (auto const& psdu : psdus)
        {
            os << "PSDU for STA_ID=" << psdu.first
               << " (" << *psdu.second << ") ";
        }
        return os;
    }

} //namespace ns3
