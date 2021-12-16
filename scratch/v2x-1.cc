/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include "ns3/output-stream-wrapper.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/enum.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/vector.h"
#include "ns3/pointer.h"
#include "ns3/config.h"
#include "ns3/names.h"
#include "ns3/node-container.h"
#include "ns3/application.h"
#include "ns3/data-rate.h"
#include "ns3/address.h"
#include "ns3/socket.h"
#include "ns3/packet-sink.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/aodv-helper.h"
#include "ns3/trace-helper.h"
#include "ns3/net-device-container.h"
#include "ns3/ipv4-interface-container.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/spectrum-channel.h"
#include "ns3/command-line.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/position-allocator.h"
#include "ns3/application-container.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/inet-socket-address.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/udp-server.h"
#include "ns3/spectrum-analyzer-helper.h"
#include "ns3/v2x-mmwave-helper.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/channel-condition-model.h"
#include "ns3/three-gpp-channel-model.h"
#include "ns3/three-gpp-antenna-array-model.h"
#include "ns3/three-gpp-propagation-loss-model.h"
#include "ns3/three-gpp-spectrum-propagation-loss-model.h"

NS_LOG_COMPONENT_DEFINE ("ExampleOfCognitiveRadioMmWaveNetDevice");

using namespace ns3;
using namespace std;

class V2xMmWaveExample
{
public:
    V2xMmWaveExample ();
    bool Configure (int argc, char **argv);
    void Run ();

private:
    uint32_t m_size;
    double m_totalTime;
    bool m_tracing;
    AsciiTraceHelper m_ascii;
    NodeContainer m_nodes;
    NodeContainer m_spectrumAnalyzerNodes;
    NetDeviceContainer m_devices;
    NetDeviceContainer m_spectrumAnalyzerDevices;
    Ipv4InterfaceContainer m_interfaces;
    ApplicationContainer m_apps;
    Ptr<MultiModelSpectrumChannel> m_spectrumChannel_6G;
    Ptr<MultiModelSpectrumChannel> m_spectrumChannel_mmWave;
    Ptr<ThreeGppPropagationLossModel> m_lossModel_6G;
    Ptr<ThreeGppPropagationLossModel> m_lossModel_mmWave;
    Ptr<ThreeGppSpectrumPropagationLossModel> m_spectrumLoss_6G;
    Ptr<ThreeGppSpectrumPropagationLossModel> m_spectrumLoss_mmWave;

private:
    void CreateNodes ();
    void CreateChannels ();
    void CreateDevices ();
    void InstallUdpApplications ();
    void CreateSpectrumAnalyzer ();
    
};

V2xMmWaveExample::V2xMmWaveExample () :
        m_size (7),
        m_totalTime (150),
        m_tracing (true)
{
}

bool
V2xMmWaveExample::Configure (int argc, char **argv)
{
    CommandLine cmd;
    cmd.AddValue ("tracing", "Write traces.", m_tracing);
    cmd.AddValue ("time", "Simulation time, s.", m_totalTime);
    cmd.Parse (argc, argv);

    return true;
}

void
V2xMmWaveExample::Run ()
{
    CreateNodes ();
    CreateChannels ();
    CreateDevices ();
    InstallUdpApplications ();
    CreateSpectrumAnalyzer ();


    NS_LOG_INFO ("Run Simulation.");
    Simulator::Stop (Seconds (m_totalTime));
    Simulator::Run ();
    Simulator::Destroy ();
    NS_LOG_INFO ("Done.");
}

void
V2xMmWaveExample::CreateNodes ()
{
    m_nodes.Create (m_size);

    InternetStackHelper internet;
    internet.Install (m_nodes);

    for (uint32_t i = 0; i < m_size; ++i)
    {
        std::ostringstream os;
        os << "node-" << i;
        Names::Add (os.str (), m_nodes.Get (i));
    }

    /*
        (2)           (6)  
         |  \       /  |
         |    \   /    |
        (1)----(3)----(5)
         |    /   \    |
         |  /       \  |
        (0)           (4)
     */
    Ptr<ListPositionAllocator> nodePositionList = CreateObject<ListPositionAllocator> ();
    nodePositionList->Add (Vector (10, 10, 0));
    nodePositionList->Add (Vector (10, 14, 0));
    nodePositionList->Add (Vector (10, 18, 0));
    nodePositionList->Add (Vector (20, 14, 0));
    nodePositionList->Add (Vector (30, 10, 0));
    nodePositionList->Add (Vector (30, 14, 0));
    nodePositionList->Add (Vector (30, 18, 0));

    MobilityHelper mobility;
    mobility.SetPositionAllocator (nodePositionList);
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (m_nodes);

    if (m_tracing)
    {
        Ptr<OutputStreamWrapper> osw = m_ascii.CreateFileStream ("v2x-1.mob");
        MobilityHelper::EnableAsciiAll(osw);
    }
}

