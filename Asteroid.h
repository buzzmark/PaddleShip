#ifndef __Asteroid_h_
#define __Asteroid_h_

#include <Ogre.h>
#include <stdlib.h>
#include "GameObject.h"
#include "Simulator.h"
#include "Ship.h"


//---------------------------------------------------------------------------

class Asteroid: public GameObject 
{
public:
	Asteroid(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim, Ship* sh);
	~Asteroid(void);
	void addToScene(int i);
	void update(void);
	void addToSimulator(void);
	void setDynamicsWorld( btDiscreteDynamicsWorld* world);
	btDiscreteDynamicsWorld* getDynamicsWorld();
	//Ogre::Vector3 getPos();

protected:
	Ogre::Vector3 asteroidVelocity;
	Ogre::Vector3 asteroidPosition;
	btVector3 asteroidRotation;
	btDiscreteDynamicsWorld* dynamicsWorld;
	float sphereSize;
	float massVal;
	Ship *ship;
};

//---------------------------------------------------------------------------

#endif // #ifndef __Asteroid_h_

//---------------------------------------------------------------------------