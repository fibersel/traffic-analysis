#include <fstream>
#include <sstream>
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"

using namespace ns3;


struct SimConfig {

    // IP
    std::string senderNetworkBase = "10.1.0.0";
    std::string receiverNetworkBase = "10.2.0.0";
    std::string subnetMask = "255.255.255.0";

    // topology
    uint32_t nSenders = 10;
    std::string fastRate = "1Mbps";
    std::string bottleneckRate = "2Mbps";
    std::string bottleneckDelay = "10ms";

    // AQM
    std::string aqm = "RED";
    double minTh = 5;
    double maxTh = 15;

    // simulation
    double simTime = 10.0;

    // logging files
    std::string queueLogFile = "queue.csv";
    std::string dropLogFile = "drops.csv";
    std::string throughputLogFile = "throughput.csv";
};


struct ThroughputState {
    Ptr<PacketSink> sink;
    std::ofstream* file;
    uint64_t lastRx = 0;
};



void QueueLog(std::ofstream* file, uint32_t oldVal, uint32_t newVal)
{
    *file << Simulator::Now().GetSeconds()
          << "," << newVal << std::endl;
}

void DropLog(std::ofstream* file, Ptr<const QueueDiscItem> item)
{
    *file << Simulator::Now().GetSeconds()
          << ",1" << std::endl;
}


void LogThroughput(ThroughputState* state)
{

    uint64_t totalRx = state->sink->GetTotalRx();

    double throughput = (totalRx - state->lastRx) * 8.0 / 1e6;

    *state->file << Simulator::Now().GetSeconds() << "," << throughput << std::endl;

    state->lastRx = totalRx;

    Simulator::Schedule(Seconds(1.0), &LogThroughput, state);
}



void LoadConfig(const std::string& filename, SimConfig& cfg) {

    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Cannot open config file: " << filename << std::endl;
        exit(1);
    }

    std::string line;

    while (std::getline(file, line)) {

        // пропускаем пустые строки и комментарии
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream iss(line);
        std::string key, value;

        if (!std::getline(iss, key, '=')) continue;
        if (!std::getline(iss, value)) continue;

        // trim (очень простая версия)
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);

        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);

        // -------- IP ----------
        if (key == "senderNetworkBase")
            cfg.senderNetworkBase = value;

        else if (key == "receiverNetworkBase")
            cfg.receiverNetworkBase = value;

        else if (key == "subnetMask")
            cfg.subnetMask = value;

        // -------- topology ----------
        else if (key == "nSenders")
            cfg.nSenders = std::stoul(value);

        else if (key == "fastRate")
            cfg.fastRate = value;

        else if (key == "bottleneckRate")
            cfg.bottleneckRate = value;

        else if (key == "bottleneckDelay")
            cfg.bottleneckDelay = value;

        // -------- AQM ----------
        else if (key == "aqm")
            cfg.aqm = value;

        else if (key == "minTh")
            cfg.minTh = std::stod(value);

        else if (key == "maxTh")
            cfg.maxTh = std::stod(value);

        // -------- simulation ----------
        else if (key == "simTime")
            cfg.simTime = std::stod(value);
        // -------- logging ----------
        else if (key == "queueLogFile")
            cfg.queueLogFile = value;
        else if (key == "dropLogFile")
            cfg.dropLogFile = value;
        
        else if (key == "throughputLogFile")
            cfg.throughputLogFile = value;
        else {
            std::cerr << "Unknown config key: " << key << std::endl;
        }
    }
}


