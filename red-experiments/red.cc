#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"

using namespace ns3;


void QueueLog(uint32_t oldVal, uint32_t newVal) {
    std::cout << Simulator::Now().GetSeconds()
              << " Queue: " << newVal << std::endl;
}

int main() {

    NodeContainer senders;
    senders.Create(10);

    NodeContainer router;
    router.Create(1);

    NodeContainer receiver;
    receiver.Create(1);

    InternetStackHelper stack;
    stack.Install(senders);
    stack.Install(router);
    stack.Install(receiver);

    // LINKS
    PointToPointHelper fast;
    fast.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
    fast.SetChannelAttribute("Delay", StringValue("2ms"));

    PointToPointHelper bottleneck;
    bottleneck.SetDeviceAttribute("DataRate", StringValue("2Mbps"));
    bottleneck.SetChannelAttribute("Delay", StringValue("10ms"));

    std::vector<NetDeviceContainer> senderDevs;

    for (int i = 0; i < 10; i++) {
        NetDeviceContainer d = fast.Install(senders.Get(i), router.Get(0));
        senderDevs.push_back(d);
    }

    NetDeviceContainer bottleneckDev =
        bottleneck.Install(router.Get(0), receiver.Get(0));

    // IP
    Ipv4AddressHelper addr;
    addr.SetBase("10.1.0.0", "255.255.255.0");

    std::vector<Ipv4InterfaceContainer> senderIf;

    for (auto &d : senderDevs) {
        senderIf.push_back(addr.Assign(d));
        addr.NewNetwork();
    }

    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(512));
    addr.SetBase("10.2.0.0", "255.255.255.0");
    Ipv4InterfaceContainer recvIf = addr.Assign(bottleneckDev);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    TrafficControlHelper tch;

    tch.Uninstall(bottleneckDev);

    // RED
    tch.SetRootQueueDisc("ns3::RedQueueDisc",
                        "MinTh", DoubleValue(10),
                        "MaxTh", DoubleValue(30),
                        "LinkBandwidth", StringValue("2Mbps"),
                        "LinkDelay", StringValue("10ms"));

    QueueDiscContainer qdisc = tch.Install(bottleneckDev);

    uint16_t port = 8080;

    for (int i = 0; i < 10; i++) {

        BulkSendHelper src("ns3::TcpSocketFactory",
            InetSocketAddress(recvIf.GetAddress(1), port));

        src.SetAttribute("MaxBytes", UintegerValue(0));

        ApplicationContainer app = src.Install(senders.Get(i));
        app.Start(Seconds(1.0));
        app.Stop(Seconds(10.0));
    }

    PacketSinkHelper sink("ns3::TcpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), port));

    ApplicationContainer sinkApp = sink.Install(receiver.Get(0));
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(10.0));

    Ptr<QueueDisc> q = qdisc.Get(0);

    q->TraceConnectWithoutContext("PacketsInQueue",
    MakeCallback(&QueueLog));


    Simulator::Stop(Seconds(10.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
