#include "NetManager.h"
#include "Packet.h"
#include "NetUpdate.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <string>

int main(int argc, char** argv) {
    NetManager net;
    if (argc < 2) {
        net.startServer();
    } else {
        net.startClient(argv[1]);
    }

    int tick = 0;

    while (true) {
        NetUpdate update = net.checkForUpdates();
        std::cout << "tick: " << tick << std::endl;
        std::cout << "data empty: " << update.data.empty() << std::endl;
        std::cout << "new conn: " << update.newConnection << std::endl;
        std::cout << "conn id: " << update.connectionId << std::endl;
        std::cout << "disconnects: " << update.disconnects.size() << std::endl;
        std::cout << "has server update: " << update.hasServerUpdate() << std::endl;
        if (update.hasServerUpdate()) {
            Packet& p = update.getServerUpdate();
            std::string msg;
            int t;

            p >> msg >> t;

            std::cout << "update: " << msg << " (@" << t << ")" << std::endl;
        }
        std::cout << std::endl;

        if (net.numClients() > 0) {
            Packet p;
            p << "Hello" << tick;
            net.messageClientsUDP(p);
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
        tick++;
    }
    
    return 0;
}