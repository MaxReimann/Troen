#include "levelcontroller.h"
// OSG
#include <osg/Group>
// bullet
#include <btBulletDynamicsCommon.h>
// troen
#include "../constants.h"
#include "../model/levelmodel.h"
#include "../view/levelview.h"
#include "../model/physicsworld.h"

#include "itemcontroller.h"
#include "../troengame.h"

using namespace troen;

LevelController::LevelController(TroenGame* troenGame, std::string levelName) : m_levelName(levelName),
m_targetItemCount(100)
{
	AbstractController();
	troenGame->registerSharedListener(this);
	m_model = m_levelModel = std::make_shared<LevelModel>(this, levelName);
	m_view = m_levelView = std::make_shared<LevelView>(m_levelModel, levelName);

	m_troenGame = troenGame;
	m_currentItemCount = 0;

	initializeSpawnPoints();
}

void LevelController::reload()
{
	removeRigidBodiesFromWorld();
	m_levelModel->reload(m_levelName);
	addRigidBodiesToWorld();

	m_levelView->reload(m_levelName);
}

btTransform LevelController::getSpawnPointForBikeWithIndex(int index)
{
	return m_initialBikePositionTransforms[index];
}

btTransform LevelController::getRandomSpawnPoint()
{
#define randf(min, max) \
	(static_cast<float>(rand()) / RAND_MAX * ((max)-(min)) + (min))

	int index = randf(0, m_initialBikePositionTransforms.size() - 1);
	return m_initialBikePositionTransforms[index];
}

void LevelController::initializeSpawnPoints()
{
	btVector3 Z_AXIS(0, 0, 1);
	m_initialBikePositionTransforms.push_back(btTransform(btQuaternion(Z_AXIS, (float)PI * 3.f / 4.f), btVector3(20, 20, BIKE_DIMENSIONS.z() / 2 + 50)));
	m_initialBikePositionTransforms.push_back(btTransform(btQuaternion(Z_AXIS, (float)PI * 1.f / 4.f), btVector3(20, -20, BIKE_DIMENSIONS.z() / 2 + 50)));
	m_initialBikePositionTransforms.push_back(btTransform(btQuaternion(Z_AXIS, (float)-PI * 1.f / 4.f), btVector3(-20, -20, BIKE_DIMENSIONS.z() / 2 + 50)));
	m_initialBikePositionTransforms.push_back(btTransform(btQuaternion(Z_AXIS, (float)-PI * 3.f / 4.f), btVector3(-20, 20, BIKE_DIMENSIONS.z() / 2 + 50)));
	m_initialBikePositionTransforms.push_back(btTransform(btQuaternion(Z_AXIS,0), btVector3(100, 100, BIKE_DIMENSIONS.z() / 2 + 50)));
	m_initialBikePositionTransforms.push_back(btTransform(btQuaternion(Z_AXIS,0), btVector3(-100, -100, BIKE_DIMENSIONS.z() / 2 + 50)));
}

osg::ref_ptr<osg::Group>  LevelController::getFloorView()
{
	return m_levelView->getFloor();
}

void LevelController::attachWorld(std::shared_ptr<PhysicsWorld> &world)
{
	m_world = world;
}

void LevelController::removeRigidBodiesFromWorld()
{
	m_world.lock()->removeRigidBodies(getRigidBodies());
}

void LevelController::addRigidBodiesToWorld()
{
	m_world.lock()->addRigidBodies(getRigidBodies(), COLGROUP_LEVEL, COLMASK_LEVEL);
}

void LevelController::addItemBox()
{
#define randf(min, max) \
		(static_cast<float>(rand()) / RAND_MAX * ((max)-(min)) + (min))

	float x = randf(0, LEVEL_SIZE) - LEVEL_SIZE / 2;
	float y = randf(0, LEVEL_SIZE) - LEVEL_SIZE / 2;
	m_positionCached = btVector3(x, y, +0.5);


	m_itemTypeCached = (ItemController::Type) (int) floor(randf(0, ItemController::Type::COUNT));

	// the item controller will remove itself
	new ItemController(m_positionCached, m_world, m_troenGame, m_itemTypeCached, m_levelView.get());


	m_currentItemCount++;
}

void LevelController::update()
{
	if (m_currentItemCount >= m_targetItemCount) {
		return;
		m_targetItemCount = 0;
	}

	// this method is called in each frame, so the amount of items will be refreshed relatively quickly
	// creating all at once would cause a lag

	addItemBox();
}

void LevelController::setBendingFactor(float bendingFactor)
{
	m_levelView->setBendingFactor(bendingFactor);
}


void LevelController::setBendingActive(bool active)
{
	m_levelView->setBendingActive(active);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// only run on master
// for each part of each geo of each object of each asset:
// send all the verts, faces, normals, colours, etc
void LevelController::commitSharedData(omega::SharedOStream& out)
{
	// std::cout << "comitting shared data" << std::endl;
	out <<  m_newitemBox ;
	if (m_newitemBox)
		out << m_positionCached << m_itemTypeCached;

	// clear per frame states
	m_newitemBox = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// only run on slaves!
void LevelController::updateSharedData(omega::SharedIStream& in)
{
	// std::cout << "updating shared data" << std::endl;
	in >> m_newitemBox;
	if (m_newitemBox){
	 	in >> m_positionCached >> m_itemTypeCached;
		new ItemController(m_positionCached, m_world, m_troenGame, m_itemTypeCached, m_levelView.get());
	}

}