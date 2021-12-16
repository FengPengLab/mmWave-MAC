/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/boolean.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/mobility-model.h"
#include "ns3/names.h"
#include "ns3/config.h"
#include "ns3/object.h"
#include "ns3/pointer.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/packet.h"
#include "ns3/object-base.h"
#include "ns3/net-device-queue-interface.h"
#include "ns3/mac48-address.h"
#include "ns3/net-device.h"
#include "ns3/spectrum-wifi-phy.h"
#include "ns3/qos-txop.h"
#include "ns3/wifi-mac.h"
#include "ns3/wifi-phy.h"
#include "ns3/wifi-ack-policy-selector.h"
#include "ns3/wifi-remote-station-manager.h"
#include "ns3/error-rate-model.h"
#include "ns3/frame-capture-model.h"
#include "ns3/preamble-detection-model.h"
#include "ns3/regular-wifi-mac.h"
#include "ns3/channel-manager.h"
#include "ns3/channel-coordinator.h"
#include "ns3/mmwave-spectrum-phy.h"
#include "ns3/mmwave-error-rate-model.h"
#include "ns3/mmwave-frame-capture-model.h"
#include "ns3/mmwave-preamble-detection-model.h"
#include "ns3/v2x-channel-scheduler.h"
#include "ns3/v2x-vsa-manager.h"
#include "ns3/v2x-contention-free-access.h"
#include "v2x-mmwave-helper.h"
namespace ns3 {
    NS_LOG_COMPONENT_DEFINE ("V2xMmWaveHelper");

    V2xCtrlPhyHelper::V2xCtrlPhyHelper ()
            : m_channel (0)
    {
        m_phy.SetTypeId ("ns3::SpectrumWifiPhy");
    }

    V2xCtrlPhyHelper::~V2xCtrlPhyHelper ()
    {
    }

    V2xCtrlPhyHelper
    V2xCtrlPhyHelper::Default (Ptr<SpectrumChannel> spectrumChannel)
    {
        V2xCtrlPhyHelper helper;
        helper.SetChannel (spectrumChannel);
        helper.SetErrorRateModel ("ns3::NistErrorRateModel");
        helper.SetPreambleDetectionModel ("ns3::ThresholdPreambleDetectionModel");
        helper.SetFrameCaptureModel("ns3::SimpleFrameCaptureModel");
        helper.Set ("TxPowerStart", DoubleValue (2.0));
        helper.Set ("TxPowerEnd", DoubleValue (2.0));
        helper.Set ("Antennas", UintegerValue (4));
        helper.Set ("MaxSupportedTxSpatialStreams", UintegerValue (4));
        helper.Set ("MaxSupportedRxSpatialStreams", UintegerValue (4));

        return helper;
    }

    Ptr<WifiPhy>
    V2xCtrlPhyHelper::Create (Ptr<Node> node, Ptr<NetDevice> device) const
    {
        Ptr<SpectrumWifiPhy> phy = m_phy.Create<SpectrumWifiPhy> ();
        phy->CreateWifiSpectrumPhyInterface (device);
        if (m_errorRateModel.IsTypeIdSet ())
        {
            Ptr<ErrorRateModel> error = m_errorRateModel.Create<ErrorRateModel> ();
            phy->SetErrorRateModel (error);
        }
        if (m_frameCaptureModel.IsTypeIdSet ())
        {
            Ptr<FrameCaptureModel> capture = m_frameCaptureModel.Create<FrameCaptureModel> ();
            phy->SetFrameCaptureModel (capture);
        }
        if (m_preambleDetectionModel.IsTypeIdSet ())
        {
            Ptr<PreambleDetectionModel> capture = m_preambleDetectionModel.Create<PreambleDetectionModel> ();
            phy->SetPreambleDetectionModel (capture);
        }
        phy->SetChannel (m_channel);
        phy->SetDevice (device);
        phy->SetMobility (node->GetObject<MobilityModel> ());
        phy->ConfigureStandardAndBand (WIFI_PHY_STANDARD_80211p, WIFI_PHY_BAND_5GHZ);
        phy->SetChannelNumber (ChannelManager::GetCch ());
        return phy;
    }

