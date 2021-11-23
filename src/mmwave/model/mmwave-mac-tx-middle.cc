/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/log.h"
#include "mmwave-mac-tx-middle.h"
#include "mmwave-mac-header.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("MmWaveMacTxMiddle");

    MmWaveMacTxMiddle::MmWaveMacTxMiddle ()
            : m_sequence (0)
    {
        NS_LOG_FUNCTION (this);
    }

    MmWaveMacTxMiddle::~MmWaveMacTxMiddle ()
    {
        NS_LOG_FUNCTION (this);
    }

    uint16_t
    MmWaveMacTxMiddle::GetNextSequenceNumberFor (const MmWaveMacHeader *hdr)
    {
        NS_LOG_FUNCTION (this);
        uint16_t retval;
        retval = m_sequence;
        m_sequence++;
        m_sequence %= 4096;
        return retval;
    }

    uint16_t
    MmWaveMacTxMiddle::GetStartingSequenceNumber ()
    {
        NS_LOG_FUNCTION (this);
        uint16_t retval;
        retval = m_sequence;
        return retval;
    }

} //namespace ns3