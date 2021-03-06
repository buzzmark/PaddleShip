#ifndef __Alien_h_
#define __Alien_h_

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#  include <OIS/OISEvents.h>
#  include <OIS/OISKeyboard.h>
#else
#  include <OISEvents.h>
#  include <OISKeyboard.h>
#endif

#include "PlayerObject.h"
//#include "SoundPlayer.h"
#include <SdkTrays.h>
//#include "AsteroidSys.h"
//#include "btTypedConstraint.h"
#include <btBulletDynamicsCommon.h>

#include <Ogre.h>
#include "BaseApplication.h" //?
#include "Ship.h"
#include "AsteroidSys.h"
#include "Simulator.h"
#include "Paddle.h"
#include "SoundPlayer.h"
#include "Alien.h"
#include "GameScreen.h"

//---------------------------------------------------------------------------

class Alien: public PlayerObject
{
public:
	Alien(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim, GameScreen* gs, Ogre::SceneNode* cm, int &ht, SoundPlayer* sPlayer, Ogre::Light* alienLt, int clId);
	virtual ~Alien(void);
	virtual void addToScene(void);
    virtual void grabCamera();
	void setCam(float xP, float yP, float zP, float xD, float yD, float zD);
	void setLight(float xP, float yP, float zP);
	virtual void addToSimulator(void);
	virtual void update(void);
	virtual void damageTaken(void);
	virtual void injectKeyDown(const OIS::KeyEvent &arg);
	virtual void injectKeyUp(const OIS::KeyEvent &arg);
	void grabAsteroid(bool tryGrab);
	void aimAsteroid(int arg);
	void shootAsteroid(int arg);
	//Ogre::Vector3 getPos();

protected:
	//int health; 
	//bool rearView;
	Ogre::Vector3 direction;
	bool left;
	bool right;
	bool forward;
	bool back;
	bool turnRight;
	bool turnLeft;
	int &health;
	bool hasAsteroid;
	bool isBound;
	btHingeConstraint* asteroidBinder;
	Asteroid * currentAsteroid;
	//btFixedConstraint* asteroidBinder;
	//btGeneric6DofContraint *asteroidBinder;
};

//---------------------------------------------------------------------------

#endif // #ifndef __Alien_h_

//---------------------------------------------------------------------------
