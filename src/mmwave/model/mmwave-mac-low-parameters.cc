/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "mmwave-mac-low-parameters.h"

namespace ns3 {

    MmWaveMacLowParameters::MmWaveMacLowParameters ()
            : m_waitAck (ACK_NONE),
              m_nextSize (0)
    {
    }

    void
    MmWaveMacLowParameters::EnableAck ()
    {
        m_waitAck = ACK_NORMAL;
    }

    void
    MmWaveMacLowParameters::DisableAck ()
    {
        m_waitAck = ACK_NONE;
    }

    void
    MmWaveMacLowParameters::EnableNextData (uint32_t size)
    {
        m_nextSize = size;
    }

    void
    MmWaveMacLowParameters::DisableNextData ()
    {
        m_nextSize = 0;
    }

    bool
    MmWaveMacLowParameters::MustWaitNormalAck () const
    {
        return (m_waitAck == ACK_NORMAL);
    }

    bool
    MmWaveMacLowParameters::HasNextPacket () const
    {
        return (m_nextSize != 0);
    }

    uint32_t
    MmWaveMacLowParameters::GetNextPacketSize () const
    {
        NS_ASSERT (HasNextPacket ());
        return m_nextSize;
    }

    std::ostream &operator << (std::ostream &os, const MmWaveMacLowParameters &params)
    {
        os << "["
           << "ack=";
        switch (params.m_waitAck)
        {
            case MmWaveMacLowParameters::ACK_NONE:
                os << "none";
                break;
            case MmWaveMacLowParameters::ACK_NORMAL:
                os << "normal";
                break;
        }
        os << "]";
        return os;
    }

} //namespace ns3