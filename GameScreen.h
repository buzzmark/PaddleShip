#ifndef __GameScreen_h_
#define __GameScreen_h_

#include <Ogre.h>
#include "BaseApplication.h" //?
#include "AsteroidSys.h"
#include "Simulator.h"
#include "Paddle.h"
#include "SoundPlayer.h"
#include "Packet.h"
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
	void addPlayerToMinimap(GameObject* enemy);
	void addEnemyToMinimap(GameObject* enemy);
	void setClient(bool client);
	void setSinglePlayer(bool single);
	void update(const Ogre::FrameEvent &evt);
	void updateClient(const Ogre::FrameEvent &evt, Packet& p);
	void updateMinimap();
	Packet getPositions();
	void setDeetsPan(OgreBites::ParamsPanel*mDeetsPan);
	void injectKeyDown(const OIS::KeyEvent &arg);
	void injectKeyUp(const OIS::KeyEvent &arg);
	void clientKey(int id, int key);
	void injectMouseMove(const OIS::MouseEvent &arg);
	void injectMouseDown(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
	void injectMouseUp(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
    Alien* createClientAlien(int id);
    void removeClientAlien(int id);
    void setClientId(int id);

    std::vector<Asteroid*> getAsteroids();
    std::vector<GameObject*> getPlayers();

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
	std::unordered_map<GameObject*, Ogre::OverlayElement*> mmPlayerIcons;

    int clientId;

    // TODO: replace Alien* with a generic PlayerObject
    std::unordered_map<int, Alien*> clientObjects;
};

//---------------------------------------------------------------------------

#endif // #ifndef __GameScreen_h_

//---------------------------------------------------------------------------
