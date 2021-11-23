/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <map>
#include <cmath>
#include "ns3/output-stream-wrapper.h"
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/aodv-module.h"
#include "ns3/ns2-mobility-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/v4ping-helper.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/netanim-module.h" 
#include "ns3/node-container.h"
#include "ns3/application-container.h"
#include "ns3/ipv4-interface-container.h"
#include "ns3/ipv4-address.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/channel-condition-model.h"
#include "ns3/three-gpp-channel-model.h"
#include "ns3/three-gpp-antenna-array-model.h"
#include "ns3/three-gpp-propagation-loss-model.h"
#include "ns3/three-gpp-spectrum-propagation-loss-model.h"
#include "ns3/cr-mmwave-helper.h"
#include "ns3/mmwave-spectrum-value-helper.h"

NS_LOG_COMPONENT_DEFINE ("ExampleOfCogGroup");

using namespace ns3;
using namespace std;

static 
void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize, Time duration, Time interval)
{
  if (duration > Seconds (0))
    {
      socket->Send (Create<Packet> (pktSize));
      Simulator::Schedule (interval, &GenerateTraffic, socket, pktSize, duration - interval, interval);
    }
  else
    {
      socket->Close ();
    }
};

static
void ReceivePacket (Ptr<Socket> socket)
{
  ;
}

class ScenarioN50
{
public:
    ScenarioN50 ();
    bool Configure (int argc, char **argv);
    void Install ();
    void CreateApp (uint32_t from, uint32_t to, uint32_t port, double startTime, double endTime);
    void CreateNodes ();
    void CreateChannels ();
    void CreateDevices ();
    void InstallInternetStack ();
    void InstallApplications ();

    uint32_t m_nodeNum;
    uint32_t m_pktSize;
    double m_duration;
    double m_interval;
    bool m_tracing;
    std::string m_fileName;

private:
    NodeContainer m_nodes;
    NetDeviceContainer m_devices;
    Ipv4InterfaceContainer m_interfaces;
    Ptr<MultiModelSpectrumChannel> m_spectrumChannel;
    Ptr<ThreeGppPropagationLossModel> m_lossModel;
    Ptr<ThreeGppSpectrumPropagationLossModel> m_spectrumLoss;
};

ScenarioN50::ScenarioN50 () :
        m_nodeNum (50),
        m_pktSize (2000),
        m_duration (300),
        m_interval (0.001),
        m_tracing (true)
{
  m_fileName = "./scratch/scenario-n50-3";
}

bool
ScenarioN50::Configure (int argc, char **argv)
{
  SeedManager::SetSeed (12345);

  // Enable AODV logs by default. Comment this if too noisy
  // LogComponentEnable("AodvRoutingProtocol", LOG_LEVEL_ALL);

  // Enable logging from the ns2 helper
  // LogComponentEnable ("Ns2MobilityHelper",LOG_LEVEL_DEBUG);

  // Parse command line attribute
  CommandLine cmd (__FILE__);
  cmd.AddValue ("m_fileName", "Ns2 movement trace file", m_fileName);
  cmd.AddValue ("nodeNum", "Number of nodes", m_nodeNum);
  cmd.AddValue ("duration", "Duration of Simulation", m_duration);
  cmd.Parse (argc,argv);

  // Check command line arguments
  if (m_fileName.empty () || m_nodeNum <= 0 || m_duration <= 0)
  {
    std::cout << "Usage of " << argv[0] << " :\n\n"
    "./waf --run \"ScenarioN50 --fileName=./scratch/scenario1-n50 --nodeNum=50 --duration=300.0 \" \n\n"
    "NOTE 1: ns2-traces-file could be an absolute or relative path.\n\n"
    "NOTE 2: Number of nodes present in the trace file must match with the command line argument and must\n"
    "        be a positive number. Note that you must know it before to be able to load it.\n\n"
    "NOTE 3: Duration must be a positive number. Note that you must know it before to be able to load it.\n\n";

    return 0;
  }

  return true;
}


void
ScenarioN50::Install ()
{
  CreateChannels ();
  CreateNodes ();
  CreateDevices ();
  InstallInternetStack ();
  InstallApplications ();
}

void
ScenarioN50::CreateChannels ()
{
  Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue(Seconds (0.1)));
  Config::SetDefault ("ns3::ThreeGppChannelConditionModel::UpdatePeriod", TimeValue(Seconds (0.1)));

  double frequency = 60e9;
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
  m_lossModel = propagationLossModelFactory.Create<ThreeGppPropagationLossModel> ();
  m_lossModel->SetAttribute ("Frequency", DoubleValue (frequency));
  m_lossModel->SetAttribute ("ShadowingEnabled", BooleanValue (false));

  // create the spectrum propagation loss model
  m_spectrumLoss = CreateObject<ThreeGppSpectrumPropagationLossModel> ();
  m_spectrumLoss->SetChannelModelAttribute ("Frequency", DoubleValue (frequency));
  m_spectrumLoss->SetChannelModelAttribute ("Scenario", StringValue (scenario));

  // create the channel condition model and associate it with the spectrum and propagation loss model
  condModel = CreateObject<NeverLosChannelConditionModel> ();
  m_spectrumLoss->SetChannelModelAttribute ("ChannelConditionModel", PointerValue (condModel));
  m_lossModel->SetChannelConditionModel (condModel);

  delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  m_spectrumChannel = CreateObject<MultiModelSpectrumChannel> ();
  m_spectrumChannel->SetPropagationDelayModel (delayModel);
  m_spectrumChannel->AddPropagationLossModel (m_lossModel);
}

