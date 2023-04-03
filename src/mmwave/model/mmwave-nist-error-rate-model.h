/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_NIST_ERROR_RATE_MODEL_H
#define MMWAVE_NIST_ERROR_RATE_MODEL_H
#include "mmwave-error-rate-model.h"
#include "mmwave-mode.h"

namespace ns3 {
 
    class MmWaveNistErrorRateModel : public MmWaveErrorRateModel
    {
    public:
        static TypeId GetTypeId ();
        MmWaveNistErrorRateModel ();

    private:
        double DoGetChunkSuccessRate (MmWaveMode mode, MmWaveTxVector txVector, double snr, uint64_t nbits) const;
        uint8_t GetBValue (MmWaveCodeRate codeRate) const;
        double CalculatePe (double p, uint8_t bValue) const;
        double GetBpskBer (double snr) const;
        double GetQpskBer (double snr) const;
        double GetQamBer (uint16_t constellationSize, double snr) const;
        double GetFecBpskBer (double snr, uint64_t nbits, uint8_t bValue) const;
        double GetFecQpskBer (double snr, uint64_t nbits, uint8_t bValue) const;
        double GetFecQamBer (uint16_t constellationSize, double snr, uint64_t nbits, uint8_t bValue) const;
    };

} //namespace ns3
#endif //MMWAVE_NIST_ERROR_RATE_MODEL_H
