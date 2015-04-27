#include <cstring>
#include <fstream>
#include "Packet.h"

static std::ofstream logger("packet.log");

//---------------------------------------------------------------------------

void Packet::mem_write_to_packet(const void* data, const int size) {
    const char* buf = (const char*) data;

    buffer.insert(buffer.end(), buf, buf + size);
    length += size;
}

void Packet::mem_read_from_packet(void* data, const int size) {
    memcpy(data, &buffer[position], size);
    position += size;
}

//---------------------------------------------------------------------------

Packet::Packet() :
    buffer(),
    length(0),
    position(0) {}

Packet::Packet(char* buf, int len) :
    buffer(buf, buf + len),
    length(len),
    position(0) {}

int Packet::size() const {
    return length;
}

const char* Packet::data() const {
    return &buffer[0];
}

//---------------------------------------------------------------------------

Packet& Packet::operator<<(const int a) {
    logger << "packet write int " << a << std::endl;
    mem_write_to_packet(&a, sizeof(int));
    return *this;
}

Packet& Packet::operator<<(const float a) {
    logger << "packet write float " << a << std::endl;
    mem_write_to_packet(&a, sizeof(float));
    return *this;
}

Packet& Packet::operator<<(const Ogre::Vector3& a) {
    logger << "packet sending vector" << a << std::endl;
    return *this << a.x << a.y << a.z;
}

Packet& Packet::operator<<(const Ogre::Quaternion& a) {
    logger << "packet sending quat" << a << std::endl;
    return *this << a.w << a.x << a.y << a.z;
}

//---------------------------------------------------------------------------

Packet& Packet::operator>>(int &a) {
    mem_read_from_packet(&a, sizeof(int));
    logger << "packet read int " << a << std::endl;
    return *this;
}

Packet& Packet::operator>>(float &a) {
    mem_read_from_packet(&a, sizeof(float));
    logger << "packet read float " << a << std::endl;
    return *this;
}

Packet& Packet::operator>>(Ogre::Vector3& a) {
    logger << "packet reading vector" << std::endl;
    return *this >> a.x >> a.y >> a.z;
}

Packet& Packet::operator>>(Ogre::Quaternion& a) {
    logger << "packet reading quat" << std::endl;
    return *this >> a.w >> a.x >> a.y >> a.z;
}

//---------------------------------------------------------------------------
