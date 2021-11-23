/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MMWAVE_PSDU_H
#define MMWAVE_PSDU_H
#include <vector>
#include <set>
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/mac48-address.h"
#include "mmwave-mac-header.h"
#include "mmwave-mac-queue-item.h"
namespace ns3 {

    class MmWavePsdu : public SimpleRefCount<MmWavePsdu> {
    public:

        MmWavePsdu(Ptr<const Packet> p, const MmWaveMacHeader &header);
        virtual ~MmWavePsdu();
        Ptr<const Packet> GetPacket() const;
        const MmWaveMacHeader &GetHeader() const;
        MmWaveMacHeader &GetHeader();
        Ptr<const Packet> GetPayload() const;
        Time GetTimeStamp() const;
        Mac48Address GetAddr1() const;
        Mac48Address GetAddr2() const;
        Time GetDuration() const;
        void SetDuration(Time duration);
        uint32_t GetSize() const;
        Ptr<MmWaveMacQueueItem> GetMpdu () const;
        void AddMmWaveMacTrailer (Ptr<Packet> packet) const;
        void Print(std::ostream &os) const;

    private:
        Ptr<MmWaveMacQueueItem> m_mpdu;
        uint32_t m_size;
    };

    std::ostream &operator<<(std::ostream &os, const MmWavePsdu &psdu);
}
#endif //MMWAVE_PSDU_H