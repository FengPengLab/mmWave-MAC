/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_MAC_LOW_PARAMETERS_H
#define MMWAVE_MAC_LOW_PARAMETERS_H
#include "ns3/uinteger.h"

namespace ns3 {
    
    class MmWaveMacLowParameters
    {
    public:
        MmWaveMacLowParameters ();
        void EnableAck ();
        void EnableNextData (uint32_t size);
        void DisableAck ();
        void DisableNextData ();
        bool MustWaitNormalAck () const;
        bool HasNextPacket () const;
        uint32_t GetNextPacketSize () const;
    private:
        friend std::ostream &operator << (std::ostream &os, const MmWaveMacLowParameters &params);
        enum
        {
            ACK_NONE,
            ACK_NORMAL
        } m_waitAck; //!< wait Ack

        uint32_t m_nextSize;
    };

    std::ostream &operator << (std::ostream &os, const MmWaveMacLowParameters &params);

} //namespace ns3
#endif //MMWAVE_MAC_LOW_PARAMETERS_H