void
V2xMmWaveExample::CreateChannels ()
{   
    Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue(MilliSeconds (100)));
    Config::SetDefault ("ns3::ThreeGppChannelConditionModel::UpdatePeriod", TimeValue(MilliSeconds (100)));

    std::string scenario = "UMi-StreetCanyon"; // 3GPP propagation scenario

    Ptr<ChannelConditionModel> condModel;
    Ptr<ConstantSpeedPropagationDelayModel> delayModel;

    ObjectFactory propagationLossModelFactory;
    ObjectFactory channelConditionModelFactory;
    if (scenario == "RMa")
    {
        propagationLossModelFactory.SetTypeId (ThreeGppRmaPropagationLossModel::GetTypeId ());
        channelConditionModelFactory.SetTypeId (ThreeGppRmaChannelConditionModel::GetTypeId ());
    }
    else if (scenario == "UMa")
    {
        propagationLossModelFactory.SetTypeId (ThreeGppUmaPropagationLossModel::GetTypeId ());
        channelConditionModelFactory.SetTypeId (ThreeGppUmaChannelConditionModel::GetTypeId ());
    }
    else if (scenario == "UMi-StreetCanyon")
    {
        propagationLossModelFactory.SetTypeId (ThreeGppUmiStreetCanyonPropagationLossModel::GetTypeId ());
        channelConditionModelFactory.SetTypeId (ThreeGppUmiStreetCanyonChannelConditionModel::GetTypeId ());
    }
    else if (scenario == "InH-OfficeOpen")
    {
        propagationLossModelFactory.SetTypeId (ThreeGppIndoorOfficePropagationLossModel::GetTypeId ());
        channelConditionModelFactory.SetTypeId (ThreeGppIndoorOpenOfficeChannelConditionModel::GetTypeId ());
    }
    else if (scenario == "InH-OfficeMixed")
    {
        propagationLossModelFactory.SetTypeId (ThreeGppIndoorOfficePropagationLossModel::GetTypeId ());
        channelConditionModelFactory.SetTypeId (ThreeGppIndoorMixedOfficeChannelConditionModel::GetTypeId ());
    }
    else
    {
        NS_FATAL_ERROR ("Unknown scenario");
    }

    // create the propagation loss model
    double frequency = 5.860e9;
    m_lossModel_6G = propagationLossModelFactory.Create<ThreeGppPropagationLossModel> ();
    m_lossModel_6G->SetAttribute ("Frequency", DoubleValue (frequency));
    m_lossModel_6G->SetAttribute ("ShadowingEnabled", BooleanValue (false));

    // create the spectrum propagation loss model
    m_spectrumLoss_6G = CreateObject<ThreeGppSpectrumPropagationLossModel> ();
    m_spectrumLoss_6G->SetChannelModelAttribute ("Frequency", DoubleValue (frequency));
    m_spectrumLoss_6G->SetChannelModelAttribute ("Scenario", StringValue (scenario));

    // create the channel condition model and associate it with the spectrum and propagation loss model
    condModel = CreateObject<NeverLosChannelConditionModel> ();
    m_spectrumLoss_6G->SetChannelModelAttribute ("ChannelConditionModel", PointerValue (condModel));
    m_lossModel_6G->SetChannelConditionModel (condModel);

    delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
    m_spectrumChannel_6G = CreateObject<MultiModelSpectrumChannel> ();
    m_spectrumChannel_6G->SetPropagationDelayModel (delayModel);
    m_spectrumChannel_6G->AddPropagationLossModel (m_lossModel_6G);


    frequency = 60e9;
    // create the propagation loss model
    m_lossModel_mmWave = propagationLossModelFactory.Create<ThreeGppPropagationLossModel> ();
    m_lossModel_mmWave->SetAttribute ("Frequency", DoubleValue (frequency));
    m_lossModel_mmWave->SetAttribute ("ShadowingEnabled", BooleanValue (false));

    // create the spectrum propagation loss model
    m_spectrumLoss_mmWave = CreateObject<ThreeGppSpectrumPropagationLossModel> ();
    m_spectrumLoss_mmWave->SetChannelModelAttribute ("Frequency", DoubleValue (frequency));
    m_spectrumLoss_mmWave->SetChannelModelAttribute ("Scenario", StringValue (scenario));

    // create the channel condition model and associate it with the spectrum and propagation loss model
    condModel = CreateObject<NeverLosChannelConditionModel> ();
    m_spectrumLoss_mmWave->SetChannelModelAttribute ("ChannelConditionModel", PointerValue (condModel));
    m_lossModel_mmWave->SetChannelConditionModel (condModel);

    delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
    m_spectrumChannel_mmWave = CreateObject<MultiModelSpectrumChannel> ();
    m_spectrumChannel_mmWave->SetPropagationDelayModel (delayModel);
    m_spectrumChannel_mmWave->AddPropagationLossModel (m_lossModel_mmWave);
}

