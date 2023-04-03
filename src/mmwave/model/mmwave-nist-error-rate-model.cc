/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include <cmath>
#include <bitset>
#include "ns3/log.h"
#include "ns3/object.h"
#include "mmwave-nist-error-rate-model.h"
#include "mmwave-tx-vector.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("MmWaveNistErrorRateModel");
    NS_OBJECT_ENSURE_REGISTERED (MmWaveNistErrorRateModel);

    TypeId
    MmWaveNistErrorRateModel::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::MmWaveNistErrorRateModel")
                .SetParent<MmWaveErrorRateModel> ()
                .SetGroupName ("MmWave")
                .AddConstructor<MmWaveNistErrorRateModel> ()
        ;
        return tid;
    }

    MmWaveNistErrorRateModel::MmWaveNistErrorRateModel ()
    {
    }

    double
    MmWaveNistErrorRateModel::GetBpskBer (double snr) const
    {
        NS_LOG_FUNCTION (this << snr);
        double z = std::sqrt (snr);
        double ber = 0.5 * erfc (z);
        NS_LOG_INFO ("bpsk snr=" << snr << " ber=" << ber);
        return ber;
    }

    double
    MmWaveNistErrorRateModel::GetQpskBer (double snr) const
    {
        NS_LOG_FUNCTION (this << snr);
        double z = std::sqrt (snr / 2.0);
        double ber = 0.5 * erfc (z);
        NS_LOG_INFO ("qpsk snr=" << snr << " ber=" << ber);
        return ber;
    }

    double
    MmWaveNistErrorRateModel::GetQamBer (uint16_t constellationSize, double snr) const
    {
        NS_LOG_FUNCTION (this << constellationSize << snr);
        NS_ASSERT (std::bitset<16> (constellationSize).count () == 1); //constellationSize has to be a power of 2
        double z = std::sqrt (snr / ((2 * (constellationSize - 1)) / 3));
        uint8_t bitsPerSymbol = std::sqrt (constellationSize);
        double ber = ((bitsPerSymbol - 1) / (bitsPerSymbol * std::log2 (bitsPerSymbol))) * erfc (z);
        NS_LOG_INFO (constellationSize << "-QAM: snr=" << snr << " ber=" << ber);
        return ber;
    }

    double
    MmWaveNistErrorRateModel::GetFecBpskBer (double snr, uint64_t nbits, uint8_t bValue) const
    {
        NS_LOG_FUNCTION (this << snr << nbits << +bValue);
        double ber = GetBpskBer (snr);
        if (ber == 0.0)
        {
            return 1.0;
        }
        double pe = CalculatePe (ber, bValue);
        pe = std::min (pe, 1.0);
        double pms = std::pow (1 - pe, nbits);
        return pms;
    }

    double
    MmWaveNistErrorRateModel::GetFecQpskBer (double snr, uint64_t nbits, uint8_t bValue) const
    {
        NS_LOG_FUNCTION (this << snr << nbits << +bValue);
        double ber = GetQpskBer (snr);
        if (ber == 0.0)
        {
            return 1.0;
        }
        double pe = CalculatePe (ber, bValue);
        pe = std::min (pe, 1.0);
        double pms = std::pow (1 - pe, nbits);
        return pms;
    }

    double
    MmWaveNistErrorRateModel::CalculatePe (double p, uint8_t bValue) const
    {
        NS_LOG_FUNCTION (this << p << +bValue);
        double D = std::sqrt (4.0 * p * (1.0 - p));
        double pe = 1.0;
        if (bValue == 1)
        {
            pe = 0.5 * (36.0 * std::pow (D, 10)
                        + 211.0 * std::pow (D, 12)
                        + 1404.0 * std::pow (D, 14)
                        + 11633.0 * std::pow (D, 16)
                        + 77433.0 * std::pow (D, 18)
                        + 502690.0 * std::pow (D, 20)
                        + 3322763.0 * std::pow (D, 22)
                        + 21292910.0 * std::pow (D, 24)
                        + 134365911.0 * std::pow (D, 26));
        }
        else if (bValue == 2)
        {
            pe = 1.0 / (2.0 * bValue) *
                 (3.0 * std::pow (D, 6)
                  + 70.0 * std::pow (D, 7)
                  + 285.0 * std::pow (D, 8)
                  + 1276.0 * std::pow (D, 9)
                  + 6160.0 * std::pow (D, 10)
                  + 27128.0 * std::pow (D, 11)
                  + 117019.0 * std::pow (D, 12)
                  + 498860.0 * std::pow (D, 13)
                  + 2103891.0 * std::pow (D, 14)
                  + 8784123.0 * std::pow (D, 15));
        }
        else if (bValue == 3)
        {
            pe = 1.0 / (2.0 * bValue) *
                 (42.0 * std::pow (D, 5)
                  + 201.0 * std::pow (D, 6)
                  + 1492.0 * std::pow (D, 7)
                  + 10469.0 * std::pow (D, 8)
                  + 62935.0 * std::pow (D, 9)
                  + 379644.0 * std::pow (D, 10)
                  + 2253373.0 * std::pow (D, 11)
                  + 13073811.0 * std::pow (D, 12)
                  + 75152755.0 * std::pow (D, 13)
                  + 428005675.0 * std::pow (D, 14));
        }
        else if (bValue == 5)
        {
            pe = 1.0 / (2.0 * bValue) *
                 (92.0 * std::pow (D, 4.0)
                  + 528.0 * std::pow (D, 5.0)
                  + 8694.0 * std::pow (D, 6.0)
                  + 79453.0 * std::pow (D, 7.0)
                  + 792114.0 * std::pow (D, 8.0)
                  + 7375573.0 * std::pow (D, 9.0)
                  + 67884974.0 * std::pow (D, 10.0)
                  + 610875423.0 * std::pow (D, 11.0)
                  + 5427275376.0 * std::pow (D, 12.0)
                  + 47664215639.0 * std::pow (D, 13.0));
        }
        else
        {
            NS_ASSERT (false);
        }
        return pe;
    }

    double
    MmWaveNistErrorRateModel::GetFecQamBer (uint16_t constellationSize, double snr, uint64_t nbits, uint8_t bValue) const
    {
        NS_LOG_FUNCTION (this << constellationSize << snr << nbits << +bValue);
        double ber = GetQamBer (constellationSize, snr);
        if (ber == 0.0)
        {
            return 1.0;
        }
        double pe = CalculatePe (ber, bValue);
        pe = std::min (pe, 1.0);
        double pms = std::pow (1 - pe, nbits);
        return pms;
    }

    uint8_t
    MmWaveNistErrorRateModel::GetBValue (MmWaveCodeRate codeRate) const
    {
        switch (codeRate)
        {
            case MMWAVE_CODE_RATE_3_4:
                return 3;
            case MMWAVE_CODE_RATE_2_3:
                return 2;
            case MMWAVE_CODE_RATE_1_2:
                return 1;
            case MMWAVE_CODE_RATE_5_6:
                return 5;
            case MMWAVE_CODE_RATE_UNDEFINED:
            default:
                NS_FATAL_ERROR ("Unknown code rate");
                break;
        }
        return 0;
    }

    double
    MmWaveNistErrorRateModel::DoGetChunkSuccessRate (MmWaveMode mode, MmWaveTxVector txVector, double snr, uint64_t nbits) const
    {
        NS_LOG_FUNCTION (this << mode << snr << nbits);
        if (mode.GetModulationClass () == MMWAVE_MOD_CLASS_OFDM)
        {
            if (mode.GetConstellationSize () == 2)
            {
                return GetFecBpskBer (snr, nbits, GetBValue (mode.GetCodeRate ()));
            }
            else if (mode.GetConstellationSize () == 4)
            {
                return GetFecQpskBer (snr, nbits, GetBValue (mode.GetCodeRate ()));
            }
            else
            {
                return GetFecQamBer (mode.GetConstellationSize (), snr, nbits, GetBValue (mode.GetCodeRate ()));
            }
        }
        return 0;
    }

} //namespace ns3
