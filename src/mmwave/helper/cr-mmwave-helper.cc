/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
#include <iostream>
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
#include "ns3/mac48-address.h"
#include "ns3/net-device.h"
#include "ns3/net-device-queue-interface.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/mmwave-spectrum-phy.h"
#include "ns3/mmwave-error-rate-model.h"
#include "ns3/mmwave-frame-capture-model.h"
#include "ns3/mmwave-preamble-detection-model.h"
#include "ns3/mmwave-spectrum-repository.h"
#include "ns3/cr-net-device.h"
#include "ns3/cr-mac-low.h"
#include "ns3/cr-mac.h"
#include "ns3/cr-net-device.h"
#include "ns3/mmwave-phy.h"
#include "cr-mmwave-helper.h"

namespace ns3 {
    NS_LOG_COMPONENT_DEFINE ("CrMmWaveHelper");

    CrPhyHelper::CrPhyHelper ()
            : m_channel (0)
    {
        m_phy.SetTypeId ("ns3::MmWaveSpectrumPhy");
    }

    CrPhyHelper::~CrPhyHelper ()
    {
    }

    CrPhyHelper
    CrPhyHelper::Default (Ptr<SpectrumChannel> spectrumChannel)
    {
        CrPhyHelper helper;
        helper.SetChannel (spectrumChannel);
        helper.SetErrorRateModel ("ns3::MmWaveTableBasedErrorRateModel");
        helper.SetPreambleDetectionModel ("ns3::MmWaveThresholdPreambleDetectionModel");
        helper.SetFrameCaptureModel ("ns3::MmWaveSimpleFrameCaptureModel");
        helper.Set ("Antennas", UintegerValue (4));
        helper.Set ("MaxSupportedTxSpatialStreams", UintegerValue (4));
        helper.Set ("MaxSupportedRxSpatialStreams", UintegerValue (4));

        return helper;
    }

    Ptr<MmWavePhy>
    CrPhyHelper::Create (Ptr<Node> node, Ptr<NetDevice> device) const
    {
        Ptr<MmWaveSpectrumPhy> phy = m_phy.Create<MmWaveSpectrumPhy> ();
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

        return phy;
    }

    void
    CrPhyHelper::AsciiPhyTxWithContext (Ptr<OutputStreamWrapper> stream,
                                        std::string context,
                                        Ptr<const Packet> p,
                                        MmWaveMode mode,
                                        MmWavePreamble preamble,
                                        uint8_t txLevel,
                                        uint8_t channelNum,
                                        uint16_t channelFreq,
                                        uint16_t channelWidth)
    {
        NS_LOG_FUNCTION (stream << context << p << mode << preamble << txLevel);
        *stream->GetStream () << "t " << Simulator::Now ().GetSeconds () << " "  << context << " " << mode << " Channel:( " << +channelNum << " " << channelFreq << " " << channelWidth << " ) " << *p << std::endl;
    }

    void
    CrPhyHelper::AsciiPhyTxWithoutContext (Ptr<OutputStreamWrapper> stream,
                                           Ptr<const Packet> p,
                                           MmWaveMode mode,
                                           MmWavePreamble preamble,
                                           uint8_t txLevel,
                                           uint8_t channelNum,
                                           uint16_t channelFreq,
                                           uint16_t channelWidth)
    {
        NS_LOG_FUNCTION (stream << p << mode << preamble << txLevel);
        *stream->GetStream () << "t " << Simulator::Now ().GetSeconds () << " " << mode << " Channel:( " << channelNum << " " << +channelFreq << " " << channelWidth << " ) " << *p << std::endl;
    }

    void
    CrPhyHelper::AsciiPhyRxWithContext (Ptr<OutputStreamWrapper> stream,
                                        std::string context,
                                        Ptr<const Packet> p,
                                        double snr,
                                        MmWaveMode mode,
                                        MmWavePreamble preamble,
                                        uint8_t channelNum,
                                        uint16_t channelFreq,
                                        uint16_t channelWidth)
    {
        NS_LOG_FUNCTION (stream << context << p << snr << mode << preamble);
        *stream->GetStream () << "r " << Simulator::Now ().GetSeconds () << " " << context << " " << mode << " Channel:( " << +channelNum << " " << channelFreq << " " << channelWidth << " ) " << *p << std::endl;
    }

