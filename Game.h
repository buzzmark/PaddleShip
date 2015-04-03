#ifndef __Game_h_
#define __Game_h_

#include "BaseApplication.h"
#include "GameScreen.h"
#include "SoundPlayer.h"
#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/Renderer.h>
#include "NetManager.h"

//---------------------------------------------------------------------------

class Game : public BaseApplication
{
public:
  Game(char *hostIP = NULL);
  virtual ~Game(void);

protected:
  virtual bool configure(void);
  virtual void createScene(void);
  virtual void createFrameListener(void);
	virtual void createCamera(void);
	virtual void createViewports(void);
  virtual void destroyScene(void);
	virtual bool frameRenderingQueued(const Ogre::FrameEvent &evt);

  virtual bool keyPressed(const OIS::KeyEvent &arg);
  virtual bool keyReleased(const OIS::KeyEvent &arg);
  virtual bool mouseMoved(const OIS::MouseEvent &arg);
  virtual bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
  virtual bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);

  virtual bool startSinglePlayer(const CEGUI::EventArgs &e);
  virtual void setUpSDL(void);
  virtual bool startHosting(const CEGUI::EventArgs &e);
  virtual bool joinGame(const CEGUI::EventArgs &e);

	CEGUI::OgreRenderer* mRenderer;
  CEGUI::Window *guiRoot;
  GameScreen *gameScreen;
  Ogre::SceneNode *mCameraNode;
  SoundPlayer *soundPlayer;
  NetManager *netMgr;
  bool gameStarted;
  bool isServer;
  bool singlePlayer;
  bool clientFound;
  char *host;
  float buffer[NETMANAGER_BUFFER_SIZE/4];
  bool test;
  Ogre::Light* shipLight;
  Ogre::Light* alienLight;

};

//---------------------------------------------------------------------------

#endif // #ifndef __Game_h_

//---------------------------------------------------------------------------