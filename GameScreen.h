#ifndef __GameScreen_h_
#define __GameScreen_h_

#include <Ogre.h>
#include "BaseApplication.h" //?
#include "AsteroidSys.h"
#include "Simulator.h"
#include "Paddle.h"
#include "SoundPlayer.h"
#include "Packet.h"
#include "Game.h"
#include <vector>
#include <unordered_map>

class Ship;
class ShipAI;
class Alien;

//---------------------------------------------------------------------------

class GameScreen
{
public:
	GameScreen(Ogre::SceneManager* sceneMgr, Ogre::SceneNode* cameraNode, SoundPlayer* sPlayer, Ogre::Light* shipLt, Ogre::Light* alienLt);
	~GameScreen(void);
	void createScene(void);
	void addPlayerToMinimap(PlayerObject* enemy);
	void addEnemyToMinimap(PlayerObject* enemy);
	void setClient(bool client);
	void setSinglePlayer(bool single);
	void update(const Ogre::FrameEvent &evt);
	void updateClient(const Ogre::FrameEvent &evt, Packet& p);
	void updateMinimap();
	void updateHealthDisplay();
	Packet getPositions();
	void setDeetsPan(OgreBites::ParamsPanel*mDeetsPan);
	void injectKeyDown(const OIS::KeyEvent &arg);
	void injectKeyUp(const OIS::KeyEvent &arg);
	void clientKey(int id, bool isDown, unsigned char key);
	void injectMouseMove(const OIS::MouseEvent &arg);
	void injectMouseDown(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
	void injectMouseUp(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
    PlayerObject* createClientObject(int id, int type);
    void removeClientObject(int id);
    void setClientId(int id);

    std::vector<Asteroid*> getAsteroids();
    std::vector<PlayerObject*> getPlayers();

protected:
	Ogre::SceneManager* mSceneMgr;
    Ogre::SceneNode* mCameraNode;
	SoundPlayer* soundPlayer;

	Simulator* sim;
	Ship* ship;
	AsteroidSys* ast1;
	ShipAI * shipAI;
	int score;
	int scoreAI;
	int alienHealth;
	bool isClient;
	bool singlePlayer;
	Ogre::OverlayContainer* mmBackground;
	std::unordered_map<PlayerObject*, Ogre::OverlayElement*> mmPlayerIcons;

    int clientId;

    std::unordered_map<int, PlayerObject*> clientObjects;
};

//---------------------------------------------------------------------------

#endif // #ifndef __GameScreen_h_

//---------------------------------------------------------------------------
