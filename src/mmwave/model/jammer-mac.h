/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
#ifndef JAMMER_MAC_H
#define JAMMER_MAC_H
#include "ns3/object.h"
#include "ns3/net-device.h"
#include "ns3/mac48-address.h"
#include "ns3/nstime.h"
#include "jammer-phy.h"
#include "mmwave-tx-vector.h"

namespace ns3 {

    class JammerMac : public Object
    {
    public:
        static TypeId GetTypeId ();
        JammerMac();
         ~JammerMac();
        void DoDispose();
        void DoInitialize ();

        Ptr<NetDevice> GetDevice() const;
        Ptr<JammerPhy> GetPhy () const;
        Mac48Address GetAddress () const;
        void ResetPhy();
        void SetDevice(Ptr<NetDevice> device);
        void SetPhy (Ptr<JammerPhy> phy);
        void SetAddress (Mac48Address address);
        void SetInterval (Time interval);
        void SetPacketSize (uint32_t size);
        MmWaveTxVector GetTxVector ();
        void NotifyTxEnd ();
        void Send ();
    private:
        EventId m_send;
        Time m_jammerSignalStart;
        Time m_jammerSignalInterval;
        uint32_t m_jammerSignalSize;
        uint16_t m_guardInterval;
        uint8_t m_defaultTxPowerLevel;          //!< Default transmission power level
        Mac48Address m_self;
        MmWaveTxVector  m_txVector;
        Ptr<NetDevice> m_device;    ///< Pointer to the device
        Ptr<JammerPhy> m_phy;
    };
}
#endif //JAMMER_MAC_H