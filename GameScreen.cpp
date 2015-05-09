#include "GameScreen.h"
#include "Ship.h"
#include "ShipAI.h"
#include "Alien.h"

//---------------------------------------------------------------------------
GameScreen::GameScreen(Ogre::SceneManager* sceneMgr, Ogre::SceneNode* cameraNode, SoundPlayer* sPlayer, Ogre::Light* shipLt, Ogre::Light* alienLt)
{
	score = 0;
	scoreAI = 0;
	alienHealth = 100;
	mSceneMgr = sceneMgr;
    mCameraNode = cameraNode;
	soundPlayer = sPlayer;
	sim = new Simulator(sceneMgr);
	std::deque<GameObject*>* objList = sim -> getObjList();
	ship = new Ship("Ship", sceneMgr, sim, this, cameraNode, score, sPlayer, shipLt);
	ship->setPaddle(new Paddle("paddle", sceneMgr, sim, ship -> getNode(), score, sPlayer));
	shipAI = new ShipAI("ShipAI",sceneMgr, sim, this, cameraNode, scoreAI, sPlayer, objList, 0);
	shipAI->setPaddle(new Paddle("paddleAI", sceneMgr, sim, ship -> getNode(), score, sPlayer));
	ast1 = new AsteroidSys(sceneMgr, sim, ship);
	isClient = false;
	singlePlayer = false;
    clientId = -1;
}
//---------------------------------------------------------------------------
GameScreen::~GameScreen(void)
{
	delete ship;
    delete shipAI;
	delete ast1;
	delete sim;

    for (auto client : clientObjects) {
        delete client.second;
    }
}
//---------------------------------------------------------------------------
void GameScreen::createScene(void)
{
	mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox");
	
	//ship
	ship->addToScene();
	ship->addToSimulator();

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
    addEnemyToMinimap(shipAI);

    minimap->show();

}
//---------------------------------------------------------------------------
void GameScreen::addPlayerToMinimap(GameObject* player){
    Ogre::Real mmWidth = 0.15;
    Ogre::OverlayManager& omgr = Ogre::OverlayManager::getSingleton();
    Ogre::OverlayElement* mmPlayerIcon = omgr.createOverlayElement( "Panel", player->getName() + "_icon");
    mmPlayerIcon->setMaterialName( "minimap_player" );
    mmBackground->addChild(mmPlayerIcon);
    mmPlayerIcon->setDimensions(0.15*mmWidth, 0.15*mmWidth*19.0/12.0);
    mmPlayerIcon->setHorizontalAlignment(Ogre::GHA_CENTER);
    mmPlayerIcon->setVerticalAlignment(Ogre::GVA_CENTER);
    mmPlayerIcons[player] = mmPlayerIcon;
}
//---------------------------------------------------------------------------
void GameScreen::addEnemyToMinimap(GameObject* enemy){
	Ogre::Real mmWidth = 0.15;
	Ogre::OverlayManager& omgr = Ogre::OverlayManager::getSingleton();
	Ogre::OverlayElement* mmEnemyIcon = omgr.createOverlayElement( "Panel", enemy->getName() + "_icon");
    mmEnemyIcon->setMaterialName( "minimap_enemy" );
    mmBackground->addChild(mmEnemyIcon);
    mmEnemyIcon->setDimensions(0.15*mmWidth, 0.15*mmWidth*19.0/12.0);
    mmEnemyIcon->setHorizontalAlignment(Ogre::GHA_CENTER);
    mmEnemyIcon->setVerticalAlignment(Ogre::GVA_CENTER);
    mmPlayerIcons[enemy] = mmEnemyIcon;
}
//---------------------------------------------------------------------------
void GameScreen::setClient(bool client){
    if (!client) {
        ship->grabCamera();
        addPlayerToMinimap(ship);
    } else {
        addEnemyToMinimap(ship);
    }

	isClient = client;
}
//---------------------------------------------------------------------------
void GameScreen::setSinglePlayer(bool single){
    if (single) {
        ship->grabCamera();
        addPlayerToMinimap(ship);
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

    int id;
    p >> id;

    while (id != -1) {
        Alien* alien;
        auto iter = clientObjects.find(id);

        if (iter == clientObjects.end()) {
            alien = createClientAlien(id);
        } else {
            alien = iter->second;
        }

        p >> pos >> rot;

        alien->setPosition(pos.x, pos.y, pos.z);
        alien->getNode()->setOrientation(rot);
        alien->setLight(pos.x, pos.y + 500, pos.z + 250);

        p >> id;
    }

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
    GameObject* myPlayerObj;

    if (!singlePlayer && isClient) {
        myPlayerObj = clientObjects[clientId];
    } else {
        myPlayerObj = ship;
    }

    for (GameObject* player : players) {
		playerRelativeX = player->getPos().x/4000.0;
		playerRelativeZ = player->getPos().z/4000.0;
		mmPlayerIcons[player]->setPosition(playerRelativeX*0.15/2.0 - iconWidth/2.0, playerRelativeZ*0.15*19.0/12.0/2.0 - iconHeight/2.0);

		//rotate texture
		if (player == myPlayerObj) {
			Ogre::Material *mat = mmPlayerIcons[player]->getMaterial().get();
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

    p << SPT_POSITIONS;

	Ogre::Vector3 pos = ship->getPos();
	Ogre::Quaternion rot = ship->getNode()->getOrientation();

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

    for (auto client : clientObjects) {
        p << client.first;

        Alien* alien = client.second;

        pos = alien->getPos();
        rot = alien->getNode()->getOrientation();
        p << pos << rot;
    }

    p << (int) -1;

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
}
//---------------------------------------------------------------------------
void GameScreen::injectKeyUp(const OIS::KeyEvent &arg)
{
	ship->injectKeyUp(arg);
}
//---------------------------------------------------------------------------
void GameScreen::clientKey(int id, int key){
    Alien* alien = clientObjects[id];
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
}
//---------------------------------------------------------------------------
std::vector<Asteroid*> GameScreen::getAsteroids() {
    return ast1->getAsteroids();
}
//---------------------------------------------------------------------------
std::vector<GameObject*> GameScreen::getPlayers() {
    std::vector<GameObject*> list({ship, shipAI});

    for (auto client : clientObjects) {
        list.push_back(client.second);
    }

    return list;
}
//---------------------------------------------------------------------------
Alien* GameScreen::createClientAlien(int id) {
    Alien* alien = new Alien("Alien" + std::to_string(id), mSceneMgr, sim, this, mCameraNode, alienHealth, sim->getObjList(), soundPlayer, NULL);

	alien->addToScene();
	alien->addToSimulator();

    if (id == clientId) {
        addPlayerToMinimap(alien);
        alien->grabCamera();
    } else {
        addEnemyToMinimap(alien);
    }

    clientObjects[id] = alien;

    return alien;
}

void GameScreen::removeClientAlien(int id) {
    Alien* alien = clientObjects[id];
    clientObjects.erase(id);

    Ogre::OverlayElement* icon = mmPlayerIcons[alien];
    mmPlayerIcons.erase(alien);
    mmBackground->removeChild(icon->getName());

    sim->removeObject(alien);
    alien->removeFromScene();
    delete alien;
}

void GameScreen::setClientId(int id) {
    clientId = id;
}
