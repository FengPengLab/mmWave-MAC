/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/boolean.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/config.h"
#include "ns3/object.h"
#include "ns3/pointer.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/packet.h"
#include "ns3/object-base.h"
#include "ns3/node.h"
#include "ns3/object.h"
#include "ns3/mobility-model.h"
#include "ns3/net-device-queue-interface.h"
#include "ns3/mac48-address.h"
#include "ns3/net-device.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/three-gpp-v2v-channel-condition-model.h"
#include "ns3/three-gpp-v2v-propagation-loss-model.h"
#include "ns3/three-gpp-channel-model.h"
#include "ns3/three-gpp-spectrum-propagation-loss-model.h"
#include "ns3/three-gpp-propagation-loss-model.h"
#include "ns3/three-gpp-antenna-array-model.h"
#include "ns3/mmwave-error-rate-model.h"
#include "ns3/mmwave-frame-capture-model.h"
#include "ns3/mmwave-preamble-detection-model.h"
#include "ns3/jammer-net-device.h"
#include "jammer-mmwave-helper.h"

namespace ns3 {
    NS_LOG_COMPONENT_DEFINE ("JammerMmWaveHelper");

    JammerPhyHelper::JammerPhyHelper ()
            : m_channel (0),
              m_channelNumber (0),
              m_standard (MMWAVE_JAMMER_60GHz_1280MHz)
    {
        m_phy.SetTypeId ("ns3::JammerPhy");
    }

    JammerPhyHelper::~JammerPhyHelper ()
    {
    }

    JammerPhyHelper
    JammerPhyHelper::Default (Ptr<SpectrumChannel> spectrumChannel)
    {
        JammerPhyHelper helper;
        helper.SetChannel (spectrumChannel);
        helper.SetErrorRateModel ("ns3::MmWaveTableBasedErrorRateModel");
        helper.SetPreambleDetectionModel ("ns3::MmWaveThresholdPreambleDetectionModel");
        helper.SetFrameCaptureModel("ns3::MmWaveSimpleFrameCaptureModel");
        helper.Set ("Antennas", UintegerValue (2));
        helper.Set ("MaxSupportedTxSpatialStreams", UintegerValue (2));
        helper.Set ("MaxSupportedRxSpatialStreams", UintegerValue (2));
        helper.SetStandard (MMWAVE_JAMMER_60GHz_1280MHz);
        return helper;
    }

    Ptr<JammerPhy>
    JammerPhyHelper::Create (Ptr<Node> node, Ptr<NetDevice> device) const
    {
        Ptr<JammerPhy> phy = m_phy.Create<JammerPhy> ();
        phy->CreateMmWaveSpectrumPhyInterface (device);
        if (m_errorRateModel.IsTypeIdSet ())
        {
            Ptr<MmWaveErrorRateModel> error = m_errorRateModel.Create<MmWaveErrorRateModel> ();
            phy->SetErrorRateModel (error);
        }
        if (m_frameCaptureModel.IsTypeIdSet ())
        {
            Ptr<MmWaveFrameCaptureModel> capture = m_frameCaptureModel.Create<MmWaveFrameCaptureModel> ();
            phy->SetFrameCaptureModel (capture);
        }
        if (m_preambleDetectionModel.IsTypeIdSet ())
        {
            Ptr<MmWavePreambleDetectionModel> capture = m_preambleDetectionModel.Create<MmWavePreambleDetectionModel> ();
            phy->SetPreambleDetectionModel (capture);
        }
        phy->SetChannel (m_channel);
        phy->SetDevice (device);
        phy->SetMobility (node->GetObject<MobilityModel> ());
        phy->ConfigureStandardAndBand (mmWaveStandards.at(m_standard).m_phyStandard, mmWaveStandards.at(m_standard).m_phyBand);
        if (m_channelNumber != 0)
        {
            phy->SetChannelNumber (m_channelNumber);
        }
        return phy;
    }

    void
    JammerPhyHelper::Set (std::string name, const AttributeValue &v)
    {
        m_phy.Set (name, v);
    }

    void
    JammerPhyHelper::SetChannel (Ptr<SpectrumChannel> channel)
    {
        m_channel = channel;
    }

    void
    JammerPhyHelper::SetChannel (std::string channelName)
    {
        Ptr<SpectrumChannel> channel = Names::Find<SpectrumChannel> (channelName);
        m_channel = channel;
    }

    void
    JammerPhyHelper::SetStandard (MmWaveStandard standard)
    {
        m_standard = standard;
    }

    void
    JammerPhyHelper::SetChannelNumber (uint8_t channelNumber)
    {
        m_channelNumber = channelNumber;
    }

