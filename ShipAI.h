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

//---------------------------------------------------------------------------
class ShipAI: public GameObject 
{
public:
	ShipAI(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim, Ogre::SceneNode* sn, int &sc, SoundPlayer* sPlayer, std::deque<GameObject*>* oList,  int ops);
	~ShipAI(void);
	void addToScene(void);
	void addToSimulator(void);
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
	SoundPlayer* soundPlayer;
	std::deque<GameObject*>* objList;
	Ogre::Vector3 direction;
	GameObject *target;
	bool gameStarted;
	float yT;

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

	bool hasDecr;
	int health; 
	bool forward;
	bool back;
	bool left;
	bool right;
	int &score;

};



//---------------------------------------------------------------------------

#endif // #ifndef __ShipAI_h_

//---------------------------------------------------------------------------
