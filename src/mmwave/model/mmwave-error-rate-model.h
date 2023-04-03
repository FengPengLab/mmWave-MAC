/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_ERROR_RATE_MODEL_H
#define MMWAVE_ERROR_RATE_MODEL_H
#include "ns3/object.h"

namespace ns3 {

    class MmWaveTxVector;
    class MmWaveMode;

    class MmWaveErrorRateModel : public Object
    {
    public:
        static TypeId GetTypeId (void);
        double CalculateSnr (MmWaveTxVector txVector, double ber) const;
        double GetChunkSuccessRate (MmWaveMode mode, MmWaveTxVector txVector, double snr, uint64_t nbits) const;

    private:
        virtual double DoGetChunkSuccessRate (MmWaveMode mode, MmWaveTxVector txVector, double snr, uint64_t nbits) const = 0;
    };

} //namespace ns3
#endif //MMWAVE_ERROR_RATE_MODEL_H
