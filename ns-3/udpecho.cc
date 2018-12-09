#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"

#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/ptr.h"//ns3::Ptr<T>;
#include "ns3/object.h"//包括类: RateErrorModel
#include"ns3/simulator.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <vector>
#include <algorithm> // std::move_backward
#include <random> // std::default_random_engine
#include <chrono> // std::chrono::system_clock
#include <time.h>
 
using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("echo");

int integerGenerate(int x)
{
    int n;
    srand(time(NULL));
    n = ((unsigned)rand() % x + 1);
    return n;
}

//Generate host pairs sequence 
bool
hostSequenceGenerate(vector<int>&hostPairs)
{
    for (int i = 1; i < 50; ++i) {
        hostPairs.push_back (i);
    }
    unsigned seed = chrono::system_clock::now ().time_since_epoch ().count ();
    shuffle (hostPairs.begin (), hostPairs.end (), default_random_engine (seed));
    return 1;
}
/*
static void
CwndChange (uint32_t oldCwnd, uint32_t newCwnd)
{
  cout<< "Cwnd:"<<Simulator::Now ().GetSeconds () << "\t" << newCwnd<<endl;
}
*/
/*
void
ReceivePacket(Ptr<const Packet> p, const Address & addr)
{
	std::cout << Simulator::Now ().GetSeconds () << "\t" << p->GetSize() <<"\n";
}
*/

int 
main (int argc, char *argv[])
{
    bool verbose = false;
    bool throughputPrintf = false;
    bool timeDelayPrintf = false;
    //bool tracing = false;

    CommandLine cmd;
    cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
    cmd.AddValue ("throughputPrintf", "printf throuthput if true", throughputPrintf);
    cmd.AddValue ("timeDelayPrintf", "print time delay if true", timeDelayPrintf);
    //cmd.AddValue ("tracing", "Enable pcap tracing", tracing);
    cmd.Parse (argc,argv);

    // Check for valid number of csma or wifi nodes
    // 250 should be enough, otherwise IP addresses 
    // soon become an issue
    if (verbose)
        {
        LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
        }

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

    NS_LOG_INFO ("Create channels.");
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
    p2p.SetQueue("ns3::DropTailQueue", "MaxPackets", UintegerValue(9986));


    NetDeviceContainer device[100];
    for(int i=0;i<100;++i)   device[i] = p2p.Install (edge[i]);

    Ptr<RateErrorModel>em=CreateObject<RateErrorModel> ();
    em->SetAttribute("ErrorRate",DoubleValue(0.0001));
    //错误率0.0001
    for(int i=1;i<=80;++i)
    device[i].Get(1)->SetAttribute("ReceiveErrorModel",PointerValue (em));

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

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    int hostNumber;
    vector<int> hostPairs;

    
    int p=1;
    double t;
    for(t=0;t<120.0;t+=0.1)
    {
        hostNumber = integerGenerate(25);
        hostSequenceGenerate(hostPairs);
        for(int hp=0;hp<hostNumber;++hp)
        {
            UdpEchoServerHelper echoServer (p);
            ApplicationContainer serverApps = echoServer.Install (r.Get (hostPairs[hp]+30));
           
            serverApps.Start (Seconds (t));
            serverApps.Stop (Seconds (t+1));

            UdpEchoClientHelper echoClient (interfaces[hostPairs[hp]+49].GetAddress (1), p);
            echoClient.SetAttribute ("MaxPackets", UintegerValue (27));
            echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.00375)));
            echoClient.SetAttribute ("PacketSize", UintegerValue (210));
            
            ApplicationContainer clientApps = echoClient.Install (r.Get (hostPairs[hp+hostNumber]+30));
            clientApps.Start (Seconds (t));
            clientApps.Stop (Seconds (t+0.1));
            p++;
        }
    }
    //
    // Tracing
    //
    //AsciiTraceHelper ascii[100];
    //for(int i=0;i<100;++i)
    //{
    //    p2p.EnableAscii(ascii[i].CreateFileStream ("test.tr"), device[i]);
    //    p2p.EnablePcap("test", device[i], false);
    //}
    /*
    Ptr<Socket> ns3UdpSocket;//[50];
    for(int i=31;i<=80;++i)
    {
        ns3UdpSocket = Socket::CreateSocket (r.Get (i), UdpSocketFactory::GetTypeId ());
        ns3UdpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&CwndChange));
    }
    */

    /*
     * Calculate Throughput and Time Delay using Flowmonitor #
     */
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    //Config::ConnectWithoutContext("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx", MakeCallback (&ReceivePacket));

    NS_LOG_INFO("Run Simulation.");
    Simulator::Stop (Seconds (120.0));
    Simulator::Run ();
    
    monitor->CheckForLostPackets ();
    //double TP=0.00,TD=0.00,tmp;
    int innn=0;
    
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
        ++innn;
        //Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
        //std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
        //std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
        //std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
        //Throughput
        if(throughputPrintf)
        {
            cout<< innn<< '\t' << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/1024 <<endl;
        }
        //Time Delay
        if(timeDelayPrintf)
        {
            cout<< innn<<'\t'<<(i->second.timeLastRxPacket.GetSeconds()-i->second.timeFirstTxPacket.GetSeconds())/(27-i->second.lostPackets)<< endl;
            //if(TD<tmp) TD=tmp;
             //cout<<innn <<'\t'<< tmp << endl;
        }
    }

    monitor->SerializeToXmlFile("echo.flowmon", true, true);


    
    Simulator::Destroy ();
    NS_LOG_INFO("Done");
    return 0;
}

