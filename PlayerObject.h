#ifndef __PlayerObject_h_
#define __PlayerObject_h_

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

class GameScreen;

//---------------------------------------------------------------------------

class PlayerObject : public GameObject {
    protected:
        GameScreen* gameScreen;
        Ogre::SceneNode* cameraNode;
        Ogre::Light* light;
        Ogre::Camera* cam;
        OgreBites::ParamsPanel* mDetailsPanel;
        SoundPlayer* soundPlayer;
        int hp;

    public:
	    PlayerObject(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim, GameScreen* gs, Ogre::SceneNode* cm, SoundPlayer* sPlayer, Ogre::Light* lt);
        virtual ~PlayerObject();

        virtual void addToScene(void) = 0;
        virtual void removeFromScene(void);
        virtual void update(void) = 0;
        virtual void grabCamera() = 0;
        virtual void setDeetsPan(OgreBites::ParamsPanel* mDeetsPan);
        virtual int getHealth();
};

//---------------------------------------------------------------------------

#endif //#ifndef __PlayerObject_h

//---------------------------------------------------------------------------