    void
    V2xCtrlPhyHelper::AsciiPhyTransmitSinkWithContext (Ptr<OutputStreamWrapper> stream,
                                                       std::string context,
                                                       Ptr<const Packet> p,
                                                       WifiMode mode,
                                                       WifiPreamble preamble,
                                                       uint8_t txLevel)
    {
        NS_LOG_FUNCTION (stream << context << p << mode << preamble << txLevel);
        *stream->GetStream () << "t " << Simulator::Now ().GetSeconds () << " "<< context << " " << mode << " " << *p << std::endl;
    }

    void
    V2xCtrlPhyHelper::AsciiPhyTransmitSinkWithoutContext (Ptr<OutputStreamWrapper> stream,
                                                          Ptr<const Packet> p,
                                                          WifiMode mode,
                                                          WifiPreamble preamble,
                                                          uint8_t txLevel)
    {
        NS_LOG_FUNCTION (stream << p << mode << preamble << txLevel);
        *stream->GetStream () << "t " << Simulator::Now ().GetSeconds ()  << " " << mode << " " << *p << std::endl;
    }

    void
    V2xCtrlPhyHelper::AsciiPhyReceiveSinkWithContext (Ptr<OutputStreamWrapper> stream,
                                                      std::string context,
                                                      Ptr<const Packet> p,
                                                      double snr,
                                                      WifiMode mode,
                                                      WifiPreamble preamble)
    {
        NS_LOG_FUNCTION (stream << context << p << snr << mode << preamble);
        *stream->GetStream () << "r " << Simulator::Now ().GetSeconds () << " " << context << " " << *p << std::endl;
    }

    void
    V2xCtrlPhyHelper::AsciiPhyReceiveSinkWithoutContext (Ptr<OutputStreamWrapper> stream,
                                                         Ptr<const Packet> p,
                                                         double snr,
                                                         WifiMode mode,
                                                         WifiPreamble preamble)
    {
        NS_LOG_FUNCTION (stream << p << snr << mode << preamble);
        *stream->GetStream () << "r " << Simulator::Now ().GetSeconds () << " " << *p << std::endl;
    }

    void
    V2xCtrlPhyHelper::Set (std::string name, const AttributeValue &v)
    {
        m_phy.Set (name, v);
    }

    void
    V2xCtrlPhyHelper::SetChannel (Ptr<SpectrumChannel> channel)
    {
        m_channel = channel;
    }

    void
    V2xCtrlPhyHelper::SetChannel (std::string channelName)
    {
        Ptr<SpectrumChannel> channel = Names::Find<SpectrumChannel> (channelName);
        m_channel = channel;
    }

    void
    V2xCtrlPhyHelper::SetAntenna (const Ptr<AntennaModel> antenna)
    {
        m_antenna = antenna;
    }

