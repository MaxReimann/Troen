#include "playermarker.h"
// OSG
#include <osg/ShapeDrawable>
#include <osg/Geode>
#include <osg/Vec4>
#include <osg/PositionAttitudeTransform>
#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/TexGen>
#include <osg/TextureCubeMap>
#include <osg/TexMat>
#include <osg/Geode>
#include <osgDB/WriteFile>
#include <osg/CullFace>
#include <osg/TexGenNode>
#include <osgDB/ReadFile>
#include <osg/MatrixTransform>
#include <osg/BlendFunc>

#include <osgUtil/CullVisitor>
// troen
#include "../constants.h"
#include "shaders.h"


using namespace troen;

#define NEAR_TRANSPARENCY_DISTANCE 50.0f
#define START_FADING_DISTANCE 400.0f

PlayerMarker::PlayerMarker(osg::Vec3 color)
{
	AbstractView();
	m_playerColor = color;
	m_node = new osg::Group();
	osg::Matrixd initialTransform;
	initialTransform.makeScale(osg::Vec3(3.0, 3.0, 3.0));
	initialTransform *= initialTransform.translate(osg::Vec3(0.0, 0.0, 30.0));
	osg::MatrixTransform* matrixTransform = new osg::MatrixTransform(initialTransform);
	

	std::cout << "[TroenGame::playerMarker] Loading Arrow " << std::endl;
	osg::Group* arrowNode = dynamic_cast<osg::Group*>(osgDB::readNodeFile("data/models/arrow.obj"));

	if (arrowNode==NULL)
	{
		std::cout  << "data/models/arrow.obj could not be loaded" << std::endl;
		return;
	}

	matrixTransform->addChild(arrowNode);
	m_node->addChild(matrixTransform);

	osg::ref_ptr<osg::StateSet> NodeState = arrowNode->getOrCreateStateSet();

	osg::ref_ptr<osg::Geode> singleGeode = dynamic_cast<osg::Geode*>(arrowNode->asGroup()->getChild(0));
	osg::ref_ptr<osg::StateSet> childState = singleGeode->getDrawable(0)->getStateSet();
	osg::StateAttribute* stateAttributeMaterial = childState->getAttribute(osg::StateAttribute::MATERIAL);

	if (stateAttributeMaterial != nullptr)
	{
		osg::Material *objMaterial = dynamic_cast<osg::Material*>(stateAttributeMaterial);
		objMaterial->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(color, 1.0));
		objMaterial->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(color/2.0, 0.0));
	


		childState->setAttributeAndModes(new osg::BlendFunc);
		childState->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		arrowNode->addCullCallback(new FadeInOutCallback(objMaterial,matrixTransform));
	
	}
}


osg::ref_ptr<osg::Group> PlayerMarker::getNode()
{
	return m_node;
}

FadeInOutCallback::FadeInOutCallback( osg::Material* mat, osg::MatrixTransform *markerTransform )
: _material(mat), _lastDistance(-1.0f), _fadingState(0)
{
	_motion = new osgAnimation::InOutCubicMotion;
	_markerTransform = markerTransform;
}
#define MAX_SCALE_DISTANCE 1500.0f

void FadeInOutCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{

	osgUtil::CullVisitor* cv = static_cast<osgUtil::CullVisitor*>(nv);
	if (!cv)
		return;
	float distance = cv->getDistanceFromEyePoint(
		node->getBound().center(), true);

	if (distance > START_FADING_DISTANCE)
	{
		float scaling_fac = std::min(std::max(distance, NEAR_TRANSPARENCY_DISTANCE), MAX_SCALE_DISTANCE) / (NEAR_TRANSPARENCY_DISTANCE*2.0 * _markerTransform->getMatrix().getScale().z());
		osg::Matrixd distance_scale;
		distance_scale.makeScale(scaling_fac, scaling_fac, scaling_fac);

		_markerTransform->setMatrix(_markerTransform->getMatrix()*distance_scale);//osg::Vec3(5, 5, 5

			
	}

	if (_fadingState != 0)
	{
		_motion->update(0.01);
		float value = _motion->getValue();
		float alpha = (_fadingState>0 ? value : 1.0f - value);
		_material->setAlpha(osg::Material::FRONT_AND_BACK, alpha);

		if (value >= 1.0)
			_fadingState = 0;

		traverse(node, nv);
		return;
	}

	if (_lastDistance>0.0f)
	{
		if (_lastDistance<START_FADING_DISTANCE && distance >= START_FADING_DISTANCE)
		{
			_fadingState = 1; _motion->reset();

		}
		else if (_lastDistance>START_FADING_DISTANCE && distance <= START_FADING_DISTANCE)
		{
			_fadingState = -1; _motion->reset();
		}
		else if (_lastDistance < NEAR_TRANSPARENCY_DISTANCE && distance < NEAR_TRANSPARENCY_DISTANCE)
		{
			_material->setAlpha(osg::Material::FRONT_AND_BACK, 0.0);
		}
		else if (_lastDistance > START_FADING_DISTANCE && distance > START_FADING_DISTANCE)
		{
			_material->setAlpha(osg::Material::FRONT_AND_BACK, 1.0);
		}
	}

	_lastDistance = distance;
	traverse(node, nv);
}