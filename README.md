# CogGroup-MAC

CogGroup-MAC based on NS-3.33





### Prerequisites

The following list of packages should be accurate through the Ubuntu  21.04 release; other releases or other Debian-based systems may slightly vary.  Ubuntu 16.04 LTS release is probably the oldest release that is  known to work with recent ns-3 releases.

**Note:** As of ns-3.30 release (August 2019), ns-3 uses  Python 3 by default, but earlier releases depend on Python 2 packages,  and at least a Python 2 interpreter is recommended.  If working with an  earlier release, one may in general substitute 'python' for 'python3' in the below (e.g. install 'python-dev' instead of 'python3-dev').

-  **minimal requirements for C++ users (release):**  This is the minimal set of packages needed to run ns-3 from a released tarball.  

```
 apt install g++ python3
```

-  **minimal requirements for Python API users (release 3.30 and newer, and ns-3-dev):** This is the minimal set of packages needed to work with Python bindings from a released tarball.

```
 apt install g++ python3 python3-dev pkg-config sqlite3
```

-  **minimal requirements for Python (development):** For use  of ns-3-allinone repository (cloned from Git), additional packages are  needed to fetch and successfully install pybindgen and netanim.

```
 apt install python3-setuptools git
```

-  **Netanim animator:**  qt5 development tools are needed for Netanim animator; qt4 will also work but we have migrated to qt5.

```
 apt install qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools
```

**Note:** For Ubuntu 20.10 and earlier, the single 'qt5-default' package suffices

```
 apt install qt5-default
```

-  Support for ns-3-pyviz visualizer

- -  For Ubuntu 18.04 and later, python-pygoocanvas is no  longer provided.  The ns-3.29 release and later upgrades the support to  GTK+ version 3, and requires these packages:

```
 apt install gir1.2-goocanvas-2.0 python3-gi python3-gi-cairo python3-pygraphviz gir1.2-gtk-3.0 ipython3  
```

- -  For ns-3.28 and earlier releases, PyViz is based on GTK+ 2, GooCanvas, and GraphViz:

```
 apt install python-pygraphviz python-kiwi python-pygoocanvas libgoocanvas-dev ipython
```

-  Support for MPI-based distributed emulation

```
apt install openmpi-bin openmpi-common openmpi-doc libopenmpi-dev
```

-  Support for bake build tool:

```
 apt install autoconf cvs bzr unrar
```

-  Debugging:

```
 apt install gdb valgrind 
```

-  Support for utils/check-style.py code style check program

```
apt install uncrustify
```

-  Doxygen and related inline documentation:

```
 apt install doxygen graphviz imagemagick
 apt install texlive texlive-extra-utils texlive-latex-extra texlive-font-utils dvipng latexmk
```

- 
  -  If you get an error such as 'convert ... not authorized  source-temp/figures/lena-dual-stripe.eps', see this post about editing  ImageMagick's security policy configuration: https://cromwell-intl.com/open-source/pdf-not-authorized.html.  In brief, you will want to make this kind of change to ImageMagick security policy:

```
   --- ImageMagick-6/policy.xml.bak	2020-04-28 21:10:08.564613444 -0700
   +++ ImageMagick-6/policy.xml	2020-04-28 21:10:29.413438798 -0700
   @@ -87,10 +87,10 @@
      <policy domain="path" rights="none" pattern="@*"/>
   -  <policy domain="coder" rights="none" pattern="PS" />
   +  <policy domain="coder" rights="read|write" pattern="PS" />
      <policy domain="coder" rights="none" pattern="PS2" />
      <policy domain="coder" rights="none" pattern="PS3" />
      <policy domain="coder" rights="none" pattern="EPS" />
   -  <policy domain="coder" rights="none" pattern="PDF" />
   +  <policy domain="coder" rights="read|write" pattern="PDF" />
      <policy domain="coder" rights="none" pattern="XPS" />
    </policymap>
```



-  The ns-3 manual and tutorial are written in reStructuredText  for Sphinx (doc/tutorial, doc/manual, doc/models), and figures typically in dia (also needs the texlive packages above):

```
 apt install python3-sphinx dia 
```

**Note:** Sphinx version >= 1.12 required for ns-3.15.  To  check your version, type "sphinx-build".  To fetch this package alone,  outside of the Ubuntu package system, try "sudo easy_install -U Sphinx".

-  GNU Scientific Library (GSL) support for more accurate 802.11b WiFi error models (not needed for OFDM):

```
 apt install gsl-bin libgsl-dev libgslcblas0
```

If the above doesn't work (doesn't detect GSL on the system), consult: https://coral.ise.lehigh.edu/jild13/2016/07/11/hello/.  But don't worry if you are not using 802.11b models.

-  To read pcap packet traces

```
apt install tcpdump
```

-  Database support for statistics framework

```
apt install sqlite sqlite3 libsqlite3-dev
```

-  Xml-based version of the config store (requires libxml2 >= version 2.7)

```
apt install libxml2 libxml2-dev
```

-  Support for generating modified python bindings 

```
 apt install cmake libc6-dev libc6-dev-i386 libclang-dev llvm-dev automake python3-pip
 python3 -m pip install --user cxxfilt
```

