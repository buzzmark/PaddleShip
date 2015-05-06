#include <OgreMath.h>
#include "Alien.h"
#include <iostream>
//---------------------------------------------------------------------------
Alien::Alien(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim, Ogre::SceneNode* cm, int &ht, std::deque<GameObject*>* oList, SoundPlayer* sPlayer, Ogre::Light* alienLt) : GameObject(nym, mgr, sim), health(ht)
{
	cameraNode = cm;
	alienLight = alienLt;
	objList = oList;
	//cam = (Ogre::Camera*) cameraNode -> getAttachedObject("PlayerCam");
	//rootNode->getParent()->removeChild(rootNode);
	//cameraNode->addChild(rootNode);
	//rootNode = cameraNode;
	//health = 100;
	hasAsteroid = false;
	isBound = false;
	left = false;
	right = false;
	forward = false;
	back = false;
	soundPlayer = sPlayer;
	float minP = 1;
  	float maxP = 5;
  	Ogre::Real xP = minP + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxP-minP)));
  	minP = 30;
  	maxP = 40;
  	Ogre::Real zP = minP + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxP-minP)));
	rootNode->setPosition(Ogre::Vector3(xP, 0, 600));
}
//---------------------------------------------------------------------------
Alien::~Alien(void)
{
}
//---------------------------------------------------------------------------
void Alien::addToScene(void)
{
	geom = sceneMgr->createEntity(name + "Ent", "alien.mesh");
	geom->setCastShadows(true);
	rootNode->attachObject(geom);

	alienLight = sceneMgr->createLight(name + "Light");
    alienLight->setType(Ogre::Light::LT_POINT);
    alienLight->setPosition(Ogre::Vector3(getPos().x + 0,getPos().y + 500,getPos().z + 250));
 
    alienLight->setDiffuseColour(0.7, 0.7, 0.7);
    alienLight->setSpecularColour(0.7, 0.7, 0.7);

	mass = 10.0f;
	shape = shape = new btSphereShape(5);
}
//---------------------------------------------------------------------------
void Alien::removeFromScene(void) {
    rootNode->detachObject(geom);
}
//---------------------------------------------------------------------------
void Alien::addToSimulator(void)
{
	GameObject::addToSimulator();

	body->setLinearFactor(btVector3(1,0,1));
	body->setAngularFactor(btVector3(0,0,0));
}
//---------------------------------------------------------------------------
void Alien::update(void)
{
	if (left && body->getLinearVelocity().getX() > -50) {
		body->applyCentralForce(btVector3(-8000,0,0));
	}
	if (right && body->getLinearVelocity().getX() < 50) {
		body->applyCentralForce(btVector3(8000,0,0));
	}
	if (forward && body->getLinearVelocity().getZ() > -50) {
		body->applyCentralForce(btVector3(0,0,-8000));
	}
	if (back && body->getLinearVelocity().getZ() < 50) {
		body->applyCentralForce(btVector3(0,0,8000));
	}
	btVector3 alienVel = body->getLinearVelocity();
	body->setLinearVelocity(btVector3(alienVel.getX()*0.99,alienVel.getY()*0.99,alienVel.getZ()*0.99));
	if (context->hit){
		//lose health
		if (health > 0) {
			health = 0;
		}
		std::stringstream healthVal;
 		healthVal << "" << health;
 		if (mDetailsPanel==NULL) {
 	 		printf("mDetailsPanel is null ptr\n");
 	 	}
 	 	mDetailsPanel->setParamValue(2, healthVal.str());
		soundPlayer->playShipHit();
	}
	alienLight->setPosition(Ogre::Vector3(getPos().x + 0,getPos().y + 500,getPos().z + 250));
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
		cam -> setPosition(Ogre::Vector3(getPos().x + 0, getPos().y + 25, getPos().z - 40));
		//printf("KEY RELEASED Camera position is: %f,%f,%f", cam->getPosition().x, cam->getPosition().y, cam->getPosition().z);
		cam-> lookAt(Ogre::Vector3(getPos().x + 0, getPos().y + 0, getPos().z + 25));
	}
	*/
}
//---------------------------------------------------------------------------
void Alien::injectKeyDown(const OIS::KeyEvent &arg)
{
	if (arg.key == OIS::KC_J){
		left = true;
	}
	if (arg.key == OIS::KC_L){
		right = true;
	}
	if (arg.key == OIS::KC_I){
		forward = true;
	}
	if (arg.key == OIS::KC_K){
		back = true;
	}
	if (arg.key == OIS::KC_G){
		if (!hasAsteroid) {
			grabAsteroid(true);	
		}
	}
	/*
	if (arg.key == OIS::KC_P){
		rearView = true;
	}
	*/
	if (arg.key == OIS::KC_LEFT){
		if (hasAsteroid) {
			aimAsteroid(7);	
		}
	}
	if (arg.key == OIS::KC_RIGHT){
		if (hasAsteroid) {
			aimAsteroid(8);	
		}
	}
	if (arg.key == OIS::KC_UP){
		if (hasAsteroid) {
			aimAsteroid(9);	
		}
	}
	if (arg.key == OIS::KC_DOWN){
		if (hasAsteroid) {
			aimAsteroid(10);	
		}
	}
}
//---------------------------------------------------------------------------
void Alien::injectKeyDown(int key)
{
	if (key == 1){
		left = true;
	}
	if (key == 2){
		right = true;
	}
	if (key == 3){
		forward = true;
	}
	if (key == 4){
		back = true;
	}
	if (key == 5){
		if (!hasAsteroid) {
			grabAsteroid(true);	
		}
	}
	/*
	if (key == 6){
		rearView = true;
	}
	*/
	if (key == 7){
		if (hasAsteroid) {
			aimAsteroid(7);	
		}
	}
	if (key == 8){
		if (hasAsteroid) {
			aimAsteroid(8);	
		}
	}
	if (key == 9){
		if (hasAsteroid) {
			aimAsteroid(9);	
		}
	}
	if (key == 10){
		if (hasAsteroid) {
			aimAsteroid(10);	
		}
	}
}
//---------------------------------------------------------------------------
void Alien::injectKeyUp(const OIS::KeyEvent &arg)
{
	if (arg.key == OIS::KC_J){
		left = false;
	}
	if (arg.key == OIS::KC_L){
		right = false;
	}
	if (arg.key == OIS::KC_I){
		forward = false;
	}
	if (arg.key == OIS::KC_K){
		back = false;
	}
	/*
	if (arg.key == OIS::KC_P){
		rearView = false;
		
	}
	*/
	if (arg.key == OIS::KC_LEFT){
		if (hasAsteroid) {
			shootAsteroid(7);	
		}
	}
	if (arg.key == OIS::KC_RIGHT){
		if (hasAsteroid) {
			shootAsteroid(8);	
		}
	}
	if (arg.key == OIS::KC_UP){
		if (hasAsteroid) {
			shootAsteroid(9);	
		}
	}
	if (arg.key == OIS::KC_DOWN){
		if (hasAsteroid) {
			shootAsteroid(10);	
		}
	}
}
//---------------------------------------------------------------------------
void Alien::injectKeyUp(int key)
{
	if (key == 1){
		left = false;
	}
	if (key == 2){
		right = false;
	}
	if (key == 3){
		forward = false;
	}
	if (key == 4){
		back = false;
	}
	/*
	if (key == 6){
		rearView = false;
	}
	*/
	if (key == 7){
		if (hasAsteroid) {
			shootAsteroid(7);	
		}
	}
	if (key == 8){
		if (hasAsteroid) {
			shootAsteroid(8);	
		}
	}
	if (key == 9){
		if (hasAsteroid) {
			shootAsteroid(9);	
		}
	}
	if (key == 10){
		if (hasAsteroid) {
			shootAsteroid(10);	
		}
	}
}
//---------------------------------------------------------------------------
void Alien::setDeetsPan(OgreBites::ParamsPanel*mDeetsPan)
{
	mDetailsPanel = mDeetsPan;
}
//---------------------------------------------------------------------------

