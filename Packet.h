#ifndef PACKET_H_
#define PACKET_H_

#include <vector>

class Packet {
    private:
        int length;
        std::vector<char> buffer;

    public:
        Packet();
        Packet(char* buf, int len);

        int size() const;
        const char* data() const;
};

#endif /* PACKET_H_ */
