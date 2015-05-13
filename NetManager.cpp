#include <stdio.h>
#include "NetManager.h"
#include "Packet.h"

NetManager::NetManager(){
    SDLNet_Init();
    socket_set = SDLNet_AllocSocketSet(16);
    udp_recv_packet = SDLNet_AllocPacket(MAX_RECV_PACKET_SIZE);
    nextClientId = 1;
    isRunning = false;
}

NetManager::~NetManager(){
    SDLNet_FreeSocketSet(socket_set);
    SDLNet_FreePacket(udp_recv_packet);

    if (isRunning) {
        if (isServer) {
            for (auto client : clients_udp) {
                int channel = client.second;
                SDLNet_UDP_Unbind(server_udp, channel);
            }
        } else {
            SDLNet_UDP_Unbind(server_udp, 0);
        }

        for (auto client : clients_tcp) {
            TCPsocket& socket = client.second;
            SDLNet_TCP_Close(socket);
        }

        SDLNet_UDP_Close(server_udp);
        SDLNet_TCP_Close(server_tcp);
    }

    SDLNet_Quit();
}

//------------------------------------------------------------
// Start fuctions
//------------------------------------------------------------

void NetManager::startServer(){
    isServer = true;

    if (SDLNet_ResolveHost(&ip, NULL, PORT) < 0)
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

    /* Open UDP socket */
    if (!(server_udp = SDLNet_UDP_Open(PORT)))
    {
        fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    if (SDLNet_AddSocket(socket_set, (SDLNet_GenericSocket) server_udp) < 0)
    {
        fprintf(stderr, "SDLNet_AddSocket: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    isRunning = true;
}

void NetManager::startClient(char* host) {
    isServer = false;

    if (SDLNet_ResolveHost(&ip, host, PORT) < 0)
    {
        fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    /* Open TCP connection to server */
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

    /* Bind UDP channel to server */
    if (!(server_udp = SDLNet_UDP_Open(PORT)))
    {
        fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    if (SDLNet_AddSocket(socket_set, (SDLNet_GenericSocket) server_udp) < 0)
    {
        fprintf(stderr, "SDLNet_AddSocket: %s\n", SDLNet_GetError());
        exit(EXIT_FAILURE);
    }

    if (SDLNet_UDP_Bind(server_udp, 0, &ip) == -1) {
        fprintf(stderr, "SDLNet_UDP_Bind: %s\n", SDLNet_GetError());
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

        SDLNet_UDP_Unbind(server_udp, clients_udp[iter->first]);
        udp_channels.erase(clients_udp[iter->first]);
        clients_udp.erase(iter->first);

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

UDPpacket* createUdpPacket(const Packet& p) {
    const char* buf = p.data();
    const int len = p.size();

    UDPpacket* send_packet = SDLNet_AllocPacket(len);
    memcpy(send_packet->data, buf, len);
    send_packet->len = len;

    return send_packet;
}

void NetManager::messageServerUDP(const Packet &p) {
    UDPpacket* send_packet = createUdpPacket(p);
    SDLNet_UDP_Send(server_udp, 0, send_packet);
    SDLNet_FreePacket(send_packet);
}

void NetManager::messageClientsUDP(const Packet& p) {
    UDPpacket* send_packet = createUdpPacket(p);

    for (auto client : clients_udp) {
        SDLNet_UDP_Send(server_udp, client.second, send_packet);
    }

    SDLNet_FreePacket(send_packet);
}

void NetManager::messageClientUDP(int clientId, const Packet& p) {
    auto iter = clients_udp.find(clientId);

    if (iter != clients_udp.end()) {
        UDPpacket* send_packet = createUdpPacket(p);
        SDLNet_UDP_Send(server_udp, iter->second, send_packet);
        SDLNet_FreePacket(send_packet);
    }
}

//------------------------------------------------------------
// Update fuctions
//------------------------------------------------------------

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

                IPaddress* client_ip = SDLNet_TCP_GetPeerAddress(connection);
                IPaddress udp_ip;
                udp_ip.host = client_ip->host;
                udp_ip.port = ip.port;

                int channel = SDLNet_UDP_Bind(server_udp, -1, &udp_ip);

                if (channel != -1) {
                    clients_udp[nextClientId] = channel;
                    udp_channels[channel] = nextClientId;
                }

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

    // check for client messages (TCP)
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

                    SDLNet_UDP_Unbind(server_udp, clients_udp[iter->first]);
                    udp_channels.erase(clients_udp[iter->first]);
                    clients_udp.erase(iter->first);

                    SDLNet_TCP_Close(client);
                    SDLNet_DelSocket(socket_set, (SDLNet_GenericSocket) client);

                    iter = clients_tcp.erase(iter);
                }

                delete[] buf;
            } else {
                update.disconnects.push_back(iter->first);

                SDLNet_UDP_Unbind(server_udp, clients_udp[iter->first]);
                udp_channels.erase(clients_udp[iter->first]);
                clients_udp.erase(iter->first);

                SDLNet_TCP_Close(client);
                SDLNet_DelSocket(socket_set, (SDLNet_GenericSocket) client);

                iter = clients_tcp.erase(iter);
            }
        } else {
            ++iter;
        }
    }

    // check for client messages (UDP)
    if (SDLNet_SocketReady(server_udp)) {
        if (SDLNet_UDP_Recv(server_udp, udp_recv_packet) == 1) {
            int channel = udp_recv_packet->channel;
            if (channel != -1) {
                int clientId = udp_channels[channel];
                Packet p((char*) udp_recv_packet->data, udp_recv_packet->len);

                if (data.find(clientId) != data.end()) {
                    queuedUdpUpdates[clientId].push(p);
                } else {
                    data[clientId] = p;
                }
            }
        }
    }

    // check for queued client messages from UDP
    for (auto client : queuedUdpUpdates) {
        int clientId = client.first;
        std::queue<Packet>& udp_queue = client.second;

        if (!udp_queue.empty() && data.find(clientId) == data.end()) {
            data[clientId] = udp_queue.front();
            udp_queue.pop();
        }
    }
}

void NetManager::clientGetData(NetUpdate& update) {
    std::unordered_map<int, Packet>& data = update.data;

    if (SDLNet_SocketReady(server_tcp)) {
        // read in server message (TCP)
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
    } else if (SDLNet_SocketReady(server_udp)) {
        // check for server message (UDP)
        if (SDLNet_UDP_Recv(server_udp, udp_recv_packet) == 1) {
            if (udp_recv_packet->channel != -1) {
                data[0] = Packet((char*) udp_recv_packet->data, udp_recv_packet->len);
            }
        }
    }
}
