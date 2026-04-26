#pragma once
#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include <fstream>

using namespace ns3;

struct ThroughputState {
    Ptr<PacketSink> sink;
    std::ofstream*  file;
    uint64_t        lastRx = 0;
};

void QueueLog(std::ofstream* file, uint32_t oldVal, uint32_t newVal);
void DropLog(std::ofstream* file, Ptr<const QueueDiscItem> item);
void LogThroughput(ThroughputState* state);
