#include "Paddle.h"
#include <iostream>
//---------------------------------------------------------------------------
Paddle::Paddle(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim, Ogre::SceneNode* shipNode, int &sc, SoundPlayer* sPlayer) : GameObject(nym, mgr, sim), score(sc)
{
	name = nym;
	Ogre::Vector3 shipPos = shipNode -> getPosition();
	rootNode->setPosition(Ogre::Vector3(shipPos.x - 8.25f, shipPos.y + 0.0f, shipPos.z + 10.0f));
	soundPlayer = sPlayer;
}
//---------------------------------------------------------------------------
Paddle::~Paddle(void)
{
}
//---------------------------------------------------------------------------
void Paddle::addToScene(void)
{
	geom = sceneMgr->createEntity(name + "Ent", "paddle.mesh");
	geom->setCastShadows(true);
	rootNode->attachObject(geom);
	

	mass = 1.0f;
	shape = new btBoxShape(btVector3(5,4,1));




}
//---------------------------------------------------------------------------
void Paddle::addToSimulator(void)
{
	GameObject::addToSimulator();

	body->setLinearFactor(btVector3(1,0,1));
}
//---------------------------------------------------------------------------
void Paddle::update(void)
{
	if(!context->hit) {
		hasIncr = false;
	}
	if (!hasIncr && context->hit){
		//increment score
		score+=1;
		std::stringstream numScore;
 	 	numScore << "" << score;
 	 	if (mDetailsPanel==NULL) {
 	 		printf("mDetailsPanel is null ptr\n");
 	 	}
 	 	mDetailsPanel->setParamValue(0, numScore.str());
		soundPlayer->playScore();
		hasIncr = true;
	}
}
//---------------------------------------------------------------------------
void Paddle::injectKeyDown(const OIS::KeyEvent &arg)
{

}
//---------------------------------------------------------------------------
void Paddle::injectKeyUp(const OIS::KeyEvent &arg)
{

}
//---------------------------------------------------------------------------
void Paddle::setDeetsPan(OgreBites::ParamsPanel*mDeetsPan)
{
	mDetailsPanel = mDeetsPan;
}
//---------------------------------------------------------------------------
void Paddle::setPaddleHinge(btHingeConstraint* paddleHinge)
{
	hinge = paddleHinge;
}
//---------------------------------------------------------------------------
btHingeConstraint* Paddle::getPaddleHinge()
{
	return hinge;
}
//---------------------------------------------------------------------------