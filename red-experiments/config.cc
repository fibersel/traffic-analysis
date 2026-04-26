#include "config.h"
#include <fstream>
#include <sstream>
#include <iostream>

static void Trim(std::string& s)
{
    s.erase(0, s.find_first_not_of(" \t"));
    s.erase(s.find_last_not_of(" \t") + 1);
}

void LoadConfig(const std::string& filename, SimConfig& cfg)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open config file: " << filename << "\n";
        exit(1);
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string key, value;
        if (!std::getline(iss, key, '=')) continue;
        if (!std::getline(iss, value))    continue;
        Trim(key);
        Trim(value);

        if      (key == "senderNetworkBase")   cfg.senderNetworkBase   = value;
        else if (key == "receiverNetworkBase") cfg.receiverNetworkBase = value;
        else if (key == "subnetMask")          cfg.subnetMask          = value;
        else if (key == "nSenders")            cfg.nSenders            = std::stoul(value);
        else if (key == "fastRate")            cfg.fastRate            = value;
        else if (key == "bottleneckRate")      cfg.bottleneckRate      = value;
        else if (key == "bottleneckDelay")     cfg.bottleneckDelay     = value;
        else if (key == "aqm")                 cfg.aqm                 = value;
        else if (key == "minTh")               cfg.minTh               = std::stod(value);
        else if (key == "maxTh")               cfg.maxTh               = std::stod(value);
        else if (key == "simTime")             cfg.simTime             = std::stod(value);
        else if (key == "queueLogFile")        cfg.queueLogFile        = value;
        else if (key == "dropLogFile")         cfg.dropLogFile         = value;
        else if (key == "throughputLogFile")   cfg.throughputLogFile   = value;
        else std::cerr << "Unknown config key: " << key << "\n";
    }
}
