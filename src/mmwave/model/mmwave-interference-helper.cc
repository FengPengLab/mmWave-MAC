/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include <numeric>
#include <algorithm>
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "mmwave-error-rate-model.h"
#include "mmwave-interference-helper.h"
#include "mmwave-phy.h"
#include "mmwave-ppdu.h"
#include "mmwave-psdu.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("MmWaveInterferenceHelper");

    MmWaveEvent::MmWaveEvent (Ptr<const MmWavePpdu> ppdu, MmWaveTxVector txVector, Time duration, RxPowerWattPerChannelBand rxPower)
            : m_ppdu (ppdu),
              m_txVector (txVector),
              m_startTime (Simulator::Now ()),
              m_endTime (m_startTime + duration),
              m_rxPowerW (rxPower)
    {
    }

    MmWaveEvent::~MmWaveEvent ()
    {
    }

    Ptr<const MmWavePpdu>
    MmWaveEvent::GetPpdu () const
    {
        return m_ppdu;
    }

    Time
    MmWaveEvent::GetStartTime () const
    {
        return m_startTime;
    }

    Time
    MmWaveEvent::GetEndTime () const
    {
        return m_endTime;
    }

    Time
    MmWaveEvent::GetDuration () const
    {
        return m_endTime - m_startTime;
    }

    double
    MmWaveEvent::GetRxPowerW () const
    {
        NS_ASSERT (m_rxPowerW.size () > 0);
        auto it = std::max_element (m_rxPowerW.begin (), m_rxPowerW.end (),
                                    [] (const std::pair<MmWaveSpectrumBand, double>& p1, const std::pair<MmWaveSpectrumBand, double>& p2) {
                                        return p1.second < p2.second;
                                    });
        return it->second;
    }

    double
    MmWaveEvent::GetRxPowerW (MmWaveSpectrumBand band) const
    {
        auto it = m_rxPowerW.find (band);
        NS_ASSERT (it != m_rxPowerW.end ());
        return it->second;
    }

    RxPowerWattPerChannelBand
    MmWaveEvent::GetRxPowerWPerBand () const
    {
        return m_rxPowerW;
    }

    MmWaveTxVector
    MmWaveEvent::GetTxVector () const
    {
        return m_txVector;
    }

    std::ostream & operator << (std::ostream &os, const MmWaveEvent &event)
    {
        os << "start=" << event.GetStartTime () << ", end=" << event.GetEndTime ()
           << ", TXVECTOR=" << event.GetTxVector ()
           << ", power=" << event.GetRxPowerW () << "W"
           << ", PPDU=" << event.GetPpdu ();
        return os;
    }

    MmWaveInterferenceHelper::NiChange::NiChange (double power, Ptr<MmWaveEvent> event)
            : m_power (power),
              m_event (event)
    {
    }

    double
    MmWaveInterferenceHelper::NiChange::GetPower () const
    {
        return m_power;
    }

    void
    MmWaveInterferenceHelper::NiChange::AddPower (double power)
    {
        m_power += power;
    }

    Ptr<MmWaveEvent>
    MmWaveInterferenceHelper::NiChange::GetEvent () const
    {
        return m_event;
    }

    MmWaveInterferenceHelper::MmWaveInterferenceHelper ()
            : m_errorRateModel (0),
              m_numRxAntennas (1),
              m_rxing (false)
    {
    }

    MmWaveInterferenceHelper::~MmWaveInterferenceHelper ()
    {
        EraseEvents ();
        m_errorRateModel = 0;
    }

    Ptr<MmWaveEvent>
    MmWaveInterferenceHelper::Add (Ptr<const MmWavePpdu> ppdu, MmWaveTxVector txVector, Time duration, RxPowerWattPerChannelBand rxPowerW)
    {
        Ptr<MmWaveEvent> event = Create<MmWaveEvent> (ppdu, txVector, duration, rxPowerW);
        AppendEvent (event);
        return event;
    }

    void
    MmWaveInterferenceHelper::AddForeignSignal (Time duration, RxPowerWattPerChannelBand rxPowerW)
    {
        MmWaveMacHeader hdr;
        hdr.SetType (MMWAVE_MAC_DATA);
        Ptr<MmWavePpdu> fakePpdu = Create<MmWavePpdu> (Create<MmWavePsdu> (Create<Packet> (0), hdr), MmWaveTxVector (), duration, MMWAVE_PHY_BAND_UNSPECIFIED);
        Add (fakePpdu, MmWaveTxVector (), duration, rxPowerW);
    }

    void
    MmWaveInterferenceHelper::RemoveBands ()
    {
        NS_LOG_FUNCTION (this);
        m_niChangesPerBand.clear ();
        m_firstPowerPerBand.clear ();
    }

    void
    MmWaveInterferenceHelper::AddBand (MmWaveSpectrumBand band)
    {
        NS_LOG_FUNCTION (this << band.first << band.second);
        NS_ASSERT (m_niChangesPerBand.find (band) == m_niChangesPerBand.end ());
        NiChanges niChanges;
        m_niChangesPerBand.insert ({band, niChanges});
        AddNiChangeEvent (Time (0), NiChange (0.0, 0), band);
        m_firstPowerPerBand.insert ({band, 0.0});
    }

    void
    MmWaveInterferenceHelper::SetNoiseFigure (double value)
    {
        m_noiseFigure = value;
    }

    void
    MmWaveInterferenceHelper::SetErrorRateModel (const Ptr<MmWaveErrorRateModel> rate)
    {
        m_errorRateModel = rate;
    }

    Ptr<MmWaveErrorRateModel>
    MmWaveInterferenceHelper::GetErrorRateModel () const
    {
        return m_errorRateModel;
    }

    void
    MmWaveInterferenceHelper::SetNumberOfReceiveAntennas (uint8_t rx)
    {
        m_numRxAntennas = rx;
    }

    Time
    MmWaveInterferenceHelper::GetEnergyDuration (double energyW, MmWaveSpectrumBand band) const
    {
        Time now = Simulator::Now ();
        auto i = GetPreviousPosition (now, band);
        Time end = i->first;
        auto ni_it = m_niChangesPerBand.find (band);
        NS_ASSERT (ni_it != m_niChangesPerBand.end ());
        for (; i != ni_it->second.end (); ++i)
        {
            double noiseInterferenceW = i->second.GetPower ();
            end = i->first;
            if (noiseInterferenceW < energyW)
            {
                break;
            }
        }
        return end > now ? end - now : MicroSeconds (0);
    }

    void
    MmWaveInterferenceHelper::AppendEvent (Ptr<MmWaveEvent> event)
    {
        NS_LOG_FUNCTION (this);
        RxPowerWattPerChannelBand rxPowerWattPerChannelBand = event->GetRxPowerWPerBand ();
        for (auto const& it : rxPowerWattPerChannelBand)
        {
            MmWaveSpectrumBand band = it.first;
            auto ni_it = m_niChangesPerBand.find (band);
            NS_ASSERT (ni_it != m_niChangesPerBand.end ());
            double previousPowerStart = 0;
            double previousPowerEnd = 0;
            previousPowerStart = GetPreviousPosition (event->GetStartTime (), band)->second.GetPower ();
            previousPowerEnd = GetPreviousPosition (event->GetEndTime (), band)->second.GetPower ();
            if (!m_rxing)
            {
                m_firstPowerPerBand.find (band)->second = previousPowerStart;
                ni_it->second.erase (++(ni_it->second.begin ()), GetNextPosition (event->GetStartTime (), band));
            }
            auto first = AddNiChangeEvent (event->GetStartTime (), NiChange (previousPowerStart, event), band);
            auto last = AddNiChangeEvent (event->GetEndTime (), NiChange (previousPowerEnd, event), band);
            for (auto i = first; i != last; ++i)
            {
                i->second.AddPower (it.second);
            }
        }
    }

    double
    MmWaveInterferenceHelper::CalculateSnr (double signal, double noiseInterference, uint16_t channelWidth, uint8_t nss) const
    {
        NS_LOG_FUNCTION (this << signal << noiseInterference << channelWidth << +nss);
        static const double BOLTZMANN = 1.3803e-23;
        double Nt = BOLTZMANN * 290 * channelWidth * 1e6;
        double noiseFloor = m_noiseFigure * Nt;
        double noise = noiseFloor + noiseInterference;
        double snr = signal / noise; //linear scale
        NS_LOG_DEBUG ("bandwidth(MHz)=" << channelWidth << ", signal(W)= " 
        << signal << ", noise(W)=" << noiseFloor << ", interference(W)=" 
        << noiseInterference << ", snr=" << RatioToDb (snr) << "dB");
        double gain = 1;
        if (m_numRxAntennas > nss)
        {
            gain = static_cast<double>(m_numRxAntennas) / nss; //compute gain offered by diversity for AWGN
        }
        NS_LOG_DEBUG ("SNR improvement thanks to diversity: " << 10 * std::log10 (gain) << "dB");
        snr *= gain;
        return snr;
    }

    double
    MmWaveInterferenceHelper::CalculateNoiseInterferenceW (Ptr<MmWaveEvent> event, NiChangesPerBand *nis, MmWaveSpectrumBand band) const
    {
        NS_LOG_FUNCTION (this << band.first << band.second);
        auto firstPower_it = m_firstPowerPerBand.find (band);
        NS_ASSERT (firstPower_it != m_firstPowerPerBand.end ());
        double noiseInterferenceW = firstPower_it->second;
        auto ni_it = m_niChangesPerBand.find (band);
        NS_ASSERT (ni_it != m_niChangesPerBand.end ());
        auto it = ni_it->second.find (event->GetStartTime ());
        for (; it != ni_it->second.end () && it->first < Simulator::Now (); ++it)
        {
            noiseInterferenceW = it->second.GetPower () - event->GetRxPowerW (band);
        }
        it = ni_it->second.find (event->GetStartTime ());
        NS_ASSERT (it != ni_it->second.end ());
        for (; it != ni_it->second.end () && it->second.GetEvent () != event; ++it);
        NiChanges ni;
        ni.emplace (event->GetStartTime (), NiChange (0, event));
        while (++it != ni_it->second.end () && it->second.GetEvent () != event)
        {
            ni.insert (*it);
        }
        ni.emplace (event->GetEndTime (), NiChange (0, event));
        nis->insert ({band, ni});
        NS_ASSERT_MSG (noiseInterferenceW >= 0, "CalculateNoiseInterferenceW returns negative value " << noiseInterferenceW);
        return noiseInterferenceW;
    }

    double
    MmWaveInterferenceHelper::CalculateChunkSuccessRate (double snir, Time duration, MmWaveMode mode, MmWaveTxVector txVector) const
    {
        if (duration.IsZero ())
        {
            return 1.0;
        }
        uint64_t rate = mode.GetDataRate (txVector.GetChannelWidth ());
        uint64_t nbits = static_cast<uint64_t> (rate * duration.GetSeconds ());
        double csr = m_errorRateModel->GetChunkSuccessRate (mode, txVector, snir, nbits);
        return csr;
    }

    double
    MmWaveInterferenceHelper::CalculatePayloadChunkSuccessRate (double snir, Time duration, MmWaveTxVector txVector) const
    {
        if (duration.IsZero ())
        {
            return 1.0;
        }
        MmWaveMode mode = txVector.GetMode ();
        uint64_t rate = mode.GetDataRate (txVector);
        uint64_t nbits = static_cast<uint64_t> (rate * duration.GetSeconds ());
        nbits /= txVector.GetNss (); //divide effective number of bits by NSS to achieve same chunk error rate as SISO for AWGN
        double csr = m_errorRateModel->GetChunkSuccessRate (mode, txVector, snir, nbits);
        return csr;
    }

    double
    MmWaveInterferenceHelper::CalculatePayloadPer (Ptr<const MmWaveEvent> event, uint16_t channelWidth, NiChangesPerBand *nis, MmWaveSpectrumBand band, std::pair<Time, Time> window) const
    {
        NS_LOG_FUNCTION (this << channelWidth << band.first << band.second << window.first << window.second);
        const MmWaveTxVector txVector = event->GetTxVector ();
        double psr = 1.0; /* Packet Success Rate */
        auto ni_it = nis->find (band)->second;
        auto j = ni_it.begin ();
        Time previous = j->first;
        MmWaveMode payloadMode = txVector.GetMode ();
        Time phyHeaderStart = j->first + MmWavePhy::GetPhyPreambleDuration (txVector); //PPDU start time + preamble
        Time phyPayloadStart = phyHeaderStart + MmWavePhy::GetPhyHeaderDuration (txVector); //PPDU start time + preamble + Header field
        Time windowStart = phyPayloadStart + window.first;
        Time windowEnd = phyPayloadStart + window.second;
        double noiseInterferenceW = m_firstPowerPerBand.find (band)->second;
        double powerW = event->GetRxPowerW (band);
        while (++j != ni_it.end ())
        {
            Time current = j->first;
            NS_LOG_DEBUG ("previous= " << previous << ", current=" << current);
            NS_ASSERT (current >= previous);
            double snr = CalculateSnr (powerW, noiseInterferenceW, channelWidth, txVector.GetNss ());
            //Case 1: Both previous and current point to the windowed payload
            if (previous >= windowStart)
            {
                psr *= CalculatePayloadChunkSuccessRate (snr, Min (windowEnd, current) - previous, txVector);
                NS_LOG_DEBUG ("Both previous and current point to the windowed payload: mode=" << payloadMode << ", psr=" << psr);
            }
            //Case 2: previous is before windowed payload and current is in the windowed payload
            else if (current >= windowStart)
            {
                psr *= CalculatePayloadChunkSuccessRate (snr, Min (windowEnd, current) - windowStart, txVector);
                NS_LOG_DEBUG ("previous is before windowed payload and current is in the windowed payload: mode=" << payloadMode << ", psr=" << psr);
            }
            noiseInterferenceW = j->second.GetPower () - powerW;
            previous = j->first;
            if (previous > windowEnd)
            {
                NS_LOG_DEBUG ("Stop: new previous=" << previous << " after time window end=" << windowEnd);
                break;
            }
        }
        double per = 1 - psr;
        return per;
    }

    double
    MmWaveInterferenceHelper::CalculatePhyHeaderPer (Ptr<const MmWaveEvent> event, NiChangesPerBand *nis, MmWaveSpectrumBand band) const
    {
        NS_LOG_FUNCTION (this << band.first << band.second);
        const MmWaveTxVector txVector = event->GetTxVector ();
        uint16_t channelWidth = txVector.GetChannelWidth ();
        double psr = 1.0; /* Packet Success Rate */
        auto ni_it = nis->find (band)->second;
        auto j = ni_it.begin ();
        Time previous = j->first;
        MmWaveMode mcsHeaderMode = MmWavePhy::GetPhyHeaderMcsMode ();
        MmWaveMode headerMode = MmWavePhy::GetPhyHeaderMode ();
        Time phyHeaderStart = j->first + MmWavePhy::GetPhyPreambleDuration (txVector); //PPDU start time + short training field (STF) + channel estimation field (CEF)
        Time phyPayloadStart = phyHeaderStart + MmWavePhy::GetPhyHeaderDuration (txVector); //PPDU start time + short training field (STF) + channel estimation field (CEF) + Header (64bits)

        double noiseInterferenceW = m_firstPowerPerBand.find (band)->second;
        double powerW = event->GetRxPowerW (band);
        while (++j != ni_it.end ())
        {
            Time current = j->first;
            NS_LOG_DEBUG ("previous= " << previous << ", current=" << current);
            NS_ASSERT (current >= previous);
            double snr = CalculateSnr (powerW, noiseInterferenceW, channelWidth, 1);
            if (previous >= phyPayloadStart)
            {
                psr *= 1;
                NS_LOG_DEBUG ("Case 1 - previous and current after payload start: nothing to do");
            }
            else if (previous >= phyHeaderStart)
            {
                if (current >= phyPayloadStart)
                {
                    psr *= CalculateChunkSuccessRate (snr, phyPayloadStart - phyHeaderStart, mcsHeaderMode, txVector);
                    NS_LOG_DEBUG ("Case 2a - previous is in Header field and current after payload start: mcs mode=" << mcsHeaderMode << ", header mode=" << headerMode << ", psr=" << psr);
                }
                else
                {
                    psr *= 1;
                    NS_LOG_DEBUG ("Case 2b - current with previous in preamble: nothing to do");
                }
            }
            else
            {
                if (current >= phyPayloadStart)
                {
                    psr *= CalculateChunkSuccessRate (snr, phyPayloadStart - phyHeaderStart, mcsHeaderMode, txVector);
                    psr *= CalculateChunkSuccessRate (snr, phyHeaderStart - previous, mcsHeaderMode, txVector);
                    NS_LOG_DEBUG ("Case 3a - previous is in the preamble and current is after payload start: mcs mode=" << mcsHeaderMode << ", header mode=" << headerMode << ", psr=" << psr);
                }
                else if (current >= phyHeaderStart)
                {
                    psr *= CalculateChunkSuccessRate (snr, phyHeaderStart - previous, mcsHeaderMode, txVector);
                    NS_LOG_DEBUG ("Case 3b - previous is in the preamble and current is in Header field: nothing to do");
                }
                else
                {
                    psr *= 1;
                    NS_LOG_DEBUG ("Case 3c - current with previous in preamble: nothing to do");
                }
            }

            noiseInterferenceW = j->second.GetPower () - powerW;
            previous = j->first;
        }

        double per = 1 - psr;
        return per;
    }

    struct MmWaveInterferenceHelper::SnrPer
    MmWaveInterferenceHelper::CalculatePayloadSnrPer (Ptr<MmWaveEvent> event, uint16_t channelWidth, MmWaveSpectrumBand band, std::pair<Time, Time> relativeMpduStartStop) const
    {
        NS_LOG_FUNCTION (this << channelWidth << band.first << band.second << relativeMpduStartStop.first << relativeMpduStartStop.second);
        NiChangesPerBand ni;
        double noiseInterferenceW = CalculateNoiseInterferenceW (event, &ni, band);
        double snr = CalculateSnr (event->GetRxPowerW (band), noiseInterferenceW, channelWidth, event->GetTxVector ().GetNss ());
        double per = CalculatePayloadPer (event, channelWidth, &ni, band, relativeMpduStartStop);

        struct SnrPer snrPer;
        snrPer.snr = snr;
        snrPer.per = per;
        return snrPer;
    }

    double
    MmWaveInterferenceHelper::CalculateSnr (Ptr<MmWaveEvent> event, uint16_t channelWidth, uint8_t nss, MmWaveSpectrumBand band) const
    {
        NiChangesPerBand ni;
        double noiseInterferenceW = CalculateNoiseInterferenceW (event, &ni, band);
        double snr = CalculateSnr (event->GetRxPowerW (band), noiseInterferenceW, channelWidth, nss);
        return snr;
    }

    struct MmWaveInterferenceHelper::SnrPer
    MmWaveInterferenceHelper::CalculatePhyHeaderSnrPer (Ptr<MmWaveEvent> event, MmWaveSpectrumBand band) const
    {
        NS_LOG_FUNCTION (this << band.first << band.second);
        NiChangesPerBand ni;
        uint16_t channelWidth = event->GetTxVector ().GetChannelWidth ();
        double noiseInterferenceW = CalculateNoiseInterferenceW (event, &ni, band);
        double snr = CalculateSnr (event->GetRxPowerW (band), noiseInterferenceW, channelWidth, 1);
        double per = CalculatePhyHeaderPer (event, &ni, band);

        struct SnrPer snrPer;
        snrPer.snr = snr;
        snrPer.per = per;
        return snrPer;
    }

    void
    MmWaveInterferenceHelper::EraseEvents ()
    {
        for (auto it : m_niChangesPerBand)
        {
            it.second.clear ();
            AddNiChangeEvent (Time (0), NiChange (0.0, 0), it.first);
            m_firstPowerPerBand.at (it.first) = 0.0;
        }
        m_rxing = false;
    }

    double
    MmWaveInterferenceHelper::DbToRatio (double dB)
    {
        return std::pow (10.0, 0.1 * dB);
    }

    double
    MmWaveInterferenceHelper::DbmToW (double dBm)
    {
        return std::pow (10.0, 0.1 * (dBm - 30.0));
    }

    double
    MmWaveInterferenceHelper::WToDbm (double w)
    {
        return 10.0 * std::log10 (w) + 30.0;
    }

    double
    MmWaveInterferenceHelper::RatioToDb (double ratio) const
    {
        return 10.0 * std::log10 (ratio);
    }

    MmWaveInterferenceHelper::NiChanges::const_iterator
    MmWaveInterferenceHelper::GetNextPosition (Time moment, MmWaveSpectrumBand band) const
    {
        auto it = m_niChangesPerBand.find (band);
        NS_ASSERT (it != m_niChangesPerBand.end ());
        return it->second.upper_bound (moment);
    }

    MmWaveInterferenceHelper::NiChanges::const_iterator
    MmWaveInterferenceHelper::GetPreviousPosition (Time moment, MmWaveSpectrumBand band) const
    {
        auto it = GetNextPosition (moment, band);
        // This is safe since there is always an NiChange at time 0,
        // before moment.
        --it;
        return it;
    }

    MmWaveInterferenceHelper::NiChanges::iterator
    MmWaveInterferenceHelper::AddNiChangeEvent (Time moment, NiChange change, MmWaveSpectrumBand band)
    {
        auto it = m_niChangesPerBand.find (band);
        NS_ASSERT (it != m_niChangesPerBand.end ());
        return it->second.insert (GetNextPosition (moment, band), std::make_pair (moment, change));
    }

    void
    MmWaveInterferenceHelper::NotifyRxStart ()
    {
        NS_LOG_FUNCTION (this);
        m_rxing = true;
    }

    void
    MmWaveInterferenceHelper::NotifyRxEnd ()
    {
        NS_LOG_FUNCTION (this);
        m_rxing = false;
        //Update m_firstPower for frame capture
        for (auto ni : m_niChangesPerBand)
        {
            if (ni.second.size () > 1)
            {
                auto it = GetPreviousPosition (Simulator::Now (), ni.first);
                it--;
                m_firstPowerPerBand.find (ni.first)->second = it->second.GetPower ();
            }
        }
    }

} //namespace ns3