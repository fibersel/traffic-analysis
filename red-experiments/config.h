#pragma once
#include <string>
#include <cstdint>

struct SimConfig {
    // IP
    std::string senderNetworkBase   = "10.1.0.0";
    std::string receiverNetworkBase = "10.2.0.0";
    std::string subnetMask          = "255.255.255.0";

    // topology
    uint32_t    nSenders        = 10;
    std::string fastRate        = "1Mbps";
    std::string bottleneckRate  = "2Mbps";
    std::string bottleneckDelay = "10ms";

    // AQM
    std::string aqm   = "RED";
    double      minTh = 5.0;
    double      maxTh = 15.0;

    // simulation
    double simTime = 10.0;

    // output files
    std::string queueLogFile      = "queue.csv";
    std::string dropLogFile       = "drops.csv";
    std::string throughputLogFile = "throughput.csv";
};

void LoadConfig(const std::string& filename, SimConfig& cfg);
