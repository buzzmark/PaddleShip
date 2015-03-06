#include "Asteroid.h"
#include <iostream>
#include <cassert>

//---------------------------------------------------------------------------
Asteroid::Asteroid(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim) : GameObject(nym, mgr, sim)
{
  float minV = -20;
  float maxV = -30;
  Ogre::Real zV = minV + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxV-minV)));
  minV = -3;
  maxV = 3;
  Ogre::Real xV = minV + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxV-minV)));
  asteroidVelocity = Ogre::Vector3(xV,0,zV);
  float minP = -40;
  float maxP = 40;
  Ogre::Real xP = minP + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxP-minP)));
  asteroidPosition = Ogre::Vector3(xP,0,maxP*2);
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
  float scale = 1.0f;

  rootNode->setScale(Ogre::Vector3(scale,scale,scale));

  if ((i%3)+1 == 1) //asteroid1.mesh
  {
    sphereSize = 5;
    massVal = scale;
    mass = massVal;
    shape = new btSphereShape(sphereSize);
  }
  else if ((i%3)+1 == 2) //asteroid2.mesh
  {
    sphereSize = 6;
    massVal = scale * 1.2;
    mass = massVal;
    shape = new btSphereShape(sphereSize);
  }
  else //asteroid3.mesh
  {
    sphereSize = 9;
    massVal = scale * 3;
    mass = massVal;
    shape = new btSphereShape(sphereSize);
  }
  
}
//---------------------------------------------------------------------------
void Asteroid::update(void){
  asteroidPosition = rootNode->getPosition();
  if (asteroidPosition.x >= 100 || asteroidPosition.y >= 100 || asteroidPosition.z >= 100 || asteroidPosition.x <= -100 || asteroidPosition.y <= -100 || asteroidPosition.z <= -100) {
    printf("OUT OF BOUNDS\n");
    float minV = -20;
    float maxV = -30;
    printf("About to do random z velocity\n");
    Ogre::Real zV = minV + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxV-minV)));
    minV = -3;
    maxV = 3;
    printf("About to do random x velocity\n");
    Ogre::Real xV = minV + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxV-minV)));
    printf("about to store velocity in asteroidVelocity\n");
    asteroidVelocity = Ogre::Vector3(xV,0,zV);
    float minP = -40;
    float maxP = 40;
    Ogre::Real xP = minP + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxP-minP)));
    asteroidPosition = Ogre::Vector3(xP,0,maxP);
    printf("root node about to reset position\n");
    rootNode->setPosition(asteroidPosition);
    printf("dynamicsWorld about to remove rigid body\n");
    dynamicsWorld->removeRigidBody(body);
    printf("dynamicsWorld removed rigid body\n");
    delete body->getMotionState();
    printf("deleted motionState\n");
    delete body;
    printf("deleted body\n");
    tr.setOrigin(btVector3(asteroidPosition.x, asteroidPosition.y, asteroidPosition.z));
    printf("set origin\n");
    Ogre::Quaternion qt = rootNode->getOrientation();
    printf("set orientation\n");
    tr.setRotation(btQuaternion(qt.x, qt.y, qt.z, qt.w));
    printf("set rotation\n");
    motionState = new OgreMotionState(tr,rootNode);
    printf("created new motionState\n");
    //shape = new btSphereShape(sphereSize);
    printf("reset shape\n");
    printf("mass is %f before reset\n", massVal);  
    //mass = massVal;
    printf("mass is %f\n", massVal);
    printf("reset mass\n");
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, inertia);
    printf("constructed rigid body\n");
    rbInfo.m_restitution = restitution;
    rbInfo.m_friction = friction;
    printf("reassigned restitution and friction\n");
    body = new btRigidBody(rbInfo);
    printf("created new rigid body\n");
    printf("Velocity values are now: %f, %f, %f\n", asteroidVelocity.x, asteroidVelocity.y, asteroidVelocity.z);
    body->setLinearVelocity( btVector3(asteroidVelocity.x, asteroidVelocity.y, asteroidVelocity.z) );
    dynamicsWorld->addRigidBody(body);
    printf("set new linear velocity\n");
  } 
  printf("NOT OUT OF BOUNDS\n");
  //GameObject::updateTransform();
  GameObject::update();
}
//---------------------------------------------------------------------------
void Asteroid::addToSimulator(void){
  GameObject::addToSimulator();
  body->setLinearVelocity( btVector3(asteroidVelocity.x, asteroidVelocity.y, asteroidVelocity.z) );
}
//---------------------------------------------------------------------------
void Asteroid::setDynamicsWorld( btDiscreteDynamicsWorld* world) {
  dynamicsWorld = world;
}

//---------------------------------------------------------------------------