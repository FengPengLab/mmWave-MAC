/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
#ifndef CR_MMWAVE_HELPER_H
#define CR_MMWAVE_HELPER_H
#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/mmwave.h"
#include "ns3/mmwave-phy.h"
#include "ns3/mmwave-mode.h"
#include "ns3/spectrum-channel.h"
#include "ns3/trace-helper.h"
#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/object-factory.h"
#include "ns3/antenna-model.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/cr-mac.h"
namespace ns3 {
    class CrPhyHelper : public AsciiTraceHelperForDevice
    {
    public:
        CrPhyHelper ();
        ~CrPhyHelper ();
        static CrPhyHelper Default (Ptr<SpectrumChannel> spectrumChannel);
        static void AsciiPhyTxWithContext (Ptr<OutputStreamWrapper> stream, std::string context, Ptr<const Packet> p, MmWaveMode mode, MmWavePreamble preamble, uint8_t txLevel,
                                           uint8_t channelNum, uint16_t channelFreq, uint16_t channelWidth);
        static void AsciiPhyTxWithoutContext (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> p, MmWaveMode mode, MmWavePreamble preamble, uint8_t txLevel,
                                              uint8_t channelNum, uint16_t channelFreq, uint16_t channelWidth);
        static void AsciiPhyRxWithContext (Ptr<OutputStreamWrapper> stream, std::string context, Ptr<const Packet> p, double snr, MmWaveMode mode, enum MmWavePreamble preamble,
                                           uint8_t channelNum, uint16_t channelFreq, uint16_t channelWidth);
        static void AsciiPhyRxWithoutContext (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> p, double snr, MmWaveMode mode, enum MmWavePreamble preamble,
                                              uint8_t channelNum, uint16_t channelFreq, uint16_t channelWidth);

        void SetChannel (Ptr<SpectrumChannel> channel);
        void SetChannel (std::string channelName);
        Ptr<MmWavePhy> Create (Ptr<Node> node, Ptr<NetDevice> device) const;
        void Set (std::string name, const AttributeValue &v);
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
        void EnableAsciiInternal (Ptr<OutputStreamWrapper> stream, std::string prefix, Ptr<NetDevice> nd, bool explicitFilename);
    protected:
        Ptr<SpectrumChannel> m_channel;
        Ptr<AntennaModel> m_antenna;
        ObjectFactory m_phy; ///< PHY object
        ObjectFactory m_errorRateModel; ///< error rate model
        ObjectFactory m_frameCaptureModel; ///< frame capture model
        ObjectFactory m_preambleDetectionModel; ///< preamble detection model
    };

    class CrMacHelper
    {
    public:
        CrMacHelper ();
        ~CrMacHelper ();
        static CrMacHelper Default ();
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

        Ptr <CrMmWaveMac> Create (Ptr <NetDevice> device) const;

    protected:
        ObjectFactory m_mac;
    };

    class CrMmWaveHelper
    {
    public:
        CrMmWaveHelper ();
        ~CrMmWaveHelper ();

        static void EnableLogComponents ();
        void SetSatandard (MmWaveStandard standard);
        int64_t AssignStreams (NetDeviceContainer c, int64_t stream);

        NetDeviceContainer Install (const CrPhyHelper &phyHelper,
                                    const CrMacHelper &macHelper,
                                    NodeContainer::Iterator first,
                                    NodeContainer::Iterator last) const;

        NetDeviceContainer Install (const CrPhyHelper &phyHelper,
                                    const CrMacHelper &macHelper,
                                    NodeContainer c) const;

        NetDeviceContainer Install (const CrPhyHelper &phyHelper,
                                    const CrMacHelper &macHelper,
                                    Ptr<Node> node) const;

        NetDeviceContainer Install (const CrPhyHelper &phyHelper,
                                    const CrMacHelper &macHelper,
                                    std::string nodeName) const;

        void SetRemoteStationManager (std::string type,
                                     std::string n0 = "", const AttributeValue &v0 = EmptyAttributeValue(),
                                     std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue(),
                                     std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue(),
                                     std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue(),
                                     std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue(),
                                     std::string n5 = "", const AttributeValue &v5 = EmptyAttributeValue(),
                                     std::string n6 = "", const AttributeValue &v6 = EmptyAttributeValue(),
                                     std::string n7 = "", const AttributeValue &v7 = EmptyAttributeValue());

        void SetSpectrumRepository (std::string type,
                                    std::string n0 = "", const AttributeValue &v0 = EmptyAttributeValue(),
                                    std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue(),
                                    std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue(),
                                    std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue(),
                                    std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue(),
                                    std::string n5 = "", const AttributeValue &v5 = EmptyAttributeValue(),
                                    std::string n6 = "", const AttributeValue &v6 = EmptyAttributeValue(),
                                    std::string n7 = "", const AttributeValue &v7 = EmptyAttributeValue());
    private:
        MmWaveStandard m_standard;
        ObjectFactory m_stationManager;
        ObjectFactory m_repository;
    };

}
#endif //CR_MMWAVE_HELPER_H
