#include "Packet.h"

Packet::Packet() : length(0), buffer() {}

Packet::Packet(char* buf, int len) : length(len), buffer(buf, buf + len) {}

int Packet::size() const {
    return length;
}

const char* Packet::data() const {
    return &buffer[0];
}