    void
    CrPhyHelper::AsciiPhyRxWithoutContext (Ptr<OutputStreamWrapper> stream,
                                           Ptr<const Packet> p,
                                           double snr,
                                           MmWaveMode mode,
                                           MmWavePreamble preamble,
                                           uint8_t channelNum,
                                           uint16_t channelFreq,
                                           uint16_t channelWidth)
    {
        NS_LOG_FUNCTION (stream << p << snr << mode << preamble);
        *stream->GetStream () << "r " << Simulator::Now ().GetSeconds () << " " << mode << " Channel:( " << +channelNum << " " << channelFreq << " " << channelWidth << " ) " << *p << std::endl;
    }

    void
    CrPhyHelper::Set (std::string name, const AttributeValue &v)
    {
        m_phy.Set (name, v);
    }

    void
    CrPhyHelper::SetChannel (Ptr<SpectrumChannel> channel)
    {
        m_channel = channel;
    }

    void
    CrPhyHelper::SetChannel (std::string channelName)
    {
        Ptr<SpectrumChannel> channel = Names::Find<SpectrumChannel> (channelName);
        m_channel = channel;
    }

    void
    CrPhyHelper::SetErrorRateModel (std::string name,
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
    CrPhyHelper::SetFrameCaptureModel (std::string name,
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
    CrPhyHelper::SetPreambleDetectionModel (std::string name,
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

    void
    CrPhyHelper::EnableAsciiInternal (Ptr<OutputStreamWrapper> stream,
                                      std::string prefix,
                                      Ptr<NetDevice> nd,
                                      bool explicitFilename)
    {
        Ptr<CrMmWaveNetDevice> device = nd->GetObject<CrMmWaveNetDevice> ();
        if (device == 0)
        {
            NS_LOG_INFO ("CrMmWaveHelper::EnableAsciiInternal(): Device " << device << " not of type ns3::CrMmWaveNetDevice");
            return;
        }

        Packet::EnablePrinting ();
        uint32_t nodeid = nd->GetNode ()->GetId ();
        uint32_t deviceid = nd->GetIfIndex ();
        std::ostringstream oss;

        if (stream == 0)
        {
            AsciiTraceHelper asciiTraceHelper;

            std::string filename;
            if (explicitFilename)
            {
                filename = prefix;
            }
            else
            {
                filename = asciiTraceHelper.GetFilenameFromDevice (prefix, device);
            }

            Ptr<OutputStreamWrapper> theStream = asciiTraceHelper.CreateFileStream (filename);
            oss.str ("");
            oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::CrMmWaveNetDevice/PhyIntraGroup/State/RxOk";
            Config::ConnectWithoutContext (oss.str (), MakeBoundCallback (&AsciiPhyRxWithoutContext, theStream));

            oss.str ("");
            oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::CrMmWaveNetDevice/PhyIntraGroup/State/Tx";
            Config::ConnectWithoutContext (oss.str (), MakeBoundCallback (&AsciiPhyTxWithoutContext, theStream));

            oss.str ("");
            oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::CrMmWaveNetDevice/PhyInterGroup/State/Tx";
            Config::ConnectWithoutContext (oss.str (), MakeBoundCallback (&AsciiPhyTxWithoutContext, theStream));

            return;
        }

        oss.str ("");
        oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::CrMmWaveNetDevice/PhyIntraGroup/State/RxOk";
        Config::Connect (oss.str (), MakeBoundCallback (&AsciiPhyRxWithContext, stream));

        oss.str ("");
        oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::CrMmWaveNetDevice/PhyIntraGroup/State/Tx";
        Config::Connect (oss.str (), MakeBoundCallback (&AsciiPhyTxWithContext, stream));

        oss.str ("");
        oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::CrMmWaveNetDevice/PhyInterGroup/State/Tx";
        Config::Connect (oss.str (), MakeBoundCallback (&AsciiPhyTxWithContext, stream));
    }

    CrMacHelper::CrMacHelper ()
    {
        SetType ("ns3::CrMmWaveMac");
    }

    CrMacHelper::~CrMacHelper ()
    {
    }

    CrMacHelper
    CrMacHelper::Default ()
    {
        CrMacHelper helper;
        return helper;
    }

    Ptr<CrMmWaveMac>
    CrMacHelper::Create (Ptr<NetDevice> device) const
    {
        Ptr<CrMmWaveMac> mac = m_mac.Create<CrMmWaveMac> ();
        mac->SetDevice (device);
        return mac;
    }

    void
    CrMacHelper::SetType (std::string type,
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

    CrMmWaveHelper::CrMmWaveHelper ()
    {
        SetRemoteStationManager("ns3::MmWaveConstantRateManager",
                                "ControlMode", StringValue("MmWaveMcs1"),
                                "DataMode", StringValue("MmWaveMcs1"));
        SetSpectrumRepository ("ns3::MmWaveSpectrumRepository",
                               "MaxRadius", DoubleValue (50),
                               "MaxSize", IntegerValue (1000),
                               "MaxDelay", TimeValue (Seconds (5)),
                               "ThresholdForPUsActivity", DoubleValue (0.1),
                               "ThresholdForSUsActivity", DoubleValue (0.1));
        m_standard = MMWAVE_COGNITIVE_RADIO_60GHz_1C_1280MHz;
    }

    CrMmWaveHelper::~CrMmWaveHelper ()
    {
    }

    void
    CrMmWaveHelper::SetSatandard (MmWaveStandard standard)
    {
        m_standard = standard;
    }

    void
    CrMmWaveHelper::SetRemoteStationManager (std::string type,
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

    void
    CrMmWaveHelper::SetSpectrumRepository (std::string type,
                                           std::string n0, const AttributeValue &v0,
                                           std::string n1, const AttributeValue &v1,
                                           std::string n2, const AttributeValue &v2,
                                           std::string n3, const AttributeValue &v3,
                                           std::string n4, const AttributeValue &v4,
                                           std::string n5, const AttributeValue &v5,
                                           std::string n6, const AttributeValue &v6,
                                           std::string n7, const AttributeValue &v7)
    {
        m_repository = ObjectFactory ();
        m_repository.SetTypeId (type);
        m_repository.Set (n0, v0);
        m_repository.Set (n1, v1);
        m_repository.Set (n2, v2);
        m_repository.Set (n3, v3);
        m_repository.Set (n4, v4);
        m_repository.Set (n5, v5);
        m_repository.Set (n6, v6);
        m_repository.Set (n7, v7);
    }

    void
    CrMmWaveHelper::EnableLogComponents ()
    {
//        LogComponentEnable ("MmWaveInterferenceHelper", LOG_LEVEL_ALL);
//        LogComponentEnable ("MmWaveInterferenceHelper", LOG_PREFIX_TIME);
//        LogComponentEnable ("MmWaveInterferenceHelper", LOG_PREFIX_NODE);

//        LogComponentEnable ("MmWaveSpectrumPhy", LOG_LEVEL_ALL);
//        LogComponentEnable ("MmWaveSpectrumPhy", LOG_PREFIX_TIME);
//        LogComponentEnable ("MmWaveSpectrumPhy", LOG_PREFIX_NODE);
//
//        LogComponentEnable ("MmWavePhy", LOG_LEVEL_ALL);
//        LogComponentEnable ("MmWavePhy", LOG_PREFIX_TIME);
//        LogComponentEnable ("MmWavePhy", LOG_PREFIX_NODE);

//        LogComponentEnable ("CrMmWaveTxop", LOG_LEVEL_ALL);
//        LogComponentEnable ("CrMmWaveTxop", LOG_PREFIX_TIME);
//        LogComponentEnable ("CrMmWaveTxop", LOG_PREFIX_NODE);

        LogComponentEnable ("CrMmWaveMac", LOG_LEVEL_ALL);
        LogComponentEnable ("CrMmWaveMac", LOG_PREFIX_TIME);
        LogComponentEnable ("CrMmWaveMac", LOG_PREFIX_NODE);

        LogComponentEnable ("CrMmWaveMacLow", LOG_LEVEL_ALL);
        LogComponentEnable ("CrMmWaveMacLow", LOG_PREFIX_TIME);
        LogComponentEnable ("CrMmWaveMacLow", LOG_PREFIX_NODE);

        LogComponentEnable ("CrMmWaveNetDevice", LOG_LEVEL_ALL);
        LogComponentEnable ("CrMmWaveNetDevice", LOG_PREFIX_TIME);
        LogComponentEnable ("CrMmWaveNetDevice", LOG_PREFIX_NODE);
    }

    NetDeviceContainer
    CrMmWaveHelper::Install (const CrPhyHelper &phyHelper,
                             const CrMacHelper &macHelper,
                             NodeContainer::Iterator first,
                             NodeContainer::Iterator last) const
    {
        NetDeviceContainer devices;
        for (NodeContainer::Iterator i = first; i != last; ++i)
        {
            Ptr<Node> node = *i;
            Ptr<CrMmWaveNetDevice> device = CreateObject<CrMmWaveNetDevice> ();
            Ptr<CrMmWaveMac> mac = macHelper.Create (device);
            Ptr<MmWavePhy> intraGroupPhy = phyHelper.Create (node, device);
            Ptr<MmWavePhy> interGroupPhy = phyHelper.Create (node, device);
            Ptr<MmWavePhy> probeGroupPhy = phyHelper.Create (node, device);
            device->SetStandard (m_standard);
            device->SetRemoteStationManager (m_stationManager.Create<MmWaveRemoteStationManager> ());
            device->SetSpectrumRepository (m_repository.Create<MmWaveSpectrumRepository> ());
            device->SetMac (mac);
            device->SetPhy (INTRA_GROUP, intraGroupPhy);
            device->SetPhy (INTER_GROUP, interGroupPhy);
            device->SetPhy (PROBE_GROUP, probeGroupPhy);
            device->SetAddress (Mac48Address::Allocate ());
            node->AddDevice (device);
            devices.Add (device);
        }
        return devices;
    }

    NetDeviceContainer
    CrMmWaveHelper::Install (const CrPhyHelper &phyHelper,
                             const CrMacHelper &macHelper,
                             NodeContainer c) const
    {
        return Install (phyHelper, macHelper, c.Begin (), c.End ());
    }

    NetDeviceContainer
    CrMmWaveHelper::Install (const CrPhyHelper &phyHelper,
                             const CrMacHelper &macHelper,
                             Ptr<Node> node) const
    {
        return Install (phyHelper, macHelper, NodeContainer (node));
    }

    NetDeviceContainer
    CrMmWaveHelper::Install (const CrPhyHelper &phyHelper,
                             const CrMacHelper &macHelper,
                             std::string nodeName) const
    {
        Ptr<Node> node = Names::Find<Node> (nodeName);
        return Install (phyHelper, macHelper, NodeContainer (node));
    }

    int64_t
    CrMmWaveHelper::AssignStreams (NetDeviceContainer c, int64_t stream)
    {
        int64_t currentStream = stream;
        Ptr<NetDevice> netDevice;
        for (NetDeviceContainer::Iterator i = c.Begin (); i != c.End (); ++i)
        {
            netDevice = (*i);
            Ptr<CrMmWaveNetDevice> device = DynamicCast <CrMmWaveNetDevice> (netDevice);
            Ptr<CrMmWaveMac> mac = device->GetMac ();
            if (mac != 0)
            {
                currentStream += mac->AssignStreams (currentStream);

                currentStream += mac->GetPhy (INTRA_GROUP)->AssignStreams (currentStream + 7);
                currentStream += mac->GetPhy (INTER_GROUP)->AssignStreams (currentStream + 7);
                currentStream += mac->GetPhy (PROBE_GROUP)->AssignStreams (currentStream + 7);

                currentStream += mac->GetMacLow (INTRA_GROUP)->AssignStreams (currentStream + 7);
                currentStream += mac->GetMacLow (INTER_GROUP)->AssignStreams (currentStream + 7);
                currentStream += mac->GetMacLow (PROBE_GROUP)->AssignStreams (currentStream + 7);
            }
        }
        return (currentStream - stream);
    }
}