#include "Simulator.h"
#include "GameScreen.h"
#include "Ship.h"
#include "PlayerObject.h"
#include <algorithm>

Simulator::Simulator(Ogre::SceneManager* mgr, GameScreen* gs) : gameScreen(gs)
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
    objList.insert(o);
    // use default collision group/mask values (dynamic/kinematic/static)
    dynamicsWorld->addRigidBody(o->getBody());
}

void Simulator::removeObject(GameObject* o)
{
    objList.erase(o);
    dynamicsWorld->removeRigidBody(o->getBody());
}

void Simulator::stepSimulation(const Ogre::Real elapsedTime, int maxSubSteps, const Ogre::Real fixedTimestep)
{
    dynamicsWorld->stepSimulation(elapsedTime, maxSubSteps, fixedTimestep);

    std::vector<PlayerObject*> players = gameScreen->getPlayers();
    std::vector<Asteroid*> asts = gameScreen->getAsteroids();

    for (PlayerObject* obj : players) {
        obj->getCollisionCallback()->ctxt.hit = false;
        Ship* ship = dynamic_cast<Ship*>(obj);
        if (ship != nullptr) {
            ship->getPaddle()->getCollisionCallback()->ctxt.hit = false;
        }
    }

    //collision call back
    for (PlayerObject* obj : players) {
        Ship* ship = dynamic_cast<Ship*>(obj);

        if (ship != nullptr) {
            GameObject* paddle = ship->getPaddle();

            for (Asteroid* ast : asts) {
                btRigidBody* astBody = ast->getBody();

                dynamicsWorld->contactPairTest(ship->getBody(), astBody, *(ship->getCollisionCallback()));
                dynamicsWorld->contactPairTest(paddle->getBody(), astBody, *(paddle->getCollisionCallback()));
            }

            for (PlayerObject* targetObj : players) {
                if (targetObj != obj) {
                    dynamicsWorld->contactPairTest(targetObj->getBody(), paddle->getBody(), *(targetObj->getCollisionCallback()));
                }
            }
        } else {
            for (Asteroid* ast : asts) {
                dynamicsWorld->contactPairTest(obj->getBody(), ast->getBody(), *(obj->getCollisionCallback()));
            }
        }
    }

    for (GameObject* obj : objList) {
        obj->update();
    }
    //mDebugDrawer->Update(); //uncomment to see collision shapes
}

btDiscreteDynamicsWorld* Simulator::getDynamicsWorld() {
    return dynamicsWorld;
}
