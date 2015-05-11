#ifndef PACKET_H_
#define PACKET_H_

#include <Ogre.h>
#include <vector>

enum PacketType {
    SPT_POSITIONS, SPT_CLIENTID, SPT_DISCONNECT, CPT_SHIPTYPE, CPT_KEYPRESS, CPT_KEYRELEASE, SPT_HEALTH
};

class Packet {
    private:
        std::vector<char> buffer;
        int length;
        int position;

        void mem_write_to_packet(const void* data, const int size);
        void mem_read_from_packet(void* data, const int size);

    public:
        Packet();
        Packet(char* buf, int len);

        int size() const;
        const char* data() const;

        Packet& operator<<(const char a);
        Packet& operator<<(const int a);
        Packet& operator<<(const float a);
        Packet& operator<<(const Ogre::Vector3& a);
        Packet& operator<<(const Ogre::Quaternion& a);

        Packet& operator>>(char& a);
        Packet& operator>>(int& a);
        Packet& operator>>(float& a);
        Packet& operator>>(Ogre::Vector3& a);
        Packet& operator>>(Ogre::Quaternion& a);
};

#endif /* PACKET_H_ */
