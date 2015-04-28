#ifndef __Alien_h_
#define __Alien_h_

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#  include <OIS/OISEvents.h>
#  include <OIS/OISKeyboard.h>
#else
#  include <OISEvents.h>
#  include <OISKeyboard.h>
#endif

#include "GameObject.h"
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


//---------------------------------------------------------------------------

class Alien: public GameObject 
{
public:
	Alien(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim, Ogre::SceneNode* cm, int &ht, std::deque<GameObject*>* oList, SoundPlayer* sPlayer, Ogre::Light* alienLt);
	~Alien(void);
	void addToScene(void);
    void grabCamera();
	void setCam(float xP, float yP, float zP, float xD, float yD, float zD);
	void setLight(float xP, float yP, float zP);
	void addToSimulator(void);
	void update(void);
	void setDeetsPan(OgreBites::ParamsPanel*mDeetsPan);
	void injectKeyDown(const OIS::KeyEvent &arg);
	void injectKeyDown(int key);
	void injectKeyUp(const OIS::KeyEvent &arg);
	void injectKeyUp(int key);
	void grabAsteroid(bool tryGrab);
	void aimAsteroid(int arg);
	void shootAsteroid(int arg);
	//Ogre::Vector3 getPos();

protected:
	Ogre::SceneNode* cameraNode;
	OgreBites::ParamsPanel* mDetailsPanel;
	SoundPlayer* soundPlayer;
	std::deque<GameObject*>* objList;
	//int health; 
	Ogre::Camera* cam;
	Ogre::Light* alienLight;
	//bool rearView;
	bool left;
	bool right;
	bool forward;
	bool back;
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
