#include "GameScreen.h"

//---------------------------------------------------------------------------
GameScreen::GameScreen(Ogre::SceneManager* sceneMgr, Ogre::SceneNode* cameraNode, SoundPlayer* sPlayer, Ogre::Light* shipLt, Ogre::Light* alienLt)
{
	score = 0;
	alienHealth = 100;
	mSceneMgr = sceneMgr;
	soundPlayer = sPlayer;
	sim = new Simulator(sceneMgr);
	std::deque<GameObject*>* objList = sim -> getObjList();
	ship = new Ship("Ship", sceneMgr, sim, cameraNode, score, sPlayer, shipLt);
	alien = new Alien("Alien", sceneMgr, sim, cameraNode, alienHealth, objList, sPlayer, alienLt);
	paddle = new Paddle("paddle", sceneMgr, sim, score, sPlayer); 
	ast1 = new AsteroidSys(sceneMgr, sim, ship);
	motorRight = true;
	isClient = false;
	singlePlayer = false;
}
//---------------------------------------------------------------------------
GameScreen::~GameScreen(void)
{
	delete ship;
	delete alien;
	delete paddle;
	delete ast1;
	delete sim;
}
//---------------------------------------------------------------------------
void GameScreen::createScene(void)
{
	mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox");
	
	//ship
	ship->addToScene();
	ship->addToSimulator();

	//paddle
	paddle->addToScene();
	paddle->addToSimulator();

	paddleHinge = new btHingeConstraint(*ship->getBody(), *paddle->getBody(), btVector3(0,0,5), btVector3(8.25,0,-5), btVector3(0,1,0), btVector3(0,1,0));
	paddleHinge->setLimit(-M_PI, 0);
	paddleHinge->enableAngularMotor(true, 100, 100);

	sim->getDynamicsWorld()->addConstraint(paddleHinge, true);

	//alien
	alien->addToScene();
	alien->addToSimulator();

    //asteroid particle system
    ast1->addToScene();
    ast1->addToSimulator(sim->getDynamicsWorld());
}
//---------------------------------------------------------------------------
void GameScreen::setClient(bool client){
	isClient = client;
}
//---------------------------------------------------------------------------
void GameScreen::setSinglePlayer(bool single){
	singlePlayer = single;
}
//---------------------------------------------------------------------------
void GameScreen::update(const Ogre::FrameEvent &evt)
{
	sim->stepSimulation(evt.timeSinceLastFrame, 1, 1/60.0f);
}
//---------------------------------------------------------------------------
void GameScreen::updateClient(const Ogre::FrameEvent &evt, Packet& p)
{
	Ogre::Vector3 pos;
	Ogre::Quaternion rot;

	p >> pos >> rot;

	ship->setPosition(pos.x, pos.y, pos.z);
	ship->getNode()->setOrientation(rot);

	p >> pos >> rot;

	alien->setPosition(pos.x, pos.y, pos.z);
	alien->getNode()->setOrientation(rot);
	alien->setCam(pos.x, pos.y + 25, pos.z + 40, pos.x, pos.y, pos.z - 25);
	alien->setLight(pos.x, pos.y + 500, pos.z + 250);

	p >> pos >> rot;

	paddle->setPosition(pos.x, pos.y, pos.z);
	paddle->getNode()->setOrientation(rot);
	
	std::deque<GameObject*> oList = *(sim->getObjList());
	int astIndex = 3;
	for(int i = 21; i < 21+7*NUM_ASTEROIDS; i+=7, astIndex++){
		Asteroid* ast = (Asteroid*)oList[astIndex];

		p >> pos >> rot;
		ast->setPosition(pos.x, pos.y, pos.z);
		ast->getNode()->setOrientation(rot);
	}

}
//---------------------------------------------------------------------------
Packet GameScreen::getPositions()
{
	Packet p;

	Ogre::Vector3 pos = ship->getPos();
	Ogre::Quaternion rot = ship->getNode()->getOrientation();

	p << pos << rot;

	pos = alien->getPos();
	rot = alien->getNode()->getOrientation();

	p << pos << rot;

	pos = paddle->getPos();
	rot = paddle->getNode()->getOrientation();

	p << pos << rot;

	std::deque<GameObject*> oList = *(sim->getObjList());
	int astIndex = 3;
	for(int i = 21; i < 21+7*NUM_ASTEROIDS; i+=7, astIndex++){
		Asteroid* ast = (Asteroid*)oList[astIndex];
		pos = ast->getPos();
		rot = ast->getNode()->getOrientation();

		p << pos << rot;
	}

	return p;
}
//---------------------------------------------------------------------------
void GameScreen::injectKeyDown(const OIS::KeyEvent &arg)
{
	
	if (arg.key == OIS::KC_SPACE){
		if (motorRight)
			paddleHinge->enableAngularMotor(true, -100, 1000);
		else
			paddleHinge->enableAngularMotor(true, 100, 1000);
		motorRight = !motorRight;
		soundPlayer->playPaddleSwing();
	}
	if (arg.key == OIS::KC_M){
		soundPlayer->soundOff();
	}
	if (arg.key == OIS::KC_N){
		soundPlayer->soundOn();
	}
	

	ship->injectKeyDown(arg);
	paddle->injectKeyDown(arg);
	if (singlePlayer) alien->injectKeyDown(arg);
}
//---------------------------------------------------------------------------
void GameScreen::injectKeyUp(const OIS::KeyEvent &arg)
{
	ship->injectKeyUp(arg);
	paddle->injectKeyUp(arg);
	if (singlePlayer) alien->injectKeyUp(arg);
}
//---------------------------------------------------------------------------
void GameScreen::clientKey(int key){
	if(key<=10) alien->injectKeyDown(key);
	else alien->injectKeyUp(key-10);
}
//---------------------------------------------------------------------------
void GameScreen::injectMouseMove(const OIS::MouseEvent &arg)
{
}
//---------------------------------------------------------------------------
void GameScreen::injectMouseDown(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{
}
//---------------------------------------------------------------------------
void GameScreen::injectMouseUp(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{
}
//---------------------------------------------------------------------------
void GameScreen::setDeetsPan(OgreBites::ParamsPanel*mDeetsPan)
{
	//mDetailsPanel = mDeetsPan;
	ship->setDeetsPan(mDeetsPan);
	paddle->setDeetsPan(mDeetsPan);
	alien->setDeetsPan(mDeetsPan);
}
//---------------------------------------------------------------------------
