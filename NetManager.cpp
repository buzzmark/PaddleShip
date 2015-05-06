#include <stdio.h>
#include "NetManager.h"
#include "Packet.h"

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
    }
}

std::unordered_map<int, TCPsocket>::iterator NetManager::messageSingleClient(std::unordered_map<int, TCPsocket>::iterator iter, const Packet &p) {
    const char* buf = p.data();
    const int len = p.size();

    TCPsocket client = iter->second;
    SDLNet_TCP_Send(client, &len, sizeof(int));

    if (SDLNet_TCP_Send(client, buf, len) < len) {
        // client probably disconnected; handle in game

        SDLNet_TCP_Close(client);
        SDLNet_DelSocket(socket_set, (SDLNet_GenericSocket) client);
        return clients.erase(iter);
    } else {
        return ++iter;
    }
}

void NetManager::messageClients(const Packet &p) {
    auto iter = clients.begin();

    while (iter != clients.end()) {
        iter = messageSingleClient(iter, p);
    }
}

void NetManager::messageClient(int clientId, const Packet &p) {
    auto iter = clients.find(clientId);

    if (iter != clients.end()) {
        messageSingleClient(iter, p);
    }
}

NetUpdate NetManager::checkForUpdates() {
    SDLNet_CheckSockets(socket_set, 0);

    NetUpdate update;

    if (isServer) {
        // check for new clients
        if (SDLNet_SocketReady(server)) {
            TCPsocket connection = SDLNet_TCP_Accept(server);
            if (connection != nullptr) {
                update.newConnection = true;
                update.connectionId = nextClientId;

                clients[nextClientId] = connection;
                SDLNet_AddSocket(socket_set, (SDLNet_GenericSocket) connection);
                nextClientId++;
            }
        }

        update.data = serverGetData();
    } else {
        update.data = clientGetData();
    }

    return update;
}

bool read_buffer(TCPsocket socket, void* vbuf, int len) {
    char* buf = reinterpret_cast<char*>(vbuf);
    int chunk_pos = 0;

    while (chunk_pos < len) {
        int chunk_len = SDLNet_TCP_Recv(socket, buf + chunk_pos, len - chunk_pos);

        if (chunk_len <= 0) {
            // lost connection
            return false;
        }

        chunk_pos += chunk_len;
    }

    return true;
}

std::unordered_map<int, Packet> NetManager::serverGetData() {
    std::unordered_map<int, Packet> data;

    // check for client messages
    auto iter = clients.begin();

    while (iter != clients.end()) {
        TCPsocket client = iter->second;
        if (SDLNet_SocketReady(client)) {
            int len;

            if (read_buffer(client, &len, sizeof(int))) {
                char* buf = new char[len];

                if (read_buffer(client, buf, len)) {
                    data[iter->first] = Packet(buf, len);
                    ++iter;
                } else {
                    SDLNet_TCP_Close(client);
                    SDLNet_DelSocket(socket_set, (SDLNet_GenericSocket) client);
                    iter = clients.erase(iter);
                }

                delete[] buf;
            } else {
                SDLNet_TCP_Close(client);
                SDLNet_DelSocket(socket_set, (SDLNet_GenericSocket) client);
                iter = clients.erase(iter);
            }
        } else {
            ++iter;
        }
    }

    return data;
}

std::unordered_map<int, Packet> NetManager::clientGetData() {
    std::unordered_map<int, Packet> data;

    if (SDLNet_SocketReady(server)) {
        int len;

        if (read_buffer(server, &len, sizeof(int))) {
            char* buf = new char[len];
            if (read_buffer(server, buf, len)) {
                data[0] = Packet(buf, len);
            } else {
                // lost connection (server probably quit)?
            }
            delete[] buf;
        } else {
            // lost connection (server probably quit)?
        }
    }

    return data;
}
