#include "PlayerObject.h"
#include "GameScreen.h"

//---------------------------------------------------------------------------
PlayerObject::PlayerObject(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim, GameScreen* gs, Ogre::SceneNode* cm, SoundPlayer* sPlayer, Ogre::Light* lt, int clId) :
    GameObject(nym, mgr, sim),
    gameScreen(gs),
    cameraNode(cm),
    light(lt),
    cam(nullptr),
    mDetailsPanel(nullptr),
    soundPlayer(sPlayer),
    clientId(clId),
    hp (100) {}
//---------------------------------------------------------------------------
PlayerObject::~PlayerObject() {}
//---------------------------------------------------------------------------
void PlayerObject::setDeetsPan(OgreBites::ParamsPanel* mDeetsPan) {
    mDetailsPanel = mDeetsPan;
}
//---------------------------------------------------------------------------
void PlayerObject::setHealth(int health){
    hp = health;
}//---------------------------------------------------------------------------
int PlayerObject::getHealth(){
    return hp;
}
