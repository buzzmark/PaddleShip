#ifndef __ShipAI_h_
#define __ShipAI_h_

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#  include <OIS/OISEvents.h>
#  include <OIS/OISKeyboard.h>
#else
#  include <OISEvents.h>
#  include <OISKeyboard.h>
#endif

#include "SoundPlayer.h"
#include <SdkTrays.h>
#include <btBulletDynamicsCommon.h>
#include <Ogre.h>
#include "BaseApplication.h" 

#include "GameObject.h"
#include "Ship.h"
#include "AsteroidSys.h"
#include "Simulator.h"
#include "Paddle.h"
#include "Alien.h"

#define NUM_PRIORITIES 4

class GameScreen;

//---------------------------------------------------------------------------
class ShipAI: public Ship
{
public:
	ShipAI(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim, GameScreen* gs, Ogre::SceneNode* sn, int &sc, SoundPlayer* sPlayer, std::deque<GameObject*>* oList,  int ops);
	~ShipAI(void);
	void addToScene(void);
	void update(void);
	void roam(void);
	void shoot(void);
	void hunt(void);
	void melee(void);
	void flee(void);
	void survivalCheck(void);
	void incomingAst(void);
	void opponentProximityCheck(void);

protected:
	Ogre::SceneNode* sceneNode;
	std::deque<GameObject*>* objList;
	PlayerObject* target;
	bool gameStarted;
	float yT;

	//both closest and alternate are saved and based on reasoning of other states, one of them becomes target
	GameObject *closest;
	GameObject *alternate;

	int paces;
	bool doneRoaming;
	bool mustFlee;
	bool doneFleeing;

	bool roamState;
	bool shootState;
	bool huntState;
	bool meleeState;
	bool fleeState;

	int numOpponents;
};



//---------------------------------------------------------------------------

#endif // #ifndef __ShipAI_h_

//---------------------------------------------------------------------------
