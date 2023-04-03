/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef V2X_MMWAVE_HELPER_H
#define V2X_MMWAVE_HELPER_H
#include "ns3/output-stream-wrapper.h"
#include "ns3/trace-helper.h"
#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/object-factory.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/channel-condition-model.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/spectrum-channel.h"
#include "ns3/antenna-model.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/friis-spectrum-propagation-loss.h"
#include "ns3/wifi-mode.h"
#include "ns3/wifi-preamble.h"
#include "ns3/wifi-phy.h"
#include "ns3/mmwave-mode.h"
#include "ns3/mmwave.h"
#include "ns3/mmwave-phy.h"
#include "ns3/v2x-data-mac.h"
#include "ns3/v2x-ctrl-mac.h"
#include "ns3/net-device.h"
namespace ns3 {
    class V2xCtrlPhyHelper : public AsciiTraceHelperForDevice
    {
    public:
        V2xCtrlPhyHelper ();
        ~V2xCtrlPhyHelper ();
        static V2xCtrlPhyHelper Default (Ptr<SpectrumChannel> spectrumChannel);
        static void AsciiPhyTransmitSinkWithContext (Ptr<OutputStreamWrapper> stream, std::string context, Ptr<const Packet> p, WifiMode mode, WifiPreamble preamble, uint8_t txLevel);
        static void AsciiPhyTransmitSinkWithoutContext (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> p, WifiMode mode, WifiPreamble preamble, uint8_t txLevel);
        static void AsciiPhyReceiveSinkWithContext (Ptr<OutputStreamWrapper> stream, std::string context, Ptr<const Packet> p, double snr, WifiMode mode, enum WifiPreamble preamble);
        static void AsciiPhyReceiveSinkWithoutContext (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> p, double snr, WifiMode mode, enum WifiPreamble preamble);

        void SetChannel (Ptr<SpectrumChannel> channel);
        void SetChannel (std::string channelName);
        void SetAntenna (Ptr<AntennaModel> antenna);
        Ptr<WifiPhy> Create (Ptr<Node> node, Ptr<NetDevice> device) const;
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
    protected:
        void EnableAsciiInternal (Ptr<OutputStreamWrapper> stream, std::string prefix, Ptr<NetDevice> nd, bool explicitFilename);
        Ptr<SpectrumChannel> m_channel;
        Ptr<AntennaModel> m_antenna;
        ObjectFactory m_phy; ///< PHY object
        ObjectFactory m_errorRateModel; ///< error rate model
        ObjectFactory m_frameCaptureModel; ///< frame capture model
        ObjectFactory m_preambleDetectionModel; ///< preamble detection model
    };

    class V2xDataPhyHelper : public AsciiTraceHelperForDevice
    {
    public:
        V2xDataPhyHelper ();
        ~V2xDataPhyHelper ();
        static V2xDataPhyHelper Default (Ptr<SpectrumChannel> spectrumChannel);
        static void AsciiPhyTransmitSinkWithContext (Ptr<OutputStreamWrapper> stream, std::string context, Ptr<const Packet> p, MmWaveMode mode, MmWavePreamble preamble, uint8_t txLevel,
                                                     uint8_t channelNum, uint16_t channelFreq, uint16_t channelWidth);
        static void AsciiPhyTransmitSinkWithoutContext (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> p, MmWaveMode mode, MmWavePreamble preamble, uint8_t txLevel,
                                                        uint8_t channelNum, uint16_t channelFreq, uint16_t channelWidth);
        static void AsciiPhyReceiveSinkWithContext (Ptr<OutputStreamWrapper> stream, std::string context, Ptr<const Packet> p, double snr, MmWaveMode mode, MmWavePreamble preamble,
                                                    uint8_t channelNum, uint16_t channelFreq, uint16_t channelWidth);
        static void AsciiPhyReceiveSinkWithoutContext (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> p, double snr, MmWaveMode mode, MmWavePreamble preamble,
                                                       uint8_t channelNum, uint16_t channelFreq, uint16_t channelWidth);

        void SetChannel (Ptr<SpectrumChannel> channel);
        void SetChannel (std::string channelName);
        Ptr<MmWavePhy> Create (Ptr<Node> node, Ptr<NetDevice> device) const;
        void Set (std::string name, const AttributeValue &v);
        void SetStandard (MmWaveStandard standard);
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
        MmWaveStandard m_standard;
        ObjectFactory m_phy; ///< PHY object
        ObjectFactory m_errorRateModel; ///< error rate model
        ObjectFactory m_frameCaptureModel; ///< frame capture model
        ObjectFactory m_preambleDetectionModel; ///< preamble detection model
    };

