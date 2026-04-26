#include <iostream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"

#include "config.h"
#include "logging.h"

using namespace ns3;


static void ConfigureAqm(TrafficControlHelper& tch, const SimConfig& cfg)
{
    if (cfg.aqm == "ARED") {
        tch.SetRootQueueDisc("ns3::RedQueueDisc",
            "ARED",          BooleanValue(true),
            "MinTh",         DoubleValue(cfg.minTh),
            "MaxTh",         DoubleValue(cfg.maxTh),
            "LinkBandwidth", StringValue(cfg.bottleneckRate),
            "LinkDelay",     StringValue(cfg.bottleneckDelay));
    } else if (cfg.aqm == "GENTLE") {
        tch.SetRootQueueDisc("ns3::RedQueueDisc",
            "Gentle",        BooleanValue(true),
            "MinTh",         DoubleValue(cfg.minTh),
            "MaxTh",         DoubleValue(cfg.maxTh),
            "LinkBandwidth", StringValue(cfg.bottleneckRate),
            "LinkDelay",     StringValue(cfg.bottleneckDelay));
    } else {
        tch.SetRootQueueDisc("ns3::RedQueueDisc",
            "MinTh",         DoubleValue(cfg.minTh),
            "MaxTh",         DoubleValue(cfg.maxTh),
            "LinkBandwidth", StringValue(cfg.bottleneckRate),
            "LinkDelay",     StringValue(cfg.bottleneckDelay));
    }
}


int main(int argc, char* argv[])
{
    std::string configFile = "config.txt";

    CommandLine cmd;
    cmd.AddValue("config", "Path to config file", configFile);
    cmd.Parse(argc, argv);

    SimConfig cfg;
    LoadConfig(configFile, cfg);

    std::cout << "AQM=" << cfg.aqm
              << "  senders=" << cfg.nSenders
              << "  bottleneck=" << cfg.bottleneckRate << "\n";

    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(512));

    // ── Nodes ──────────────────────────────────────────────────────────────
    NodeContainer senders, router, receiver;
    senders.Create(cfg.nSenders);
    router.Create(1);
    receiver.Create(1);

    InternetStackHelper stack;
    stack.Install(senders);
    stack.Install(router);
    stack.Install(receiver);

    // ── Links ──────────────────────────────────────────────────────────────
    PointToPointHelper fastLink;
    fastLink.SetDeviceAttribute("DataRate", StringValue(cfg.fastRate));
    fastLink.SetChannelAttribute("Delay", StringValue("2ms"));

    PointToPointHelper bottleneckLink;
    bottleneckLink.SetDeviceAttribute("DataRate", StringValue(cfg.bottleneckRate));
    bottleneckLink.SetChannelAttribute("Delay", StringValue(cfg.bottleneckDelay));

    std::vector<NetDeviceContainer> senderDevs;
    for (uint32_t i = 0; i < cfg.nSenders; ++i)
        senderDevs.push_back(fastLink.Install(senders.Get(i), router.Get(0)));

    NetDeviceContainer bottleneckDev =
        bottleneckLink.Install(router.Get(0), receiver.Get(0));

    // ── IP Addresses ───────────────────────────────────────────────────────
    Ipv4AddressHelper addr;
    addr.SetBase(cfg.senderNetworkBase.c_str(), cfg.subnetMask.c_str());
    for (auto& d : senderDevs) {
        addr.Assign(d);
        addr.NewNetwork();
    }

    addr.SetBase(cfg.receiverNetworkBase.c_str(), cfg.subnetMask.c_str());
    Ipv4InterfaceContainer recvIf = addr.Assign(bottleneckDev);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // ── AQM ────────────────────────────────────────────────────────────────
    TrafficControlHelper tch;
    tch.Uninstall(bottleneckDev);
    ConfigureAqm(tch, cfg);
    QueueDiscContainer qdisc = tch.Install(bottleneckDev);

    // ── Applications ───────────────────────────────────────────────────────
    const uint16_t port = 8080;

    BulkSendHelper bulkSend("ns3::TcpSocketFactory",
                            InetSocketAddress(recvIf.GetAddress(1), port));
    bulkSend.SetAttribute("MaxBytes", UintegerValue(0));
    for (uint32_t i = 0; i < cfg.nSenders; ++i) {
        ApplicationContainer app = bulkSend.Install(senders.Get(i));
        app.Start(Seconds(1.0));
        app.Stop(Seconds(cfg.simTime));
    }

    PacketSinkHelper sinkHelper("ns3::TcpSocketFactory",
                                InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApp = sinkHelper.Install(receiver.Get(0));
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(cfg.simTime));

    // ── Logging ────────────────────────────────────────────────────────────
    std::ofstream queueFile(cfg.queueLogFile);
    std::ofstream dropFile(cfg.dropLogFile);
    std::ofstream throughputFile(cfg.throughputLogFile);

    Ptr<PacketSink>  sink = DynamicCast<PacketSink>(sinkApp.Get(0));
    ThroughputState  thrState{sink, &throughputFile};
    Simulator::Schedule(Seconds(1.0), &LogThroughput, &thrState);

    Ptr<QueueDisc> q = qdisc.Get(0);
    q->TraceConnectWithoutContext("PacketsInQueue",
                                  MakeBoundCallback(&QueueLog, &queueFile));
    q->TraceConnectWithoutContext("Drop",
                                  MakeBoundCallback(&DropLog, &dropFile));

    // ── Run ────────────────────────────────────────────────────────────────
    Simulator::Stop(Seconds(cfg.simTime));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
