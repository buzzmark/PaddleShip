#include <OgreMath.h>
#include "Ship.h"
#include <iostream>
//---------------------------------------------------------------------------
Ship::Ship(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim, Ogre::SceneNode* cm, int &sc, SoundPlayer* sPlayer, Ogre::Light* shipLt) : GameObject(nym, mgr, sim), score(sc)
{
	cameraNode = cm;
	shipLight = shipLt;
	cam = (Ogre::Camera*) cameraNode -> getAttachedObject("PlayerCam");
	hasDecr = false;
	//rootNode->getParent()->removeChild(cameraNode);
	//cameraNode->addChild(rootNode);
	//changedView = false;
	rearView = false;
	//rootNode->addChild(cameraNode);
	health = 100;
	left = false;
	right = false;
	forward = false;
	back = false;
	soundPlayer = sPlayer;
}
//---------------------------------------------------------------------------
Ship::~Ship(void)
{
}
//---------------------------------------------------------------------------
void Ship::addToScene(void)
{
	geom = sceneMgr->createEntity("shipEnt", "rocket.mesh");
	geom->setCastShadows(true);
	rootNode->attachObject(geom);

	shipLight = sceneMgr->createLight("shipLight");
    shipLight->setType(Ogre::Light::LT_POINT);
    shipLight->setPosition(Ogre::Vector3(0, 500, -250));
 
    shipLight->setDiffuseColour(0.7, 0.7, 0.7);
    shipLight->setSpecularColour(0.7, 0.7, 0.7);

	mass = 10.0f;
	shape = new btCapsuleShapeZ(3.0f, 15.0f);
}
//---------------------------------------------------------------------------
void Ship::addToSimulator(void)
{
	GameObject::addToSimulator();

	body->setLinearFactor(btVector3(1,0,1));
	body->setAngularFactor(btVector3(0,0,0));
}
//---------------------------------------------------------------------------
void Ship::update(void)
{
	if (sqrt(getPos().x*getPos().x+getPos().z*getPos().z) > 4000){
		body->applyCentralForce(btVector3(-5*getPos().x,0,-5*getPos().z));
	}
	if (left && body->getLinearVelocity().getX() < 250) {
		body->applyCentralForce(btVector3(1000,0,0));
	}
	if (right && body->getLinearVelocity().getX() > -250) {
		body->applyCentralForce(btVector3(-1000,0,0));
	}
	if (forward && body->getLinearVelocity().getZ() < 250) {
		body->applyCentralForce(btVector3(0,0,1000));
	}
	if (back && body->getLinearVelocity().getZ() > -250) {
		body->applyCentralForce(btVector3(0,0,-1000));
	}
	if(!context->hit) {
		hasDecr = false;
	}
	if (!hasDecr && context->hit){
		//lose health
		if (score > 0) {
			score-=1;
		}
		std::stringstream scoreVal;
 		scoreVal << "" << score;
 		if (mDetailsPanel==NULL) {
 	 		printf("mDetailsPanel is null ptr\n");
 	 	}
 	 	mDetailsPanel->setParamValue(0, scoreVal.str());
		soundPlayer->playShipHit();
		hasDecr = true;
	}

	shipLight->setPosition(Ogre::Vector3(getPos().x + 0,getPos().y + 500,getPos().z - 250));

	if (rearView) {
		//changedView = true;
		//Ogre::Camera* cam = (Ogre::Camera*) cameraNode -> getAttachedObject("PlayerCam");
		//cameraNode->setPosition(Ogre::Vector3(getPos().x + 0,getPos().y + 25,getPos().z + 40));
		cam -> setPosition(Ogre::Vector3(getPos().x + 0, getPos().y + 25, getPos().z + 40));
		//printf("KEY PRESSED Camera position is: %f,%f,%f", cam->getPosition().x, cam->getPosition().y, cam->getPosition().z);
		cam -> lookAt(Ogre::Vector3(getPos().x + 0, getPos().y +0, getPos().z -25));
	} else {
		//changedView = false;
		//Ogre::Camera* cam = (Ogre::Camera*) cameraNode -> getAttachedObject("PlayerCam");
		//cameraNode->setPosition(Ogre::Vector3(getPos().x + 0,getPos().y + 25,getPos().z - 40));
		cam -> setPosition(Ogre::Vector3(getPos().x + 0, getPos().y + 25, getPos().z - 40));
		//printf("KEY RELEASED Camera position is: %f,%f,%f", cam->getPosition().x, cam->getPosition().y, cam->getPosition().z);
		cam-> lookAt(Ogre::Vector3(getPos().x + 0, getPos().y + 0, getPos().z + 25));
	}
	
}
//---------------------------------------------------------------------------
void Ship::injectKeyDown(const OIS::KeyEvent &arg)
{
	if (arg.key == OIS::KC_A){
		left = true;
		
	}
	if (arg.key == OIS::KC_D){
		right = true;
	}
	if (arg.key == OIS::KC_W){
		forward = true;
	}
	if (arg.key == OIS::KC_S){
		back = true;
	}
	
	if (arg.key == OIS::KC_E){
		rearView = true;
		
		//Ogre::Camera* cam = (Ogre::Camera*) cameraNode -> getAttachedObject("PlayerCam");
		//cameraNode->setPosition(Ogre::Vector3(getPos().x + 0,getPos().y + 25,getPos().z + 40));
		//cam -> setPosition(Ogre::Vector3(getPos().x + 0, getPos().y + 25, getPos().z + 40));
		//cam -> lookAt(Ogre::Vector3(getPos().x + 0, getPos().y +0, getPos().z -25));
		
	}
	

}
//---------------------------------------------------------------------------
void Ship::injectKeyUp(const OIS::KeyEvent &arg)
{
	if (arg.key == OIS::KC_A){
		left = false;
	}
	if (arg.key == OIS::KC_D){
		right = false;
	}
	if (arg.key == OIS::KC_W){
		forward = false;
	}
	if (arg.key == OIS::KC_S){
		back = false;
	}
	
	if (arg.key == OIS::KC_E){
		rearView = false;
		
		//Ogre::Camera* cam = (Ogre::Camera*) cameraNode -> getAttachedObject("PlayerCam");
		//cameraNode->setPosition(Ogre::Vector3(getPos().x + 0,getPos().y + 25,getPos().z - 40));
		//cam -> setPosition(Ogre::Vector3(getPos().x + 0, getPos().y + 25, getPos().z - 40));
		//cam-> lookAt(Ogre::Vector3(getPos().x + 0, getPos().y + 0, getPos().z + 25));
		
	}
	
}
//---------------------------------------------------------------------------
void Ship::setDeetsPan(OgreBites::ParamsPanel*mDeetsPan)
{
	mDetailsPanel = mDeetsPan;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
/*
Ogre::Vector3 Ship::getPos()
{
	return GameObject::getPos();
}
*/
