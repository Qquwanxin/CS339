#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ptr.h"//ns3::Ptr<T>;
#include "ns3/object.h"//包括类: RateErrorModel
#include "ns3/delay-jitter-estimation.h"//类: delayJitter.
#include "ns3/nstime.h"//Time的定义
#include "ts3/tcp-socket.h"//ns3TcpSocket定义
#include <iostream>
using namespace std;
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TA_Example");

//MyApp不是本来就存在的，是需要自己写的（Copy:fifth.cc）
class MyApp : public Application 
{
public:

  MyApp ();
  virtual ~MyApp();

  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

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


//NS-3中，有一个回调机制，方便我们来输出具体某个条件发生改变时就自动执行某个函数，回调的实现是TraceConnectWithoutContext函数，举个例子，在我的时延仿真中，输出时延的代码是这样写的：
//定义一个全局变量函数，时延的计算函数
//一个Packet类型的指针p，一个Address类型的变量address
//Packet类型内置变量两个，一个Packet，一个address
//RecordRx函数：DelayJitterEstimation类的函数
//void ns3::DelayJitterEstimation::RecordRx(Ptr<const Packet> packet)
//参数：packet	the packet received
//作用：更新delay和jitter的累积
//jitter:一种我还没弄懂的偏差，据说是时钟频率的偏差
//GetLastDelay和GetLastJitter: 返回更新过的delay和jitter.
//Definition at line 110 of file delay-jitter-estimation.cc.
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

int
main (int argc,char *argv)
{
    //用户可用命令行来访问代码中全局变量和NS3的属性
    CommandLine cmd;
    cmd.Parse (argc, argv);
    
    //将时间分辨率设置为1纳秒，恰好是一个默认值：
    //Time::SetResolution (Time::NS);

    //启用在 Echo Client 和 Echo Server 应用中内置的两个日志组件：
    LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create (2);
    //创建两个节点；
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
    //设置链路的传输速率为5Mbps，时延为2ms；
    NetDeviceContainer devices;
    devices = pointToPoint.Install (nodes);
    //为每个节点添加网络设备
    
    //Ptr<B> b = Create<B> ();是一个对 new 操作符的封装
    Ptr<RateErrorModel>em=CreateObject<RateErrorModel> ();
    //Ptr:类似boost::intrusive_ptr的智能指针
    //新建了一个RateErrorModel类型的指针em
    em->SetAttribute("ErrorRate",DoubleValue(0.00001));
    //SetAttribute:设置属性，错误率0.0001
    devices.Get(1)->SetAttribute("ReceiveErrorModel",PointerValue (em));
    //n1属性设置中，添加接受错误模型，值和刚刚创建的em一致
    //创建一个错误模型，讲错误率设置为0.00001，仿真 TCP 协议的重传机制。
    
    InternetStackHelper stack;
    stack.Install (nodes);
    //为每个节点安装协议栈；
    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.252");
    Ipv4InterfaceContainer interfaces = address.Assign (devices);
    //为每个节点的网络设备添加 IP 地址；
    //这样一个简单的网络拓扑就建立完成。

    //接下来就是为这个网络节点添加应用程序，让他们在这个网络中模拟传输数据，具体代码如下：
    uint16_t sinkPort = 8080;//端口定义为8080
    Address sinkAddress(InetSocketAddress (interfaces.GetAddress (1), sinkPort));
    //sinkAddress复制后面结果的地址
    //InetSocketAddress (Ipv4Address ipv4, uint16_t port)前一个参数是上面Ipv4InterfaceContainer类型的interfaces，后面一个是刚刚定义的端口
    PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny (), sinkPort));
    //PacketSinkHelper(std::string protocol,Address 	address)，protocol：接收传送的协议，string类型。为应用程序创建套接字的套接字工厂类型。address是sink的地址
    ApplicationContainer sinkApps = packetSinkHelper.Install (nodes.Get (1));
    //熟悉的创建后Install步骤，将接受数据的应用程序设置在 Node.Get(1)节点上

    //起始终止时间
    sinkApps.Start (Seconds (0.));
    sinkApps.Stop (Seconds (10.));

    //新建一个MyApp类型的指针app
    //MyAPP：
    /* MyApp::MyApp():
     *   m_socket (0),          一个socket
     *   m_peer (),             一个地址
     *   m_packetSize (0),      包的size
     *   m_nPackets (0),         包数量
     *   m_dataRate (0),        传输速率
     *   m_sendEvent (),        
     *   m_running (false),      
     *   m_packetsSent (0)      
     */

    //MyAPP的setup：
    /*void MyApp::Setup(
     *	 Ptr<Socket> socket,        Socket类型
     *   Address 	 address,       地址
     *   uint32_t 	 packetSize,    包大小
     *   uint32_t 	 nPackets,      包数量
     *   DataRate 	 dataRate       传输速率
     *   )
     */   	
    Ptr<MyApp> app = CreateObject<MyApp>();
    app->Setup(ns3TcpSocket, sinkAddress, 1040, 1000, DataRate ("1Mbps"));//ns3TCPSocket类型
    nodes.Get (0)->AddApplication(app);
    //AddApplication：参数是一个指针类型的Application，返回app列表中它的index
    app->SetStartTime (Seconds (1.));
    app->SetStopTime (Seconds (10.));
    //将发送数据的应用程序设置在 Node.Get(0)；发送起始时间为1s；结束时间为10s；
    //这样网络拓扑和节点之间应用程序的设定已完成，接下来就是应用统计模块，输出节点之间具体通信性能的参数，及时延，吞吐量，抖动率，丢包率；
   
    //在main 函数中使用回调机制：
    /* bool ns3::ObjectBase::TraceConnectWithoutContext( 
     * std::string name,        目标路径源名称
     * const CallbackBase &cb   连接路径源的callback
     * )
     */
    //当接受端节点每收到一个 TCP 包，就会执行一次 CalculateDelay 函数，计算这个数据包在网络中传输的时延，并输出；
    sinkApps.Get(0)->TraceConnectWithoutContext("Rx", MakeCallback(&CalculateDelay));
   

    //这样就完成了程序的编写；接下来就是输出具体数据：
    //在终端打开，到指定的文件夹中，输入
    //./waf --run scratch/delay >delay.dat 2>&1
    //按指定格式输出.dat 文件之后，再在终端用 GNUPLOT 来作出.dat 文件中的图形即 可
    //（GNUPLOT的使用可以参考dsec.pku.edu.cn/~tanghz/gnuplot.htm）：当然不画图，
    //用 print 输出，或者 log 日志的形式也是可以的（如 cwnd website）。
    Simulator::Run ();
    Simulator::Destroy ();
    return 0;
}