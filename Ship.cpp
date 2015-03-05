#include <OgreMath.h>
#include "Ship.h"

//---------------------------------------------------------------------------
Ship::Ship(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim, Ogre::SceneNode* cam) : GameObject(nym, mgr, sim)
{
	cameraNode = cam;
	velocity = Ogre::Vector3(0.0f, 0.0f, 0.0f);
	rootNode->getParent()->removeChild(rootNode);
	cameraNode->addChild(rootNode);
	//rootNode->yaw(Ogre::Radian(M_PI));
	paddle = new Paddle("paddle", mgr, sim, rootNode);
}
//---------------------------------------------------------------------------
Ship::~Ship(void)
{
	delete paddle;
}
//---------------------------------------------------------------------------
void Ship::addToScene(void)
{
	geom = sceneMgr->createEntity("shipEnt", "rocket.mesh");
	geom->setCastShadows(true);
	rootNode->attachObject(geom);
	float scale = 1.0f;
	rootNode->setScale(Ogre::Vector3(scale,scale,scale));

	mass = 0.0f; //kinematic
	shape = new btCapsuleShapeZ(3.0f, 15.0f);

	paddle->addToScene();
}
//---------------------------------------------------------------------------
void Ship::addToSimulator(void)
{
	GameObject::addToSimulator();
	paddle->addToSimulator();
}
//---------------------------------------------------------------------------
void Ship::update(void)
{
	//rootNode->translate(velocity);
	//GameObject::update();
	paddle->update();
}
//---------------------------------------------------------------------------
void Ship::injectKeyDown(const OIS::KeyEvent &arg)
{
	if (arg.key == OIS::KC_A){
		velocity += Ogre::Vector3(-2.0f, 0.0f, 0.0f);
	}
	if (arg.key == OIS::KC_D){
		velocity += Ogre::Vector3(2.0f, 0.0f, 0.0f);
	}
	paddle->injectKeyDown(arg);
}
//---------------------------------------------------------------------------
void Ship::injectKeyUp(const OIS::KeyEvent &arg)
{
	if (arg.key == OIS::KC_A){
		velocity -= Ogre::Vector3(-2.0f, 0.0f, 0.0f);
	}
	if (arg.key == OIS::KC_D){
		velocity -= Ogre::Vector3(2.0f, 0.0f, 0.0f);
	}
	paddle->injectKeyUp(arg);
}
//---------------------------------------------------------------------------