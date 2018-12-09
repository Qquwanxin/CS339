#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <vector>
#include <algorithm> // std::move_backward
#include <random> // std::default_random_engine
#include <chrono> // std::chrono::system_clock
#include <time.h>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
 
using namespace ns3;
using namespace std;
 
NS_LOG_COMPONENT_DEFINE ("update");
int integerGenerate(int x)
{
  int n;
  srand(time(NULL));
  n = ((unsigned)rand() % x + 1);
  return n;
}
//Generate host pairs sequence 
bool hostSequenceGenerate(vector<int>&hostPairs)
{
  for (int i = 1; i < 50; ++i) 
  {
    hostPairs.push_back (i);
  }
  unsigned seed = chrono::system_clock::now ().time_since_epoch ().count ();
  shuffle (hostPairs.begin (), hostPairs.end (), default_random_engine (seed));
  return 1;
}
//Delay
static void
CalculateDelay (Ptr<const Packet>p,const Address &address)
{
    static float k = 0;
    k++;
    static float m = -1;
    static float n = 0;
    n += (p->GetUid() - m)/2-1;//Packet类型内置函数，packet以Uid为标识。GetUid，获得包的uid，m是之前包的uid
    delayJitter.RecordRx(p);//更新延迟和偏差
    Time t = delayJitter.GetLastDelay();//t是延迟
    cout << Simulator::Now().GetSeconds () << "\t" << t.GetMilliSeconxds() << endl;//Now():现在模拟的虚拟时间。GetSeconds()和GetMilliSeconds(),该单元中存储的大概时间
    m = p->GetUid();//m是包的Uid
}


class MyApp : public Application
{
public:
  MyApp ();
  virtual ~MyApp();

  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);
  void ChangeRate(DataRate newrate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
};

MyApp::MyApp ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_nPackets (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0)
{
}

MyApp::~MyApp()
{
  m_socket = 0;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

void
MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
  {
    Simulator::Cancel (m_sendEvent);
  }

  if (m_socket)
  {
    m_socket->Close ();
  }
}

void
MyApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if (++m_packetsSent < m_nPackets)
  {
    ScheduleTx ();
  }
}

void
MyApp::ScheduleTx (void)
{
  if (m_running)
  {
    Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
    m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
  }
}

void
MyApp::ChangeRate(DataRate newrate)
{
  m_dataRate = newrate;
  return;
}
void
IncRate (Ptr<MyApp> app, DataRate rate)
{
  app->ChangeRate(rate);
  return;
}


