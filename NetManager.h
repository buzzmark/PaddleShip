#ifndef NETMANAGER_H_
#define NETMANAGER_H_

#include <unordered_map>
#include <SDL_net.h>
#include "Packet.h"

class NetManager
{
    private:
        IPaddress ip;
        TCPsocket server;
        std::unordered_map<int, TCPsocket> clients;
        SDLNet_SocketSet socket_set;
        bool isServer;
        bool isRunning;
        int nextClientId;

    public:
        NetManager();
        ~NetManager();

        void startServer();
        void startClient(char* host);

        void messageServer(const Packet& p);
        void messageClients(const Packet& p);
        std::unordered_map<int, Packet> getData();
};

#endif /* NETMANAGER_H_ */
