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
    clientFound = false;
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

    //warning gui
    CEGUI::Window *warningMessage = wmgr.createWindow("TaharezLook/Button", "warningMessage");
    warningMessage->setText("Warning: return to the arena");
    warningMessage->setPosition( CEGUI::UVector2( CEGUI::UDim( 0.25f, 0.0f ), CEGUI::UDim( 0.25f, 0.0f ) ) );
    warningMessage->setSize(CEGUI::USize(CEGUI::UDim(0.50, 0), CEGUI::UDim(0.05, 0)));
    guiRoot->addChild(warningMessage);
    warningMessage->setVisible(false);

    //minimap
    //CEGUI::Window *minimap = wmgr.loadLayoutFromFile("minimap.layout");
    //guiRoot->addChild(minimap);

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
        if(isServer && !clientFound){
            netMgr->checkForUpdates();  // accept connection
            if(netMgr->numClients() > 0){
                clientFound = true;
                gameStarted = true;
                gameScreen->setClient(false);
                gameScreen->setSinglePlayer(false);
                CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().hide();
                guiRoot->getChild("mainMenu")->setVisible(false);
            }
        }
    }
    else{
        if (singlePlayer)
            gameScreen->update(evt); //render game
        else if (!isServer) {
            NetUpdate serverUpdate = netMgr->checkForUpdates();

            if (serverUpdate.hasServerUpdate()) {
                gameScreen->updateClient(evt, serverUpdate.getServerUpdate());
            }
        } else if (isServer){
            NetUpdate clientUpdate = netMgr->checkForUpdates();
            auto& clientData = clientUpdate.data;

            auto iter = clientData.begin();
            if (iter != clientData.end()) {
                gameScreen->clientKey(iter->second.data()[0]);
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

    if(singlePlayer || isServer)
        gameScreen->injectKeyDown(arg);
    else {
        int message = -1;
        
        if (arg.key == OIS::KC_J){
            message = 1;
        }
        else if (arg.key == OIS::KC_L){
            message = 2;
        }
        else if (arg.key == OIS::KC_I){
            message = 3;
        }
        else if (arg.key == OIS::KC_K){
            message = 4;
        }
        else if (arg.key == OIS::KC_G){
            message = 5;
        }
        else if (arg.key == OIS::KC_P){
            message = 6;
        }
        else if (arg.key == OIS::KC_LEFT){
            message = 7;
        }
        else if (arg.key == OIS::KC_RIGHT){
            message = 8;
        }
        else if (arg.key == OIS::KC_UP){
            message = 9;
        }
        else if (arg.key == OIS::KC_DOWN){
            message = 10;
        }

        if (message != -1)
            netMgr->messageServer(Packet((char*) &message, sizeof(int)));
    }

    CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
    context.injectKeyDown((CEGUI::Key::Scan)arg.key);
    context.injectChar((CEGUI::Key::Scan)arg.text);

    return BaseApplication::keyPressed(arg);
}
//---------------------------------------------------------------------------
bool Game::keyReleased(const OIS::KeyEvent &arg)
{
    if(singlePlayer || isServer)
        gameScreen->injectKeyUp(arg);
    else {
        int message = -1;
        
        if (arg.key == OIS::KC_J){
            message = 11;
        }
        else if (arg.key == OIS::KC_L){
            message = 12;
        }
        else if (arg.key == OIS::KC_I){
            message = 13;
        }
        else if (arg.key == OIS::KC_K){
            message = 14;
        }
        else if (arg.key == OIS::KC_G){
            message = 15;
        }
        else if (arg.key == OIS::KC_P){
            message = 16;
        }
        else if (arg.key == OIS::KC_LEFT){
            message = 17;
        }
        else if (arg.key == OIS::KC_RIGHT){
            message = 18;
        }
        else if (arg.key == OIS::KC_UP){
            message = 19;
        }
        else if (arg.key == OIS::KC_DOWN){
            message = 20;
        }

        if (message != -1)
            netMgr->messageServer(Packet((char*) &message, sizeof(int)));
    }

    //mCameraMan->injectKeyUp(arg);

    CEGUI::System::getSingleton().getDefaultGUIContext().injectKeyUp((CEGUI::Key::Scan)arg.key);

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
    gameScreen->injectMouseMove(arg);
    //mCameraMan->injectMouseMove(arg);

    CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
    context.injectMouseMove(arg.state.X.rel, arg.state.Y.rel);
    // Scroll wheel.
    if (arg.state.Z.rel)
        context.injectMouseWheelChange(arg.state.Z.rel / 120.0f);
    return true;
}
//---------------------------------------------------------------------------
bool Game::mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{
    if (mTrayMgr->injectMouseDown(arg, id)) return true;
    gameScreen->injectMouseDown(arg, id);
    //mCameraMan->injectMouseDown(arg, id);
    CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonDown(convertButton(id));
    return true;
}
//---------------------------------------------------------------------------
bool Game::mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{
    if (mTrayMgr->injectMouseUp(arg, id)) return true;
    gameScreen->injectMouseUp(arg, id);
    //mCameraMan->injectMouseUp(arg, id);
    CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonUp(convertButton(id));
    return true;
}
//---------------------------------------------------------------------------
bool Game::startSinglePlayer(const CEGUI::EventArgs &e)
{
    singlePlayer = true;
    gameStarted = true;
    guiRoot->getChild("mainMenu")->setVisible(false);
    gameScreen->setSinglePlayer(true);
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

    if(!host){
        guiRoot->getChild("mainMenu/infoBox")->setText("Host must be provided as command line argument");
        return true;
    }

    netMgr->startClient(host);

    std::cout << "connected to " << host << std::endl;
    
    gameScreen->setClient(true);
    gameScreen->setSinglePlayer(false);
    gameStarted = true;

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
