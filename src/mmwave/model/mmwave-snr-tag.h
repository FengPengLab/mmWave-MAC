/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_SNR_TAG_H
#define MMWAVE_SNR_TAG_H
#include "ns3/tag.h"
namespace ns3 {

    class MmWaveSnrTag : public Tag
    {
    public:
        static TypeId GetTypeId ();
        TypeId GetInstanceTypeId () const;

        MmWaveSnrTag ();

        uint32_t GetSerializedSize () const;
        void Serialize (TagBuffer i) const;
        void Deserialize (TagBuffer i);
        void Print (std::ostream &os) const;

        void Set (double snr);
        double Get () const;

    private:
        double m_snr;  //!< SNR value in linear scale
    };

}
#endif //MMWAVE_SNR_TAG_H