    class V2xCtrlMacHelper
    {
    public:
        V2xCtrlMacHelper ();
        ~V2xCtrlMacHelper ();
        static V2xCtrlMacHelper Default ();
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

        Ptr <V2xCtrlMac> Create (Ptr <NetDevice> device) const;

    protected:
        ObjectFactory m_mac; ///< MAC object factory
    };

    class V2xDataMacHelper
    {
    public:
        V2xDataMacHelper ();
        ~V2xDataMacHelper ();
        static V2xDataMacHelper Default ();
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

        Ptr <V2xDataMac> Create (Ptr <NetDevice> device) const;

    protected:
        ObjectFactory m_mac; ///< MAC object factory
    };

    class V2xMmWaveHelper
    {
    public:
        V2xMmWaveHelper ();
        ~V2xMmWaveHelper ();

        static void EnableLogComponents ();
        int64_t AssignStreams (NetDeviceContainer c, int64_t stream);
        void CreateMacForChannel (std::vector<uint32_t>  channelNumbers);
        void CreatePhys (uint32_t phys);
        NetDeviceContainer Install (const V2xCtrlPhyHelper &ctrlPhyHelper,
                                    const V2xCtrlMacHelper &ctrlMacHelper,
                                    const V2xDataPhyHelper &dataPhyHelper,
                                    const V2xDataMacHelper &dataMacHelper,
                                    NodeContainer::Iterator first,
                                    NodeContainer::Iterator last) const;

        NetDeviceContainer Install (const V2xCtrlPhyHelper &ctrlPhyHelper,
                                    const V2xCtrlMacHelper &ctrlMacHelper,
                                    const V2xDataPhyHelper &dataPhyHelper,
                                    const V2xDataMacHelper &dataMacHelper,
                                    NodeContainer c) const;

        NetDeviceContainer Install (const V2xCtrlPhyHelper &ctrlPhyHelper,
                                    const V2xCtrlMacHelper &ctrlMacHelper,
                                    const V2xDataPhyHelper &dataPhyHelper,
                                    const V2xDataMacHelper &dataMacHelper,
                                    Ptr<Node> node) const;

        NetDeviceContainer Install (const V2xCtrlPhyHelper &ctrlPhyHelper,
                                    const V2xCtrlMacHelper &ctrlMacHelper,
                                    const V2xDataPhyHelper &dataPhyHelper,
                                    const V2xDataMacHelper &dataMacHelper,
                                    std::string nodeName) const;

        void SetChannelScheduler (std::string type,
                                  std::string n0 = "", const AttributeValue &v0 = EmptyAttributeValue (),
                                  std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                                  std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                                  std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                                  std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue (),
                                  std::string n5 = "", const AttributeValue &v5 = EmptyAttributeValue (),
                                  std::string n6 = "", const AttributeValue &v6 = EmptyAttributeValue (),
                                  std::string n7 = "", const AttributeValue &v7 = EmptyAttributeValue ());

        void SetCtrlRemoteStationManager(std::string type,
                                         std::string n0 = "", const AttributeValue &v0 = EmptyAttributeValue(),
                                         std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue(),
                                         std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue(),
                                         std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue(),
                                         std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue(),
                                         std::string n5 = "", const AttributeValue &v5 = EmptyAttributeValue(),
                                         std::string n6 = "", const AttributeValue &v6 = EmptyAttributeValue(),
                                         std::string n7 = "", const AttributeValue &v7 = EmptyAttributeValue());

        void SetDataRemoteStationManager(std::string type,
                                         std::string n0 = "", const AttributeValue &v0 = EmptyAttributeValue(),
                                         std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue(),
                                         std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue(),
                                         std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue(),
                                         std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue(),
                                         std::string n5 = "", const AttributeValue &v5 = EmptyAttributeValue(),
                                         std::string n6 = "", const AttributeValue &v6 = EmptyAttributeValue(),
                                         std::string n7 = "", const AttributeValue &v7 = EmptyAttributeValue());

    private:
        ObjectFactory m_ctrlStationManager; ///< station manager
        ObjectFactory m_dataStationManager; ///< station manager
        ObjectFactory m_channelScheduler;
        std::vector<uint32_t> m_ctrlMacsForChannelNumber; ///< MACs for channel number
        uint32_t m_ctrlPhysNumber; ///< Phy number
    };
}

#endif /* V2X_MMWAVE_HELPER_H */