int 
main (int argc, char *argv[])
{
  // Users may find it convenient to turn on explicit debugging
  // for selected modules; the below lines suggest how to do this
  LogComponentEnable ("update", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  //string lat = "2ms";
  //string rate = "500kb/s"; // P2P link
  // Set up some default values for the simulation.  Use the 
  //Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (210));
  //Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("448kb/s"));
  std::string lat = "2ms";
  std::string rate = "500kb/s"; // P2P link
  bool enableFlowMonitor = false;

  LogComponentEnable ("update", LOG_LEVEL_ALL);
  
  CommandLine cmd;
  cmd.AddValue ("latency", "P2P link Latency in miliseconds", lat);
  cmd.AddValue ("rate", "P2P data rate in bps", rate);
  cmd.AddValue ("EnableMonitor", "Enable Flow Monitor", enableFlowMonitor);
  cmd.Parse (argc, argv);
 
  // Here, we will explicitly create four nodes.  In more sophisticated
  // topologies, we could configure a node factory.
  NS_LOG_INFO ("Create nodes.");
  NodeContainer r;
  r.Create(81);
  NodeContainer edge[100];
   
  //1-30号路由器之间的连接
  edge[0] = NodeContainer (r.Get (1), r.Get (2));
  edge[1] = NodeContainer (r.Get (1), r.Get (4));
  edge[2] = NodeContainer (r.Get (1), r.Get (5));
  edge[3] = NodeContainer (r.Get (2), r.Get (3));
  edge[4] = NodeContainer (r.Get (3), r.Get (4));
  edge[5] = NodeContainer (r.Get (3), r.Get (8));
  edge[6] = NodeContainer (r.Get (4), r.Get (6));
  edge[7] = NodeContainer (r.Get (5), r.Get (6));
  edge[8] = NodeContainer (r.Get (5), r.Get (9));
  edge[9] = NodeContainer (r.Get (6), r.Get (7));
  edge[10] = NodeContainer (r.Get (7), r.Get (8));
  edge[11] = NodeContainer (r.Get (7), r.Get (13));
  edge[12] = NodeContainer (r.Get (8), r.Get (10));
  edge[13] = NodeContainer (r.Get (9), r.Get (11));
  edge[14] = NodeContainer (r.Get (9), r.Get (12));
  edge[15] = NodeContainer (r.Get (10), r.Get (13));
  edge[16] = NodeContainer (r.Get (10), r.Get (16));
  edge[17] = NodeContainer (r.Get (11), r.Get (12));
  edge[18] = NodeContainer (r.Get (11), r.Get (14));
  edge[19] = NodeContainer (r.Get (11), r.Get (22));
  edge[20] = NodeContainer (r.Get (12), r.Get (13));
  edge[21] = NodeContainer (r.Get (12), r.Get (14));
  edge[22] = NodeContainer (r.Get (13), r.Get (20));
  edge[23] = NodeContainer (r.Get (14), r.Get (15));
  edge[24] = NodeContainer (r.Get (14), r.Get (17));
  edge[25] = NodeContainer (r.Get (16), r.Get (20));
  edge[26] = NodeContainer (r.Get (17), r.Get (18));
  edge[27] = NodeContainer (r.Get (17), r.Get (19));
  edge[28] = NodeContainer (r.Get (18), r.Get (19));
  edge[29] = NodeContainer (r.Get (18), r.Get (23));
  edge[30] = NodeContainer (r.Get (19), r.Get (20));
  edge[31] = NodeContainer (r.Get (19), r.Get (23));
  edge[32] = NodeContainer (r.Get (20), r.Get (21));
  edge[33] = NodeContainer (r.Get (20), r.Get (24));
  edge[34] = NodeContainer (r.Get (20), r.Get (26));
  edge[35] = NodeContainer (r.Get (21), r.Get (29));
  edge[36] = NodeContainer (r.Get (22), r.Get (23));
  edge[37] = NodeContainer (r.Get (22), r.Get (25));
  edge[38] = NodeContainer (r.Get (22), r.Get (30));
  edge[39] = NodeContainer (r.Get (23), r.Get (24));
  edge[40] = NodeContainer (r.Get (23), r.Get (25));
  edge[41] = NodeContainer (r.Get (24), r.Get (25));
  edge[42] = NodeContainer (r.Get (24), r.Get (28));
  edge[43] = NodeContainer (r.Get (25), r.Get (27));
  edge[44] = NodeContainer (r.Get (26), r.Get (28));
  edge[45] = NodeContainer (r.Get (26), r.Get (29));
  edge[46] = NodeContainer (r.Get (27), r.Get (28));
  edge[47] = NodeContainer (r.Get (27), r.Get (30));
  edge[48] = NodeContainer (r.Get (28), r.Get (29));
  edge[49] = NodeContainer (r.Get (29), r.Get (30));
  //1-50号主机和路由器相连接  
  edge[50]= NodeContainer(r.Get(1), r.Get(31));
  edge[51]= NodeContainer(r.Get(1), r.Get(32));
  edge[52]= NodeContainer(r.Get(2), r.Get(33));
  edge[53]= NodeContainer(r.Get(2), r.Get(34));
  edge[54]= NodeContainer(r.Get(3), r.Get(35));
  edge[55]= NodeContainer(r.Get(3), r.Get(36));
  edge[56]= NodeContainer(r.Get(4), r.Get(37));
  edge[57]= NodeContainer(r.Get(4), r.Get(38));
  edge[58]= NodeContainer(r.Get(5), r.Get(39));
  edge[59]= NodeContainer(r.Get(5), r.Get(40));
  edge[60]= NodeContainer(r.Get(6), r.Get(41));
  edge[61]= NodeContainer(r.Get(8), r.Get(42));
  edge[62]= NodeContainer(r.Get(8), r.Get(43));
  edge[63]= NodeContainer(r.Get(8), r.Get(44));
  edge[64]= NodeContainer(r.Get(9), r.Get(45));
  edge[65]= NodeContainer(r.Get(9), r.Get(46));
  edge[66]= NodeContainer(r.Get(9), r.Get(47));
  edge[67]= NodeContainer(r.Get(13), r.Get(48));
  edge[68]= NodeContainer(r.Get(10), r.Get(49));
  edge[69]= NodeContainer(r.Get(11), r.Get(50));
  edge[70]= NodeContainer(r.Get(11), r.Get(51));
  edge[71]= NodeContainer(r.Get(12), r.Get(52));
  edge[72]= NodeContainer(r.Get(14), r.Get(53));
  edge[73]= NodeContainer(r.Get(14), r.Get(54));
  edge[74]= NodeContainer(r.Get(15), r.Get(55));
  edge[75]= NodeContainer(r.Get(16), r.Get(56));
  edge[76]= NodeContainer(r.Get(16), r.Get(57));
  edge[77]= NodeContainer(r.Get(16), r.Get(58));
  edge[78]= NodeContainer(r.Get(17), r.Get(59));
  edge[79]= NodeContainer(r.Get(17), r.Get(60));
  edge[80]= NodeContainer(r.Get(17), r.Get(61));
  edge[81]= NodeContainer(r.Get(18), r.Get(62));
  edge[82]= NodeContainer(r.Get(19), r.Get(63));
  edge[83]= NodeContainer(r.Get(20), r.Get(64));
  edge[84]= NodeContainer(r.Get(21), r.Get(65));
  edge[85]= NodeContainer(r.Get(21), r.Get(66));
  edge[86]= NodeContainer(r.Get(21), r.Get(67));
  edge[87]= NodeContainer(r.Get(21), r.Get(68));
  edge[88]= NodeContainer(r.Get(22), r.Get(69));
  edge[89]= NodeContainer(r.Get(26), r.Get(70));
  edge[90]= NodeContainer(r.Get(30), r.Get(71));
  edge[91]= NodeContainer(r.Get(30), r.Get(72));
  edge[92]= NodeContainer(r.Get(30), r.Get(73));
  edge[93]= NodeContainer(r.Get(28), r.Get(74));
  edge[94]= NodeContainer(r.Get(29), r.Get(75));
  edge[95]= NodeContainer(r.Get(29), r.Get(76));
  edge[96]= NodeContainer(r.Get(29), r.Get(77));
  edge[97]= NodeContainer(r.Get(24), r.Get(78));
  edge[98]= NodeContainer(r.Get(27), r.Get(79));
  edge[99]= NodeContainer(r.Get(25), r.Get(80));
    
  InternetStackHelper internet;
  internet.Install (r);
  
  // We create the channels first without any IP addressing information
  NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
  
  NetDeviceContainer device[100];
  for(int i=0;i<100;++i)   device[i] = p2p.Install (edge[i]);

  NS_LOG_INFO ("Build Error Rate Model.");
  Ptr<RateErrorModel>em=CreateObject<RateErrorModel> ();
  em->SetAttribute("ErrorRate",DoubleValue(0.00001));
  //错误率0.0001
  for(int i=1;i<=80;++i)
  device[i].Get(1)->SetAttribute("ReceiveErrorModel",PointerValue (em));
   
  // Later, we add IP addresses.
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;

  Ipv4InterfaceContainer interfaces[100];
  
  ipv4.SetBase ("10.1.0.0", "255.255.255.0");
  interfaces[0] = ipv4.Assign (device[0]);
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  interfaces[1] = ipv4.Assign (device[1]);
  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  interfaces[2] = ipv4.Assign (device[2]);
  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  interfaces[3] = ipv4.Assign (device[3]);
  ipv4.SetBase ("10.1.4.0", "255.255.255.0");
  interfaces[4] = ipv4.Assign (device[4]);
  ipv4.SetBase ("10.1.5.0", "255.255.255.0");
  interfaces[5] = ipv4.Assign (device[5]);
  ipv4.SetBase ("10.1.6.0", "255.255.255.0");
  interfaces[6] = ipv4.Assign (device[6]);
  ipv4.SetBase ("10.1.7.0", "255.255.255.0");
  interfaces[7] = ipv4.Assign (device[7]);
  ipv4.SetBase ("10.1.8.0", "255.255.255.0");
  interfaces[8] = ipv4.Assign (device[8]);
  ipv4.SetBase ("10.1.9.0", "255.255.255.0");
  interfaces[9] = ipv4.Assign (device[9]);
  ipv4.SetBase ("10.1.10.0", "255.255.255.0");
  interfaces[10] = ipv4.Assign (device[10]);
  ipv4.SetBase ("10.1.11.0", "255.255.255.0");
  interfaces[11] = ipv4.Assign (device[11]);
  ipv4.SetBase ("10.1.12.0", "255.255.255.0");
  interfaces[12] = ipv4.Assign (device[12]);
  ipv4.SetBase ("10.1.13.0", "255.255.255.0");
  interfaces[13] = ipv4.Assign (device[13]);
  ipv4.SetBase ("10.1.14.0", "255.255.255.0");
  interfaces[14] = ipv4.Assign (device[14]);
  ipv4.SetBase ("10.1.15.0", "255.255.255.0");
  interfaces[15] = ipv4.Assign (device[15]);
  ipv4.SetBase ("10.1.16.0", "255.255.255.0");
  interfaces[16] = ipv4.Assign (device[16]);
  ipv4.SetBase ("10.1.17.0", "255.255.255.0");
  interfaces[17] = ipv4.Assign (device[17]);
  ipv4.SetBase ("10.1.18.0", "255.255.255.0");
  interfaces[18] = ipv4.Assign (device[18]);
  ipv4.SetBase ("10.1.19.0", "255.255.255.0");
  interfaces[19] = ipv4.Assign (device[19]);
  ipv4.SetBase ("10.1.20.0", "255.255.255.0");
  interfaces[20] = ipv4.Assign (device[20]);
  ipv4.SetBase ("10.1.21.0", "255.255.255.0");
  interfaces[21] = ipv4.Assign (device[21]);
  ipv4.SetBase ("10.1.22.0", "255.255.255.0");
  interfaces[22] = ipv4.Assign (device[22]);
  ipv4.SetBase ("10.1.23.0", "255.255.255.0");
  interfaces[23] = ipv4.Assign (device[23]);
  ipv4.SetBase ("10.1.24.0", "255.255.255.0");
  interfaces[24] = ipv4.Assign (device[24]);
  ipv4.SetBase ("10.1.25.0", "255.255.255.0");
  interfaces[25] = ipv4.Assign (device[25]);
  ipv4.SetBase ("10.1.26.0", "255.255.255.0");
  interfaces[26] = ipv4.Assign (device[26]);
  ipv4.SetBase ("10.1.27.0", "255.255.255.0");
  interfaces[27] = ipv4.Assign (device[27]);
  ipv4.SetBase ("10.1.28.0", "255.255.255.0");
  interfaces[28] = ipv4.Assign (device[28]);
  ipv4.SetBase ("10.1.29.0", "255.255.255.0");
  interfaces[29] = ipv4.Assign (device[29]);
  ipv4.SetBase ("10.1.30.0", "255.255.255.0");
  interfaces[30] = ipv4.Assign (device[30]);
  ipv4.SetBase ("10.1.31.0", "255.255.255.0");
  interfaces[31] = ipv4.Assign (device[31]);
  ipv4.SetBase ("10.1.32.0", "255.255.255.0");
  interfaces[32] = ipv4.Assign (device[32]);
  ipv4.SetBase ("10.1.33.0", "255.255.255.0");
  interfaces[33] = ipv4.Assign (device[33]);
  ipv4.SetBase ("10.1.34.0", "255.255.255.0");
  interfaces[34] = ipv4.Assign (device[34]);
  ipv4.SetBase ("10.1.35.0", "255.255.255.0");
  interfaces[35] = ipv4.Assign (device[35]);
  ipv4.SetBase ("10.1.36.0", "255.255.255.0");
  interfaces[36] = ipv4.Assign (device[36]);
  ipv4.SetBase ("10.1.37.0", "255.255.255.0");
  interfaces[37] = ipv4.Assign (device[37]);
  ipv4.SetBase ("10.1.38.0", "255.255.255.0");
  interfaces[38] = ipv4.Assign (device[38]);
  ipv4.SetBase ("10.1.39.0", "255.255.255.0");
  interfaces[39] = ipv4.Assign (device[39]);
  ipv4.SetBase ("10.1.40.0", "255.255.255.0");
  interfaces[40] = ipv4.Assign (device[40]);
  ipv4.SetBase ("10.1.41.0", "255.255.255.0");
  interfaces[41] = ipv4.Assign (device[41]);
  ipv4.SetBase ("10.1.42.0", "255.255.255.0");
  interfaces[42] = ipv4.Assign (device[42]);
  ipv4.SetBase ("10.1.43.0", "255.255.255.0");
  interfaces[43] = ipv4.Assign (device[43]);
  ipv4.SetBase ("10.1.44.0", "255.255.255.0");
  interfaces[44] = ipv4.Assign (device[44]);
  ipv4.SetBase ("10.1.45.0", "255.255.255.0");
  interfaces[45] = ipv4.Assign (device[45]);
  ipv4.SetBase ("10.1.46.0", "255.255.255.0");
  interfaces[46] = ipv4.Assign (device[46]);
  ipv4.SetBase ("10.1.47.0", "255.255.255.0");
  interfaces[47] = ipv4.Assign (device[47]);
  ipv4.SetBase ("10.1.48.0", "255.255.255.0");
  interfaces[48] = ipv4.Assign (device[48]);
  ipv4.SetBase ("10.1.49.0", "255.255.255.0");
  interfaces[49] = ipv4.Assign (device[49]);
  ipv4.SetBase ("10.1.50.0", "255.255.255.0");
  interfaces[50] = ipv4.Assign (device[50]);
  ipv4.SetBase ("10.1.51.0", "255.255.255.0");
  interfaces[51] = ipv4.Assign (device[51]);
  ipv4.SetBase ("10.1.52.0", "255.255.255.0");
  interfaces[52] = ipv4.Assign (device[52]);
  ipv4.SetBase ("10.1.53.0", "255.255.255.0");
  interfaces[53] = ipv4.Assign (device[53]);
  ipv4.SetBase ("10.1.54.0", "255.255.255.0");
  interfaces[54] = ipv4.Assign (device[54]);
  ipv4.SetBase ("10.1.55.0", "255.255.255.0");
  interfaces[55] = ipv4.Assign (device[55]);
  ipv4.SetBase ("10.1.56.0", "255.255.255.0");
  interfaces[56] = ipv4.Assign (device[56]);
  ipv4.SetBase ("10.1.57.0", "255.255.255.0");
  interfaces[57] = ipv4.Assign (device[57]);
  ipv4.SetBase ("10.1.58.0", "255.255.255.0");
  interfaces[58] = ipv4.Assign (device[58]);
  ipv4.SetBase ("10.1.59.0", "255.255.255.0");
  interfaces[59] = ipv4.Assign (device[59]);
  ipv4.SetBase ("10.1.60.0", "255.255.255.0");
  interfaces[60] = ipv4.Assign (device[60]);
  ipv4.SetBase ("10.1.61.0", "255.255.255.0");
  interfaces[61] = ipv4.Assign (device[61]);
  ipv4.SetBase ("10.1.62.0", "255.255.255.0");
  interfaces[62] = ipv4.Assign (device[62]);
  ipv4.SetBase ("10.1.63.0", "255.255.255.0");
  interfaces[63] = ipv4.Assign (device[63]);
  ipv4.SetBase ("10.1.64.0", "255.255.255.0");
  interfaces[64] = ipv4.Assign (device[64]);
  ipv4.SetBase ("10.1.65.0", "255.255.255.0");
  interfaces[65] = ipv4.Assign (device[65]);
  ipv4.SetBase ("10.1.66.0", "255.255.255.0");
  interfaces[66] = ipv4.Assign (device[66]);
  ipv4.SetBase ("10.1.67.0", "255.255.255.0");
  interfaces[67] = ipv4.Assign (device[67]);
  ipv4.SetBase ("10.1.68.0", "255.255.255.0");
  interfaces[68] = ipv4.Assign (device[68]);
  ipv4.SetBase ("10.1.69.0", "255.255.255.0");
  interfaces[69] = ipv4.Assign (device[69]);
  ipv4.SetBase ("10.1.70.0", "255.255.255.0");
  interfaces[70] = ipv4.Assign (device[70]);
  ipv4.SetBase ("10.1.71.0", "255.255.255.0");
  interfaces[71] = ipv4.Assign (device[71]);
  ipv4.SetBase ("10.1.72.0", "255.255.255.0");
  interfaces[72] = ipv4.Assign (device[72]);
  ipv4.SetBase ("10.1.73.0", "255.255.255.0");
  interfaces[73] = ipv4.Assign (device[73]);
  ipv4.SetBase ("10.1.74.0", "255.255.255.0");
  interfaces[74] = ipv4.Assign (device[74]);
  ipv4.SetBase ("10.1.75.0", "255.255.255.0");
  interfaces[75] = ipv4.Assign (device[75]);
  ipv4.SetBase ("10.1.76.0", "255.255.255.0");
  interfaces[76] = ipv4.Assign (device[76]);
  ipv4.SetBase ("10.1.77.0", "255.255.255.0");
  interfaces[77] = ipv4.Assign (device[77]);
  ipv4.SetBase ("10.1.78.0", "255.255.255.0");
  interfaces[78] = ipv4.Assign (device[78]);
  ipv4.SetBase ("10.1.79.0", "255.255.255.0");
  interfaces[79] = ipv4.Assign (device[79]);
  ipv4.SetBase ("10.1.80.0", "255.255.255.0");
  interfaces[80] = ipv4.Assign (device[80]);
  ipv4.SetBase ("10.1.81.0", "255.255.255.0");
  interfaces[81] = ipv4.Assign (device[81]);
  ipv4.SetBase ("10.1.82.0", "255.255.255.0");
  interfaces[82] = ipv4.Assign (device[82]);
  ipv4.SetBase ("10.1.83.0", "255.255.255.0");
  interfaces[83] = ipv4.Assign (device[83]);
  ipv4.SetBase ("10.1.84.0", "255.255.255.0");
  interfaces[84] = ipv4.Assign (device[84]);
  ipv4.SetBase ("10.1.85.0", "255.255.255.0");
  interfaces[85] = ipv4.Assign (device[85]);
  ipv4.SetBase ("10.1.86.0", "255.255.255.0");
  interfaces[86] = ipv4.Assign (device[86]);
  ipv4.SetBase ("10.1.87.0", "255.255.255.0");
  interfaces[87] = ipv4.Assign (device[87]);
  ipv4.SetBase ("10.1.88.0", "255.255.255.0");
  interfaces[88] = ipv4.Assign (device[88]);
  ipv4.SetBase ("10.1.89.0", "255.255.255.0");
  interfaces[89] = ipv4.Assign (device[89]);
  ipv4.SetBase ("10.1.90.0", "255.255.255.0");
  interfaces[90] = ipv4.Assign (device[90]);
  ipv4.SetBase ("10.1.91.0", "255.255.255.0");
  interfaces[91] = ipv4.Assign (device[91]);
  ipv4.SetBase ("10.1.92.0", "255.255.255.0");
  interfaces[92] = ipv4.Assign (device[92]);
  ipv4.SetBase ("10.1.93.0", "255.255.255.0");
  interfaces[93] = ipv4.Assign (device[93]);
  ipv4.SetBase ("10.1.94.0", "255.255.255.0");
  interfaces[94] = ipv4.Assign (device[94]);
  ipv4.SetBase ("10.1.95.0", "255.255.255.0");
  interfaces[95] = ipv4.Assign (device[95]);
  ipv4.SetBase ("10.1.96.0", "255.255.255.0");
  interfaces[96] = ipv4.Assign (device[96]);
  ipv4.SetBase ("10.1.97.0", "255.255.255.0");
  interfaces[97] = ipv4.Assign (device[97]);
  ipv4.SetBase ("10.1.98.0", "255.255.255.0");
  interfaces[98] = ipv4.Assign (device[98]);
  ipv4.SetBase ("10.1.99.0", "255.255.255.0");
  interfaces[99] = ipv4.Assign (device[99]);

  NS_LOG_INFO ("Enable static global routing.");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  NS_LOG_INFO ("Create Applications.");
  int hostNumber;
  vector<int> hostPairs;
  double t;
  
  for(t=0;t<1.2;t+=0.1)
  {
    hostNumber = integerGenerate(25);
    hostSequenceGenerate(hostPairs);
    for(int hp=0;hp<hostNumber;++hp)
    {
      //sink
      uint16_t sinkPort = 1;
      Address sinkAddress (InetSocketAddress (interfaces[hostPairs[hp]+49].GetAddress (1), sinkPort)); // interface of sink
      PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
      ApplicationContainer sinkApps = packetSinkHelper.Install (r.Get (hostPairs[hp]+30)); //sink
      sinkApps.Start (Seconds (t));
      sinkApps.Stop (Seconds (t+1));

      //source
      Ptr<Socket> ns3UdpSocket = Socket::CreateSocket (r.Get (hostPairs[hp+hostNumber]+30), UdpSocketFactory::GetTypeId ()); //source
      // Create UDP application at source
      Ptr<MyApp> app = CreateObject<MyApp> ();
      app->Setup (ns3UdpSocket, sinkAddress, 1040, 100000, DataRate ("250Kbps"));
      r.Get (hostPairs[hp+hostNumber]+30)->AddApplication (app);
      app->SetStartTime (Seconds (t));
      app->SetStopTime (Seconds (t+0.1);
      
      ++sinkPort;
    }
  }

  Ptr<FlowMonitor> flowmon;
  FlowMonitorHelper flowmonHelper;
  flowmon = flowmonHelper.InstallAll ();

  sinkApps.Get(0)->TraceConnectWithoutContext("Rx", MakeCallback(&CalculateDelay));

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds(120.0));
  Simulator::Run ();
      flowmon->CheckForLostPackets ();
      flowmon->SerializeToXmlFile("lab-2.flowmon", true, true);
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
  return 0;
}