and you will want to install castxml and pygccxml as per the instructions for python bindings (or through the *bake* build tool as described in the tutorial).  The 'castxml' and 'pygccxml' packages provided by Ubuntu 18.04 and earlier are not recommended; a  source build (coordinated via bake) is recommended.  If you plan to work with bindings or rescan them for any ns-3 C++ changes you might make,  please read the [chapter in the manual](https://www.nsnam.org/docs/manual/html/python.html) on this topic.

**Note:** Ubuntu versions (through 19.04) and systems based on it (e.g. Linux Mint 18) default to an old version of clang and llvm  (3.8), when simply 'libclang-dev' and 'llvm-dev' are specified.     The  packaging on these 3.8 versions is broken.  Users of Ubuntu will want to explicitly install a newer version by specifying 'libclang-6.0-dev' and 'llvm-6.0-dev'.  Other versions newer than 6.0 may work (not tested).

-  A GTK-based configuration system

```
 apt install libgtk-3-dev
```

-  To experiment with virtual machines and ns-3

```
 apt install vtun lxc uml-utilities
```

-  Support for openflow module (requires libxml2-dev if not installed above) and Boost development libraries

```
apt install libxml2 libxml2-dev libboost-all-dev
```

### Downloading a release of ns-3 as a source archive

Type the following:

```
$ cd
$ mkdir workspace
$ cd workspace
$ wget https://www.nsnam.org/release/ns-allinone-3.33.tar.bz2
$ tar xjf ns-allinone-3.33.tar.bz2
```

Following these steps, if you change into the directory `ns-allinone-3.33`, you should see a number of files and directories

```
$ cd ns-allinone-3.32
$ ls
bake      constants.py   ns-3.33                            README
build.py  netanim-3.108  pybindgen-0.21.0                   util.py
```

### Building with `build.py`

Change into the directory `ns-allinone-3.33` under your `~/workspace` directory. Type the following:

```
$ ./build.py --enable-examples --enable-tests
```

### Building with Waf

Up to this point, we have used either the build.py script, or the bake tool, to get started with building *ns-3*.  These tools are useful for building *ns-3* and supporting libraries, and they call into the *ns-3* directory to call the Waf build tool to do the actual building. An installation of Waf is bundled with the *ns-3* source code. Most users quickly transition to using Waf directly to configure and build *ns-3*.  So, to proceed, please change your working directory to the *ns-3* directory that you have initially built.

Now go ahead and switch back to the debug build that includes the examples and tests.

```
$ ./waf clean
$ ./waf configure --build-profile=debug --enable-examples --enable-tests
```

The build system is now configured and you can build the debug versions of the *ns-3* programs by simply typing:

```
$ ./waf
```

Although the above steps made you build the *ns-3* part of the system twice, now you know how to change the configuration and build optimized code.



### Extending CogGroup-MAC Module

Copy and paste folder `~/CogGroup-MAC/src/` and folder `~/CogGroup-MAC/scratch/` in `~/CogGroup-MAC/` into `~/workspace/ns-allinone-3.32/ns-3.33` directory, This step will overwrite some files.



### Rebuilding with Waf

Type the following:

```
$ ./waf clean
$ ./waf configure --build-profile=debug --enable-examples --enable-tests
$ ./waf
```

### Running an example of CogGroup-MAC

Type the following:

```
$ ./waf --run scratch/n50-1
```

**n50-1.cc**

```
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
  m_fileName = "./scratch/scenario-n50-1";
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
  helper.SetSatandard (MMWAVE_COGNITIVE_RADIO_60GHz_40MHz);
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
  CreateApp (21, 23, 80, 0, m_duration);
  CreateApp (23, 26, 80, 0, m_duration);
  CreateApp (25, 27, 81, 0, m_duration);
  CreateApp (15, 14, 83, 0, m_duration);
  CreateApp (14, 12, 84, 0, m_duration);
  CreateApp (12, 10, 85, 0, m_duration);
  CreateApp (38, 39, 86, 0, m_duration);
  CreateApp (36, 37, 88, 0, m_duration);
  CreateApp (30, 29, 89, 0, m_duration);
  CreateApp (29, 31, 90, 0, m_duration);
  CreateApp (31, 35, 91, 0, m_duration);
  CreateApp (33, 34, 93, 0, m_duration);
  CreateApp (41, 45, 96, 0, m_duration);
  CreateApp (45, 40, 97, 0, m_duration);
  CreateApp (40, 46, 98, 0, m_duration);
  CreateApp (46, 43, 99, 0, m_duration);
  CreateApp (16, 18, 100, 0, m_duration);
  CreateApp (18, 19, 101, 0, m_duration);
  CreateApp (19, 17, 102, 0, m_duration);
  CreateApp (6, 4, 103, 0, m_duration);
  CreateApp (2, 0, 105, 0, m_duration);
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
```



**Note:**

**helper.SetSatandard (MMWAVE_COGNITIVE_RADIO_XXX);**This parameter **MMWAVE_COGNITIVE_RADIO_XXX** specifies the multi-channel specification definition used by coggroup Mac, where the center frequency and bandwidth of all candidate channels are defined. 

**MMWAVE_COGNITIVE_RADIO_60GHz_40MHz:** shows that the current coggroup MAC candidate channels are four channels with 40MHz bandwidth in the 60GHz range.
