#ifndef __Asteroid_h_
#define __Asteroid_h_

#include <Ogre.h>
#include <stdlib.h>
#include "GameObject.h"
#include "Simulator.h"
#include "Packet.h"

//---------------------------------------------------------------------------

class Asteroid: public GameObject 
{
public:
	Asteroid(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim, int i);
	~Asteroid(void);
	void addToScene();
	void update(void);
	void addToSimulator(void);
	void setDynamicsWorld( btDiscreteDynamicsWorld* world);
	btDiscreteDynamicsWorld* getDynamicsWorld();

    void writeToPacket(Packet& p);
    void readFromPacket(Packet& p);
    bool getNetState() const;
    void resetNetState();
    void setHeld(bool h);

protected:
	Ogre::Vector3 asteroidVelocity;
	Ogre::Vector3 asteroidPosition;
	btVector3 asteroidRotation;
	btDiscreteDynamicsWorld* dynamicsWorld;
    int asteroidNum;
	float sphereSize;
	float massVal;

    btVector3 netVel;
    bool netState;
    bool isHeld;
};

//---------------------------------------------------------------------------

#endif // #ifndef __Asteroid_h_

//---------------------------------------------------------------------------
