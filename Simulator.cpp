#include "Simulator.h"
#include <algorithm>

Simulator::Simulator(Ogre::SceneManager* mgr)
{
    ///collision configuration contains default setup for memory, collision setup.
    collisionConfiguration = new btDefaultCollisionConfiguration();
    ///use the default collision dispatcher. For parallel processing you can use a different dispatcher
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    ///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
    overlappingPairCache = new btDbvtBroadphase();
    ///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
    solver = new btSequentialImpulseConstraintSolver();
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,overlappingPairCache,solver,collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0, 0, 0)); //we're in space
    //keep track of the shapes, we release memory at exit.
    //make sure to re-use collision shapes among rigid bodies whenever possible!
    btAlignedObjectArray<btCollisionShape*> collisionShapes;

    sceneMgr = mgr;
    mDebugDrawer = new CDebugDraw(mgr, dynamicsWorld);
    dynamicsWorld->setDebugDrawer(mDebugDrawer);
}

Simulator::~Simulator()
{
}

void Simulator::addObject (GameObject* o)
{
    objList.push_back(o);
    // use default collision group/mask values (dynamic/kinematic/static)
    dynamicsWorld->addRigidBody(o->getBody());
}

void Simulator::removeObject(GameObject* o)
{
    objList.erase(std::remove(objList.begin(), objList.end(), o), objList.end());
    dynamicsWorld->removeRigidBody(o->getBody());
}

void Simulator::stepSimulation(const Ogre::Real elapsedTime, int maxSubSteps, const Ogre::Real fixedTimestep)
{
    dynamicsWorld->stepSimulation(elapsedTime, maxSubSteps, fixedTimestep);
    
    //collision call back
    objList[0]->getCollisionCallback()->ctxt.hit = false;
    objList[1]->getCollisionCallback()->ctxt.hit = false;
    objList[2]->getCollisionCallback()->ctxt.hit = false;
    for (int i = 3; i < objList.size(); i++) {
        dynamicsWorld->contactPairTest(objList[0]->getBody(), objList[i]->getBody(), *(objList[0]->getCollisionCallback()));
        dynamicsWorld->contactPairTest(objList[1]->getBody(), objList[i]->getBody(), *(objList[1]->getCollisionCallback()));
    }
    dynamicsWorld->contactPairTest(objList[2]->getBody(), objList[1]->getBody(), *(objList[2]->getCollisionCallback()));
    for (unsigned int i = 0; i < objList.size(); i++)
        objList[i]->update();
    //mDebugDrawer->Update(); //uncomment to see collision shapes
}

btDiscreteDynamicsWorld* Simulator::getDynamicsWorld() {
    return dynamicsWorld;
}

std::deque<GameObject*>* Simulator::getObjList () {
    return &objList;
}
