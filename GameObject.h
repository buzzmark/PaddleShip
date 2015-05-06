#ifndef __GameObject_h_
#define __GameObject_h_

#include <btBulletDynamicsCommon.h>
#include "OgreMotionState.h"
#include <Ogre.h>
#include "BulletContactCallback.h"

class Simulator;

class GameObject {
public:
    GameObject(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim);
    virtual ~GameObject(void);
    void updateTransform();
    virtual void addToSimulator();
    btRigidBody* getBody();
    virtual void update();
    BulletContactCallback* getCollisionCallback();
    virtual Ogre::Vector3 getPos();
    Ogre::SceneNode* getNode();
    OgreMotionState* getMotionState();
    btTransform* getTransform();
    btScalar getMass();
    btCollisionShape * getShape() ;
    btVector3 getInertia();
    btScalar getFriction();
    btScalar getRestitution();
    void setMotionState(OgreMotionState* newState);
    void setBody(btRigidBody* newBody);  
    void setPosition(float x, float y, float z);
    std::string getName();

protected:
    Ogre::String name;
    Ogre::SceneManager* sceneMgr;
    Ogre::SceneNode* rootNode;
    Ogre::Entity* geom;
    OgreMotionState* motionState;

    Simulator* simulator;
    btCollisionShape* shape;
    btRigidBody* body;
    btTransform tr;
    btVector3 inertia;

    btScalar mass;
    btScalar restitution;
    btScalar friction;
    bool kinematic;
    bool needsUpdates;

    CollisionContext* context;
    BulletContactCallback* cCallBack;

};

//---------------------------------------------------------------------------

#endif // #ifndef __GameScreen_h_

//---------------------------------------------------------------------------
