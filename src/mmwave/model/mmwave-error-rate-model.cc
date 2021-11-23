/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "mmwave-error-rate-model.h"
#include "mmwave-tx-vector.h"

namespace ns3 {

    NS_OBJECT_ENSURE_REGISTERED (MmWaveErrorRateModel);

    TypeId MmWaveErrorRateModel::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::MmWaveErrorRateModel")
                .SetParent<Object> ()
                .SetGroupName ("MmWave")
        ;
        return tid;
    }

    double
    MmWaveErrorRateModel::CalculateSnr (MmWaveTxVector txVector, double ber) const
    {
        double low, high, precision;
        low = 1e-25;
        high = 1e25;
        precision = 2e-12;
        while (high - low > precision)
        {
            NS_ASSERT (high >= low);
            double middle = low + (high - low) / 2;
            if ((1 - GetChunkSuccessRate (txVector.GetMode (), txVector, middle, 1)) > ber)
            {
                low = middle;
            }
            else
            {
                high = middle;
            }
        }
        return low;
    }

    double
    MmWaveErrorRateModel::GetChunkSuccessRate (MmWaveMode mode, MmWaveTxVector txVector, double snr, uint64_t nbits) const
    {
        return DoGetChunkSuccessRate (mode, txVector, snr, nbits);
    }

} //namespace ns3