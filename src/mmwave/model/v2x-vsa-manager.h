/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef V2X_VSA_MANAGER_H
#define V2X_VSA_MANAGER_H
#include <vector>
#include "ns3/vsa-manager.h"
#include "v2x-net-device.h"

namespace ns3 {

    class V2xVsaManager : public Object
    {
    public:
        static TypeId GetTypeId (void);
        V2xVsaManager (void);
        virtual ~V2xVsaManager (void);
        void SetV2xMmWaveNetDevice (Ptr<V2xMmWaveNetDevice> device);
        void SetWaveVsaCallback (Callback<bool, Ptr<const Packet>,const Address &, uint32_t, uint32_t>  vsaCallback);
        void SendVsa (const VsaInfo &vsaInfo);
        void RemoveAll (void);
        void RemoveByChannel (uint32_t channelNumber);
        void RemoveByOrganizationIdentifier (const OrganizationIdentifier &oi);
    private:
        void DoDispose (void);
        void DoInitialize (void);
        bool ReceiveVsc (Ptr<WifiMac> mac, const OrganizationIdentifier &oi, Ptr<const Packet> vsc, const Address &src);
        const static uint32_t VSA_REPEAT_PERIOD = 5;
        struct VsaWork
        {
            Mac48Address peer; ///< peer
            OrganizationIdentifier oi; ///< OI
            Ptr<Packet> vsc; ///< VSC
            uint32_t channelNumber; ///< channel number
            enum VsaTransmitInterval sentInterval; ///< VSA transmit interval
            Time repeatPeriod; ///< repeat period
            EventId repeat; ///< repeat ID
        };
        
        void DoRepeat (VsaWork *vsa);
        void DoSendVsa (enum VsaTransmitInterval  interval, uint32_t channel, Ptr<Packet> vsc, OrganizationIdentifier oi, Mac48Address peer);

        Callback<bool, Ptr<const Packet>,const Address &, uint32_t, uint32_t> m_vsaReceived; ///< VSA received callback
        std::vector<VsaWork *> m_vsas; ///< VSAs
        Ptr<V2xMmWaveNetDevice> m_device; ///< the device
    };

}
#endif //V2X_VSA_MANAGER_H
