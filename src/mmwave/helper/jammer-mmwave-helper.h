/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
#ifndef JAMMER_MMWAVE_HELPER_H
#define JAMMER_MMWAVE_HELPER_H
#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/spectrum-channel.h"
#include "ns3/trace-helper.h"
#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/object-factory.h"
#include "ns3/antenna-model.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/mmwave.h"
#include "ns3/mmwave-phy.h"
#include "ns3/mmwave-mode.h"
#include "ns3/jammer-mac.h"
#include "ns3/jammer-phy.h"
namespace ns3 {
    class JammerPhyHelper
    {
    public:
        JammerPhyHelper ();
        ~JammerPhyHelper ();
        static JammerPhyHelper Default (Ptr<SpectrumChannel> spectrumChannel);

        void SetChannel (Ptr<SpectrumChannel> channel);
        void SetChannel (std::string channelName);
        Ptr<JammerPhy> Create (Ptr<Node> node, Ptr<NetDevice> device) const;
        void Set (std::string name, const AttributeValue &v);
        void SetStandard (MmWaveStandard standard);
        void SetChannelNumber (uint8_t channelNumber);
        void SetErrorRateModel (std::string name,
                                std::string n0 = "", const AttributeValue &v0 = EmptyAttributeValue (),
                                std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                                std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                                std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                                std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue (),
                                std::string n5 = "", const AttributeValue &v5 = EmptyAttributeValue (),
                                std::string n6 = "", const AttributeValue &v6 = EmptyAttributeValue (),
                                std::string n7 = "", const AttributeValue &v7 = EmptyAttributeValue ());

        void SetFrameCaptureModel (std::string name,
                                   std::string n0 = "", const AttributeValue &v0 = EmptyAttributeValue (),
                                   std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                                   std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                                   std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                                   std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue (),
                                   std::string n5 = "", const AttributeValue &v5 = EmptyAttributeValue (),
                                   std::string n6 = "", const AttributeValue &v6 = EmptyAttributeValue (),
                                   std::string n7 = "", const AttributeValue &v7 = EmptyAttributeValue ());

        void SetPreambleDetectionModel (std::string name,
                                        std::string n0 = "", const AttributeValue &v0 = EmptyAttributeValue (),
                                        std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                                        std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                                        std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                                        std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue (),
                                        std::string n5 = "", const AttributeValue &v5 = EmptyAttributeValue (),
                                        std::string n6 = "", const AttributeValue &v6 = EmptyAttributeValue (),
                                        std::string n7 = "", const AttributeValue &v7 = EmptyAttributeValue ());

    protected:
        Ptr<SpectrumChannel> m_channel;
        Ptr<AntennaModel> m_antenna;
        uint8_t m_channelNumber;
        MmWaveStandard m_standard;
        ObjectFactory m_phy; ///< PHY object
        ObjectFactory m_errorRateModel; ///< error rate model
        ObjectFactory m_frameCaptureModel; ///< frame capture model
        ObjectFactory m_preambleDetectionModel; ///< preamble detection model
    };

    class JammerMacHelper
    {
    public:
        JammerMacHelper ();
        ~JammerMacHelper ();
        static JammerMacHelper Default ();
        void SetType (std::string type,
                      std::string n0 = "", const AttributeValue &v0 = EmptyAttributeValue (),
                      std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                      std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                      std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                      std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue (),
                      std::string n5 = "", const AttributeValue &v5 = EmptyAttributeValue (),
                      std::string n6 = "", const AttributeValue &v6 = EmptyAttributeValue (),
                      std::string n7 = "", const AttributeValue &v7 = EmptyAttributeValue (),
                      std::string n8 = "", const AttributeValue &v8 = EmptyAttributeValue (),
                      std::string n9 = "", const AttributeValue &v9 = EmptyAttributeValue (),
                      std::string n10 = "", const AttributeValue &v10 = EmptyAttributeValue ());

        Ptr <JammerMac> Create (Ptr <NetDevice> device) const;

    protected:
        ObjectFactory m_mac;
    };

    class JammerMmWaveHelper
    {
    public:
        JammerMmWaveHelper ();
        ~JammerMmWaveHelper ();
        int64_t AssignStreams (NetDeviceContainer c, int64_t stream);
        static void EnableLogComponents ();
        NetDeviceContainer Install (const JammerPhyHelper &phyHelper,
                                    const JammerMacHelper &macHelper,
                                    NodeContainer::Iterator first,
                                    NodeContainer::Iterator last) const;

        NetDeviceContainer Install (const JammerPhyHelper &phyHelper,
                                    const JammerMacHelper &macHelper,
                                    NodeContainer c) const;

        NetDeviceContainer Install (const JammerPhyHelper &phyHelper,
                                    const JammerMacHelper &macHelper,
                                    Ptr<Node> node) const;

        NetDeviceContainer Install (const JammerPhyHelper &phyHelper,
                                    const JammerMacHelper &macHelper,
                                    std::string nodeName) const;

        void SetRemoteStationManager(std::string type,
                                     std::string n0 = "", const AttributeValue &v0 = EmptyAttributeValue(),
                                     std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue(),
                                     std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue(),
                                     std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue(),
                                     std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue(),
                                     std::string n5 = "", const AttributeValue &v5 = EmptyAttributeValue(),
                                     std::string n6 = "", const AttributeValue &v6 = EmptyAttributeValue(),
                                     std::string n7 = "", const AttributeValue &v7 = EmptyAttributeValue());

    private:
        ObjectFactory m_stationManager;
        ObjectFactory m_repository;
    };

}
#endif //JAMMER_MMWAVE_HELPER_H
