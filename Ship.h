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


//---------------------------------------------------------------------------

class Ship: public GameObject 
{
public:
	Ship(Ogre::SceneManager* sceneMgr, Ogre::SceneNode* cameraNode);
	~Ship(void);
	void addToScene(void);
	void update(void);

	void injectKeyDown(const OIS::KeyEvent &arg);
	void injectKeyUp(const OIS::KeyEvent &arg);

protected:
	Ogre::SceneNode *shipNode;
	Ogre::Vector3 velocity;
	Ogre::Vector3 position;
};

//---------------------------------------------------------------------------

#endif // #ifndef __Ship_h_

//---------------------------------------------------------------------------