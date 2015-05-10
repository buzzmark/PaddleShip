#include "ShipAI.h"
#include "GameScreen.h"

//---------------------------------------------------------------------------
ShipAI::ShipAI(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim, GameScreen* gs, Ogre::SceneNode* sn, int &sc, SoundPlayer* sPlayer, int ops) : Ship(nym, mgr, sim, gs, NULL, sc, sPlayer, NULL)
{
	sceneNode = sn;
	numOpponents = ops;
	target = NULL;
	gameStarted = true;
	//paddle = paddleAI;
	paddle = NULL;
	yT = 0;
	motorRight = true;
	paddleHinge = NULL;

	paces = 1000; //tentative
	doneRoaming = false;
	mustFlee = false;
	doneFleeing = false;

	roamState = true;
	shootState = false;
	huntState = false;
	meleeState = false;
	fleeState = false;

	hasDecr = false;
	health = 100;
	left = false;
	right = false;
	forward = false;
	back = false;

	rootNode->setPosition(Ogre::Vector3(0, 0, 600));
}

//---------------------------------------------------------------------------
ShipAI::~ShipAI(void)
{
}
//---------------------------------------------------------------------------
void ShipAI::addToScene(void)
{
	geom = sceneMgr->createEntity(name + "Ent", "rocket.mesh");
	geom->setCastShadows(true);
	rootNode->attachObject(geom);

	mass = 10.0f;
	shape = new btCapsuleShapeZ(3.0f, 15.0f);

	Ogre::ParticleSystem* pSys = sceneMgr->createParticleSystem(name + "PS", "ship_particles");
    Ogre::SceneNode* pNode = (Ogre::SceneNode*)rootNode->createChild(Ogre::Vector3(0,-1,-8));
    pNode->attachObject(pSys);

    paddle->addToScene();
}
//---------------------------------------------------------------------------
void ShipAI::update(void)
{
	//processing self and surroundings
	printf("Starting update\n");
	survivalCheck();
	incomingAst();
	opponentProximityCheck();

	//taking action based on state(s)
	if (roamState) {
		roam();
	}
	if (shootState) {
		meleeState = false;
		shoot();
	}
	if (huntState) {
		roamState = false;
		hunt();
	}
	if (meleeState) {
		shootState = false;
		melee();
	}
	if (fleeState) {
		flee();
	}

	//health changes
	if(!context->hit) {
		hasDecr = false;
	}
	if (!hasDecr && context->hit){
		//lose health
		if (health > 0) {
			health-=10;
			//printf("AI injured\n");
		}
		if (health < 30) {
			mustFlee = true;
		}
		/*
		std::stringstream scoreVal;
 		scoreVal << "" << score;
 		if (mDetailsPanel==NULL) {
 	 		printf("mDetailsPanel is null ptr\n");
 	 	}
 	 	*/
		soundPlayer->playShipHit();
		hasDecr = true;
	}
	
}

