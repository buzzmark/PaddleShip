#include <cstdlib>
#include <time.h>
#include "Game.h"
#include "GameScreen.h"
#include "NetUpdate.h"
#include <SdkCameraMan.h>

using namespace OgreBites;

//---------------------------------------------------------------------------
Game::Game(char *hostIP)
{
    srand(time(0));
    gameStarted = false;
    netMgr = NULL;
    isServer = false;
    host = hostIP;
    test = true;
    lastNetUpdate = std::chrono::steady_clock::now();
}
//---------------------------------------------------------------------------
Game::~Game(void)
{
    if(netMgr) delete netMgr;
}
//---------------------------------------------------------------------------
bool Game::configure(void)
{
    // Show the configuration dialog and initialise the system.
    // You can skip this and use root.restoreConfig() to load configuration
    // settings if you were sure there are valid ones saved in ogre.cfg.
    if(mRoot->showConfigDialog())
    {
        // If returned true, user clicked OK so initialise.
        // Here we choose to let the system create a default rendering window by passing 'true'.
        mWindow = mRoot->initialise(true, "Game Render Window");

        return true;
    }
    else
    {
        return false;
    }
}
//---------------------------------------------------------------------------
void Game::createCamera(void)
{
    // create the camera
    mCamera = mSceneMgr->createCamera("PlayerCam");
    // set its position, direction  
    mCameraNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("CameraNode");
    mCameraNode->attachObject(mCamera);
    mCameraNode->setPosition(Ogre::Vector3(0,0,0));

    mCamera->setPosition(Ogre::Vector3(0,25,-40));
    mCamera->lookAt(Ogre::Vector3(0,0,25));
    // set the near clip distance
    mCamera->setNearClipDistance(5);
 
    mCameraMan = new OgreBites::SdkCameraMan(mCamera);   // create a default camera controller
    mCameraMan->setStyle(CS_MANUAL);
}
//---------------------------------------------------------------------------
void Game::createViewports(void)
{
    // Create one viewport, entire window
    Ogre::Viewport* vp = mWindow->addViewport(mCamera);
    vp->setBackgroundColour(Ogre::ColourValue(0,0,0));
    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));    
}
//---------------------------------------------------------------------------
void Game::createFrameListener(void){
    BaseApplication::createFrameListener();
    gameScreen->setDeetsPan(mDetailsPanel);
}
//---------------------------------------------------------------------------
void Game::createScene(void)
{
    setUpSDL();

    //gui
    mRenderer = &CEGUI::OgreRenderer::bootstrapSystem();
    CEGUI::ImageManager::setImagesetDefaultResourceGroup("Imagesets");
    CEGUI::Font::setDefaultResourceGroup("Fonts");
    CEGUI::Scheme::setDefaultResourceGroup("Schemes");
    CEGUI::WidgetLookManager::setDefaultResourceGroup("LookNFeel");
    CEGUI::WindowManager::setDefaultResourceGroup("Layouts");
    CEGUI::SchemeManager::getSingleton().createFromFile("TaharezLook.scheme");
    CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().setDefaultImage("TaharezLook/MouseArrow");
    CEGUI::SchemeManager::getSingleton().createFromFile("VanillaSkin.scheme");

    CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();
    
    guiRoot = wmgr.loadLayoutFromFile("main_menu.layout");
    CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(guiRoot);
    
    CEGUI::Window *mainMenu = guiRoot->getChild("mainMenu");

    mainMenu->getChild("sPButton")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&Game::startSinglePlayer, this));
    mainMenu->getChild("hostButton")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&Game::startHosting, this));
    mainMenu->getChild("joinButton")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&Game::joinGame, this));
    mainMenu->getChild("infoBox")->setText(host ? host : "");
    ((CEGUI::RadioButton*)mainMenu->getChild("selectPaddleShip"))->setSelected(true);

    //warning gui
    CEGUI::Window *warningMessage = wmgr.createWindow("TaharezLook/Button", "warningMessage");
    warningMessage->setText("Warning: return to the arena");
    warningMessage->setPosition( CEGUI::UVector2( CEGUI::UDim( 0.25f, 0.0f ), CEGUI::UDim( 0.25f, 0.0f ) ) );
    warningMessage->setSize(CEGUI::USize(CEGUI::UDim(0.50, 0), CEGUI::UDim(0.05, 0)));
    guiRoot->addChild(warningMessage);
    warningMessage->setVisible(false);

    //death gui
    CEGUI::Window *deathMessage = wmgr.createWindow("TaharezLook/Button", "deathMessage");
    deathMessage->setText("You died! You will respawn when the round is over.");
    deathMessage->setPosition( CEGUI::UVector2( CEGUI::UDim( 0.25f, 0.0f ), CEGUI::UDim( 0.25f, 0.0f ) ) );
    deathMessage->setSize(CEGUI::USize(CEGUI::UDim(0.50, 0), CEGUI::UDim(0.05, 0)));
    guiRoot->addChild(deathMessage);
    deathMessage->setVisible(false);

    //health gui
    CEGUI::Window *healthCounter = wmgr.createWindow("TaharezLook/StaticText", "healthCounter");
    healthCounter->setText("HP:  100");
    healthCounter->setPosition( CEGUI::UVector2( CEGUI::UDim( 0.0f, 0.0f ), CEGUI::UDim( 0.95f, 0.0f ) ) );
    healthCounter->setSize(CEGUI::USize(CEGUI::UDim(0.1, 0), CEGUI::UDim(0.05, 0)));
    guiRoot->addChild(healthCounter);

    //sound
    soundPlayer = new SoundPlayer();
    soundPlayer->startBgMusic();
    gameScreen = new GameScreen(mSceneMgr, mCameraNode, soundPlayer, shipLight, alienLight);

    //game screen
    gameScreen->createScene();

    //Lights
    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.1, 0.1, 0.1));
    mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);
    
}
//---------------------------------------------------------------------------
void Game::destroyScene(void){
    delete gameScreen;
    delete soundPlayer;
    BaseApplication::destroyScene(); 
}
//---------------------------------------------------------------------------
bool Game::frameRenderingQueued(const Ogre::FrameEvent &evt){
    if(!gameStarted){
        if(isServer){
            gameStarted = true;
            gameScreen->setClient(false);
            gameScreen->setSinglePlayer(false);
            gameScreen->setClientId(0);
            gameScreen->createClientObject(0, shipType);
            CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().hide();
            guiRoot->getChild("mainMenu")->setVisible(false);
        }
    }
    else{
        if (singlePlayer)
            gameScreen->update(evt); //render game
        else if (!isServer) {
            NetUpdate serverUpdate = netMgr->checkForUpdates();

            if (!serverUpdate.disconnects.empty()) {
                std::cout << "Server disconnected" << std::endl;
                exit(1);
            }

            if (serverUpdate.hasServerUpdate()) {
                // TODO: refactor by moving out to separate method
                Packet& p = serverUpdate.getServerUpdate();

                int packetType;
                int id;
                p >> packetType;

                switch (packetType) {
                    case SPT_POSITIONS:
                        gameScreen->updateClient(evt, p);
                        break;
                    case SPT_CLIENTID:
                        p >> id;
                        gameScreen->setClientId(id);
                        break;
                    case SPT_DISCONNECT:
                        p >> id;
                        gameScreen->removeClientObject(id);
                        break;
                    default:
                        std::cerr << "Warning: unrecognized server packet type " << packetType;
                        break;
                }
            }
        } else if (isServer){
            NetUpdate clientUpdate = netMgr->checkForUpdates();

            for (int id : clientUpdate.disconnects) {
                Packet p;
                p << SPT_DISCONNECT << id;
                netMgr->messageClients(p);

                gameScreen->removeClientObject(id);
            }

            if (clientUpdate.newConnection) {
                int id = clientUpdate.connectionId;

                Packet p;
                p << SPT_CLIENTID << id;
                netMgr->messageClient(id, p);
            }

            auto& clientData = clientUpdate.data;

            for (auto data : clientData) {
                Packet& p = data.second;
                int id = data.first;

                int packetType;
                char value;

                p >> packetType >> value;

                switch (packetType) {
                    case CPT_SHIPTYPE:
                        gameScreen->createClientObject(id, value);
                        break;
                    case CPT_KEYPRESS:
                    case CPT_KEYRELEASE:
                        gameScreen->clientKey(id, packetType == CPT_KEYPRESS, value);
                        break;
                    default:
                        std::cerr << "Warning: unrecognized client packet type " << packetType;
                        break;
                }
            }

            gameScreen->update(evt);

            auto now = std::chrono::steady_clock::now();
            if (now - lastNetUpdate > std::chrono::milliseconds(16)) {
                Packet p = gameScreen->getPositions();
                netMgr->messageClients(p);
            }
        }
    }
        

    CEGUI::System::getSingleton().injectTimePulse(evt.timeSinceLastFrame);
    return BaseApplication::frameRenderingQueued(evt);
}
//---------------------------------------------------------------------------
bool Game::keyPressed(const OIS::KeyEvent &arg){
    if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up

    CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
    context.injectKeyDown((CEGUI::Key::Scan)arg.key);
    context.injectChar((CEGUI::Key::Scan)arg.text);

    if(arg.key == OIS::KC_ESCAPE){
        mShutDown = true;
        return true;
    }


    if(!gameStarted) return true;

    if(singlePlayer || isServer)
        gameScreen->injectKeyDown(arg);
    else {
        Packet p;
        p << CPT_KEYPRESS << (char) arg.key;
        netMgr->messageServer(p);
    }

    return BaseApplication::keyPressed(arg);
}
//---------------------------------------------------------------------------
bool Game::keyReleased(const OIS::KeyEvent &arg)
{
    CEGUI::System::getSingleton().getDefaultGUIContext().injectKeyUp((CEGUI::Key::Scan)arg.key);
    if(!gameStarted) return true;

    if(singlePlayer || isServer)
        gameScreen->injectKeyUp(arg);
    else {
        Packet p;
        p << CPT_KEYRELEASE << (char) arg.key;
        netMgr->messageServer(p);
    }

    //mCameraMan->injectKeyUp(arg);

    return true;
}
//---------------------------------------------------------------------------
CEGUI::MouseButton convertButton(OIS::MouseButtonID buttonID)
{
    switch (buttonID)
    {
    case OIS::MB_Left:
        return CEGUI::LeftButton;
 
    case OIS::MB_Right:
        return CEGUI::RightButton;
 
    case OIS::MB_Middle:
        return CEGUI::MiddleButton;
 
    default:
        return CEGUI::LeftButton;
    }
}
//---------------------------------------------------------------------------
bool Game::mouseMoved(const OIS::MouseEvent &arg)
{
    if (mTrayMgr->injectMouseMove(arg)) return true;

    CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
    context.injectMouseMove(arg.state.X.rel, arg.state.Y.rel);
    // Scroll wheel.
    if (arg.state.Z.rel)
        context.injectMouseWheelChange(arg.state.Z.rel / 120.0f);

    if(!gameStarted) return true;

    gameScreen->injectMouseMove(arg);
    //mCameraMan->injectMouseMove(arg);
    
    return true;
}
//---------------------------------------------------------------------------
bool Game::mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{
    if (mTrayMgr->injectMouseDown(arg, id)) return true;
   
    //mCameraMan->injectMouseDown(arg, id);
    CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonDown(convertButton(id));

    if(!gameStarted) return true;

    gameScreen->injectMouseDown(arg, id);
    return true;
}
//---------------------------------------------------------------------------
bool Game::mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{
    if (mTrayMgr->injectMouseUp(arg, id)) return true;
    
    //mCameraMan->injectMouseUp(arg, id);
    CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonUp(convertButton(id));

    if(!gameStarted) return true;

    gameScreen->injectMouseUp(arg, id);
    return true;
}
//---------------------------------------------------------------------------
bool Game::startSinglePlayer(const CEGUI::EventArgs &e)
{
    singlePlayer = true;
    gameStarted = true;
    shipType = ((CEGUI::RadioButton*)guiRoot->getChild("mainMenu/selectPaddleShip"))->isSelected() ? PADDLE_SHIP : ALIEN_SHIP;
    guiRoot->getChild("mainMenu")->setVisible(false);
    gameScreen->setSinglePlayer(true);
    gameScreen->setClientId(0);
    gameScreen->createClientObject(0, shipType);
    CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().hide();
    return true;
}
//---------------------------------------------------------------------------
void Game::setUpSDL(void)
{
    netMgr = new NetManager();
    
}
//---------------------------------------------------------------------------
bool Game::startHosting(const CEGUI::EventArgs &e)
{
    singlePlayer = false;
    isServer = true;
    shipType = ((CEGUI::RadioButton*)guiRoot->getChild("mainMenu/selectPaddleShip"))->isSelected() ? PADDLE_SHIP : ALIEN_SHIP;
    guiRoot->getChild("mainMenu/sPButton")->setVisible(false);
    guiRoot->getChild("mainMenu/hostButton")->setVisible(false);
    guiRoot->getChild("mainMenu/joinButton")->setVisible(false);
    guiRoot->getChild("mainMenu/infoBox")->setText("Waiting for another player...");
    netMgr->startServer();
    return true;
}
//---------------------------------------------------------------------------
bool Game::joinGame(const CEGUI::EventArgs &e)
{
    singlePlayer = false;
    isServer = false;
    shipType = ((CEGUI::RadioButton*)guiRoot->getChild("mainMenu/selectPaddleShip"))->isSelected() ? PADDLE_SHIP : ALIEN_SHIP;

    host = (char*)guiRoot->getChild("mainMenu/infoBox")->getText().c_str();
    netMgr->startClient(host);
    std::cout << "connected to " << host << std::endl;

    gameScreen->setClient(true);
    gameScreen->setSinglePlayer(false);
    gameStarted = true;

    Packet p;
    p << CPT_SHIPTYPE << (char) shipType;
    netMgr->messageServer(p);

    guiRoot->getChild("mainMenu")->setVisible(false);
    CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().hide();

    return true;
}
//---------------------------------------------------------------------------

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
    int main(int argc, char *argv[])
#endif
    {
        #if OGRE_DOUBLE_PRECISION == 1
            printf("Ogre must be compiled without double precision\n");
            exit(1)
        #endif

        // Create application object
        char* host = argc > 1 ? argv[1] : NULL;
        Game app(host);

        try {
            app.go();
        } catch(Ogre::Exception& e)  {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBox(NULL, e.getFullDescription().c_str(), "An exception has occurred!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
            std::cerr << "An exception has occurred: " <<
                e.getFullDescription().c_str() << std::endl;
#endif
        }

        return 0;
    }

#ifdef __cplusplus
}
#endif

//---------------------------------------------------------------------------
