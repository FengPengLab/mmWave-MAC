/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_MAC_TX_MIDDLE_H
#define MMWAVE_MAC_TX_MIDDLE_H
#include <map>
#include "ns3/simple-ref-count.h"

namespace ns3 {

    class MmWaveMacHeader;
    class Mac48Address;

    class MmWaveMacTxMiddle : public SimpleRefCount<MmWaveMacTxMiddle>
    {
    public:
        MmWaveMacTxMiddle ();
        ~MmWaveMacTxMiddle ();
        
        uint16_t GetNextSequenceNumberFor (const MmWaveMacHeader *hdr);
        uint16_t GetStartingSequenceNumber ();
    private:
        uint16_t m_sequence; ///< current sequence number
    };

} //namespace ns3
#endif //MMWAVE_MAC_TX_MIDDLE_H