    void
    V2xCtrlPhyHelper::SetErrorRateModel (std::string name,
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
    V2xCtrlPhyHelper::SetFrameCaptureModel (std::string name,
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
    V2xCtrlPhyHelper::SetPreambleDetectionModel (std::string name,
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
    V2xCtrlPhyHelper::EnableAsciiInternal (Ptr<OutputStreamWrapper> stream, std::string prefix, Ptr<NetDevice> nd, bool explicitFilename)
    {
        Ptr<V2xMmWaveNetDevice> device = nd->GetObject<V2xMmWaveNetDevice> ();
        if (device == 0)
        {
            NS_LOG_INFO ("EnableAsciiInternal(): Device " << device << " not of type ns3::V2xMmWaveNetDevice");
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
            oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::V2xMmWaveNetDevice/CtrlPhyEntities/*/$ns3::WifiPhy/State/RxOk";
            Config::ConnectWithoutContext (oss.str (), MakeBoundCallback (&AsciiPhyReceiveSinkWithoutContext, theStream));

            oss.str ("");
            oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::V2xMmWaveNetDevice/CtrlPhyEntities/*/$ns3::WifiPhy/State/Tx";
            Config::ConnectWithoutContext (oss.str (), MakeBoundCallback (&AsciiPhyTransmitSinkWithoutContext, theStream));
            return;
        }

        oss.str ("");
        oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::V2xMmWaveNetDevice/CtrlPhyEntities/*/$ns3::WifiPhy/State/RxOk";
        Config::Connect (oss.str (), MakeBoundCallback (&AsciiPhyReceiveSinkWithContext, stream));

        oss.str ("");
        oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::V2xMmWaveNetDevice/CtrlPhyEntities/*/$ns3::WifiPhy/State/Tx";
        Config::Connect (oss.str (), MakeBoundCallback (&AsciiPhyTransmitSinkWithContext, stream));
    }

    V2xDataPhyHelper::V2xDataPhyHelper ()
            : m_channel (0)
    {
        m_phy.SetTypeId ("ns3::MmWaveSpectrumPhy");
    }

    V2xDataPhyHelper::~V2xDataPhyHelper ()
    {
    }

    V2xDataPhyHelper
    V2xDataPhyHelper::Default (Ptr<SpectrumChannel> spectrumChannel)
    {
        V2xDataPhyHelper helper;
        helper.SetChannel (spectrumChannel);
        helper.SetErrorRateModel ("ns3::MmWaveTableBasedErrorRateModel");
        helper.SetPreambleDetectionModel ("ns3::MmWaveThresholdPreambleDetectionModel");
        helper.SetFrameCaptureModel("ns3::MmWaveSimpleFrameCaptureModel");
        helper.Set ("Antennas", UintegerValue (4));
        helper.Set ("MaxSupportedTxSpatialStreams", UintegerValue (4));
        helper.Set ("MaxSupportedRxSpatialStreams", UintegerValue (4));
        helper.SetStandard (MMWAVE_SUB6GHz_ASSISTED_60GHz_1280MHz);

        return helper;
    }

    Ptr<MmWavePhy>
    V2xDataPhyHelper::Create (Ptr<Node> node, Ptr<NetDevice> device) const
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
        phy->ConfigureStandardAndBand (mmWaveStandards.at(m_standard).m_phyStandard, mmWaveStandards.at(m_standard).m_phyBand);
        return phy;
    }

    void
    V2xDataPhyHelper::AsciiPhyTransmitSinkWithContext (Ptr<OutputStreamWrapper> stream,
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
    V2xDataPhyHelper::AsciiPhyTransmitSinkWithoutContext (Ptr<OutputStreamWrapper> stream,
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
    V2xDataPhyHelper::AsciiPhyReceiveSinkWithContext (Ptr<OutputStreamWrapper> stream,
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
    V2xDataPhyHelper::AsciiPhyReceiveSinkWithoutContext (Ptr<OutputStreamWrapper> stream,
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
    V2xDataPhyHelper::Set (std::string name, const AttributeValue &v)
    {
        m_phy.Set (name, v);
    }

    void
    V2xDataPhyHelper::SetStandard (MmWaveStandard standard)
    {
        m_standard = standard;
    }

    void
    V2xDataPhyHelper::SetChannel (Ptr<SpectrumChannel> channel)
    {
        m_channel = channel;
    }

    void
    V2xDataPhyHelper::SetChannel (std::string channelName)
    {
        Ptr<SpectrumChannel> channel = Names::Find<SpectrumChannel> (channelName);
        m_channel = channel;
    }

    void
    V2xDataPhyHelper::SetErrorRateModel (std::string name,
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
    V2xDataPhyHelper::SetFrameCaptureModel (std::string name,
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
    V2xDataPhyHelper::SetPreambleDetectionModel (std::string name,
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
    V2xDataPhyHelper::EnableAsciiInternal (Ptr<OutputStreamWrapper> stream, std::string prefix, Ptr<NetDevice> nd, bool explicitFilename)
    {
        Ptr<V2xMmWaveNetDevice> device = nd->GetObject<V2xMmWaveNetDevice> ();
        if (device == 0)
        {
            NS_LOG_INFO ("EnableAsciiInternal(): Device " << device << " not of type ns3::V2xMmWaveNetDevice");
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
            oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::V2xMmWaveNetDevice/DataPhy/State/RxOk";
            Config::ConnectWithoutContext (oss.str (), MakeBoundCallback (&AsciiPhyReceiveSinkWithoutContext, theStream));

            oss.str ("");
            oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::V2xMmWaveNetDevice/DataPhy/State/Tx";
            Config::ConnectWithoutContext (oss.str (), MakeBoundCallback (&AsciiPhyTransmitSinkWithoutContext, theStream));

            return;
        }

        oss.str ("");
        oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::V2xMmWaveNetDevice/DataPhy/State/RxOk";
        Config::Connect (oss.str (), MakeBoundCallback (&AsciiPhyReceiveSinkWithContext, stream));

        oss.str ("");
        oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::V2xMmWaveNetDevice/DataPhy/State/Tx";
        Config::Connect (oss.str (), MakeBoundCallback (&AsciiPhyTransmitSinkWithContext, stream));
    }

    V2xCtrlMacHelper::V2xCtrlMacHelper ()
    {
        SetType ("ns3::V2xCtrlMac", "QosSupported", BooleanValue (false));
    }

    V2xCtrlMacHelper::~V2xCtrlMacHelper ()
    {
    }

    V2xCtrlMacHelper
    V2xCtrlMacHelper::Default ()
    {
        V2xCtrlMacHelper helper;
        return helper;
    }

    Ptr<V2xCtrlMac>
    V2xCtrlMacHelper::Create (Ptr<NetDevice> device) const
    {
        Ptr<V2xCtrlMac> mac = m_mac.Create<V2xCtrlMac> ();
        mac->SetDevice (device);
        return mac;
    }

    void
    V2xCtrlMacHelper::SetType (std::string type,
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

    V2xDataMacHelper::V2xDataMacHelper ()
    {
        SetType ("ns3::V2xDataMac");
    }

    V2xDataMacHelper::~V2xDataMacHelper ()
    {
    }

    V2xDataMacHelper
    V2xDataMacHelper::Default ()
    {
        V2xDataMacHelper helper;
        return helper;
    }

    Ptr<V2xDataMac>
    V2xDataMacHelper::Create (Ptr<NetDevice> device) const
    {
        Ptr<V2xDataMac> mac = m_mac.Create<V2xDataMac> ();
        mac->SetDevice (device);
        return mac;
    }

    void
    V2xDataMacHelper::SetType (std::string type,
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

    V2xMmWaveHelper::V2xMmWaveHelper ()
    {
        CreateMacForChannel (ChannelManager::GetWaveChannels ());
        CreatePhys (1);
        SetChannelScheduler ("ns3::V2xDefaultChannelScheduler");
        SetCtrlRemoteStationManager("ns3::ConstantRateWifiManager",
                                    "DataMode", StringValue ("OfdmRate6MbpsBW10MHz"),
                                    "ControlMode",StringValue ("OfdmRate6MbpsBW10MHz"),
                                    "NonUnicastMode", StringValue ("OfdmRate6MbpsBW10MHz"));
        SetDataRemoteStationManager("ns3::MmWaveConstantRateManager",
                                    "ControlMode", StringValue("MmWaveMcs1"),
                                    "DataMode", StringValue("MmWaveMcs1"));
    }

    V2xMmWaveHelper::~V2xMmWaveHelper ()
    {
    }

    void
    V2xMmWaveHelper::SetChannelScheduler (std::string type,
                                           std::string n0, const AttributeValue &v0,
                                           std::string n1, const AttributeValue &v1,
                                           std::string n2, const AttributeValue &v2,
                                           std::string n3, const AttributeValue &v3,
                                           std::string n4, const AttributeValue &v4,
                                           std::string n5, const AttributeValue &v5,
                                           std::string n6, const AttributeValue &v6,
                                           std::string n7, const AttributeValue &v7)
    {
        m_channelScheduler = ObjectFactory ();
        m_channelScheduler.SetTypeId (type);
        m_channelScheduler.Set (n0, v0);
        m_channelScheduler.Set (n1, v1);
        m_channelScheduler.Set (n2, v2);
        m_channelScheduler.Set (n3, v3);
        m_channelScheduler.Set (n4, v4);
        m_channelScheduler.Set (n5, v5);
        m_channelScheduler.Set (n6, v6);
        m_channelScheduler.Set (n7, v7);
    }

    void
    V2xMmWaveHelper::SetCtrlRemoteStationManager (std::string type,
                                                  std::string n0, const AttributeValue &v0,
                                                  std::string n1, const AttributeValue &v1,
                                                  std::string n2, const AttributeValue &v2,
                                                  std::string n3, const AttributeValue &v3,
                                                  std::string n4, const AttributeValue &v4,
                                                  std::string n5, const AttributeValue &v5,
                                                  std::string n6, const AttributeValue &v6,
                                                  std::string n7, const AttributeValue &v7)
    {
        m_ctrlStationManager = ObjectFactory ();
        m_ctrlStationManager.SetTypeId (type);
        m_ctrlStationManager.Set (n0, v0);
        m_ctrlStationManager.Set (n1, v1);
        m_ctrlStationManager.Set (n2, v2);
        m_ctrlStationManager.Set (n3, v3);
        m_ctrlStationManager.Set (n4, v4);
        m_ctrlStationManager.Set (n5, v5);
        m_ctrlStationManager.Set (n6, v6);
        m_ctrlStationManager.Set (n7, v7);
    }

    void
    V2xMmWaveHelper::SetDataRemoteStationManager (std::string type,
                                                  std::string n0, const AttributeValue &v0,
                                                  std::string n1, const AttributeValue &v1,
                                                  std::string n2, const AttributeValue &v2,
                                                  std::string n3, const AttributeValue &v3,
                                                  std::string n4, const AttributeValue &v4,
                                                  std::string n5, const AttributeValue &v5,
                                                  std::string n6, const AttributeValue &v6,
                                                  std::string n7, const AttributeValue &v7)
    {
        m_dataStationManager = ObjectFactory ();
        m_dataStationManager.SetTypeId (type);
        m_dataStationManager.Set (n0, v0);
        m_dataStationManager.Set (n1, v1);
        m_dataStationManager.Set (n2, v2);
        m_dataStationManager.Set (n3, v3);
        m_dataStationManager.Set (n4, v4);
        m_dataStationManager.Set (n5, v5);
        m_dataStationManager.Set (n6, v6);
        m_dataStationManager.Set (n7, v7);
    }

    void
    V2xMmWaveHelper::CreateMacForChannel (std::vector<uint32_t>  channelNumbers)
    {
        if (channelNumbers.size () == 0)
        {
            NS_FATAL_ERROR ("the MAC entities is at least one");
        }
        for (std::vector<uint32_t>::iterator i = channelNumbers.begin (); i != channelNumbers.end (); ++i)
        {
            if (!ChannelManager::IsWaveChannel (*i))
            {
                NS_FATAL_ERROR ("the channel number " << (*i) << " is not a valid channel number");
            }
        }
        m_ctrlMacsForChannelNumber = channelNumbers;
    }

    void
    V2xMmWaveHelper::CreatePhys (uint32_t phys)
    {
        if (phys == 0)
        {
            NS_FATAL_ERROR ("the PHY entities is at least one");
        }
        if (phys > ChannelManager::GetNumberOfWaveChannels ())
        {
            NS_FATAL_ERROR ("the number of assigned PHY entities is more than the number of valid channels");
        }
        m_ctrlPhysNumber = phys;
    }

    void
    V2xMmWaveHelper::EnableLogComponents ()
    {
//        LogComponentEnable ("V2xDataMacLow", LOG_LEVEL_ALL);
//        LogComponentEnable ("V2xDataMacLow", LOG_PREFIX_TIME);
//        LogComponentEnable ("V2xDataMacLow", LOG_PREFIX_NODE);
//
//        LogComponentEnable ("V2xDataMac", LOG_LEVEL_ALL);
//        LogComponentEnable ("V2xDataMac", LOG_PREFIX_TIME);
//        LogComponentEnable ("V2xDataMac", LOG_PREFIX_NODE);
//
//        LogComponentEnable ("V2xContentionFreeAccess", LOG_LEVEL_ALL);
//        LogComponentEnable ("V2xContentionFreeAccess", LOG_PREFIX_TIME);
//        LogComponentEnable ("V2xContentionFreeAccess", LOG_PREFIX_NODE);

//        LogComponentEnable ("V2xMmWaveNetDevice", LOG_LEVEL_ALL);
//        LogComponentEnable ("V2xMmWaveNetDevice", LOG_PREFIX_TIME);
//        LogComponentEnable ("V2xMmWaveNetDevice", LOG_PREFIX_NODE);
    }

    NetDeviceContainer
    V2xMmWaveHelper::Install (const V2xCtrlPhyHelper &ctrlPhyHelper, const V2xCtrlMacHelper &ctrlMacHelper,
                              const V2xDataPhyHelper &dataPhyHelper, const V2xDataMacHelper &dataMacHelper,
                              NodeContainer::Iterator first, NodeContainer::Iterator last) const
    {
        NetDeviceContainer devices;
        for (NodeContainer::Iterator i = first; i != last; ++i)
        {
            Ptr<Node> node = *i;
            Ptr<V2xMmWaveNetDevice> device = CreateObject<V2xMmWaveNetDevice> ();
            device->SetChannelManager (CreateObject<ChannelManager> ());
            device->SetChannelCoordinator (CreateObject<ChannelCoordinator> ());
            device->SetVsaManager (CreateObject<V2xVsaManager> ());
            device->SetContentionFreeAccess (CreateObject<V2xContentionFreeAccess> ());
            device->SetChannelScheduler (m_channelScheduler.Create<V2xChannelScheduler> ());

            Ptr<V2xDataMac> dataMac = dataMacHelper.Create (device);
            Ptr<MmWavePhy> dataPhy = dataPhyHelper.Create (node, device);
            device->SetDataRemoteStationManager (m_dataStationManager.Create<MmWaveRemoteStationManager> ());
            device->SetDataMac (dataMac);
            device->SetDataPhy (dataPhy);

            for (uint32_t j = 0; j != m_ctrlPhysNumber; ++j)
            {
                Ptr<WifiPhy> phy = ctrlPhyHelper.Create (node, device);
                device->AddCtrlPhy (phy);
            }

            for (std::vector<uint32_t>::const_iterator k = m_ctrlMacsForChannelNumber.begin (); k != m_ctrlMacsForChannelNumber.end (); ++k)
            {
                Ptr<V2xCtrlMac> ctrlMac = ctrlMacHelper.Create (device);
                ctrlMac->EnableForWave (device);
                ctrlMac->SetWifiRemoteStationManager (m_ctrlStationManager.Create<WifiRemoteStationManager> ());
                ctrlMac->ConfigureStandard (WIFI_STANDARD_80211p);
                device->AddCtrlMac (*k, ctrlMac);
            }

            device->SetAddress (Mac48Address::Allocate ());
            node->AddDevice (device);
            devices.Add (device);

//            Ptr<NetDeviceQueueInterface> ndqi = CreateObject<NetDeviceQueueInterface> ();
//            Ptr<MmWaveMacQueue> macQueue = device->GetMacQueue ();
//            ndqi->GetTxQueue (0)->ConnectQueueTraces (macQueue);
//            device->AggregateObject (ndqi);
        }
        return devices;
    }

    NetDeviceContainer
    V2xMmWaveHelper::Install (const V2xCtrlPhyHelper &ctrlPhyHelper,
                              const V2xCtrlMacHelper &ctrlMacHelper,
                              const V2xDataPhyHelper &dataPhyHelper,
                              const V2xDataMacHelper &dataMacHelper,
                              NodeContainer c) const
    {
        return Install (ctrlPhyHelper, ctrlMacHelper, dataPhyHelper, dataMacHelper, c.Begin (), c.End ());
    }

    NetDeviceContainer
    V2xMmWaveHelper::Install (const V2xCtrlPhyHelper &ctrlPhyHelper,
                              const V2xCtrlMacHelper &ctrlMacHelper,
                              const V2xDataPhyHelper &dataPhyHelper,
                              const V2xDataMacHelper &dataMacHelper,
                              Ptr<Node> node) const
    {
        return Install (ctrlPhyHelper, ctrlMacHelper, dataPhyHelper, dataMacHelper, NodeContainer (node));
    }

    NetDeviceContainer
    V2xMmWaveHelper::Install (const V2xCtrlPhyHelper &ctrlPhyHelper,
                              const V2xCtrlMacHelper &ctrlMacHelper,
                              const V2xDataPhyHelper &dataPhyHelper,
                              const V2xDataMacHelper &dataMacHelper,
                              std::string nodeName) const
    {
        Ptr<Node> node = Names::Find<Node> (nodeName);
        return Install (ctrlPhyHelper, ctrlMacHelper, dataPhyHelper, dataMacHelper, NodeContainer (node));
    }

    int64_t
    V2xMmWaveHelper::AssignStreams (NetDeviceContainer c, int64_t stream)
    {
        int64_t currentStream = stream;
        Ptr<NetDevice> netDevice;
        for (NetDeviceContainer::Iterator i = c.Begin (); i != c.End (); ++i)
        {
            netDevice = (*i);
            Ptr<V2xMmWaveNetDevice> mmWave = DynamicCast<V2xMmWaveNetDevice> (netDevice);
            if (mmWave)
            {
                currentStream += mmWave->AssignStreams (currentStream);
                currentStream += mmWave->GetDataPhy ()->AssignStreams (currentStream);

                std::vector<Ptr<WifiPhy> > phys = mmWave->GetCtrlPhys ();
                for (std::vector<Ptr<WifiPhy> >::iterator j = phys.begin (); j != phys.end (); ++j)
                {
                    currentStream += (*j)->AssignStreams (currentStream);
                }

                std::map<uint32_t, Ptr<V2xCtrlMac>> macs = mmWave->GetCtrlMacs ();
                for ( std::map<uint32_t, Ptr<V2xCtrlMac> >::iterator k = macs.begin (); k != macs.end (); ++k)
                {
                    Ptr<RegularWifiMac> rmac = DynamicCast<RegularWifiMac> (k->second);

                    Ptr<WifiRemoteStationManager> manager = rmac->GetWifiRemoteStationManager ();
                    PointerValue ptr;
                    rmac->GetAttribute ("Txop", ptr);
                    Ptr<Txop> txop = ptr.Get<Txop> ();
                    currentStream += txop->AssignStreams (currentStream);

                    rmac->GetAttribute ("VO_Txop", ptr);
                    Ptr<QosTxop> vo_txop = ptr.Get<QosTxop> ();
                    currentStream += vo_txop->AssignStreams (currentStream);

                    rmac->GetAttribute ("VI_Txop", ptr);
                    Ptr<QosTxop> vi_txop = ptr.Get<QosTxop> ();
                    currentStream += vi_txop->AssignStreams (currentStream);

                    rmac->GetAttribute ("BE_Txop", ptr);
                    Ptr<QosTxop> be_txop = ptr.Get<QosTxop> ();
                    currentStream += be_txop->AssignStreams (currentStream);

                    rmac->GetAttribute ("BK_Txop", ptr);
                    Ptr<QosTxop> bk_txop = ptr.Get<QosTxop> ();
                    currentStream += bk_txop->AssignStreams (currentStream);
                }
            }
        }
        return (currentStream - stream);
    }

}

