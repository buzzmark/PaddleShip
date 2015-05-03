#ifndef __GameScreen_h_
#define __GameScreen_h_

#include <Ogre.h>
#include "BaseApplication.h" //?
#include "Ship.h"
#include "AsteroidSys.h"
#include "Simulator.h"
#include "Paddle.h"
#include "SoundPlayer.h"
#include "Alien.h"
#include "ShipAI.h"
#include "Packet.h"
#include <vector>

//---------------------------------------------------------------------------

class GameScreen
{
public:
	GameScreen(Ogre::SceneManager* sceneMgr, Ogre::SceneNode* cameraNode, SoundPlayer* sPlayer, Ogre::Light* shipLt, Ogre::Light* alienLt);
	~GameScreen(void);
	void createScene(void);
	void setClient(bool client);
	void setSinglePlayer(bool single);
	void update(const Ogre::FrameEvent &evt);
	void updateClient(const Ogre::FrameEvent &evt, Packet& p);
	Packet getPositions();
	void setDeetsPan(OgreBites::ParamsPanel*mDeetsPan);
	void injectKeyDown(const OIS::KeyEvent &arg);
	void injectKeyUp(const OIS::KeyEvent &arg);
	void clientKey(int key);
	void injectMouseMove(const OIS::MouseEvent &arg);
	void injectMouseDown(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
	void injectMouseUp(const OIS::MouseEvent &arg, OIS::MouseButtonID id);

    std::vector<Asteroid*> getAsteroids();
    std::vector<GameObject*> getPlayers();

protected:
	Ogre::SceneManager* mSceneMgr;
	SoundPlayer* soundPlayer;

	Simulator* sim;
	Ship* ship;
	Paddle* paddle;
	btHingeConstraint* paddleHinge;
	btHingeConstraint* paddleHingeAI;
	AsteroidSys* ast1;
	Alien * alien;
	ShipAI * shipAI;
	Paddle* paddleAI;
	bool motorRight;
	int score;
	int scoreAI;
	int alienHealth;
	bool isClient;
	bool singlePlayer;
};

//---------------------------------------------------------------------------

#endif // #ifndef __GameScreen_h_

//---------------------------------------------------------------------------
