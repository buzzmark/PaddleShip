#include "NetUpdate.h"

NetUpdate::NetUpdate() : data(), newConnection(false), connectionId(-1) {}

bool NetUpdate::hasServerUpdate() {
    return (data.find(0) != data.end());
}

Packet& NetUpdate::getServerUpdate() {
    return (data.find(0)->second);
}
