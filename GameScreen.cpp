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
    resetTimer = -1;
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
    checkBounds();
    if (resetTimer == -1){
        int count = 0;
        for (auto player : getPlayers())
            if (player->getHealth() > 0)
                count++;
        if (count < 2)
            resetTimer = 1500;
    } else if (resetTimer-- == 0){
        reset();
    }
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
void GameScreen::updateClientPlayers(Packet& p) {
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

    updateMinimap();
    checkBounds();
}
//---------------------------------------------------------------------------
void GameScreen::updateClientAsteroids(Packet& p) {
    std::vector<Asteroid*> asteroids = getAsteroids();
    for (Asteroid* ast : asteroids) {
        ast->readFromPacket(p);
    }
}
//---------------------------------------------------------------------------
void GameScreen::updateClientAsteroidsIncremental(Packet& p) {
    std::vector<Asteroid*> asteroids = getAsteroids();

    int id;
    p >> id;

    while (id != -1) {
        Asteroid* ast = asteroids[id];
        ast->readFromPacket(p);
        p >> id;
    }
}
//---------------------------------------------------------------------------
void GameScreen::updateClient(const Ogre::FrameEvent &evt, Packet& p)
{
    updateClientPlayers(p);
    updateClientAsteroids(p);
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

    PlayerObject* player;

    if (id == -1) player = shipAI;
    else player = clientObjects.find(id)->second;

    player->setHealth(hp);
    if (hp <= 0) player->getParticleSystem()->setEmitting(false);

    if (id == clientId) {
        CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->getChild("healthCounter")->setText((char*)message.c_str());
        if (hp <= 0) {
            CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->getChild("warningMessage")->setVisible(false);
            CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->getChild("deathMessage")->setVisible(true);
        }
    }

}
//---------------------------------------------------------------------------
void GameScreen::checkBounds(){
    PlayerObject* player = getCurrentPlayer();
    if (player == nullptr || player->getHealth() <= 0) return;
    Ogre::Vector3 pos = player->getPos();
    if (!warningVisible && sqrt(pos.x*pos.x+pos.z*pos.z) > 4000){
        CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->getChild("warningMessage")->setVisible(true);
        warningVisible = true;
    }
    else if (warningVisible && sqrt(pos.x*pos.x+pos.z*pos.z) <= 4000){
        CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->getChild("warningMessage")->setVisible(false);
        warningVisible = false;
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
void GameScreen::writePlayerPositions(Packet& p) {
    writePlayerObject(p, shipAI);

    for (auto client : clientObjects) {
        p << client.first;
        writePlayerObject(p, client.second);
    }

    p << (int) -1;
}
//---------------------------------------------------------------------------
void GameScreen::writeAsteroidPositions(Packet& p) {
    std::vector<Asteroid*> asteroids = getAsteroids();
    for (Asteroid* ast : asteroids) {
        ast->writeToPacket(p);
    }
}
//---------------------------------------------------------------------------
void GameScreen::writeAsteroidPositionsIncremental(Packet& p) {
    std::vector<Asteroid*> asteroids = getAsteroids();

    for (int i = 0; i < asteroids.size(); ++i) {
        Asteroid* ast = asteroids[i];
        if (ast->getNetState()) {
            p << i;
            ast->writeToPacket(p);
            ast->resetNetState();
        }
    }

    p << (int) -1;
}
//---------------------------------------------------------------------------
Packet GameScreen::getPositions()
{
	Packet p;

    p << (char) SPT_POSITIONS;
    writePlayerPositions(p);
    writeAsteroidPositions(p);

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

    if (arg.key == OIS::KC_P){
        reset();
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
    if (clientId == 0) {
    	player->addToSimulator();
    }

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
    if (id == 0) {
        shipAI->addToSimulator();
    }
}

void GameScreen::setNetManager(NetManager* nm) {
    netManager = nm;
}

NetManager* GameScreen::getNetManager() {
    return netManager;
}

void GameScreen::reset() {
    if (!isClient || singlePlayer){
        for (auto player : getPlayers()){
            //reset ship position
            sim->getDynamicsWorld()->removeRigidBody(player->getBody());
            delete player->getBody()->getMotionState();
            delete player->getBody();
            Ogre::Vector3 pos = randomSpawnPoint();
            player->getTransform()->setOrigin(btVector3(pos.x, pos.y, pos.z));
            Ogre::Quaternion qt = player->getNode()->getOrientation();
            player->getTransform()->setRotation(btQuaternion(qt.x, qt.y, qt.z, qt.w));
            player->setMotionState(new OgreMotionState(*(player -> getTransform()), player -> getNode()));
            btRigidBody::btRigidBodyConstructionInfo rbInfo(player->getMass(), player->getMotionState(), player->getShape(), player->getInertia());
            rbInfo.m_restitution = player->getRestitution();
            rbInfo.m_friction = player->getFriction();
            player->setBody(new btRigidBody(rbInfo));
            player->getBody()->setLinearFactor(btVector3(1,0,1));
            player->getBody()->setAngularFactor(btVector3(0,0,0));
            player->getBody()->forceActivationState(DISABLE_DEACTIVATION);
            sim->getDynamicsWorld()->addRigidBody(player->getBody());
        }
        if (!singlePlayer){
            Packet p;
            p << (char) SPT_RESET;
            netManager->messageClientsTCP(p);
        }
    }
    for (auto player : getPlayers()){
        player->setHealth(100);
        player->getParticleSystem()->setEmitting(true);
        
    }
    //reset all messages
    std::string message = std::string("HP:  ") + std::to_string(getCurrentPlayer()->getHealth());
    CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->getChild("healthCounter")->setText((char*)message.c_str());
    CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->getChild("warningMessage")->setVisible(false);
    CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->getChild("deathMessage")->setVisible(false);

}

Ogre::Vector3 GameScreen::randomSpawnPoint(){
    float maxP = 2000;
    float minP = -maxP;
    Ogre::Real xP = minP + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxP-minP)));
    Ogre::Real zP = minP + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxP-minP)));

    return Ogre::Vector3(xP,0,zP);
}