//---------------------------------------------------------------------------
/* Default state of AI */
void ShipAI::roam(void) 
{
  printf("Roam function \n");
  if (sqrt(getPos().x*getPos().x+getPos().z*getPos().z) > 4000){
		body->applyCentralForce(btVector3(-5*getPos().x,0,-5*getPos().z));
  }

  if (paces >= 1000) {
  	printf("Roam function randomize torque\n");
  	//melee();
  	paces = 0;

	float minT = -2000;
	float maxT = 2000;
	yT = minT + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maxT-minT)));

	if (!doneFleeing) {
		doneRoaming = true;
	}
	doneFleeing = false;

  } else if (paces<=500){
  	printf("Roam function turn\n");
  	printf("paces: %d", paces);
  	body->setAngularFactor(btVector3(0,1,0));
	body->applyTorque(btVector3(0, yT,0));
	body->setAngularFactor(btVector3(0,0,0));

	paces++;
  } else {
  	printf("Roam function move forward\n");
  	body->setAngularVelocity(btVector3(0,((body->getAngularVelocity()).getY())*0.95,0));

  	direction = rootNode->getOrientation() * Ogre::Vector3(0,0,1);
	body->applyCentralForce(btVector3(6000*direction.x, 6000*direction.y, 6000*direction.z));

	btVector3 shipVel = body->getLinearVelocity();
	body->setLinearVelocity(btVector3(shipVel.getX()*0.99,shipVel.getY()*0.99,shipVel.getZ()*0.99));

  	paces++;
  }

}
//---------------------------------------------------------------------------
void ShipAI::shoot(void)
{
	//Enter code for how to shoot laser beam here
}
//---------------------------------------------------------------------------
void ShipAI::hunt(void) 
{
	paces = 0;
	body->setAngularVelocity(btVector3(0,((body->getAngularVelocity()).getY())*0.95,0));
	
	
	direction = rootNode->getOrientation() * Ogre::Vector3(0,0,1);
	printf("direction: (%f, %f, %f)\n", direction.x, direction.y, direction.z);
	printf("AI position: (%f, %f, %f)\n", getPos().x, getPos().y, getPos().z);
	printf("target position: (%f, %f, %f)\n", target->getPos().x, target->getPos().y, target->getPos().z);
	Ogre::Vector3 huntPath = (target->getPos() - getPos()).normalisedCopy();
	printf("huntPath: (%f, %f, %f)\n", huntPath.x, huntPath.y, huntPath.z);
	int count = 0;

	Ogre::Quaternion amountRotation = direction.getRotationTo(huntPath);
	printf("amountRotation: (%f,%f,%f)\n", amountRotation.x, amountRotation.y, amountRotation.z);
	int turn;
	if (amountRotation.y < 0) {
		printf("turn left\n");
		turn = -1;
	} else {
		printf("turn right\n");
		turn = 1;
	}

	if (((int)(10*direction.x)) != ((int)(10*huntPath.x)) || ((int)(10*direction.z)) != ((int)(10*huntPath.z))) {
		count++;
		printf("direction1: (%f, %f, %f)\n", direction.x, direction.y, direction.z);
		printf("huntPath1: (%f, %f, %f)\n", huntPath.x, huntPath.y, huntPath.z);
		body->setAngularFactor(btVector3(0,1,0));
		body->applyTorque(btVector3(0,turn*10000,0));
		body->setAngularFactor(btVector3(0,0,0));
		direction = rootNode->getOrientation() * Ogre::Vector3(0,0,1);
		printf("direction2: (%d, %d, %d)\n", (int)(10*direction.x), (int)(10*direction.y), (int)(10*direction.z));
		printf("huntPath2: (%d, %d, %d)\n", (int)(10*huntPath.x), (int)(10*huntPath.y), (int)(10*huntPath.z));
		//printf("num times through loop: %d\n", count);
	}
	btVector3 shipVel = body->getLinearVelocity();
	body->setLinearVelocity(btVector3(shipVel.getX()*0.99,shipVel.getY()*0.99,shipVel.getZ()*0.99));
	meleeState = false;
	if (((int)(10*direction.x)) == ((int)(10*huntPath.x)) && ((int)(10*direction.z)) == ((int)(10*huntPath.z))) {
		printf("direction3: (%d, %d, %d)\n", (int)(10*direction.x), (int)(10*direction.y), (int)(10*direction.z));
		printf("huntPath3: (%d, %d, %d)\n", (int)(10*huntPath.x), (int)(10*huntPath.y), (int)(10*huntPath.z));
		//body->setAngularFactor(btVector3(0,0,0));
		body->setAngularVelocity(btVector3(0,((body->getAngularVelocity()).getY())*0.95,0));
		body->applyCentralForce(btVector3(6000*direction.x, 6000*direction.y, 6000*direction.z));

		btVector3 shipVel = body->getLinearVelocity();
		body->setLinearVelocity(btVector3(shipVel.getX()*0.99,shipVel.getY()*0.99,shipVel.getZ()*0.99));

		float dtProd = direction.dotProduct(target->getPos() - getPos());
		bool inFront = dtProd > 0;
		if (inFront && getPos().squaredDistance(target->getPos()) < 2500) {
			meleeState = true;
		} else {
			meleeState = false;
		}
	}
}
//---------------------------------------------------------------------------
void ShipAI::melee(void)
{
	paddleHinge = paddle -> getPaddleHinge();
	if (motorRight)
			paddleHinge->enableAngularMotor(true, -100, 1000);
	else
		paddleHinge->enableAngularMotor(true, 100, 1000);
	motorRight = !motorRight;
	soundPlayer->playPaddleSwing();

}
//---------------------------------------------------------------------------
void ShipAI::flee(void)
{
	//Code to Flee
	//if enemy is near, begin fleeing and continue fleeing until survival check deems otherwise

	//doneFleeing = true;
}
//---------------------------------------------------------------------------
/*checks self-health*/
void ShipAI::survivalCheck(void)
{
	if (health < 30 && paces < 100 && (doneRoaming || mustFlee)) {
		if (mustFlee && paces > 0) {
			printf("Health is low\n");
			paces = 0;
		}
		fleeState = true;
		roamState = false;
		huntState = false;
		meleeState = false;
		shootState = false;
		paces++;
	} else {
		mustFlee = false;
		doneRoaming = false;
		fleeState = false;
		roamState = true;
	}
}
//---------------------------------------------------------------------------
/*checks if an asteroid might hit self and initiates melee move if so */
void ShipAI::incomingAst(void)
{
	direction = rootNode->getOrientation() * Ogre::Vector3(0,0,1);
	std::vector<Asteroid*>astList = gameScreen->getAsteroids();

	for (Asteroid* ast : astList) {
		float dtProd = direction.dotProduct(ast->getPos() - getPos());
		bool inFront = dtProd > 0;
		if (inFront && getPos().squaredDistance(ast->getPos()) < 2500) {
			melee();
		}
	}
	
}
//---------------------------------------------------------------------------
/*checks proximity of opponents and chooses target based on health*/
void ShipAI::opponentProximityCheck(void)
{
	
	direction = rootNode->getOrientation() * Ogre::Vector3(0,0,1);
	std::vector<PlayerObject*>playerList = gameScreen->getPlayers();
	int nearbyOps = 0;
	float dist;
	for (PlayerObject* player : playerList) {
		dist= getPos().squaredDistance(player->getPos());
		if (player != this && getPos().squaredDistance(player->getPos()) <= 10000) {
			if (target == NULL || player->getHealth() <= 20 || player->getHealth() > target->getHealth()) {
				target = player;
				huntState = true;
				roamState = false;
			}
			nearbyOps++;
			printf("Incremented nearbyOps\n");
			//melee();
		}
	}
	
	
	if (nearbyOps == 0 && health >= 30) {
		target = NULL;
		//paces = 1000;
		// btVector3 shipVel = body->getLinearVelocity();
		// body->setLinearVelocity(btVector3(shipVel.getX()*0.99,shipVel.getY()*0.99,shipVel.getZ()*0.99));
		huntState = false;
		roamState = true;
	}
	

}
//---------------------------------------------------------------------------
