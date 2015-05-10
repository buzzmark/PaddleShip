#include "PlayerObject.h"
#include "GameScreen.h"

//---------------------------------------------------------------------------
PlayerObject::PlayerObject(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim, GameScreen* gs, Ogre::SceneNode* cm, SoundPlayer* sPlayer, Ogre::Light* lt) :
    GameObject(nym, mgr, sim),
    gameScreen(gs),
    cameraNode(cm),
    light(lt),
    cam(nullptr),
    mDetailsPanel(nullptr),
    soundPlayer(sPlayer),
    hp (100) {}
//---------------------------------------------------------------------------
PlayerObject::~PlayerObject() {}
//---------------------------------------------------------------------------
void PlayerObject::setDeetsPan(OgreBites::ParamsPanel* mDeetsPan) {
    mDetailsPanel = mDeetsPan;
}
//---------------------------------------------------------------------------
int PlayerObject::getHealth(){
	return hp;
}
