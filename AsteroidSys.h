#ifndef __AsteroidSys_h_
#define __AsteroidSys_h_

#include <vector>
#include "Asteroid.h"
#define NUM_ASTEROIDS 250

//---------------------------------------------------------------------------

class AsteroidSys
{
public:
	AsteroidSys(Ogre::SceneManager* mgr, Simulator* sim);
	virtual ~AsteroidSys(void);
	void update(void);
	void addToScene(void);
	void addToSimulator(btDiscreteDynamicsWorld* dynamicsWorld);
    std::vector<Asteroid*> getAsteroids();
protected:
	std::vector<Asteroid*> asteroidSystem;
};

//---------------------------------------------------------------------------

#endif // #ifndef __AsteroidSys_h_

//---------------------------------------------------------------------------
