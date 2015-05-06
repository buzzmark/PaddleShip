#ifndef NETUPDATE_H_
#define NETUPDATE_H_

#include <unordered_map>
#include "Packet.h"

struct NetUpdate {
    std::unordered_map<int, Packet> data;
    bool newConnection;
    int connectionId;
    std::vector<int> disconnects;

    NetUpdate();

    bool hasServerUpdate();
    Packet& getServerUpdate();
};

#endif /* NETUPDATE_H_ */
