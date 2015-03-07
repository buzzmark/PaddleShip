#include "Paddle.h"
#include <iostream>
//---------------------------------------------------------------------------
Paddle::Paddle(Ogre::String nym, Ogre::SceneManager* mgr, Simulator* sim, int &sc) : GameObject(nym, mgr, sim), score(sc)
{
	//rootNode->getParent()->removeChild(rootNode);
	//shipNode->addChild(rootNode);
	rootNode->setPosition(Ogre::Vector3(-8.25f, 0.0f, 10.0f));

}
//---------------------------------------------------------------------------
Paddle::~Paddle(void)
{
}
//---------------------------------------------------------------------------
void Paddle::addToScene(void)
{
	geom = sceneMgr->createEntity("paddleEnt", "paddle.mesh");
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
	if (context->hit){
		//increment score
		score+=2;
		std::stringstream numScore;
 	 	numScore << "" << score;
 	 	if (mDetailsPanel==NULL) {
 	 		printf("mDetailsPanel is null ptr\n");
 	 	}
 	 	mDetailsPanel->setParamValue(0, numScore.str());
		//play sound?
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