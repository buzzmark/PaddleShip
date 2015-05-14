#include "GameScreen.h"
#include "Ship.h"
#include "ShipAI.h"
#include "Alien.h"
#include "Game.h"

//---------------------------------------------------------------------------
GameScreen::GameScreen(Ogre::SceneManager* sceneMgr, Ogre::SceneNode* cameraNode, SoundPlayer* sPlayer, Ogre::Light* shipLt, Ogre::Light* alienLt)
{
	score = 0;
	scoreAI = 0;
	alienHealth = 100;
	mSceneMgr = sceneMgr;
    mCameraNode = cameraNode;
	soundPlayer = sPlayer;
	sim = new Simulator(sceneMgr, this);
	shipAI = new ShipAI("ShipAI",sceneMgr, sim, this, cameraNode, scoreAI, sPlayer, 0);
	shipAI->setPaddle(new Paddle("paddleAI", sceneMgr, sim, shipAI -> getNode(), score, sPlayer));
	ast1 = new AsteroidSys(sceneMgr, sim);
	isClient = false;
	singlePlayer = false;
    clientId = -1;
}
//---------------------------------------------------------------------------
GameScreen::~GameScreen(void)
{
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
void GameScreen::addPlayerToMinimap(PlayerObject* player){
    Ogre::Real mmWidth = 0.15;
    Ogre::OverlayManager& omgr = Ogre::OverlayManager::getSingleton();
    Ogre::OverlayElement* mmPlayerIcon = omgr.createOverlayElement( "Panel", player->getName() + "_icon");
    mmPlayerIcon->setMaterialName( "minimap_player" );
    mmBackground->addChild(mmPlayerIcon);
    mmPlayerIcon->setDimensions(0.15*mmWidth, 0.15*mmWidth*19.0/12.0);
    mmPlayerIcon->setHorizontalAlignment(Ogre::GHA_CENTER);
    mmPlayerIcon->setVerticalAlignment(Ogre::GVA_CENTER);
    mmPlayerIcons[player] = mmPlayerIcon;
    if(dynamic_cast<Alien*>(player) != nullptr)
        reverseSymbol = true;
    else
        reverseSymbol = false;
}
//---------------------------------------------------------------------------
void GameScreen::addEnemyToMinimap(PlayerObject* enemy){
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
	isClient = client;
}
//---------------------------------------------------------------------------
bool GameScreen::getIsClient() const {
    return isClient;
}
//---------------------------------------------------------------------------
void GameScreen::setSinglePlayer(bool single){
	singlePlayer = single;
}
//---------------------------------------------------------------------------
bool GameScreen::isSinglePlayer() const {
    return singlePlayer;
}
//---------------------------------------------------------------------------
void GameScreen::update(const Ogre::FrameEvent &evt)
{
	sim->stepSimulation(evt.timeSinceLastFrame, 1, 1/60.0f);
	updateMinimap();
    updateHealthDisplay();
}
//---------------------------------------------------------------------------
void updatePlayerObject(Packet &p, PlayerObject* player, char type) {
	Ogre::Vector3 pos;
	Ogre::Quaternion rot;

    p >> pos >> rot;

    player->setPosition(pos.x, pos.y, pos.z);
    player->getNode()->setOrientation(rot);

    if (type == ALIEN_SHIP) {
        Alien* alien = static_cast<Alien*>(player);
        alien->setLight(pos.x, pos.y + 500, pos.z + 250);
    } else {
        Ship* playerShip = static_cast<Ship*>(player);

        p >> pos >> rot;

        Paddle *paddle = playerShip->getPaddle();
        paddle->setPosition(pos.x, pos.y, pos.z);
        paddle->getNode()->setOrientation(rot);
    }
}
//---------------------------------------------------------------------------
void GameScreen::updateClient(const Ogre::FrameEvent &evt, Packet& p)
{
    char type;

    p >> type;
    updatePlayerObject(p, shipAI, type);

    int id;
    p >> id;

    while (id != -1) {
        char type;
        p >> type;

        PlayerObject* player;
        auto iter = clientObjects.find(id);

        if (iter == clientObjects.end()) {
            player = createClientObject(id, type);
        } else {
            player = iter->second;
        }

        updatePlayerObject(p, player, type);

        p >> id;
    }

    int numAsteroids;
    p >> numAsteroids;

	Ogre::Vector3 pos;
	Ogre::Quaternion rot;
    std::vector<Asteroid*> asteroids = getAsteroids();

    for (Asteroid* ast : asteroids) {
        p >> pos >> rot;
		ast->setPosition(pos.x, pos.y, pos.z);
		ast->getNode()->setOrientation(rot);
    }

    updateMinimap();
    
    //need to update health display with value from server
}
//---------------------------------------------------------------------------
void GameScreen::updateMinimap()
{
	Ogre::Real iconWidth = 0.15*0.15;
	Ogre::Real iconHeight= 0.15*0.15*19.0/12.0;

	std::vector<PlayerObject*> players = getPlayers();
    PlayerObject* myPlayerObj = getCurrentPlayer();

    for (PlayerObject* player : players) {
        if (player != myPlayerObj && ((PlayerObject*)player)->getHealth() <= 0 && mmPlayerIcons[player]->getMaterialName() != "minimap_dead"){
            if(player == shipAI) printf("!!!!!!!!!!!!!!!!!!!!!\n");
            mmPlayerIcons[player]->setMaterialName("minimap_dead");
        }
        else if (player != myPlayerObj && ((PlayerObject*)player)->getHealth() > 0 && mmPlayerIcons[player]->getMaterialName() == "minimap_dead"){
            mmPlayerIcons[player]->setMaterialName("minimap_enemy");
        }
		Ogre::Real playerRelativeX = player->getPos().x/4000.0;
		Ogre::Real playerRelativeZ = player->getPos().z/4000.0;
		mmPlayerIcons[player]->setPosition(playerRelativeX*0.15/2.0 - iconWidth/2.0, playerRelativeZ*0.15*19.0/12.0/2.0 - iconHeight/2.0);

		//rotate texture
		if (player == myPlayerObj) {
			Ogre::Material *mat = mmPlayerIcons[player]->getMaterial().get();
			Ogre::TextureUnitState *texture = mat->getTechnique(0)->getPass(0)->getTextureUnitState(0);
			Ogre::Vector3 dir = player->getNode()->getOrientation() * Ogre::Vector3(0,0,1);
			double rot = atan(dir.x/dir.z);
			int neg = dir.z < 0 ? 0 : M_PI;
			texture->setTextureRotate(Ogre::Radian(rot+neg+reverseSymbol*M_PI));
		} 

	}


}
//---------------------------------------------------------------------------
void GameScreen::updateHealthDisplay()
{
    PlayerObject* player = getCurrentPlayer();

    if (player != nullptr) {
        std::string message = std::string("HP:  ") + std::to_string(getCurrentPlayer()->getHealth());
        CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->getChild("healthCounter")->setText((char*)message.c_str());
    }
}
//---------------------------------------------------------------------------
void GameScreen::updateHealthDisplay(int id, int hp)
{
    std::string message = std::string("HP: ") + std::to_string(hp);

    if (id == -1) {
        shipAI->setHealth(hp);
    }
    else clientObjects.find(id)->second->setHealth(hp);

    if (id == clientId) {
        CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->getChild("healthCounter")->setText((char*)message.c_str());
        if (hp == 0) {
            CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->getChild("warningMessage")->setVisible(false);
            CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->getChild("deathMessage")->setVisible(true);
        }
    }

}
//---------------------------------------------------------------------------
void writePlayerObject(Packet &p, PlayerObject* player) {
    Ship* playerShip = dynamic_cast<Ship*>(player);

    Ogre::Vector3 pos;
    Ogre::Quaternion rot;

    if (playerShip != nullptr) {
        p << (char) PADDLE_SHIP;

        pos = player->getPos();
        rot = player->getNode()->getOrientation();
        p << pos << rot;

        Paddle* paddle = playerShip->getPaddle();
        pos = paddle->getPos();
        rot = paddle->getNode()->getOrientation();

        p << pos << rot;
    } else {
        p << (char) ALIEN_SHIP;

        pos = player->getPos();
        rot = player->getNode()->getOrientation();
        p << pos << rot;
    }
}
//---------------------------------------------------------------------------
Packet GameScreen::getPositions()
{
	Packet p;

    p << (char) SPT_POSITIONS;

    writePlayerObject(p, shipAI);

    for (auto client : clientObjects) {
        p << client.first;
        writePlayerObject(p, client.second);
    }

    p << (int) -1;

    std::vector<Asteroid*> asteroids = getAsteroids();
    p << (int) asteroids.size();

    Ogre::Vector3 pos;
    Ogre::Quaternion rot;

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

    PlayerObject* player = getCurrentPlayer();
    if (player != nullptr) {
    	getCurrentPlayer()->injectKeyDown(arg);
    }
}
//---------------------------------------------------------------------------
void GameScreen::injectKeyUp(const OIS::KeyEvent &arg)
{
    PlayerObject* player = getCurrentPlayer();
    if (player != nullptr) {
        getCurrentPlayer()->injectKeyUp(arg);
    }
}
//---------------------------------------------------------------------------
void GameScreen::clientKey(int id, bool isDown, unsigned char key){
    PlayerObject* player = clientObjects[id];
    OIS::KeyEvent event(nullptr, (OIS::KeyCode) key, 0);

    if (isDown) {
        player->injectKeyDown(event);
    } else {
        player->injectKeyUp(event);
    }
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
std::vector<Asteroid*> GameScreen::getAsteroids() {
    return ast1->getAsteroids();
}
//---------------------------------------------------------------------------
std::vector<PlayerObject*> GameScreen::getPlayers() {
    std::vector<PlayerObject*> list({shipAI});

    for (auto client : clientObjects) {
        list.push_back(client.second);
    }

    return list;
}
//---------------------------------------------------------------------------
PlayerObject* GameScreen::getCurrentPlayer() {
    auto iter = clientObjects.find(clientId);
    if (iter != clientObjects.end()) {
        return iter->second;
    } else {
        return nullptr;
    }
}
//---------------------------------------------------------------------------
PlayerObject* GameScreen::createClientObject(int id, int type) {
    PlayerObject* player;
    if (type == ALIEN_SHIP) {
        player = new Alien("Alien" + std::to_string(id), mSceneMgr, sim, this, mCameraNode, alienHealth, soundPlayer, NULL, id);
    } else {
        std::string name = "Ship" + std::to_string(id);
        Ship* playerShip = new Ship(name, mSceneMgr, sim, this, mCameraNode, score, soundPlayer, NULL, id);
        playerShip->setPaddle(new Paddle(name + "Paddle", mSceneMgr, sim, playerShip->getNode(), score, soundPlayer));
        player = playerShip;
    }

	player->addToScene();
	player->addToSimulator();

    if (id == clientId) {
        addPlayerToMinimap(player);
        player->grabCamera();
    } else {
        addEnemyToMinimap(player);
    }

    clientObjects[id] = player;

    return player;
}

void GameScreen::removeClientObject(int id) {
    PlayerObject* player = clientObjects[id];
    clientObjects.erase(id);

    if (player != nullptr) {
        Ogre::OverlayElement* icon = mmPlayerIcons[player];
        mmPlayerIcons.erase(player);
        mmBackground->removeChild(icon->getName());

        player->removeFromSimulator();
        player->removeFromScene();
        delete player;
    }
}

void GameScreen::setClientId(int id) {
    clientId = id;
}

void GameScreen::setNetManager(NetManager* nm) {
    netManager = nm;
}

NetManager* GameScreen::getNetManager() {
    return netManager;
}