void
V2xMmWaveExample::CreateDevices ()
{   
    V2xCtrlPhyHelper ctrlPhyHelper = V2xCtrlPhyHelper::Default (m_spectrumChannel_6G);
    V2xCtrlMacHelper ctrlMacHelper = V2xCtrlMacHelper::Default ();

    V2xDataMacHelper dataMacHelper = V2xDataMacHelper::Default ();
    V2xDataPhyHelper dataPhyHelper = V2xDataPhyHelper::Default (m_spectrumChannel_mmWave);
    
    V2xMmWaveHelper helper;

    helper.EnableLogComponents ();
    m_devices = helper.Install (ctrlPhyHelper, ctrlMacHelper, dataPhyHelper, dataMacHelper, m_nodes);
    helper.AssignStreams (m_devices, 100);

    Ptr<ThreeGppAntennaArrayModel> a;

    for (NetDeviceContainer::Iterator i = m_devices.Begin (); i != m_devices.End (); ++i)
    {
        a = CreateObjectWithAttributes<ThreeGppAntennaArrayModel> ("NumColumns", UintegerValue (2), "NumRows", UintegerValue (2));
        m_spectrumLoss_6G->AddDevice ((*i), a);
    }

    for (NetDeviceContainer::Iterator i = m_devices.Begin (); i != m_devices.End (); ++i)
    {
        a = CreateObjectWithAttributes<ThreeGppAntennaArrayModel> ("NumColumns", UintegerValue (2), "NumRows", UintegerValue (2));
        m_spectrumLoss_mmWave->AddDevice ((*i), a);
    }
    
    Ipv4AddressHelper ipv4;
    ipv4.SetBase ("10.1.1.0", "255.255.255.0");
    m_interfaces = ipv4.Assign (m_devices);

    if (m_tracing)
    {
        Ptr<OutputStreamWrapper> osw = m_ascii.CreateFileStream ( "v2x-1.tr");
        dataPhyHelper.EnableAsciiAll (osw);
    }
}

