#include "Asteroid.h"
#include <iostream>
#include <cassert>

//---------------------------------------------------------------------------
Asteroid::Asteroid(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim) : GameObject(nym, mgr, sim)
{
  float minV = -10;
  float maxV = 10;
  Ogre::Real zV = minV + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxV-minV)));
  minV = -1;
  maxV = 1;
  Ogre::Real yV = minV + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxV-minV)));
  minV = -10;
  maxV = 10;
  Ogre::Real xV = minV + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxV-minV)));

  float maxP = 4000;
  float minP = -maxP;
  Ogre::Real xP = minP + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxP-minP)));
  Ogre::Real zP = minP + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxP-minP)));
  minP = -25;
  maxP = 25;
  Ogre::Real yP = minP + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxP-minP)));

  float minR = -35;
  float maxR = 35;
  Ogre::Real zR = minR + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxR-minR)));
  Ogre::Real yR = minR + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxR-minR)));
  Ogre::Real xR = minR + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxR-minR)));

  asteroidVelocity = Ogre::Vector3(xV,yV,zV);
  asteroidPosition = Ogre::Vector3(xP,yP,zP);
  asteroidRotation = btVector3(xR, yR, zR);
  rootNode->setPosition(asteroidPosition);
}
//---------------------------------------------------------------------------
Asteroid::~Asteroid(void)
{
}
//---------------------------------------------------------------------------
void Asteroid::addToScene(int i){
  std::stringstream entName;
  entName << "asteroidEntity" << i;
  std::stringstream meshName;
  meshName << "asteroid" << (i%3)+1 << ".mesh";

  geom = sceneMgr->createEntity(entName.str(), meshName.str());
  geom->setCastShadows(true);

  rootNode->attachObject(geom);
  float scale = 2.0f;

  rootNode->setScale(Ogre::Vector3(scale,scale,scale));

  if ((i%3)+1 == 1) //asteroid1.mesh
  {
    sphereSize = 2*5;
    massVal = scale;
    mass = massVal;
    shape = new btSphereShape(sphereSize);
  }
  else if ((i%3)+1 == 2) //asteroid2.mesh
  {
    sphereSize = 2*6;
    massVal = scale * 1.2;
    mass = massVal;
    shape = new btSphereShape(sphereSize);
  }
  else //asteroid3.mesh
  {
    sphereSize = 2*9;
    massVal = scale * 3;
    mass = massVal;
    shape = new btSphereShape(sphereSize);
  }
  
}
//---------------------------------------------------------------------------
float minabs(float a, float b){
  
}
//---------------------------------------------------------------------------
void Asteroid::update(void){
  asteroidPosition = rootNode->getPosition();
  if (sqrt(asteroidPosition.x*asteroidPosition.x+asteroidPosition.z*asteroidPosition.z) > 4000.0f || abs(asteroidPosition.y) > 100.0f) {
    body->applyCentralForce(btVector3(-0.00125*getPos().x, -0.00125*getPos().y, -0.00125*getPos().z));

    /*//printf("OUT OF BOUNDS\n");
    float minV = -10;
    float maxV = 10;
    Ogre::Real zV = minV + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxV-minV)));
    minV = -1;
    maxV = 1;
    Ogre::Real yV = minV + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxV-minV)));
    minV = -10;
    maxV = 10;
    Ogre::Real xV = minV + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxV-minV)));

    float maxP = 4000;
    float minP = -maxP;
    Ogre::Real xP = minP + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxP-minP)));
    Ogre::Real zP = minP + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxP-minP)));
    minP = -25;
    maxP = 25;
    Ogre::Real yP = minP + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxP-minP)));

    asteroidVelocity = Ogre::Vector3(xV,yV,zV);
    asteroidPosition = Ogre::Vector3(xP,yP,zP);
    rootNode->setPosition(asteroidPosition);

    //printf("dynamicsWorld about to remove rigid body\n");
    dynamicsWorld->removeRigidBody(body);
    //printf("dynamicsWorld removed rigid body\n");
    delete body->getMotionState();
    //printf("deleted motionState\n");
    delete body;
    //printf("deleted body\n");
    tr.setOrigin(btVector3(asteroidPosition.x, asteroidPosition.y, asteroidPosition.z));
    //printf("set origin\n");
    Ogre::Quaternion qt = rootNode->getOrientation();
    //printf("set orientation\n");
    tr.setRotation(btQuaternion(qt.x, qt.y, qt.z, qt.w));
    //printf("set rotation\n");
    motionState = new OgreMotionState(tr,rootNode);
    //printf("created new motionState\n");
    //shape = new btSphereShape(sphereSize);
    //printf("reset shape\n");
    //printf("mass is %f before reset\n", massVal);  
    //mass = massVal;
    //printf("mass is %f\n", massVal);
    //printf("reset mass\n");
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, inertia);
    //printf("constructed rigid body\n");
    rbInfo.m_restitution = restitution;
    rbInfo.m_friction = friction;
    //printf("reassigned restitution and friction\n");
    body = new btRigidBody(rbInfo);
    //printf("created new rigid body\n");
    //printf("Velocity values are now: %f, %f, %f\n", asteroidVelocity.x, asteroidVelocity.y, asteroidVelocity.z);
    body->setLinearVelocity( btVector3(asteroidVelocity.x, asteroidVelocity.y, asteroidVelocity.z) );
    dynamicsWorld->addRigidBody(body);
    //printf("set new linear velocity\n");
    */
  } 
  ////printf("NOT OUT OF BOUNDS\n");
  //GameObject::updateTransform();
  GameObject::update();
}
//---------------------------------------------------------------------------
void Asteroid::addToSimulator(void){
  GameObject::addToSimulator();
  body->setLinearVelocity( btVector3(asteroidVelocity.x, asteroidVelocity.y, asteroidVelocity.z) );
  body->applyTorqueImpulse(asteroidRotation*mass);
}
//---------------------------------------------------------------------------
void Asteroid::setDynamicsWorld( btDiscreteDynamicsWorld* world) {
  dynamicsWorld = world;
}

//---------------------------------------------------------------------------

btDiscreteDynamicsWorld* Asteroid::getDynamicsWorld() {
  return dynamicsWorld;
}
/*
Ogre::Vector3 Asteroid::getPos()
{
  return GameObject::getPos();
}
*/
