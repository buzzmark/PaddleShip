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
#include <SdkTrays.h>

//---------------------------------------------------------------------------

class Ship: public GameObject 
{
public:
	Ship(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim, Ogre::SceneNode* cam);
	~Ship(void);
	void addToScene(void);
	void addToSimulator(void);
	void update(void);
	void setDeetsPan(OgreBites::ParamsPanel*mDeetsPan);
	void injectKeyDown(const OIS::KeyEvent &arg);
	void injectKeyUp(const OIS::KeyEvent &arg);

protected:
	Ogre::SceneNode* cameraNode;
	OgreBites::ParamsPanel* mDetailsPanel;
	int health; 
	bool left;
	bool right;
};

//---------------------------------------------------------------------------

#endif // #ifndef __Ship_h_

//---------------------------------------------------------------------------