#ifndef __Simulator_h_
#define __Simulator_h_

#include <Ogre.h>
#include "BulletContactCallback.h"
#include "GameObject.h"
#include "DebugDraw.hpp"

class Simulator {
protected:
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btBroadphaseInterface* overlappingPairCache;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld; //inherits from CollisionWorld
	btConstraintSolver* mConstraintsolver;
	Ogre::SceneManager* sceneMgr;
	std::deque<GameObject*> objList;

	CDebugDraw* mDebugDrawer;
public:
	Simulator(Ogre::SceneManager* mgr);
	~Simulator();

	void addObject(GameObject* o);
	bool removeObject(GameObject* o);
	void stepSimulation(const Ogre::Real elapsedTime, int maxSubSteps = 1, const Ogre::Real fixedTimestep = 1.0f/60.0f);
	btDiscreteDynamicsWorld* getDynamicsWorld();
};

//---------------------------------------------------------------------------

#endif // #ifndef __Ship_h_

//---------------------------------------------------------------------------