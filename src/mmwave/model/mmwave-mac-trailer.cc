/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/object.h"
#include "mmwave-mac-trailer.h"

namespace ns3 {

    NS_OBJECT_ENSURE_REGISTERED (MmWaveMacTrailer);

    MmWaveMacTrailer::MmWaveMacTrailer ()
    {
    }

    MmWaveMacTrailer::~MmWaveMacTrailer ()
    {
    }

    TypeId
    MmWaveMacTrailer::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::MmWaveMacTrailer")
                .SetParent<Trailer> ()
                .SetGroupName ("MmWave")
                .AddConstructor<MmWaveMacTrailer> ()
        ;
        return tid;
    }

    TypeId
    MmWaveMacTrailer::GetInstanceTypeId (void) const
    {
        return GetTypeId ();
    }

    void
    MmWaveMacTrailer::Print (std::ostream &os) const
    {
    }

    uint32_t
    MmWaveMacTrailer::GetSerializedSize (void) const
    {
        return MMWAVE_MAC_FCS_LENGTH;
    }

    void
    MmWaveMacTrailer::Serialize (Buffer::Iterator start) const
    {
        start.Prev (MMWAVE_MAC_FCS_LENGTH);
        start.WriteU32 (0);
    }

    uint32_t
    MmWaveMacTrailer::Deserialize (Buffer::Iterator start)
    {
        return MMWAVE_MAC_FCS_LENGTH;
    }

} //namespace ns3