void Alien::grabAsteroid(bool tryGrab)
{
	if (tryGrab) {
		std::deque<GameObject*> oList = *objList;
		/*
		for (int i = 5; i < oList.size(); i++) {
			if (!hasAsteroid && (oList[i] -> getPos()).z <= ((rootNode ->getPosition()).z - 20) && ((oList[i] -> getPos()).z <= (rootNode ->getPosition()).z) && (oList[i] -> getPos()).x <= ((rootNode ->getPosition()).x + 20) && (oList[i] -> getPos()).x >= ((rootNode ->getPosition()).x - 20)) {
				hasAsteroid = true;
				currentAsteroid = (Asteroid *) oList[i];
				currentAsteroid -> getDynamicsWorld() -> removeRigidBody(currentAsteroid -> getBody());
				delete currentAsteroid -> getBody() -> getMotionState();
   	 			delete currentAsteroid -> getBody();
   	 			Ogre::Vector3 alienPos = getPos();
				currentAsteroid -> getTransform() -> setOrigin(btVector3(alienPos.x, alienPos.y + 17, alienPos.z));
				Ogre::Quaternion qt = currentAsteroid -> getNode()->getOrientation();
				currentAsteroid -> getTransform() -> setRotation(btQuaternion(qt.x, qt.y, qt.z, qt.w));
				currentAsteroid -> setMotionState(new OgreMotionState(*(currentAsteroid -> getTransform()), currentAsteroid -> getNode()));
				btRigidBody::btRigidBodyConstructionInfo rbInfo(currentAsteroid -> getMass(), currentAsteroid -> getMotionState(), currentAsteroid -> getShape(), currentAsteroid -> getInertia());
				rbInfo.m_restitution = currentAsteroid -> getRestitution();
				rbInfo.m_friction = currentAsteroid -> getFriction();
				currentAsteroid -> setBody(new btRigidBody(rbInfo));
				currentAsteroid -> getDynamicsWorld() -> addRigidBody(currentAsteroid -> getBody());
				asteroidBinder = new btHingeConstraint(*body, *currentAsteroid -> getBody(), btVector3(0,0,0), btVector3(0,-17,0), btVector3(0,1,0), btVector3(0,1,0));
				asteroidBinder->setLimit(M_PI/2, M_PI/2);
				currentAsteroid -> getDynamicsWorld() ->addConstraint(asteroidBinder, true);
				isBound = true;
			}
		}
		*/
		int i = 5;
		while (!hasAsteroid && i < oList.size()) {
			if ((oList[i] -> getPos()).z >= ((rootNode ->getPosition()).z - 300) && ((oList[i] -> getPos()).z <= (rootNode ->getPosition()).z + 300) && (oList[i] -> getPos()).x <= ((rootNode ->getPosition()).x + 300) && (oList[i] -> getPos()).x >= ((rootNode ->getPosition()).x - 300)) {
				hasAsteroid = true;
				currentAsteroid = (Asteroid *) oList[i];
				currentAsteroid -> getDynamicsWorld() -> removeRigidBody(currentAsteroid -> getBody());
				delete currentAsteroid -> getBody() -> getMotionState();
   	 			delete currentAsteroid -> getBody();
   	 			Ogre::Vector3 alienPos = getPos();
				currentAsteroid -> getTransform() -> setOrigin(btVector3(alienPos.x, alienPos.y + 17, alienPos.z));
				Ogre::Quaternion qt = currentAsteroid -> getNode()->getOrientation();
				currentAsteroid -> getTransform() -> setRotation(btQuaternion(qt.x, qt.y, qt.z, qt.w));
				currentAsteroid -> setMotionState(new OgreMotionState(*(currentAsteroid -> getTransform()), currentAsteroid -> getNode()));
				btRigidBody::btRigidBodyConstructionInfo rbInfo(currentAsteroid -> getMass(), currentAsteroid -> getMotionState(), currentAsteroid -> getShape(), currentAsteroid -> getInertia());
				rbInfo.m_restitution = currentAsteroid -> getRestitution();
				rbInfo.m_friction = currentAsteroid -> getFriction();
				currentAsteroid -> setBody(new btRigidBody(rbInfo));
				currentAsteroid -> getDynamicsWorld() -> addRigidBody(currentAsteroid -> getBody());
				asteroidBinder = new btHingeConstraint(*body, *currentAsteroid -> getBody(), btVector3(0,0,0), btVector3(0,-17,0), btVector3(0,1,0), btVector3(0,1,0));
				asteroidBinder->setLimit(M_PI/2, M_PI/2);
				currentAsteroid -> getDynamicsWorld() ->addConstraint(asteroidBinder, true);
				isBound = true;
			}
			i++;
			//printf("Value of i is: %d\n", i);
		}
	}
}

