#include "ShipAI.h"

//---------------------------------------------------------------------------
ShipAI::ShipAI(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim, Ogre::SceneNode* sn, int &sc, SoundPlayer* sPlayer, std::deque<GameObject*>* oList,  int ops) : Ship(nym, mgr, sim, NULL, sc, sPlayer, NULL)
{
	sceneNode = sn;
	numOpponents = ops;
	objList = oList;
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
	geom = sceneMgr->createEntity("shipAIEnt", "rocket.mesh");
	geom->setCastShadows(true);
	rootNode->attachObject(geom);

	mass = 10.0f;
	shape = new btCapsuleShapeZ(3.0f, 15.0f);

    paddle->addToScene();
}
//---------------------------------------------------------------------------
void ShipAI::update(void)
{
	//processing self and surroundings
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
		if (score > 0) {
			score-=10;
		}
		if (score < 30) {
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

  if (sqrt(getPos().x*getPos().x+getPos().z*getPos().z) > 4000){
		body->applyCentralForce(btVector3(-5*getPos().x,0,-5*getPos().z));
  }

  if (paces >= 1000) {
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
  	body->setAngularFactor(btVector3(0,1,0));
	body->applyTorque(btVector3(0, yT,0));
	body->setAngularFactor(btVector3(0,0,0));

	paces++;
  } else {
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

	doneFleeing = true;
}
//---------------------------------------------------------------------------
/*checks self-health*/
void ShipAI::survivalCheck(void)
{
	if (health < 30 && paces < 100 && (doneRoaming || mustFlee)) {
		if (mustFlee && paces > 0) {
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
/*checks if an asteroid might hit self */
void ShipAI::incomingAst(void)
{
	/*
	std::deque<GameObject*> oList = *objList;
	for (int i = 3; i < oList.size(); i++) {

	}
	*/
}
//---------------------------------------------------------------------------
/*checks proximity of opponents and chooses target based on health*/
void ShipAI::opponentProximityCheck(void)
{
	/*
	std::deque<GameObject*> oList = *objList;
	for (int i = 3; i < oList.size(); i++) {
		(getPos()).squaredDistance(oList(i) -> getPos());

	}
	*/

}
//---------------------------------------------------------------------------
