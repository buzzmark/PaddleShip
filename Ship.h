#ifndef __Ship_h_
#define __Ship_h_

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#  include <OIS/OISEvents.h>
#  include <OIS/OISKeyboard.h>
#else
#  include <OISEvents.h>
#  include <OISKeyboard.h>
#endif

#include "GameObject.h"
#include "SoundPlayer.h"
#include "Paddle.h"
#include "Simulator.h"
#include <SdkTrays.h>

//---------------------------------------------------------------------------

class Ship: public GameObject 
{
public:
	Ship(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim, Ogre::SceneNode* cm, int &sc, SoundPlayer* sPlayer, Ogre::Light* shipLt);
	~Ship(void);
	virtual void addToScene(void);
	virtual void addToSimulator(void);
	virtual void update(void);
    void grabCamera(void);
	void setDeetsPan(OgreBites::ParamsPanel*mDeetsPan);
	void injectKeyDown(const OIS::KeyEvent &arg);
	void injectKeyUp(const OIS::KeyEvent &arg);
    void setPaddle(Paddle* p);
    Paddle* getPaddle();
	//Ogre::Vector3 getPos();

protected:
	Ogre::SceneNode* cameraNode;
	Ogre::Light* shipLight;
	Ogre::Camera* cam;
	OgreBites::ParamsPanel* mDetailsPanel;
	SoundPlayer* soundPlayer;
    bool motorRight;
	bool hasDecr;
	bool rearView;
	//bool changedView;
	int health; 
	bool forward;
	bool back;
	bool left;
	bool right;
	bool turnRight;
	bool turnLeft;
	int &score;
	bool outOfBounds;

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
