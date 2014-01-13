#include "levelview.h"
// STD
#include <math.h>
// OSG
#include <osg/ImageStream>
#include <osg/Material>
#include <osg/MatrixTransform>
#include <osg/Texture2D>
#include <osg/TexMat>
#include <osg/Group>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/TexGen>
#include <osg/TexGenNode>
// bullet
#include <btBulletDynamicsCommon.h>
// troen
#include "../model/levelmodel.h"
#include "shaders.h"
#include "../constants.h"

using namespace troen;


LevelView::LevelView(std::shared_ptr<LevelModel> model)
{
	AbstractView();
	m_model = model;

	int levelSize = m_model->getLevelSize();

	m_node = new osg::Group();

	m_node->addChild(constructObstacles(levelSize));
	m_node->addChild(constructWalls(levelSize));
	m_node->addChild(constructFloors(levelSize));
}

osg::ref_ptr<osg::Group> LevelView::constructWalls(int levelSize)
{
	osg::ref_ptr<osg::Group> wallsGroup = new osg::Group();

    osg::ref_ptr<osg::Group> walls = constructGroupForBoxes(m_model->getWalls());
	addShaderAndUniforms(walls, shaders::OUTER_WALL, levelSize);
	walls->setNodeMask(CAMERA_MASK_MAIN);
	wallsGroup->addChild(walls);

	osg::ref_ptr<osg::Group> radarWalls = constructRadarElementsForBoxes(m_model->getWalls());
	radarWalls->setNodeMask(CAMERA_MASK_RADAR);
	wallsGroup->addChild(radarWalls);

    return wallsGroup;
}

osg::ref_ptr<osg::Group> LevelView::constructFloors(int levelSize)
{
	osg::ref_ptr<osg::Group> floorsGroup = new osg::Group();

	osg::ref_ptr<osg::Group> floors = constructGroupForBoxes(m_model->getFloors());
    addShaderAndUniforms(floors, shaders::GRID, levelSize);
	floors->setNodeMask(CAMERA_MASK_MAIN);
	floorsGroup->addChild(floors);

	//osg::ref_ptr<osg::Group> radarFloors = constructRadarElementsForBoxes(m_model->getFloors());
	//radarFloors->setNodeMask(CAMERA_MASK_RADAR);
	//floorsGroup->addChild(radarFloors);

    return floorsGroup;
}

osg::ref_ptr<osg::Group> LevelView::constructObstacles(int levelSize)
{
	osg::ref_ptr<osg::Group> obstaclesGroup = new osg::Group();

	osg::ref_ptr<osg::Group> obstacles = constructGroupForBoxes(m_model->getObstacles()); 
	osg::StateSet *obstaclesStateSet = obstacles->getOrCreateStateSet();
	obstaclesStateSet->ref();
	osg::Uniform* textureMapU = new osg::Uniform("diffuseTexture", 0);
	obstaclesStateSet->addUniform(textureMapU);
	setTexture(obstaclesStateSet, "data/textures/troen_box_tex.tga", 0);
	addShaderAndUniforms(obstacles, shaders::DEFAULT, levelSize);
	obstacles->setNodeMask(CAMERA_MASK_MAIN);
	obstaclesGroup->addChild(obstacles);

	osg::ref_ptr<osg::Group> radarObstacles = constructRadarElementsForBoxes(m_model->getObstacles());
	radarObstacles->setNodeMask(CAMERA_MASK_RADAR);
	obstaclesGroup->addChild(radarObstacles);

	return obstaclesGroup;
}

void LevelView::addShaderAndUniforms(osg::ref_ptr<osg::Group>& group, int shaderIndex, int levelSize)
{
	osg::StateSet *stateSet = group->getOrCreateStateSet();
	stateSet->ref();

	stateSet->setAttributeAndModes(shaders::m_allShaderPrograms[shaderIndex], osg::StateAttribute::ON);
	stateSet->addUniform(new osg::Uniform("levelSize", levelSize));
	stateSet->addUniform(new osg::Uniform("modelID", DEFAULT));
}