void
ScenarioN50::CreateNodes ()
{
  std::cout << "Creating " << (unsigned)m_nodeNum << " nodes.\n";
  m_nodes.Create (m_nodeNum);
}

void
ScenarioN50::CreateDevices ()
{
  CrMacHelper macHelper = CrMacHelper::Default();
  CrPhyHelper phyHelper = CrPhyHelper::Default(m_spectrumChannel);
  
  CrMmWaveHelper helper;
  
  helper.EnableLogComponents ();
  helper.SetSatandard (MMWAVE_COGNITIVE_RADIO_60GHz_160MHz);
  m_devices = helper.Install (phyHelper, macHelper, m_nodes);
  helper.AssignStreams (m_devices, 100);

  Ptr<ThreeGppAntennaArrayModel> a;

  for (NetDeviceContainer::Iterator i = m_devices.Begin (); i != m_devices.End (); ++i)
  {
      a = CreateObjectWithAttributes<ThreeGppAntennaArrayModel> ("NumColumns", UintegerValue (2), "NumRows", UintegerValue (2));
      m_spectrumLoss->AddDevice ((*i), a);
  }
  
  if (m_tracing)
  {
    AsciiTraceHelper ascii;
    Ptr<OutputStreamWrapper> osw = ascii.CreateFileStream (m_fileName + ".tr");
    phyHelper.EnableAsciiAll (osw);
  }
}

void
ScenarioN50::InstallInternetStack ()
{
  // AodvHelper aodv;
  // you can configure AODV attributes here using aodv.Set(name, value)
  InternetStackHelper stack;
  // stack.SetRoutingHelper (aodv); // has effect on the next Install ()
  stack.Install (m_nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.0.0.0");
  m_interfaces = address.Assign (m_devices);

  // if (m_tracing)
  //   {
  //     Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (m_fileName + ".routes", std::ios::out);
  //     aodv.PrintRoutingTableAllAt (Seconds (10), routingStream);
  //   }
}

void
ScenarioN50::CreateApp (uint32_t from, uint32_t to, uint32_t port, double startTime, double endTime)
{
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");

  Ptr<Socket> recvSink = Socket::CreateSocket (m_nodes.Get (to), tid);
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), port);
  recvSink->Bind (local);
  recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

  Ptr<Socket> source = Socket::CreateSocket (m_nodes.Get (from), tid);
  InetSocketAddress remote = InetSocketAddress (m_interfaces.GetAddress (to, 0), port);
  source->SetAllowBroadcast (true);
  source->Connect (remote);

  Simulator::ScheduleWithContext (source->GetNode ()->GetId (), Seconds (startTime), 
                                  &GenerateTraffic, source, m_pktSize, 
                                  Seconds (endTime), Seconds (m_interval));
}

void
ScenarioN50::InstallApplications ()
{
  CreateApp (21, 23, 80, 0, m_duration);//1
  CreateApp (23, 26, 80, 0, m_duration);//1
  CreateApp (25, 27, 81, 0, m_duration);//1
  CreateApp (15, 14, 83, 0, m_duration);//1
  CreateApp (14, 12, 84, 0, m_duration);//1
  CreateApp (12, 10, 85, 0, m_duration);//1
  CreateApp (38, 39, 86, 0, m_duration);//1
  CreateApp (36, 37, 88, 0, m_duration);//1
  CreateApp (30, 29, 89, 0, m_duration);//1
  CreateApp (29, 31, 90, 0, m_duration);//1
  CreateApp (31, 35, 91, 0, m_duration);//1
  CreateApp (33, 34, 93, 0, m_duration);//1
  CreateApp (41, 45, 96, 0, m_duration);//1
  CreateApp (45, 40, 97, 0, m_duration);//1
  CreateApp (40, 46, 98, 0, m_duration);//1
  CreateApp (46, 43, 99, 0, m_duration);//1
  CreateApp (16, 18, 100, 0, m_duration);//1
  CreateApp (18, 19, 101, 0, m_duration);//1
  CreateApp (19, 17, 102, 0, m_duration);//1
  CreateApp (6, 4, 103, 0, m_duration);//1
  CreateApp (2, 0, 105, 0, m_duration);//1
  
// 21 23 26
// 24 25 27 22
// 15 14 12 10
// 38 39 36 37 
// 30 29 31 35 33 34
// 42 44 41 45 40 46 43
// 16 18 19 17
// 7 3 8
// 6 4 2 0 1
}

int main (int argc, char *argv[])
{
  ScenarioN50 test;
  if (!test.Configure (argc, argv))
    NS_FATAL_ERROR ("Configuration failed. Aborted.");

  test.Install ();

  // Create Ns2MobilityHelper with the specified trace log file as parameter
  Ns2MobilityHelper ns2 = Ns2MobilityHelper ("./scratch/scenario-n50.ns_movements");
  ns2.Install (); // configure movements for each node, while reading trace file

  AnimationInterface anim(test.m_fileName + ".xml");
  anim.EnablePacketMetadata (true);
  anim.SetMobilityPollInterval (Seconds (1));

  std::cout << "Starting simulation for " << test.m_duration << " s ...\n";
  Simulator::Stop (Seconds (test.m_duration));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}


