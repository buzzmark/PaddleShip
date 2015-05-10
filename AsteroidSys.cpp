#include "AsteroidSys.h"

//---------------------------------------------------------------------------
AsteroidSys::AsteroidSys(Ogre::SceneManager* mgr, Simulator* sim)
{
  for (int i = 0; i < NUM_ASTEROIDS; i++) {
    std::stringstream nodeName;
    nodeName << "asteroidNode" << i;
    asteroidSystem.push_back(new Asteroid(nodeName.str(), mgr, sim));
  }

}
//---------------------------------------------------------------------------
AsteroidSys::~AsteroidSys(void)
{
    for (Asteroid* ast : asteroidSystem) {
        delete ast;
    }
}
//---------------------------------------------------------------------------
void AsteroidSys::update(void){
    for (Asteroid* ast : asteroidSystem) {
        ast->update();
    }
}
//---------------------------------------------------------------------------
void AsteroidSys::addToScene(void){
  for (int i=0; i < asteroidSystem.size(); i++) {
    asteroidSystem[i]-> addToScene(i);
  }
}
//---------------------------------------------------------------------------
void AsteroidSys::addToSimulator(btDiscreteDynamicsWorld* dynamicsWorld){
    for (Asteroid* ast : asteroidSystem) {
        ast->addToSimulator();
        ast->setDynamicsWorld(dynamicsWorld);
    }
}
//---------------------------------------------------------------------------
std::vector<Asteroid*> AsteroidSys::getAsteroids() {
    return asteroidSystem;
}
