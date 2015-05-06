#include "GameObject.h"
#include "Simulator.h"

GameObject::GameObject(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim)
{
    name = nym;
    sceneMgr = mgr;
    simulator = sim;
    rootNode = sceneMgr->getRootSceneNode()->createChildSceneNode(name);
    shape = NULL;
    tr.setIdentity();
    mass = 0.0f;
    inertia.setZero();
    
    geom = NULL;
    motionState = NULL;
    body = NULL;
    restitution = 0.95f;
    friction = 0.5f;
    kinematic = false;
    needsUpdates = false;

    context = NULL;
    cCallBack = NULL;
}

GameObject::~GameObject()
{
}

void GameObject::updateTransform()
{
    Ogre::Vector3 pos = rootNode->getPosition();
    tr.setOrigin(btVector3(pos.x, pos.y, pos.z));
    Ogre::Quaternion qt = rootNode->getOrientation();
    tr.setRotation(btQuaternion(qt.x, qt.y, qt.z, qt.w));
    if (motionState) motionState->updateTransform(tr);
}

void GameObject::addToSimulator() 
{
    
    //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
    updateTransform();
    motionState = new OgreMotionState(tr, rootNode);
    //rigidbody is dynamic if and only if mass is non zero, otherwise static
    if (mass != 0.0f) shape->calculateLocalInertia(mass, inertia);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, inertia);
    rbInfo.m_restitution = restitution;
    rbInfo.m_friction = friction;
    body = new btRigidBody(rbInfo);
    
    context = new CollisionContext();
    cCallBack = new BulletContactCallback(*body, *context);

    simulator->addObject(this);
    
}

btRigidBody* GameObject::getBody()
{
    return body;
}

void GameObject::update()
{
}

BulletContactCallback* GameObject::getCollisionCallback()
{
    return cCallBack;
}


Ogre::Vector3 GameObject::getPos()
{
  return rootNode->getPosition();
}

Ogre::SceneNode* GameObject::getNode() {
    return rootNode;
}

OgreMotionState* GameObject::getMotionState() {
    return motionState;
}

btTransform* GameObject::getTransform() {
    return &tr;
}

btScalar GameObject::getMass() {
    return mass;
}

btCollisionShape * GameObject::getShape() {
    return shape;
}

btVector3 GameObject::getInertia() {
    return inertia;
}

btScalar GameObject::getFriction() {
    return friction;
}

btScalar GameObject::getRestitution() {
    return restitution;
}

void GameObject::setMotionState(OgreMotionState* newState) {
    motionState = newState;
}

void GameObject::setBody(btRigidBody* newBody) {
    body = newBody;
}

void GameObject::setPosition(float x, float y, float z){
    rootNode->setPosition(Ogre::Vector3(x,y,z));
}

std::string GameObject::getName() {
    return name;
}
