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

#define IFRAMES_ON_HIT 180
#define IFRAMES_ON_SPAWN 180
#define IFRAMES_ON_DEFEND 90 //TODO use to stop damage when hitting asteroids
#define FRAMES_PER_OOB_DAMAGE 360
#define OOB_DAMAGE 1

//---------------------------------------------------------------------------


class PlayerObject : public GameObject {
    protected:
        GameScreen* gameScreen;
        Ogre::SceneNode* cameraNode;
        Ogre::Light* light;
        Ogre::Camera* cam;
        OgreBites::ParamsPanel* mDetailsPanel;
        SoundPlayer* soundPlayer;
        int clientId;
        int hp;
        int iframes;
        bool outOfBounds;
        int outOfBoundsTimer;

    public:
	    PlayerObject(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim, GameScreen* gs, Ogre::SceneNode* cm, SoundPlayer* sPlayer, Ogre::Light* lt, int clId);
        virtual ~PlayerObject();

        virtual void addToSimulator(void);
        virtual void update(void) = 0;
        virtual void grabCamera() = 0;
        virtual void setDeetsPan(OgreBites::ParamsPanel* mDeetsPan);
        virtual void setHealth(int health);
        virtual int getHealth();
        virtual void injectKeyDown(const OIS::KeyEvent &arg) = 0;
        virtual void injectKeyUp(const OIS::KeyEvent &arg) = 0;
};

//---------------------------------------------------------------------------

#endif //#ifndef __PlayerObject_h

//---------------------------------------------------------------------------
