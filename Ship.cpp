#include <OgreMath.h>
#include "Ship.h"
#include <iostream>
#include "GameScreen.h"
#include "Packet.h"
#include "NetManager.h"
#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/Renderer.h>

//---------------------------------------------------------------------------
Ship::Ship(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim, GameScreen* gs, Ogre::SceneNode* cm, int &sc, SoundPlayer* sPlayer, Ogre::Light* shipLt, int clId) : PlayerObject(nym, mgr, sim, gs, cm, sPlayer, shipLt, clId), score(sc), clientId(clId)
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
	left = false;
	right = false;
	forward = false;
	back = false;
	turnRight = false;
	turnLeft = false;
	/*
	deltDirection = Ogre::Vector3(0,0,0);
	prevDirection = rootNode->getOrientation() * Ogre::Vector3(0,0,1);
	beforeMove = getPos();
	//cam -> setPosition(Ogre::Vector3(0, 25, -40));
	camPos = Ogre::Vector3(getPos().x + 0, getPos().y + 25, getPos().z - 40);
	*/
    motorRight = true;
    paddle = NULL;

    float maxP = 2000;
    float minP = -maxP;
    Ogre::Real xP = minP + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxP-minP)));
    Ogre::Real zP = minP + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxP-minP)));
	rootNode->setPosition(Ogre::Vector3(xP,0,zP));
}
//---------------------------------------------------------------------------
Ship::~Ship(void)
{
    delete paddle;
}
//---------------------------------------------------------------------------
void Ship::addToScene(void)
{
	geom = sceneMgr->createEntity(name + "Ent", "rocket.mesh");
	geom->setCastShadows(true);
	rootNode->attachObject(geom);

	light = sceneMgr->createLight(name + "Light");
    light->setType(Ogre::Light::LT_POINT);
    light->setPosition(Ogre::Vector3(0, 500, -250));
 
    light->setDiffuseColour(0.7, 0.7, 0.7);
    light->setSpecularColour(0.7, 0.7, 0.7);

	mass = 10.0f;
	shape = new btCapsuleShapeZ(3.0f, 15.0f);

	pSys = sceneMgr->createParticleSystem(name + "PS", "ship_particles");
    Ogre::SceneNode* pNode = (Ogre::SceneNode*)rootNode->createChild(Ogre::Vector3(0,-1,-8));
    pNode->attachObject(pSys);

    paddle->addToScene();
}
//---------------------------------------------------------------------------
void Ship::addToSimulator(void)
{
	PlayerObject::addToSimulator();

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
void Ship::removeFromScene(void) {
    PlayerObject::removeFromScene();
    rootNode->removeAllChildren();
    paddle->removeFromScene();
}
//---------------------------------------------------------------------------
void Ship::removeFromSimulator(void) {
    PlayerObject::removeFromSimulator();
    paddle->removeFromSimulator();
}
//---------------------------------------------------------------------------
void Ship::update(void)
{
	if (hp <= 0) return;
	if (!outOfBounds && sqrt(getPos().x*getPos().x+getPos().z*getPos().z) > 4000){
		outOfBounds = true;
		outOfBoundsTimer = FRAMES_PER_OOB_DAMAGE;
	} else if (outOfBounds && sqrt(getPos().x*getPos().x+getPos().z*getPos().z) <= 4000){
		outOfBounds = false;
	}
	if (outOfBounds && outOfBoundsTimer-- <= 0){
		outOfBoundsTimer = FRAMES_PER_OOB_DAMAGE;
		hp -= OOB_DAMAGE;
		damageTaken();
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
	if (turnRight && body->getAngularVelocity().getY() > -5) {
		body->setAngularFactor(btVector3(0,1,0));
		body->applyTorque(btVector3(0,-1000,0));
		body->setAngularFactor(btVector3(0,0,0));
	}

	if (turnLeft && body->getAngularVelocity().getY() < 5) {
		body->setAngularFactor(btVector3(0,1,0));
		body->applyTorque(btVector3(0,1000,0));
		body->setAngularFactor(btVector3(0,0,0));
	} 

	if (!turnLeft && !turnRight) {
		body->setAngularVelocity(btVector3(0,((body->getAngularVelocity()).getY())*0.95,0));
	}

	//counteracts inertia when changing direction
	if (turnRight && body->getAngularVelocity().getY() > 0) {
		body->setAngularVelocity(btVector3(0,((body->getAngularVelocity()).getY())*0.95,0));
	}

	if (turnLeft && body->getAngularVelocity().getY() < 0) {
		body->setAngularVelocity(btVector3(0,((body->getAngularVelocity()).getY())*0.95,0));
	}

	if(!context->hit) {
		hasDecr = false;
	}

	if (!hasDecr && context->hit && iframes <= 0){
		//lose health
		if (hp > 0) {
			hp-=20;
			damageTaken();
		}
		soundPlayer->playShipHit();
		hasDecr = true;
	}
	if(iframes > 0) iframes--;

	light->setPosition(Ogre::Vector3(getPos().x + 0,getPos().y + 500,getPos().z - 250));
}
//---------------------------------------------------------------------------
void Ship::damageTaken(void){
	if (hp < 0) hp = 0;
	if (hp == 0){
		pSys->setEmitting(false);
		if (gameScreen->getCurrentPlayer() == this) {
			CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->getChild("warningMessage")->setVisible(false);
			outOfBounds = false;
			CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->getChild("deathMessage")->setVisible(true);
		}
	}
	if (hp > 0 && iframes < IFRAMES_ON_HIT)
		iframes = IFRAMES_ON_HIT;
    if (!gameScreen->getIsClient() && !gameScreen->isSinglePlayer()) {
        Packet p;
        p << (char) SPT_HEALTH << clientId << hp;
        gameScreen->getNetManager()->messageClientsTCP(p);
    }
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
Ogre::ParticleSystem* Ship::getParticleSystem()
{
    return pSys;
}
//---------------------------------------------------------------------------
