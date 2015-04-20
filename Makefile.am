ACLOCAL_AMFLAGS= -I m4
noinst_HEADERS= Game.h Paddle.h Ship.h Alien.h Asteroid.h GameObject.h AsteroidSys.h Simulator.h GameScreen.h SoundPlayer.h BaseApplication.h BulletContactCallback.h OgreMotionState.h NetManager.h DebugDraw.hpp DynamicLineDrawer.hpp

bin_PROGRAMS= PaddleShip
PaddleShip_CPPFLAGS= -I$(top_srcdir)
PaddleShip_SOURCES= Game.cpp Paddle.cpp Ship.cpp Alien.cpp Asteroid.cpp GameObject.cpp AsteroidSys.cpp Simulator.cpp GameScreen.cpp SoundPlayer.cpp BaseApplication.cpp NetManager.cpp DebugDraw.cpp DynamicLineDrawer.cpp
PaddleShip_CXXFLAGS= $(OGRE_CFLAGS) $(OIS_CFLAGS) $(bullet_CFLAGS) $(sdl_CFLAGS) $(CEGUI_CFLAGS) -std=c++11
PaddleShip_LDADD= $(OGRE_LIBS) $(OIS_LIBS) $(bullet_LIBS) $(sdl_LIBS) $(CEGUI_LIBS) $(CEGUI_OGRE_LIBS)
PaddleShip_LDFLAGS= -lOgreOverlay -lboost_system -lSDL -lSDL_mixer -lSDL_net

EXTRA_DIST= buildit makeit
AUTOMAKE_OPTIONS= foreign
