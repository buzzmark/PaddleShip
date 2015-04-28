#include "GameScreen.h"

//---------------------------------------------------------------------------
GameScreen::GameScreen(Ogre::SceneManager* sceneMgr, Ogre::SceneNode* cameraNode, SoundPlayer* sPlayer, Ogre::Light* shipLt, Ogre::Light* alienLt)
{
	score = 0;
	scoreAI = 0;
	alienHealth = 100;
	mSceneMgr = sceneMgr;
	soundPlayer = sPlayer;
	sim = new Simulator(sceneMgr);
	std::deque<GameObject*>* objList = sim -> getObjList();
	ship = new Ship("Ship", sceneMgr, sim, cameraNode, score, sPlayer, shipLt);
	alien = new Alien("Alien", sceneMgr, sim, cameraNode, alienHealth, objList, sPlayer, alienLt);
	paddle = new Paddle("paddle", sceneMgr, sim, ship -> getNode(), score, sPlayer); 
	shipAI = new ShipAI("ShipAI",sceneMgr, sim, cameraNode, scoreAI, sPlayer, objList, 0);
	paddleAI = new Paddle("paddleAI", sceneMgr, sim, shipAI -> getNode(), scoreAI, sPlayer); 
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

    //ship AI
	shipAI->addToScene();
	shipAI->addToSimulator();

	//paddle for ship AI
	paddleAI->addToScene();
	paddleAI->addToSimulator();

	paddleHingeAI = new btHingeConstraint(*shipAI->getBody(), *paddleAI->getBody(), btVector3(0,0,5), btVector3(8.25,0,-5), btVector3(0,1,0), btVector3(0,1,0));
	paddleHingeAI->setLimit(-M_PI, 0);
	paddleHingeAI->enableAngularMotor(true, 100, 100);

	sim->getDynamicsWorld()->addConstraint(paddleHingeAI, true);
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
void GameScreen::updateClient(const Ogre::FrameEvent &evt, float * positions)
{
	ship->setPosition(positions[0], positions[1], positions[2]);
	ship->getNode()->setOrientation(Ogre::Quaternion(positions[3],positions[4],positions[5],positions[6]));
	alien->setPosition(positions[7], positions[8], positions[9]);
	alien->getNode()->setOrientation(Ogre::Quaternion(positions[10],positions[11],positions[12],positions[13]));
	alien->setCam(positions[7], positions[8] + 25, positions[9] + 40, positions[7], positions[8], positions[9] - 25);
	alien ->setLight(positions[7], positions[8] + 500, positions[9] + 250);
	paddle->setPosition(positions[14], positions[15], positions[16]);
	paddle->getNode()->setOrientation(Ogre::Quaternion(positions[17],positions[18],positions[19],positions[20]));
	
	std::deque<GameObject*> oList = *(sim->getObjList());
	int astIndex = 3;
	for(int i = 21; i < 21+7*NUM_ASTEROIDS; i+=7, astIndex++){
		Asteroid* ast = (Asteroid*)oList[astIndex];
		ast->setPosition(positions[i], positions[i+1], positions[i+2]);
		ast->getNode()->setOrientation(Ogre::Quaternion(positions[i+3],positions[i+4],positions[i+5],positions[i+6]));
	}

}
//---------------------------------------------------------------------------
int GameScreen::getPositions(float * positions)
{
	Ogre::Vector3 pos = ship->getPos();
	positions[0] = pos.x;
	positions[1] = pos.y;
	positions[2] = pos.z;
	Ogre::Quaternion rot = ship->getNode()->getOrientation();
	positions[3] = rot.w;
	positions[4] = rot.x;
	positions[5] = rot.y;
	positions[6] = rot.z;

	pos = alien->getPos();
	positions[7] = pos.x;
	positions[8] = pos.y;
	positions[9] = pos.z;
	rot = alien->getNode()->getOrientation();
	positions[10] = rot.w;
	positions[11] = rot.x;
	positions[12] = rot.y;
	positions[13] = rot.z;

	pos = paddle->getPos();
	positions[14] = pos.x;
	positions[15] = pos.y;
	positions[16] = pos.z;
	rot = paddle->getNode()->getOrientation();
	positions[17] = rot.w;
	positions[18] = rot.x;
	positions[19] = rot.y;
	positions[20] = rot.z;

	std::deque<GameObject*> oList = *(sim->getObjList());
	int astIndex = 3;
	for(int i = 21; i < 21+7*NUM_ASTEROIDS; i+=7, astIndex++){
		Asteroid* ast = (Asteroid*)oList[astIndex];
		pos = ast->getPos();
		positions[i] = pos.x;
		positions[i+1] = pos.y;
		positions[i+2] = pos.z;
		rot = ast->getNode()->getOrientation();
		positions[i+3] = rot.w;
		positions[i+4] = rot.x;
		positions[i+5] = rot.y;
		positions[i+6] = rot.z;
	}

	return (21+7*NUM_ASTEROIDS)*sizeof(float);
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