//---------------------------------------------------------------------------

void Alien::shootAsteroid(int arg) {
	if (isBound) {
		currentAsteroid -> getDynamicsWorld() ->removeConstraint(asteroidBinder);
		isBound = false;
	}

	//shoot asteroid in direction of choice
	if (arg == 7){
		currentAsteroid -> getBody()->setLinearVelocity(btVector3(-200,0,0));
	} else if (arg == 8){
		currentAsteroid -> getBody()->setLinearVelocity(btVector3(200,0,0));
	} else if (arg == 9){
		currentAsteroid -> getBody()->setLinearVelocity(btVector3(0,0,-200));
	} else if (arg == 10){
		currentAsteroid -> getBody()->setLinearVelocity(btVector3(0,0,200));
	}

	hasAsteroid = false;
}

//---------------------------------------------------------------------------

void Alien::aimAsteroid(int arg) {
	currentAsteroid -> getDynamicsWorld() ->removeConstraint(asteroidBinder);
	isBound = false;

	//reset position of asteroid to shoot in plane of alien
	currentAsteroid -> getDynamicsWorld() -> removeRigidBody(currentAsteroid -> getBody());
	delete currentAsteroid -> getBody() -> getMotionState();
   	delete currentAsteroid -> getBody();
   	Ogre::Vector3 alienPos = getPos();
	if (arg == 7){
		currentAsteroid -> getTransform() -> setOrigin(btVector3(alienPos.x - 17, alienPos.y, alienPos.z));
		if (currentAsteroid -> getBody() -> getLinearVelocity().getX()) {
			asteroidBinder = new btHingeConstraint(*body, *currentAsteroid -> getBody(), btVector3(0,0,0), btVector3(17,0,0), btVector3(1,0,0), btVector3(1,0,0));
			isBound = true;
		}
	} else if (arg == 8){
		currentAsteroid -> getTransform() -> setOrigin(btVector3(alienPos.x + 17, alienPos.y, alienPos.z));
		if (currentAsteroid -> getBody() -> getLinearVelocity().getX()) {
			asteroidBinder = new btHingeConstraint(*body, *currentAsteroid -> getBody(), btVector3(0,0,0), btVector3(-17,0,0), btVector3(1,0,0), btVector3(1,0,0));
			isBound = true;
		}
	} else if (arg == 9){
		currentAsteroid -> getTransform() -> setOrigin(btVector3(alienPos.x, alienPos.y, alienPos.z - 17));
		if (currentAsteroid -> getBody() -> getLinearVelocity().getZ()) {
			asteroidBinder = new btHingeConstraint(*body, *currentAsteroid -> getBody(), btVector3(0,0,0), btVector3(0,0,17), btVector3(0,0,1), btVector3(0,0,1));
			isBound = true;
		}
	} else if (arg == 10){
		currentAsteroid -> getTransform() -> setOrigin(btVector3(alienPos.x, alienPos.y, alienPos.z + 17));
		if (currentAsteroid -> getBody() -> getLinearVelocity().getZ()) {
			asteroidBinder = new btHingeConstraint(*body, *currentAsteroid -> getBody(), btVector3(0,0,0), btVector3(0,0,-17), btVector3(0,0,1), btVector3(0,0,1));
			isBound = true;
		}
	}

	//add constraint at this new position
	if (isBound) {
		asteroidBinder->setLimit(M_PI/2, M_PI/2);
		currentAsteroid -> getDynamicsWorld() ->addConstraint(asteroidBinder, true);
	}

	//recreate rigid body at this position
	Ogre::Quaternion qt = currentAsteroid -> getNode()->getOrientation();
	currentAsteroid -> getTransform() -> setRotation(btQuaternion(qt.x, qt.y, qt.z, qt.w));
	currentAsteroid -> setMotionState(new OgreMotionState(*(currentAsteroid -> getTransform()), currentAsteroid -> getNode()));
	btRigidBody::btRigidBodyConstructionInfo rbInfo(currentAsteroid -> getMass(), currentAsteroid -> getMotionState(), currentAsteroid -> getShape(), currentAsteroid -> getInertia());
	rbInfo.m_restitution = currentAsteroid -> getRestitution();
	rbInfo.m_friction = currentAsteroid -> getFriction();
	currentAsteroid -> setBody(new btRigidBody(rbInfo));
	currentAsteroid -> getDynamicsWorld() -> addRigidBody(currentAsteroid -> getBody());
}

void Alien::grabCamera() {
	cam = (Ogre::Camera*) cameraNode -> detachObject("PlayerCam");
	cam->setPosition(Ogre::Vector3(0,25,40));
	cam->lookAt(Ogre::Vector3(0,0,-25));
	rootNode -> attachObject(cam);
}

void Alien::setCam(float xP, float yP, float zP, float xD, float yD, float zD) {
	cam -> setPosition(Ogre::Vector3(xP, yP, zP));
	cam-> lookAt(Ogre::Vector3(xD, yD, zD));
}

void Alien::setLight(float xP, float yP, float zP) {
	alienLight->setPosition(Ogre::Vector3(xP,yP,zP));
    //alienLight->setDiffuseColour(1.0, 1.0, 1.0);
    //alienLight->setSpecularColour(1.0, 1.0, 1.0);
}
