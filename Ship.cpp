#include <OgreMath.h>
#include "Ship.h"
#include <iostream>
#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/Renderer.h>

//---------------------------------------------------------------------------
Ship::Ship(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim, Ogre::SceneNode* cm, int &sc, SoundPlayer* sPlayer, Ogre::Light* shipLt) : PlayerObject(nym, mgr, sim, cm, sPlayer, shipLt), score(sc)
{
	//cam = (Ogre::Camera*) cameraNode -> getAttachedObject("PlayerCam");
	//cam = (Ogre::Camera*) cameraNode -> detachObject("PlayerCam");
	//rootNode -> attachObject(cam);
	hasDecr = false;
	//printf("Position of cameraNode: (%f, %f, %f)\n", (cameraNode -> getPosition()).x,(cameraNode -> getPosition()).y, (cameraNode -> getPosition()).z);
	//printf("Position of camera object: (%f, %f, %f)\n", (cam -> getPosition()).x,(cam -> getPosition()).y, (cam -> getPosition()).z);
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
	turnRight = false;
	turnLeft = false;
	outOfBounds = false;
	/*
	deltDirection = Ogre::Vector3(0,0,0);
	prevDirection = rootNode->getOrientation() * Ogre::Vector3(0,0,1);
	beforeMove = getPos();
	//cam -> setPosition(Ogre::Vector3(0, 25, -40));
	camPos = Ogre::Vector3(getPos().x + 0, getPos().y + 25, getPos().z - 40);
	*/
    motorRight = true;
    paddle = NULL;
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

	light = sceneMgr->createLight("light");
    light->setType(Ogre::Light::LT_POINT);
    light->setPosition(Ogre::Vector3(0, 500, -250));
 
    light->setDiffuseColour(0.7, 0.7, 0.7);
    light->setSpecularColour(0.7, 0.7, 0.7);

	mass = 10.0f;
	shape = new btCapsuleShapeZ(3.0f, 15.0f);

    paddle->addToScene();
}
//---------------------------------------------------------------------------
void Ship::addToSimulator(void)
{
	GameObject::addToSimulator();

	body->setLinearFactor(btVector3(1,0,1));
	body->setAngularFactor(btVector3(0,0,0));

    paddle->addToSimulator();

	paddleHinge = new btHingeConstraint(*getBody(), *paddle->getBody(), btVector3(0,0,5), btVector3(8.25,0,-5), btVector3(0,1,0), btVector3(0,1,0));
	paddleHinge->setLimit(-M_PI, 0);
	paddleHinge->enableAngularMotor(true, 100, 100);

	simulator->getDynamicsWorld()->addConstraint(paddleHinge, true);
	paddle -> setPaddleHinge(paddleHinge);
}
//---------------------------------------------------------------------------
void Ship::update(void)
{
	if (!outOfBounds && sqrt(getPos().x*getPos().x+getPos().z*getPos().z) > 4000){
		CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->getChild("warningMessage")->setVisible(true);
		outOfBounds = true;
	} else if (outOfBounds && sqrt(getPos().x*getPos().x+getPos().z*getPos().z) < 4000){
		CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->getChild("warningMessage")->setVisible(false);
		outOfBounds = false;
	}

	if (forward && body->getLinearVelocity().getZ() < 250) {
		direction = rootNode->getOrientation() * Ogre::Vector3(0,0,1);
		body->applyCentralForce(btVector3(10000*direction.x, 10000*direction.y, 10000*direction.z));
	}
	if (!forward && !back && body->getLinearVelocity().getZ() < 250) {
		direction = rootNode->getOrientation() * Ogre::Vector3(0,0,1);
		body->applyCentralForce(btVector3(6000*direction.x, 6000*direction.y, 6000*direction.z));
	}
	btVector3 shipVel = body->getLinearVelocity();
	body->setLinearVelocity(btVector3(shipVel.getX()*0.99,shipVel.getY()*0.99,shipVel.getZ()*0.99));
	if (turnRight) {
		body->setAngularFactor(btVector3(0,1,0));
		body->applyTorque(btVector3(0,-1000,0));
		body->setAngularFactor(btVector3(0,0,0));
	}
	if (turnLeft) {
		body->setAngularFactor(btVector3(0,1,0));
		body->applyTorque(btVector3(0,1000,0));
		body->setAngularFactor(btVector3(0,0,0));
	} 
	if (!turnLeft && !turnRight) {
		body->setAngularVelocity(btVector3(0,((body->getAngularVelocity()).getY())*0.95,0));
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

	light->setPosition(Ogre::Vector3(getPos().x + 0,getPos().y + 500,getPos().z - 250));
	/*
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
		Ogre::Real x = getPos().x - beforeMove.x;
		Ogre::Real y = getPos().y - beforeMove.y;
		Ogre::Real z = getPos().z - beforeMove.z;
		Ogre::Vector3 forwardMove = Ogre::Vector3(x, y, z);
		camPos = Ogre::Vector3((float)camPos.x + (float)forwardMove.x, (float)camPos.y + (float)forwardMove.y, (float)camPos.z + (float)forwardMove.z);
		beforeMove = getPos();

		direction = rootNode->getOrientation() * Ogre::Vector3(0,0,1);
		deltDirection = Ogre::Vector3((float)prevDirection.x - (float)direction.x, (float)prevDirection.y - (float)direction.y, (float)prevDirection.z - (float)direction.z);

		//printf("Direction is: (%f,%f,%f)\n", direction.x, direction.y, direction.z);
		//printf("prevDirection is: (%f,%f,%f)\n", prevDirection.x, prevDirection.y, prevDirection.z);
		//printf("deltDirection is: (%f,%f,%f)\n", deltDirection.x, deltDirection.y, deltDirection.z);
		prevDirection = rootNode->getOrientation() * Ogre::Vector3(0,0,1);

		camPos = Ogre::Vector3((float)camPos.x + (float)deltDirection.x, (float)camPos.y + (float)deltDirection.y, (float)camPos.z + (float)deltDirection.z);
		//printf("camPos is: (%f,%f,%f)\n", camPos.x, camPos.y, camPos.z);
		cam -> setPosition(camPos);
		//cam -> setDirection(Ogre::Vector3(direction.x,direction.y, direction.z));
		//printf("KEY RELEASED Camera position is: %f,%f,%f", cam->getPosition().x, cam->getPosition().y, cam->getPosition().z);
		cam-> lookAt(Ogre::Vector3(getPos().x + (direction.x*2), getPos().y + (direction.y*2), getPos().z + (direction.z*2)));
	}
	*/
}
//---------------------------------------------------------------------------
void Ship::injectKeyDown(const OIS::KeyEvent &arg)
{
	if (arg.key == OIS::KC_SPACE){
		if (motorRight)
			paddleHinge->enableAngularMotor(true, -100, 1000);
		else
			paddleHinge->enableAngularMotor(true, 100, 1000);
		motorRight = !motorRight;
		soundPlayer->playPaddleSwing();
	}
	if (arg.key == OIS::KC_W){
		beforeMove = getPos();
		forward = true;
	}
	if (arg.key == OIS::KC_S){
		back = true;
	}
	if (arg.key == OIS::KC_D){
		//prevDirection = rootNode->getOrientation() * Ogre::Vector3(0,0,1);
		turnRight = true;
	}
	if (arg.key == OIS::KC_A){
		//prevDirection = rootNode->getOrientation() * Ogre::Vector3(0,0,1);
		turnLeft = true;
	}
	if (arg.key == OIS::KC_C){
		rearView = true;
		
		//Ogre::Camera* cam = (Ogre::Camera*) cameraNode -> getAttachedObject("PlayerCam");
		//cameraNode->setPosition(Ogre::Vector3(getPos().x + 0,getPos().y + 25,getPos().z + 40));
		//cam -> setPosition(Ogre::Vector3(getPos().x + 0, getPos().y + 25, getPos().z + 40));
		//cam -> lookAt(Ogre::Vector3(getPos().x + 0, getPos().y +0, getPos().z -25));
		
	}
	
    paddle->injectKeyDown(arg);
}

void Ship::grabCamera() {
	cam = (Ogre::Camera*) cameraNode -> detachObject("PlayerCam");
	rootNode -> attachObject(cam);
}

//---------------------------------------------------------------------------
void Ship::injectKeyUp(const OIS::KeyEvent &arg)
{
	if (arg.key == OIS::KC_W){
		forward = false;
	}
	if (arg.key == OIS::KC_S){
		back = false;
	}
	if (arg.key == OIS::KC_D){
		//prevDirection = rootNode->getOrientation() * Ogre::Vector3(0,0,1);
		turnRight = false;
	}
	if (arg.key == OIS::KC_A){
		//prevDirection = rootNode->getOrientation() * Ogre::Vector3(0,0,1);
		turnLeft = false;
	}
	if (arg.key == OIS::KC_C){
		rearView = false;
		
		//Ogre::Camera* cam = (Ogre::Camera*) cameraNode -> getAttachedObject("PlayerCam");
		//cameraNode->setPosition(Ogre::Vector3(getPos().x + 0,getPos().y + 25,getPos().z - 40));
		//cam -> setPosition(Ogre::Vector3(getPos().x + 0, getPos().y + 25, getPos().z - 40));
		//cam-> lookAt(Ogre::Vector3(getPos().x + 0, getPos().y + 0, getPos().z + 25));
		
	}
	
    paddle->injectKeyUp(arg);
}
//---------------------------------------------------------------------------
void Ship::setDeetsPan(OgreBites::ParamsPanel*mDeetsPan)
{
	PlayerObject::setDeetsPan(mDeetsPan);
	paddle->setDeetsPan(mDeetsPan);
}
//---------------------------------------------------------------------------
void Ship::setPaddle(Paddle* p)
{
    paddle = p;
}
//---------------------------------------------------------------------------
Paddle* Ship::getPaddle()
{
    return paddle;
}
//---------------------------------------------------------------------------
