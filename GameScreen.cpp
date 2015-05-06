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
	ship->setPaddle(new Paddle("paddle", sceneMgr, sim, ship -> getNode(), score, sPlayer));
	alien = new Alien("Alien", sceneMgr, sim, cameraNode, alienHealth, objList, sPlayer, alienLt);
	shipAI = new ShipAI("ShipAI",sceneMgr, sim, cameraNode, scoreAI, sPlayer, objList, 0);
	shipAI->setPaddle(new Paddle("paddleAI", sceneMgr, sim, ship -> getNode(), score, sPlayer));
	ast1 = new AsteroidSys(sceneMgr, sim, ship);
	isClient = false;
	singlePlayer = false;
}
//---------------------------------------------------------------------------
GameScreen::~GameScreen(void)
{
	delete ship;
	delete alien;
    delete shipAI;
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

	//alien
	alien->addToScene();
	alien->addToSimulator();

    //asteroid particle system
    ast1->addToScene();
    ast1->addToSimulator(sim->getDynamicsWorld());

    //ship AI
	shipAI->addToScene();
	shipAI->addToSimulator();

	//minimap
	Ogre::OverlayManager& omgr = Ogre::OverlayManager::getSingleton();
    Ogre::Overlay* minimap = omgr.create( "minimap" );
    mmBackground = static_cast<Ogre::OverlayContainer*>(omgr.createOverlayElement( "Panel", "minimap_background"));
    mmBackground->setMaterialName( "minimap" );
    Ogre::Real mmWidth = 0.15;
    mmBackground->setDimensions(mmWidth, mmWidth*19.0/12.0);
    mmBackground->setHorizontalAlignment(Ogre::GHA_RIGHT);
    mmBackground->setLeft(-mmWidth);
    minimap->add2D(mmBackground);

    //will probably have to change this later, based on what players exist when the game starts
    addPlayerToMinimap(ship);
    addEnemyToMinimap(alien);
    addEnemyToMinimap(shipAI);

    minimap->show();

}
//---------------------------------------------------------------------------
void GameScreen::addPlayerToMinimap(GameObject* player){
    Ogre::Real mmWidth = 0.15;
    Ogre::OverlayManager& omgr = Ogre::OverlayManager::getSingleton();
    Ogre::OverlayElement* mmPlayerIcon = omgr.createOverlayElement( "Panel", "player_icon");
    mmPlayerIcon->setMaterialName( "minimap_player" );
    mmBackground->addChild(mmPlayerIcon);
    mmPlayerIcon->setDimensions(0.15*mmWidth, 0.15*mmWidth*19.0/12.0);
    mmPlayerIcon->setHorizontalAlignment(Ogre::GHA_CENTER);
    mmPlayerIcon->setVerticalAlignment(Ogre::GVA_CENTER);
    mmPlayerIcons.push_back(mmPlayerIcon);
}
//---------------------------------------------------------------------------
void GameScreen::addEnemyToMinimap(GameObject* enemy){
	Ogre::Real mmWidth = 0.15;
	Ogre::OverlayManager& omgr = Ogre::OverlayManager::getSingleton();
	int i = mmPlayerIcons.size();
	Ogre::OverlayElement* mmEnemyIcon = omgr.createOverlayElement( "Panel", "enemy" + std::to_string(i));
    mmEnemyIcon->setMaterialName( "minimap_enemy" );
    mmBackground->addChild(mmEnemyIcon);
    mmEnemyIcon->setDimensions(0.15*mmWidth, 0.15*mmWidth*19.0/12.0);
    mmEnemyIcon->setHorizontalAlignment(Ogre::GHA_CENTER);
    mmEnemyIcon->setVerticalAlignment(Ogre::GVA_CENTER);
    mmPlayerIcons.push_back(mmEnemyIcon);
}
//---------------------------------------------------------------------------
void GameScreen::setClient(bool client){
    if (client) {
        alien->grabCamera();
    } else {
        ship->grabCamera();
    }

	isClient = client;
}
//---------------------------------------------------------------------------
void GameScreen::setSinglePlayer(bool single){
    if (single) {
        ship->grabCamera();
    }

	singlePlayer = single;
}
//---------------------------------------------------------------------------
void GameScreen::update(const Ogre::FrameEvent &evt)
{
	sim->stepSimulation(evt.timeSinceLastFrame, 1, 1/60.0f);
	updateMinimap();
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
	//alien->setCam(pos.x, pos.y + 25, pos.z + 40, pos.x, pos.y, pos.z - 25);
	alien->setLight(pos.x, pos.y + 500, pos.z + 250);

	p >> pos >> rot;

    Paddle* paddle = ship->getPaddle();
	paddle->setPosition(pos.x, pos.y, pos.z);
	paddle->getNode()->setOrientation(rot);

	p >> pos >> rot;
	
    shipAI->setPosition(pos.x, pos.y, pos.z);
    shipAI->getNode()->setOrientation(rot);

	p >> pos >> rot;

    Paddle* paddleAI = shipAI->getPaddle();
	paddleAI->setPosition(pos.x, pos.y, pos.z);
	paddleAI->getNode()->setOrientation(rot);

    int numAsteroids;
    p >> numAsteroids;

    std::vector<Asteroid*> asteroids = getAsteroids();
    for (Asteroid* ast : asteroids) {
        p >> pos >> rot;
		ast->setPosition(pos.x, pos.y, pos.z);
		ast->getNode()->setOrientation(rot);
    }

    updateMinimap();
}
//---------------------------------------------------------------------------
void GameScreen::updateMinimap()
{
	Ogre::Real iconWidth = 0.15*0.15;
	Ogre::Real iconHeight= 0.15*0.15*19.0/12.0;
	Ogre::Real playerRelativeX; //TODO change ship to current player
	Ogre::Real playerRelativeZ; //TODO change ship to current player


	std::vector<GameObject*> players = getPlayers();
	for (int i = 0; i < players.size(); i++){
		GameObject* player = players[i];
		playerRelativeX = player->getPos().x/4000.0;
		playerRelativeZ = player->getPos().z/4000.0;
		mmPlayerIcons[i]->setPosition(playerRelativeX*0.15/2.0 - iconWidth/2.0, playerRelativeZ*0.15*19.0/12.0/2.0 - iconHeight/2.0);
		
		//rotate texture
		if (player == ship) { //TODO change to "if the current player"
			Ogre::Material *mat = mmPlayerIcons[i]->getMaterial().get();
			Ogre::TextureUnitState *texture = mat->getTechnique(0)->getPass(0)->getTextureUnitState(0);
			Ogre::Vector3 dir = player->getNode()->getOrientation() * Ogre::Vector3(0,0,1);
			double rot = atan(dir.x/dir.z);
			int neg = dir.z < 0 ? 0 : M_PI;
			texture->setTextureRotate(Ogre::Radian(rot+neg));
		} 

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

    Paddle* paddle = ship->getPaddle();
	pos = paddle->getPos();
	rot = paddle->getNode()->getOrientation();

	p << pos << rot;

	pos = shipAI->getPos();
	rot = shipAI->getNode()->getOrientation();

	p << pos << rot;

    Paddle* paddleAI = shipAI->getPaddle();
	pos = paddleAI->getPos();
	rot = paddleAI->getNode()->getOrientation();

	p << pos << rot;

    std::vector<Asteroid*> asteroids = getAsteroids();
    p << (int) asteroids.size();

    for (Asteroid* ast : asteroids) {
		pos = ast->getPos();
		rot = ast->getNode()->getOrientation();

		p << pos << rot;
    }

	return p;
}
//---------------------------------------------------------------------------
void GameScreen::injectKeyDown(const OIS::KeyEvent &arg)
{
	if (arg.key == OIS::KC_M){
		soundPlayer->soundOff();
	}
	if (arg.key == OIS::KC_N){
		soundPlayer->soundOn();
	}

	ship->injectKeyDown(arg);
	if (singlePlayer) alien->injectKeyDown(arg);
}
//---------------------------------------------------------------------------
void GameScreen::injectKeyUp(const OIS::KeyEvent &arg)
{
	ship->injectKeyUp(arg);
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
	alien->setDeetsPan(mDeetsPan);
}
//---------------------------------------------------------------------------
std::vector<Asteroid*> GameScreen::getAsteroids() {
    return ast1->getAsteroids();
}
//---------------------------------------------------------------------------
std::vector<GameObject*> GameScreen::getPlayers() {
    return std::vector<GameObject*>({ship, alien, shipAI});
}
