/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_PHY_ERROR_RATE_MODEL_H
#define MMWAVE_PHY_ERROR_RATE_MODEL_H
#include "mmwave-error-rate-model.h"

namespace ns3 {

    class MmWavePhyErrorRateModel : public MmWaveErrorRateModel
    {
    public:
        static TypeId GetTypeId (void);
        MmWavePhyErrorRateModel ();
        
    private:
        double DoGetChunkSuccessRate (MmWaveMode mode, MmWaveTxVector txVector, double snr, uint64_t nbits) const;
        double GetBpskBer (double snr, uint32_t signalSpread, uint64_t phyRate) const;
        double GetQamBer (double snr, unsigned int m, uint32_t signalSpread, uint64_t phyRate) const;
        uint32_t Factorial (uint32_t k) const;
        double Binomial (uint32_t k, double p, uint32_t n) const;
        double CalculatePdOdd (double ber, unsigned int d) const;
        double CalculatePdEven (double ber, unsigned int d) const;
        double CalculatePd (double ber, unsigned int d) const;
        double GetFecBpskBer (double snr, uint64_t nbits, uint32_t signalSpread, uint64_t phyRate, uint32_t dFree, uint32_t adFree) const;
        double GetFecQamBer (double snr, uint64_t nbits, uint32_t signalSpread, uint64_t phyRate, uint32_t m, uint32_t dfree, uint32_t adFree, uint32_t adFreePlusOne) const;
    };

} //namespace ns3
#endif //MMWAVE_PHY_ERROR_RATE_MODEL_H