int main(int argc, char* argv[]) {

    SimConfig cfg;

    std::string configFile = "config.txt";

    CommandLine cmd;
    cmd.AddValue("config", "Path to config file", configFile);
    cmd.Parse(argc, argv);
    
    LoadConfig(configFile, cfg);
    
    std::cout << "Loaded config:" << std::endl;
    std::cout << "AQM = " << cfg.aqm << std::endl;
    std::cout << "Senders = " << cfg.nSenders << std::endl;

    // Parse params
    std::string aqmType = "RED"; // по умолчанию

    cmd.AddValue("aqm", "AQM type: RED or ARED", aqmType);
    cmd.Parse(argc, argv);

    // set up network
    NodeContainer senders;
    senders.Create(cfg.nSenders);

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
    fast.SetDeviceAttribute("DataRate", StringValue(cfg.fastRate));
    fast.SetChannelAttribute("Delay", StringValue("2ms"));

    PointToPointHelper bottleneck;
    bottleneck.SetDeviceAttribute("DataRate", StringValue(cfg.bottleneckRate));
    bottleneck.SetChannelAttribute("Delay", StringValue(cfg.bottleneckDelay));

    std::vector<NetDeviceContainer> senderDevs;

    for (int i = 0; i < cfg.nSenders; i++) {
        NetDeviceContainer d = fast.Install(senders.Get(i), router.Get(0));
        senderDevs.push_back(d);
    }

    NetDeviceContainer bottleneckDev =
        bottleneck.Install(router.Get(0), receiver.Get(0));

    // IP
    Ipv4AddressHelper addr;
    addr.SetBase(cfg.senderNetworkBase.c_str(), cfg.subnetMask.c_str());
    std::vector<Ipv4InterfaceContainer> senderIf;

    for (auto &d : senderDevs) {
        senderIf.push_back(addr.Assign(d));
        addr.NewNetwork();
    }

    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(512));
    addr.SetBase(cfg.receiverNetworkBase.c_str(), cfg.subnetMask.c_str());
    Ipv4InterfaceContainer recvIf = addr.Assign(bottleneckDev);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    TrafficControlHelper tch;

    tch.Uninstall(bottleneckDev);

    if (cfg.aqm == "ARED")
    {
        tch.SetRootQueueDisc("ns3::RedQueueDisc",
                             "ARED", BooleanValue(true),
                             "MinTh", DoubleValue(cfg.minTh),
                             "MaxTh", DoubleValue(cfg.maxTh),
                             "LinkBandwidth", StringValue(cfg.bottleneckRate),
                             "LinkDelay", StringValue(cfg.bottleneckDelay));
    }
    else if (cfg.aqm == "GENTLE")
    {
        tch.SetRootQueueDisc("ns3::RedQueueDisc",
                             "MinTh", DoubleValue(cfg.minTh),
                             "MaxTh", DoubleValue(cfg.maxTh),
                             "Gentle", BooleanValue(true),
                             "LinkBandwidth", StringValue(cfg.bottleneckRate),
                             "LinkDelay", StringValue(cfg.bottleneckDelay));
    }
    else
    {
        tch.SetRootQueueDisc("ns3::RedQueueDisc",
                             "MinTh", DoubleValue(cfg.minTh),
                             "MaxTh", DoubleValue(cfg.maxTh),
                             "LinkBandwidth", StringValue(cfg.bottleneckRate),
                             "LinkDelay", StringValue(cfg.bottleneckDelay));
    }

    std::ofstream queueFile(cfg.queueLogFile);
    std::ofstream dropFile(cfg.dropLogFile);
    std::ofstream throughputFile(cfg.throughputLogFile);

    QueueDiscContainer qdisc = tch.Install(bottleneckDev);

    uint16_t port = 8080;

    for (uint32_t i = 0; i < cfg.nSenders; i++) {

        BulkSendHelper src("ns3::TcpSocketFactory", InetSocketAddress(recvIf.GetAddress(1), port));

        src.SetAttribute("MaxBytes", UintegerValue(0));

        ApplicationContainer app = src.Install(senders.Get(i));
        app.Start(Seconds(1.0));
        app.Stop(Seconds(cfg.simTime));
    }

    PacketSinkHelper sinkHelper("ns3::TcpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), port));
    
    ApplicationContainer sinkApp = sinkHelper.Install(receiver.Get(0));

    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(cfg.simTime));

    Ptr<PacketSink> sink = DynamicCast<PacketSink>(sinkApp.Get(0));

    Simulator::Schedule(Seconds(1.0), [](){
        std::cout << "Simulation started traffic..." << std::endl;
    });
    ThroughputState* state = new ThroughputState();
    state->sink = sink;
    state->file = &throughputFile;

    Simulator::Schedule(Seconds(1.0), &LogThroughput, state);

    Ptr<QueueDisc> q = qdisc.Get(0);

    q->TraceConnectWithoutContext(
        "PacketsInQueue",
        MakeBoundCallback(&QueueLog, &queueFile)
    );
    qdisc.Get(0)->TraceConnectWithoutContext(
        "Drop",
        MakeBoundCallback(&DropLog, &dropFile)
    );

    Simulator::Stop(Seconds(cfg.simTime));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
