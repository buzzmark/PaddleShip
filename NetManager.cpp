#include <stdio.h>
#include <fstream>
#include "NetManager.h"
#include "Packet.h"

static std::ofstream logger("net.log");

NetManager::NetManager(){
    SDLNet_Init();
    socket_set = SDLNet_AllocSocketSet(16);
    nextClientId = 1;
}

NetManager::~NetManager(){
    SDLNet_FreeSocketSet(socket_set);

    if (isRunning) {
        SDLNet_TCP_Close(server);

        for (auto client : clients) {
            TCPsocket& socket = client.second;
            SDLNet_TCP_Close(socket);
        }
    }

    SDLNet_Quit();
}

//------------------------------------------------------------
// Start fuctions
//------------------------------------------------------------

void NetManager::startServer(){
    isServer = true;

    if (SDLNet_ResolveHost(&ip, NULL, 49152) < 0)
    {
        fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    /* Open a connection with the IP provided (listen on the host's port) */
    if (!(server = SDLNet_TCP_Open(&ip)))
    {
        fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    if (SDLNet_AddSocket(socket_set, (SDLNet_GenericSocket) server) < 0)
    {
        fprintf(stderr, "SDLNet_AddSocket: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    isRunning = true;
    logger << "started server" << std::endl;
}

void NetManager::startClient(char* host) {
    isServer = false;

    if (SDLNet_ResolveHost(&ip, host, 49152) < 0)
    {
        fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    if (!(server = SDLNet_TCP_Open(&ip)))
    {
        fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    if (SDLNet_AddSocket(socket_set, (SDLNet_GenericSocket) server) < 0)
    {
        fprintf(stderr, "SDLNet_AddSocket: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    isRunning = true;
    logger << "started client to " << host << std::endl;
}

int NetManager::numClients() const {
    return clients.size();
}

//------------------------------------------------------------
// Message fuctions
//------------------------------------------------------------

void NetManager::messageServer(const Packet &p) {
    const char* buf = p.data();
    const int len = p.size();

    SDLNet_TCP_Send(server, &len, sizeof(int));

    if (SDLNet_TCP_Send(server, buf, len) < len) {
        // server probably disconnected; handle
        logger << "send failure" << std::endl;
        exit(EXIT_FAILURE);
    }
    logger << "sent " << len << " packet to server" << std::endl;
}

void NetManager::messageClients(const Packet &p) {
    const char* buf = p.data();
    const int len = p.size();

    auto iter = clients.begin();

    while (iter != clients.end()) {
        TCPsocket client = iter->second;
        SDLNet_TCP_Send(client, &len, sizeof(int));

        if (SDLNet_TCP_Send(client, buf, len) < len) {
            // client probably disconnected; handle in game
            logger << "send failure to client" << iter->first << std::endl;

            SDLNet_TCP_Close(client);
            SDLNet_DelSocket(socket_set, (SDLNet_GenericSocket) client);
            iter = clients.erase(iter);
        } else {
            ++iter;
        }
    }
    logger << "sent " << len << " packet to clients" << std::endl;
}

std::unordered_map<int, Packet> NetManager::checkForUpdates() {
    SDLNet_CheckSockets(socket_set, 0);

    logger << "checking for updates" << std::endl;

    if (isServer) {
        // check for new clients
        if (SDLNet_SocketReady(server)) {
            TCPsocket connection = SDLNet_TCP_Accept(server);
            if (connection != nullptr) {
                clients[nextClientId] = connection;
                SDLNet_AddSocket(socket_set, (SDLNet_GenericSocket) connection);
                logger << "Added client " << nextClientId << std::endl;
                nextClientId++;
            }
        }

        return serverGetData();
    } else {
        return clientGetData();
    }
}

std::unordered_map<int, Packet> NetManager::serverGetData() {
    std::unordered_map<int, Packet> data;

    logger << "serverGetData" << std::endl;

    // check for client messages
    auto iter = clients.begin();

    while (iter != clients.end()) {
        TCPsocket client = iter->second;
        if (SDLNet_SocketReady(client)) {
            int len;

            if (SDLNet_TCP_Recv(client, &len, sizeof(int)) <= 0) {
                logger << "client " << iter->first << " disconnected" << std::endl;
                SDLNet_TCP_Close(client);
                SDLNet_DelSocket(socket_set, (SDLNet_GenericSocket) client);
                iter = clients.erase(iter);
                exit(EXIT_FAILURE);
            } else {
                char* buf = new char[len];
                if (SDLNet_TCP_Recv(client, buf, len) <= 0) {
                    logger << "client " << iter->first << " disconnected" << std::endl;
                    SDLNet_TCP_Close(client);
                    SDLNet_DelSocket(socket_set, (SDLNet_GenericSocket) client);
                    iter = clients.erase(iter);
                    exit(EXIT_FAILURE);
                } else {
                    logger << "received " << len << " packet from client " << iter->first << std::endl;
                    data[iter->first] = Packet(buf, len);
                    ++iter;
                }
                delete[] buf;
            }
        } else {
            ++iter;
        }
    }

    return data;
}

std::unordered_map<int, Packet> NetManager::clientGetData() {
    std::unordered_map<int, Packet> data;

    logger << "clientGetData" << std::endl;

    if (SDLNet_SocketReady(server)) {
        int len;

        if (SDLNet_TCP_Recv(server, &len, sizeof(int)) <= 0) {
            logger << "lost connection to server" << std::endl;
            // lost connection (server probably quit)?
        } else {
            char* buf = new char[len];
            if (SDLNet_TCP_Recv(server, buf, len) <= 0) {
                // lost connection (server probably quit)?
                logger << "lost connection to server" << std::endl;
                exit(EXIT_FAILURE);
            } else {
                logger << "received " << len << " packet from server" << std::endl;
                data[0] = Packet(buf, len);
            }
            delete[] buf;
        }
    }

    return data;
}