    void
    JammerPhyHelper::SetErrorRateModel (std::string name,
                                    std::string n0, const AttributeValue &v0,
                                    std::string n1, const AttributeValue &v1,
                                    std::string n2, const AttributeValue &v2,
                                    std::string n3, const AttributeValue &v3,
                                    std::string n4, const AttributeValue &v4,
                                    std::string n5, const AttributeValue &v5,
                                    std::string n6, const AttributeValue &v6,
                                    std::string n7, const AttributeValue &v7)
    {
        m_errorRateModel = ObjectFactory ();
        m_errorRateModel.SetTypeId (name);
        m_errorRateModel.Set (n0, v0);
        m_errorRateModel.Set (n1, v1);
        m_errorRateModel.Set (n2, v2);
        m_errorRateModel.Set (n3, v3);
        m_errorRateModel.Set (n4, v4);
        m_errorRateModel.Set (n5, v5);
        m_errorRateModel.Set (n6, v6);
        m_errorRateModel.Set (n7, v7);
    }

    void
    JammerPhyHelper::SetFrameCaptureModel (std::string name,
                                       std::string n0, const AttributeValue &v0,
                                       std::string n1, const AttributeValue &v1,
                                       std::string n2, const AttributeValue &v2,
                                       std::string n3, const AttributeValue &v3,
                                       std::string n4, const AttributeValue &v4,
                                       std::string n5, const AttributeValue &v5,
                                       std::string n6, const AttributeValue &v6,
                                       std::string n7, const AttributeValue &v7)
    {
        m_frameCaptureModel = ObjectFactory ();
        m_frameCaptureModel.SetTypeId (name);
        m_frameCaptureModel.Set (n0, v0);
        m_frameCaptureModel.Set (n1, v1);
        m_frameCaptureModel.Set (n2, v2);
        m_frameCaptureModel.Set (n3, v3);
        m_frameCaptureModel.Set (n4, v4);
        m_frameCaptureModel.Set (n5, v5);
        m_frameCaptureModel.Set (n6, v6);
        m_frameCaptureModel.Set (n7, v7);
    }

    void
    JammerPhyHelper::SetPreambleDetectionModel (std::string name,
                                            std::string n0, const AttributeValue &v0,
                                            std::string n1, const AttributeValue &v1,
                                            std::string n2, const AttributeValue &v2,
                                            std::string n3, const AttributeValue &v3,
                                            std::string n4, const AttributeValue &v4,
                                            std::string n5, const AttributeValue &v5,
                                            std::string n6, const AttributeValue &v6,
                                            std::string n7, const AttributeValue &v7)
    {
        m_preambleDetectionModel = ObjectFactory ();
        m_preambleDetectionModel.SetTypeId (name);
        m_preambleDetectionModel.Set (n0, v0);
        m_preambleDetectionModel.Set (n1, v1);
        m_preambleDetectionModel.Set (n2, v2);
        m_preambleDetectionModel.Set (n3, v3);
        m_preambleDetectionModel.Set (n4, v4);
        m_preambleDetectionModel.Set (n5, v5);
        m_preambleDetectionModel.Set (n6, v6);
        m_preambleDetectionModel.Set (n7, v7);
    }

    JammerMacHelper::JammerMacHelper ()
    {
        SetType ("ns3::JammerMac");
    }

    JammerMacHelper::~JammerMacHelper ()
    {
    }

    JammerMacHelper
    JammerMacHelper::Default ()
    {
        JammerMacHelper helper;
        return helper;
    }

    Ptr<JammerMac>
    JammerMacHelper::Create (Ptr<NetDevice> device) const
    {
        Ptr<JammerMac> mac = m_mac.Create<JammerMac> ();
        mac->SetDevice (device);
        return mac;
    }

    void
    JammerMacHelper::SetType (std::string type,
                          std::string n0, const AttributeValue &v0,
                          std::string n1, const AttributeValue &v1,
                          std::string n2, const AttributeValue &v2,
                          std::string n3, const AttributeValue &v3,
                          std::string n4, const AttributeValue &v4,
                          std::string n5, const AttributeValue &v5,
                          std::string n6, const AttributeValue &v6,
                          std::string n7, const AttributeValue &v7,
                          std::string n8, const AttributeValue &v8,
                          std::string n9, const AttributeValue &v9,
                          std::string n10, const AttributeValue &v10)
    {
        m_mac.SetTypeId (type);
        m_mac.Set (n0, v0);
        m_mac.Set (n1, v1);
        m_mac.Set (n2, v2);
        m_mac.Set (n3, v3);
        m_mac.Set (n4, v4);
        m_mac.Set (n5, v5);
        m_mac.Set (n6, v6);
        m_mac.Set (n7, v7);
        m_mac.Set (n8, v8);
        m_mac.Set (n9, v9);
        m_mac.Set (n10, v10);
    }

