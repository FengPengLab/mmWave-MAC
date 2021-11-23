/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "mmwave-table-based-error-rate-model.h"
#include <cmath>
#include <algorithm>
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "mmwave-table-based-error-rate-model.h"
#include "mmwave-phy-error-rate-model.h"
#include "mmwave-tx-vector.h"

namespace ns3 {

    NS_OBJECT_ENSURE_REGISTERED (MmWaveTableBasedErrorRateModel);
    NS_LOG_COMPONENT_DEFINE ("MmWaveTableBasedErrorRateModel");

    TypeId
    MmWaveTableBasedErrorRateModel::GetTypeId ()
    {
        static TypeId tid = TypeId ("ns3::MmWaveTableBasedErrorRateModel")
                .SetParent<MmWaveErrorRateModel> ()
                .SetGroupName ("MmWave")
                .AddConstructor<MmWaveTableBasedErrorRateModel> ()
                .AddAttribute ("FallbackErrorRateModel",
                               "Ptr to the fallback error rate model to be used when no matching value is found in a table",
                               PointerValue (CreateObject<MmWavePhyErrorRateModel> ()),
                               MakePointerAccessor (&MmWaveTableBasedErrorRateModel::m_fallbackErrorModel),
                               MakePointerChecker <MmWaveErrorRateModel> ())
                .AddAttribute ("SizeThreshold",
                               "Threshold in bytes over which the table for large size frames is used",
                               UintegerValue (400),
                               MakeUintegerAccessor (&MmWaveTableBasedErrorRateModel::m_threshold),
                               MakeUintegerChecker<uint64_t> ())
        ;
        return tid;
    }

    MmWaveTableBasedErrorRateModel::MmWaveTableBasedErrorRateModel ()
    {
        NS_LOG_FUNCTION (this);
    }

    MmWaveTableBasedErrorRateModel::~MmWaveTableBasedErrorRateModel ()
    {
        NS_LOG_FUNCTION (this);
        m_fallbackErrorModel = 0;
    }


    double
    MmWaveTableBasedErrorRateModel::RoundSnr (double snr, double precision) const
    {
        NS_LOG_FUNCTION (this << snr);
        double multiplier = std::round (std::pow (10.0, precision));
        return std::floor (snr * multiplier + 0.5) / multiplier;
    }

    uint8_t
    MmWaveTableBasedErrorRateModel::GetMcsForMode (MmWaveMode mode)
    {
        uint8_t mcs = 0xff; // Initialize to invalid mcs
        mcs = mode.GetMcsValue ();
        NS_ABORT_MSG_IF (mcs == 0xff, "Error, MCS value for mode not found");
        return mcs;
    }

    double
    MmWaveTableBasedErrorRateModel::DoGetChunkSuccessRate (MmWaveMode mode, MmWaveTxVector txVector, double snr, uint64_t nbits) const
    {
        NS_LOG_FUNCTION (this << mode << txVector << snr << nbits);
        uint64_t size = std::max<uint64_t> (1, (nbits / 8));
        double roundedSnr = RoundSnr (RatioToDb (snr), SNR_PRECISION);
        uint8_t mcs = GetMcsForMode (mode);
        bool ldpc = txVector.IsLdpc ();
        NS_LOG_FUNCTION (this << +mcs << roundedSnr << size << ldpc);
        
        if (mcs > (ldpc ? MMWAVE_ERROR_TABLE_LDPC_MAX_NUM_MCS : MMWAVE_ERROR_TABLE_BCC_MAX_NUM_MCS))
        {
            NS_LOG_WARN ("Table missing for MCS: " << +mcs << " in MmWaveTableBasedErrorRateModel: use fallback error rate model");
            return m_fallbackErrorModel->GetChunkSuccessRate (mode, txVector, snr, nbits);
        }

        auto errorTable = (ldpc ? MmWaveAwgnErrorTableLdpc1458 : (size < m_threshold ? MmWaveAwgnErrorTableBcc32 : MmWaveAwgnErrorTableBcc1458));
        auto itVector = errorTable[mcs];
        auto itTable = std::find_if (itVector.begin(), itVector.end(),
                                     [&roundedSnr](const std::pair<double, double>& element) {
                                         return element.first == roundedSnr;
                                     });
        double minSnr = itVector.begin ()->first;
        double maxSnr = (--itVector.end ())->first;
        double per;
        if (itTable == itVector.end ())
        {
            if (roundedSnr < minSnr)
            {
                per = 1.0;
            }
            else if (roundedSnr > maxSnr)
            {
                per = 0.0;
            }
            else
            {
                double a = 0.0, b = 0.0, previousSnr = 0.0, nextSnr = 0.0;
                for (auto i = itVector.begin (); i != itVector.end (); ++i)
                {
                    if (i->first < roundedSnr)
                    {
                        previousSnr = i->first;
                        a = i->second;
                    }
                    else
                    {
                        nextSnr = i->first;
                        b = i->second;
                        break;
                    }
                }
                per = a + (roundedSnr - previousSnr) * (b - a) / (nextSnr - previousSnr);
            }
        }
        else
        {
            per = itTable->second;
        }

        uint16_t tableSize = (ldpc ? MMWAVE_ERROR_TABLE_LDPC_FRAME_SIZE : (size < m_threshold ? MMWAVE_ERROR_TABLE_BCC_SMALL_FRAME_SIZE : MMWAVE_ERROR_TABLE_BCC_LARGE_FRAME_SIZE));
        if (size != tableSize)
        {
            per = (1.0 - std::pow ((1 - per), (static_cast<double> (size) / tableSize)));
        }

        return 1.0 - per;
    }

    double
    MmWaveTableBasedErrorRateModel::RatioToDb (double ratio) const
    {
        return 10.0 * std::log10 (ratio);
    }

} //namespace ns3