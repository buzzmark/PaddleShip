#ifndef __Paddle_h_
#define __Paddle_h_

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#  include <OIS/OISEvents.h>
#  include <OIS/OISKeyboard.h>
#else
#  include <OISEvents.h>
#  include <OISKeyboard.h>
#endif

#include "GameObject.h"
#include "SoundPlayer.h"
#include <SdkTrays.h>

//---------------------------------------------------------------------------

class Paddle: public GameObject 
{
public:
	Paddle(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim, Ogre::SceneNode* shipNode, int &sc, SoundPlayer* sPlayer);
	~Paddle(void);
	void addToScene(void);
	void addToSimulator(void);
	void update(void);
	void injectKeyDown(const OIS::KeyEvent &arg);
	void injectKeyUp(const OIS::KeyEvent &arg);
	void setPaddleHinge(btHingeConstraint* paddleHinge);
	btHingeConstraint* getPaddleHinge();

protected:
	Ogre::SceneNode* mShipNode;
	SoundPlayer* soundPlayer;
	btHingeConstraint* hinge;
	int &score;
	bool hasIncr;
	Ogre::String name;
};

//---------------------------------------------------------------------------

#endif // #ifndef __Paddle_h_

//---------------------------------------------------------------------------