    JammerMmWaveHelper::JammerMmWaveHelper ()
    {
        SetRemoteStationManager("ns3::MmWaveConstantRateManager",
                                "ControlMode", StringValue("MmWaveMcs0"),
                                "DataMode", StringValue("MmWaveMcs0"));
    }

    JammerMmWaveHelper::~JammerMmWaveHelper ()
    {
    }

    NetDeviceContainer
    JammerMmWaveHelper::Install (const JammerPhyHelper &phyHelper,
                             const JammerMacHelper &macHelper,
                             NodeContainer::Iterator first,
                             NodeContainer::Iterator last) const
    {
        NetDeviceContainer devices;
        uint32_t index = 0;
        for (NodeContainer::Iterator i = first; i != last; ++i)
        {
            index++;
            Ptr<Node> node = *i;
            Ptr<JammerNetDevice> device = CreateObject<JammerNetDevice> ();
            Ptr<JammerMac> mac = macHelper.Create (device);
            Ptr<JammerPhy> phy = phyHelper.Create (node, device);
            device->SetMac (mac);
            device->SetPhy (phy);
            node->AddDevice (device);
            devices.Add (device);
        }
        return devices;
    }

    NetDeviceContainer
    JammerMmWaveHelper::Install (const JammerPhyHelper &phyHelper,
                             const JammerMacHelper &macHelper,
                             NodeContainer c) const
    {
        return Install (phyHelper, macHelper, c.Begin (), c.End ());
    }

    NetDeviceContainer
    JammerMmWaveHelper::Install (const JammerPhyHelper &phyHelper,
                             const JammerMacHelper &macHelper,
                             Ptr<Node> node) const
    {
        return Install (phyHelper, macHelper, NodeContainer (node));
    }

    NetDeviceContainer
    JammerMmWaveHelper::Install (const JammerPhyHelper &phyHelper,
                             const JammerMacHelper &macHelper,
                             std::string nodeName) const
    {
        Ptr<Node> node = Names::Find<Node> (nodeName);
        return Install (phyHelper, macHelper, NodeContainer (node));
    }

    int64_t
    JammerMmWaveHelper::AssignStreams (NetDeviceContainer c, int64_t stream)
    {
        int64_t currentStream = stream;
        Ptr<NetDevice> netDevice;

        for (NetDeviceContainer::Iterator i = c.Begin (); i != c.End (); ++i)
        {
            netDevice = (*i);
            Ptr<JammerNetDevice> mmWave = DynamicCast<JammerNetDevice> (netDevice);
            Ptr<JammerPhy> phy = mmWave->GetPhy ();
            currentStream += phy->AssignStreams (currentStream);
        }
        return (currentStream - stream);
    }

    void
    JammerMmWaveHelper::EnableLogComponents ()
    {
        LogComponentEnable ("JammerPhy", LOG_LEVEL_ALL);
        LogComponentEnable ("JammerPhy", LOG_PREFIX_TIME);
        LogComponentEnable ("JammerPhy", LOG_PREFIX_NODE);

        LogComponentEnable ("JammerMac", LOG_LEVEL_ALL);
        LogComponentEnable ("JammerMac", LOG_PREFIX_TIME);
        LogComponentEnable ("JammerMac", LOG_PREFIX_NODE);

        LogComponentEnable ("JammerNetDevice", LOG_LEVEL_ALL);
        LogComponentEnable ("JammerNetDevice", LOG_PREFIX_TIME);
        LogComponentEnable ("JammerNetDevice", LOG_PREFIX_NODE);
    }

    void
    JammerMmWaveHelper::SetRemoteStationManager (std::string type,
                                                 std::string n0, const AttributeValue &v0,
                                                 std::string n1, const AttributeValue &v1,
                                                 std::string n2, const AttributeValue &v2,
                                                 std::string n3, const AttributeValue &v3,
                                                 std::string n4, const AttributeValue &v4,
                                                 std::string n5, const AttributeValue &v5,
                                                 std::string n6, const AttributeValue &v6,
                                                 std::string n7, const AttributeValue &v7)
    {
        m_stationManager = ObjectFactory ();
        m_stationManager.SetTypeId (type);
        m_stationManager.Set (n0, v0);
        m_stationManager.Set (n1, v1);
        m_stationManager.Set (n2, v2);
        m_stationManager.Set (n3, v3);
        m_stationManager.Set (n4, v4);
        m_stationManager.Set (n5, v5);
        m_stationManager.Set (n6, v6);
        m_stationManager.Set (n7, v7);
    }
}
