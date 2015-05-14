#include "Asteroid.h"
#include <iostream>
#include <cassert>

//---------------------------------------------------------------------------
Asteroid::Asteroid(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim, int i) : GameObject(nym, mgr, sim), asteroidNum(i)
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

  netVel = btVector3(xV, yV, zV);
  netState = false;
  isHeld = false;
}
//---------------------------------------------------------------------------
Asteroid::~Asteroid(void)
{
}
//---------------------------------------------------------------------------
void Asteroid::addToScene(){
  std::stringstream entName;
  entName << "asteroidEntity" << asteroidNum;
  std::stringstream meshName;
  meshName << "asteroid" << (asteroidNum%3)+1 << ".mesh";

  geom = sceneMgr->createEntity(entName.str(), meshName.str());
  geom->setCastShadows(true);

  rootNode->attachObject(geom);
  float scale = 2.0f;

  rootNode->setScale(Ogre::Vector3(scale,scale,scale));

  if ((asteroidNum%3)+1 == 1) //asteroid1.mesh
  {
    sphereSize = 2*5;
    massVal = scale;
    mass = massVal;
    shape = new btSphereShape(sphereSize);
  }
  else if ((asteroidNum%3)+1 == 2) //asteroid2.mesh
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
  }
  if (asteroidPosition.y > 40) body->applyCentralForce(btVector3(0,-5,0));
  if (asteroidPosition.y < -40) body->applyCentralForce(btVector3(0,5,0));

  netState |= (getBody()->getLinearVelocity().distance2(netVel) > 25);
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
//---------------------------------------------------------------------------
void Asteroid::writeToPacket(Packet& p) {
    btRigidBody* body = getBody();

    const btTransform& tf = body->getWorldTransform();
    const btVector3& lv = body->getLinearVelocity();
    const btVector3& av = body->getAngularVelocity();

    p << tf << lv << av;
}

void Asteroid::readFromPacket(Packet& p) {
    btRigidBody* body = getBody();

    btTransform tf;
    btVector3 lv, av;

    p >> tf >> lv >> av;

    body->setWorldTransform(tf);

    const btVector3& origin = tf.getOrigin();
    const btQuaternion& rot = tf.getRotation();
    const btVector3& rotAxis = rot.getAxis();

    getNode()->setPosition(origin.x(), origin.y(), origin.z());
    getNode()->setOrientation(Ogre::Quaternion(rot.getW(), rotAxis.x(), rotAxis.y(), rotAxis.z()));

    getMotionState()->updateTransform(tf);

    body->setLinearVelocity(lv);
    body->setAngularVelocity(av);
}

bool Asteroid::getNetState() const {
    return netState;
}

void Asteroid::resetNetState() {
    netVel = body->getLinearVelocity();
    netState = isHeld;
}

void Asteroid::setHeld(bool h) {
    isHeld = h;
    netState |= h;
}
