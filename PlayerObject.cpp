#include "PlayerObject.h"

//---------------------------------------------------------------------------
PlayerObject::PlayerObject(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim, Ogre::SceneNode* cm, SoundPlayer* sPlayer, Ogre::Light* lt) :
    GameObject(nym, mgr, sim),
    cameraNode(cm),
    light(lt),
    soundPlayer(sPlayer) {}
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
