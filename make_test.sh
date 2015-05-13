#!/bin/bash
g++ NetTest.cpp NetManager.cpp NetUpdate.cpp Packet.cpp -std=c++11 -I/usr/include/SDL -lSDL -DDISABLE_OGRE -lSDL_net -o nettest