osg::ref_ptr<osg::Group> LevelView::constructGroupForBoxes(std::vector<BoxModel> &boxes)
{
	osg::ref_ptr<osg::Group> boxGroup = new osg::Group();

	for (int i = 0; i < boxes.size(); ++i)
	{
		btVector3 dimensions = boxes[i].dimensions;
		btVector3 position = boxes[i].center;
		btQuaternion rotation = boxes[i].rotation;

		// create obstacle
		osg::ref_ptr<osg::Box> box
				= new osg::Box(osg::Vec3(0,0,0), dimensions.x(), dimensions.y(), dimensions.z());
		osg::ref_ptr<osg::ShapeDrawable> boxDrawable
				= new osg::ShapeDrawable(box);
		osg::ref_ptr<osg::Geode> boxGeode
				= new osg::Geode();
		boxGeode->addDrawable(boxDrawable);

		// place objects in world space
		osg::Matrixd initialTransform;
		initialTransform.makeRotate(btToOSGQuat(rotation));
		initialTransform *= initialTransform.translate(btToOSGVec3(position));

		osg::ref_ptr<osg::MatrixTransform> matrixTransform = new osg::MatrixTransform(initialTransform);
		matrixTransform->addChild(boxGeode);

		boxGroup->addChild(matrixTransform);
	}

	return boxGroup;
}

osg::ref_ptr<osg::Group> LevelView::constructRadarElementsForBoxes(std::vector<BoxModel> &boxes)
{
	osg::ref_ptr<osg::Group> radarBoxGroup = new osg::Group();

	for (int i = 0; i < boxes.size(); ++i)
	{
		btVector3 dimensions = boxes[i].dimensions;
		btVector3 position = boxes[i].center;
		btQuaternion rotation = boxes[i].rotation;

		int threshold = 50;
		if (dimensions.x() < threshold) dimensions.setX(dimensions.x() + threshold);
		if (dimensions.y() < threshold) dimensions.setY(dimensions.y() + threshold);
		if (dimensions.y() < threshold) dimensions.setZ(dimensions.z() + threshold);

		osg::ref_ptr<osg::Box> box
			= new osg::Box(osg::Vec3(0, 0, 0), dimensions.x(), dimensions.y(), dimensions.z());
		osg::ref_ptr<osg::ShapeDrawable> mark_shape = new osg::ShapeDrawable(box);
		osg::ref_ptr<osg::Geode> mark_node = new osg::Geode;
		mark_node->addDrawable(mark_shape.get());
		mark_node->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

		// radar material
		float alpha = 0.5f;
		osg::Vec4 color(1, 1, 1, alpha);
		osg::ref_ptr<osg::Material> material = new osg::Material;
		material->setColorMode(osg::Material::AMBIENT);
		material->setAmbient(osg::Material::FRONT_AND_BACK,
			osg::Vec4(0.8f, 0.8f, 0.8f, alpha));
		material->setDiffuse(osg::Material::FRONT_AND_BACK,
			color * 0.8f);
		material->setSpecular(osg::Material::FRONT_AND_BACK, color);
		material->setShininess(osg::Material::FRONT_AND_BACK, 1.0f);

		// place objects in world space
		osg::Matrixd initialTransform;
		initialTransform.makeRotate(btToOSGQuat(rotation));
		initialTransform *= initialTransform.translate(btToOSGVec3(position));

		osg::ref_ptr<osg::MatrixTransform> matrixTransformRadar = new osg::MatrixTransform(initialTransform);
		matrixTransformRadar->addChild(mark_node);
		matrixTransformRadar->getOrCreateStateSet()->setAttributeAndModes(
			material.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

		radarBoxGroup->addChild(matrixTransformRadar);
	}

	return radarBoxGroup;
}

void LevelView::setTexture(osg::ref_ptr<osg::StateSet> stateset, std::string filePath, int unit)
{

	osg::Image* image = osgDB::readImageFile(filePath);
	if (!image)
		std::cout << "[TroenGame::levelView]  File \"" << filePath << "\" not found." << std::endl;
	else
	{
		osg::Texture2D* texture = new osg::Texture2D;
		texture->setImage(image);
		texture->setResizeNonPowerOfTwoHint(false);
		stateset->setTextureAttributeAndModes(unit, texture, osg::StateAttribute::ON);

	}
}


