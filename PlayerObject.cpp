#include "PlayerObject.h"
#include "GameScreen.h"

//---------------------------------------------------------------------------
PlayerObject::PlayerObject(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim, GameScreen* gs, Ogre::SceneNode* cm, SoundPlayer* sPlayer, Ogre::Light* lt) :
    GameObject(nym, mgr, sim),
    gameScreen(gs),
    cameraNode(cm),
    light(lt),
    soundPlayer(sPlayer),
    hp (100) {}
//---------------------------------------------------------------------------
PlayerObject::~PlayerObject() {}
//---------------------------------------------------------------------------
void PlayerObject::setDeetsPan(OgreBites::ParamsPanel* mDeetsPan) {
    mDetailsPanel = mDeetsPan;
}
//---------------------------------------------------------------------------
void PlayerObject::removeFromScene(void) {
    rootNode->detachObject(geom);
}
//---------------------------------------------------------------------------
int PlayerObject::getHealth(){
	return hp;
}
