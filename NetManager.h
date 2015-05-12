#ifndef NETMANAGER_H_
#define NETMANAGER_H_

#include <unordered_map>
#include <SDL_net.h>
#include "Packet.h"
#include "NetUpdate.h"

#define MAX_RECV_PACKET_SIZE 8192

class NetManager
{
    private:
        IPaddress ip;
        TCPsocket server_tcp;
        UDPsocket server_udp;
        std::unordered_map<int, TCPsocket> clients_tcp;
        std::unordered_map<int, int> clients_udp;
        std::unordered_map<int, int> udp_channels;
        SDLNet_SocketSet socket_set;
        UDPpacket* udp_recv_packet;
        bool isServer;
        bool isRunning;
        int nextClientId;

        std::vector<int> queuedDisconnects;
        std::unordered_map<int, std::queue<Packet>> queuedUdpUpdates;

        std::unordered_map<int, TCPsocket>::iterator messageSingleClientTCP(std::unordered_map<int, TCPsocket>::iterator iter, const Packet &p);

        void serverGetData(NetUpdate& update);
        void clientGetData(NetUpdate& update);

    public:
        NetManager();
        ~NetManager();

        void startServer();
        void startClient(char* host);
        int numClients() const;

        void messageServerTCP(const Packet& p);
        void messageClientsTCP(const Packet& p);
        void messageClientTCP(int clientId, const Packet& p);
        void messageServerUDP(const Packet& p);
        void messageClientsUDP(const Packet& p);
        void messageClientUDP(int clientId, const Packet& p);

        NetUpdate checkForUpdates();
};

#endif /* NETMANAGER_H_ */