void
V2xMmWaveExample::InstallUdpApplications ()
{
    uint16_t port = 4000;
    uint32_t MaxPacketSize = 2000;
    Time interPacketInterval = Seconds (0.001);
    
    // flow 1
    port++;
    UdpClientHelper client1 (Address (m_interfaces.GetAddress (3)), port);
    UdpServerHelper server1 (port);

    client1.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
    client1.SetAttribute ("Interval", TimeValue (interPacketInterval));
    client1.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));

    m_apps = client1.Install (m_nodes.Get (0));
    m_apps.Start (Seconds (0.0));
    m_apps.Stop (Seconds (m_totalTime));

    m_apps = server1.Install (m_nodes.Get (3));
    m_apps.Start (Seconds (0.0));
    m_apps.Stop (Seconds (m_totalTime));

    // flow 2
    port++;
    UdpClientHelper client2 (Address (m_interfaces.GetAddress (3)), port);
    UdpServerHelper server2 (port);

    client2.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
    client2.SetAttribute ("Interval", TimeValue (interPacketInterval));
    client2.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));

    m_apps = client2.Install (m_nodes.Get (1));
    m_apps.Start (Seconds (0.0));
    m_apps.Stop (Seconds (m_totalTime));

    m_apps = server2.Install (m_nodes.Get (3));
    m_apps.Start (Seconds (0.0));
    m_apps.Stop (Seconds (m_totalTime));

    // flow 3
    port++;
    UdpClientHelper client3 (Address (m_interfaces.GetAddress (3)), port);
    UdpServerHelper server3 (port);

    client3.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
    client3.SetAttribute ("Interval", TimeValue (interPacketInterval));
    client3.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));

    m_apps = client3.Install (m_nodes.Get (2));
    m_apps.Start (Seconds (0.0));
    m_apps.Stop (Seconds (m_totalTime));

    m_apps = server3.Install (m_nodes.Get (3));
    m_apps.Start (Seconds (0.0));
    m_apps.Stop (Seconds (m_totalTime));

    // flow 4
    port++;
    UdpClientHelper client4 (Address (m_interfaces.GetAddress (4)), port);
    UdpServerHelper server4 (port);

    client4.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
    client4.SetAttribute ("Interval", TimeValue (interPacketInterval));
    client4.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));

    m_apps = client4.Install (m_nodes.Get (3));
    m_apps.Start (Seconds (0.0));
    m_apps.Stop (Seconds (m_totalTime));

    m_apps = server4.Install (m_nodes.Get (4));
    m_apps.Start (Seconds (0.0));
    m_apps.Stop (Seconds (m_totalTime));

    // flow 5
    port++;
    UdpClientHelper client5 (Address (m_interfaces.GetAddress (5)), port);
    UdpServerHelper server5 (port);

    client5.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
    client5.SetAttribute ("Interval", TimeValue (interPacketInterval));
    client5.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));

    m_apps = client5.Install (m_nodes.Get (3));
    m_apps.Start (Seconds (0.0));
    m_apps.Stop (Seconds (m_totalTime));

    m_apps = server5.Install (m_nodes.Get (5));
    m_apps.Start (Seconds (0.0));
    m_apps.Stop (Seconds (m_totalTime));

    // flow 6
    port++;
    UdpClientHelper client6 (Address (m_interfaces.GetAddress (6)), port);
    UdpServerHelper server6 (port);

    client6.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
    client6.SetAttribute ("Interval", TimeValue (interPacketInterval));
    client6.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));

    m_apps = client6.Install (m_nodes.Get (3));
    m_apps.Start (Seconds (0.0));
    m_apps.Stop (Seconds (m_totalTime));

    m_apps = server6.Install (m_nodes.Get (6));
    m_apps.Start (Seconds (0.0));
    m_apps.Stop (Seconds (m_totalTime));
}

void
V2xMmWaveExample::CreateSpectrumAnalyzer ()
{
    m_spectrumAnalyzerNodes.Create (1);
    Ptr<ListPositionAllocator> nodePositionList = CreateObject<ListPositionAllocator> ();
    nodePositionList->Add (Vector (20.0, 15.0, 0.0));
    MobilityHelper mobility;
    mobility.SetPositionAllocator (nodePositionList);
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (m_spectrumAnalyzerNodes);

    uint32_t centerFrequency = 60000;
    uint16_t channelWidth = 1280;
    uint32_t bandBandwidth = 78125;
    uint16_t guardBandwidth = 1280;

    Ptr<SpectrumModel> SpectrumModel_1 = MmWaveSpectrumValueHelper::GetSpectrumModel (centerFrequency, channelWidth, bandBandwidth, guardBandwidth);
    SpectrumAnalyzerHelper spectrumAnalyzer_1;
    spectrumAnalyzer_1.SetChannel (m_spectrumChannel_mmWave);
    spectrumAnalyzer_1.SetRxSpectrumModel (SpectrumModel_1);
    spectrumAnalyzer_1.SetPhyAttribute ("Resolution", TimeValue (MilliSeconds (100)));
    spectrumAnalyzer_1.SetPhyAttribute ("NoisePowerSpectralDensity", DoubleValue (1e-15));  // -120 dBm/Hz
    spectrumAnalyzer_1.EnableAsciiAll ("v2x_topo_a_60GHz_output");
    m_spectrumAnalyzerDevices = spectrumAnalyzer_1.Install (m_spectrumAnalyzerNodes);
}

int
main (int argc, char *argv[])
{
    V2xMmWaveExample ex;
    if (!ex.Configure (argc, argv))
    {
        NS_FATAL_ERROR ("Configuration failed.");
    }
    ex.Run ();
    return 0;
}