#include <stdio.h>
#include "NetManager.h"
#include "Packet.h"

NetManager::NetManager(){
    SDLNet_Init();
    socket_set = SDLNet_AllocSocketSet(16);
    nextClientId = 1;
    isRunning = false;
}

NetManager::~NetManager(){
    SDLNet_FreeSocketSet(socket_set);

    if (isRunning) {
        SDLNet_TCP_Close(server_tcp);

        for (auto client : clients_tcp) {
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
    if (!(server_tcp = SDLNet_TCP_Open(&ip)))
    {
        fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    if (SDLNet_AddSocket(socket_set, (SDLNet_GenericSocket) server_tcp) < 0)
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

    if (!(server_tcp = SDLNet_TCP_Open(&ip)))
    {
        fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    if (SDLNet_AddSocket(socket_set, (SDLNet_GenericSocket) server_tcp) < 0)
    {
        fprintf(stderr, "SDLNet_AddSocket: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    isRunning = true;
}

int NetManager::numClients() const {
    return clients_tcp.size();
}

//------------------------------------------------------------
// Message fuctions
//------------------------------------------------------------

void NetManager::messageServerTCP(const Packet &p) {
    const char* buf = p.data();
    const int len = p.size();

    SDLNet_TCP_Send(server_tcp, &len, sizeof(int));

    if (SDLNet_TCP_Send(server_tcp, buf, len) < len) {
        queuedDisconnects.push_back(0);
    }
}

std::unordered_map<int, TCPsocket>::iterator NetManager::messageSingleClientTCP(std::unordered_map<int, TCPsocket>::iterator iter, const Packet &p) {
    const char* buf = p.data();
    const int len = p.size();

    TCPsocket client = iter->second;
    SDLNet_TCP_Send(client, &len, sizeof(int));

    if (SDLNet_TCP_Send(client, buf, len) < len) {
        queuedDisconnects.push_back(iter->first);

        SDLNet_TCP_Close(client);
        SDLNet_DelSocket(socket_set, (SDLNet_GenericSocket) client);
        return clients_tcp.erase(iter);
    } else {
        return ++iter;
    }
}

void NetManager::messageClientsTCP(const Packet &p) {
    auto iter = clients_tcp.begin();

    while (iter != clients_tcp.end()) {
        iter = messageSingleClientTCP(iter, p);
    }
}

void NetManager::messageClientTCP(int clientId, const Packet &p) {
    auto iter = clients_tcp.find(clientId);

    if (iter != clients_tcp.end()) {
        messageSingleClientTCP(iter, p);
    }
}

NetUpdate NetManager::checkForUpdates() {
    SDLNet_CheckSockets(socket_set, 0);

    NetUpdate update;
    update.disconnects = queuedDisconnects;
    queuedDisconnects.clear();

    if (isServer) {
        // check for new clients
        if (SDLNet_SocketReady(server_tcp)) {
            TCPsocket connection = SDLNet_TCP_Accept(server_tcp);
            if (connection != nullptr) {
                update.newConnection = true;
                update.connectionId = nextClientId;

                clients_tcp[nextClientId] = connection;
                SDLNet_AddSocket(socket_set, (SDLNet_GenericSocket) connection);
                nextClientId++;
            }
        }

        serverGetData(update);
    } else {
        clientGetData(update);
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

void NetManager::serverGetData(NetUpdate& update) {
    std::unordered_map<int, Packet>& data = update.data;

    // check for client messages
    auto iter = clients_tcp.begin();

    while (iter != clients_tcp.end()) {
        TCPsocket client = iter->second;
        if (SDLNet_SocketReady(client)) {
            int len;

            if (read_buffer(client, &len, sizeof(int))) {
                char* buf = new char[len];

                if (read_buffer(client, buf, len)) {
                    data[iter->first] = Packet(buf, len);
                    ++iter;
                } else {
                    update.disconnects.push_back(iter->first);

                    SDLNet_TCP_Close(client);
                    SDLNet_DelSocket(socket_set, (SDLNet_GenericSocket) client);
                    iter = clients_tcp.erase(iter);
                }

                delete[] buf;
            } else {
                update.disconnects.push_back(iter->first);

                SDLNet_TCP_Close(client);
                SDLNet_DelSocket(socket_set, (SDLNet_GenericSocket) client);
                iter = clients_tcp.erase(iter);
            }
        } else {
            ++iter;
        }
    }
}

void NetManager::clientGetData(NetUpdate& update) {
    std::unordered_map<int, Packet>& data = update.data;

    if (SDLNet_SocketReady(server_tcp)) {
        int len;

        if (read_buffer(server_tcp, &len, sizeof(int))) {
            char* buf = new char[len];
            if (read_buffer(server_tcp, buf, len)) {
                data[0] = Packet(buf, len);
            } else {
                update.disconnects.push_back(0);
            }
            delete[] buf;
        } else {
            update.disconnects.push_back(0);
        }
    }
}
