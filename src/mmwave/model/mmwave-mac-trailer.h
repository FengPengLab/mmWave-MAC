/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_MAC_TRAILER_H
#define MMWAVE_MAC_TRAILER_H
#include "ns3/trailer.h"
#include "ns3/type-id.h"
#include "mmwave.h"

namespace ns3 {

    class MmWaveMacTrailer : public Trailer
    {
    public:
        MmWaveMacTrailer ();
        virtual ~MmWaveMacTrailer ();

        static TypeId GetTypeId (void);
        TypeId GetInstanceTypeId (void) const;
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator start) const;
        uint32_t Deserialize (Buffer::Iterator start);
    };

} //namespace ns3

#endif //MMWAVE_MAC_TRAILER_H
