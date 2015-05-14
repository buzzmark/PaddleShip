#ifndef __Ship_h_
#define __Ship_h_

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#  include <OIS/OISEvents.h>
#  include <OIS/OISKeyboard.h>
#else
#  include <OISEvents.h>
#  include <OISKeyboard.h>
#endif

#include "PlayerObject.h"
#include "SoundPlayer.h"
#include "Paddle.h"
#include "Simulator.h"
#include <SdkTrays.h>

class GameScreen;

//---------------------------------------------------------------------------

class Ship: public PlayerObject
{
public:
	Ship(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim, GameScreen* gs, Ogre::SceneNode* cm, int &sc, SoundPlayer* sPlayer, Ogre::Light* shipLt, int clId);
	virtual ~Ship(void);
	virtual void addToScene(void);
	virtual void addToSimulator(void);
    virtual void removeFromScene(void);
    virtual void removeFromSimulator(void);
	virtual void update(void);
    virtual void grabCamera(void);
	virtual void injectKeyDown(const OIS::KeyEvent &arg);
	virtual void injectKeyUp(const OIS::KeyEvent &arg);
    void setPaddle(Paddle* p);
    Paddle* getPaddle();
	//Ogre::Vector3 getPos();

protected:
    bool motorRight;
	bool hasDecr;
	bool rearView;
	//bool changedView;
	bool forward;
	bool back;
	bool left;
	bool right;
	bool turnRight;
	bool turnLeft;
	int &score;
	bool outOfBounds;
    int clientId;

    Ogre::Vector3 direction;
    Ogre::Vector3 prevDirection;
    Ogre::Vector3 deltDirection; 
    Ogre::Vector3 camPos;
    Ogre::Vector3 beforeMove;

    Paddle* paddle;
	btHingeConstraint* paddleHinge;
};

//---------------------------------------------------------------------------

#endif // #ifndef __Ship_h_

//---------------------------------------------------------------------------
