#include <cstring>
#include <algorithm>
#include "Packet.h"

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

Packet& Packet::operator<<(const char a) {
    mem_write_to_packet(&a, sizeof(char));
    return *this;
}

Packet& Packet::operator<<(const int a) {
    mem_write_to_packet(&a, sizeof(int));
    return *this;
}

Packet& Packet::operator<<(const float a) {
    mem_write_to_packet(&a, sizeof(float));
    return *this;
}

Packet& Packet::operator<<(const Ogre::Vector3& a) {
    return *this << a.x << a.y << a.z;
}

Packet& Packet::operator<<(const Ogre::Quaternion& a) {
    return *this << a.w << a.x << a.y << a.z;
}

Packet& Packet::operator<<(const std::string& a) {
    buffer.insert(buffer.end(), a.begin(), a.end());
    buffer.push_back(0);
    length += a.size() + 1;
    return *this;
}

//---------------------------------------------------------------------------

Packet& Packet::operator>>(char &a) {
    mem_read_from_packet(&a, sizeof(char));
    return *this;
}

Packet& Packet::operator>>(int &a) {
    mem_read_from_packet(&a, sizeof(int));
    return *this;
}

Packet& Packet::operator>>(float &a) {
    mem_read_from_packet(&a, sizeof(float));
    return *this;
}

Packet& Packet::operator>>(Ogre::Vector3& a) {
    return *this >> a.x >> a.y >> a.z;
}

Packet& Packet::operator>>(Ogre::Quaternion& a) {
    return *this >> a.w >> a.x >> a.y >> a.z;
}

Packet& Packet::operator>>(std::string& a) {
    a = std::string(&buffer[position]);
    position = find(buffer.begin() + position, buffer.end(), (char) 0) + 1 - buffer.begin();
    return *this;
}

//---------------------------------------------------------------------------
