/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_MAC_RX_MIDDLE_H
#define MMWAVE_MAC_RX_MIDDLE_H
#include <map>
#include "ns3/simple-ref-count.h"
#include "ns3/callback.h"
#include "ns3/ptr.h"
#include "ns3/callback.h"

namespace ns3 {
    class Packet;
    class Mac48Address;
    class MmWaveMacHeader;
    class MmWaveMacQueueItem;
    class MmWaveOriginatorRxStatus;

    class MmWaveMacRxMiddle : public SimpleRefCount<MmWaveMacRxMiddle>
    {
    public:
        typedef Callback<void, Ptr<MmWaveMacQueueItem>> ForwardUpCallback;

        MmWaveMacRxMiddle ();
        ~MmWaveMacRxMiddle ();

        void SetForwardCallback (ForwardUpCallback callback);
        void Receive (Ptr<MmWaveMacQueueItem> mpdu);
        
    private:
        MmWaveOriginatorRxStatus* Lookup (const MmWaveMacHeader* hdr);
        bool IsDuplicate (const MmWaveMacHeader* hdr, MmWaveOriginatorRxStatus *originator) const;
        Ptr<const Packet> HandleFragments (Ptr<const Packet> packet, const MmWaveMacHeader* hdr, MmWaveOriginatorRxStatus *originator);

        typedef std::map <Mac48Address, MmWaveOriginatorRxStatus *, std::less<Mac48Address> > Originators;
        typedef std::map <std::pair<Mac48Address, uint8_t>, MmWaveOriginatorRxStatus *, std::less<std::pair<Mac48Address,uint8_t> > > QosOriginators;
        typedef std::map <Mac48Address, MmWaveOriginatorRxStatus *, std::less<Mac48Address> >::iterator OriginatorsI;
        typedef std::map <std::pair<Mac48Address, uint8_t>, MmWaveOriginatorRxStatus *, std::less<std::pair<Mac48Address,uint8_t> > >::iterator QosOriginatorsI;

        Originators m_originatorStatus; ///< originator status
        ForwardUpCallback m_callback; ///< forward up callback
    };

} //namespace ns3

#endif //MMWAVE_MAC_RX_MIDDLE_H
