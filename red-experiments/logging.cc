#include "logging.h"

void QueueLog(std::ofstream* file, uint32_t /*oldVal*/, uint32_t newVal)
{
    *file << Simulator::Now().GetSeconds() << "," << newVal << "\n";
}

void DropLog(std::ofstream* file, Ptr<const QueueDiscItem> /*item*/)
{
    *file << Simulator::Now().GetSeconds() << ",1\n";
}

void LogThroughput(ThroughputState* state)
{
    uint64_t totalRx   = state->sink->GetTotalRx();
    double   throughput = (totalRx - state->lastRx) * 8.0 / 1e6;

    *state->file << Simulator::Now().GetSeconds() << "," << throughput << "\n";

    state->lastRx = totalRx;
    Simulator::Schedule(Seconds(1.0), &LogThroughput, state);
}
