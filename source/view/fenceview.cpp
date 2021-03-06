#include "fenceview.h"
// OSG
#include <osg/ShapeDrawable>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Vec4>
#include <osg/PositionAttitudeTransform>
#include <osg/ref_ptr>
// troen
#include "../constants.h"
#include "shaders.h"
#include "../model/fencemodel.h"
#include "../controller/fencecontroller.h"

#define P_OSGVEC(v)  std::cout << v.x() << " " << v.y() << " " << v.z() << std::endl;

using namespace troen;

FenceView::FenceView(FenceController* fenceController, osg::Vec3 color, std::shared_ptr<AbstractModel>& model) :
AbstractView(),
m_model(std::static_pointer_cast<FenceModel>(model)),
m_playerColor(color),
m_fenceController(fenceController), m_masterNode(false)
{
	initializeFence();
	initializeShader();
	m_start = false;
	m_fenceCleared = false;
	m_maxFencePartsCached = 0;
}

void FenceView::initializeFence()
{
	m_fenceHeight = FENCE_HEIGHT_VIEW;

	m_coordinates = new osg::Vec3Array();
	m_coordinates->setDataVariance(osg::Object::DYNAMIC);

	m_relativeHeights = new osg::FloatArray();
	m_relativeHeights->setDataVariance(osg::Object::DYNAMIC);

	// this value could need adaption; will avoid time-intensive array resizing
	m_coordinates->reserveArray(10000);
	m_relativeHeights->reserveArray(10000);

	m_geometry = new osg::Geometry();
	m_geometry->setVertexArray(m_coordinates);

	// set the relative height between 0 and 1 as an additional vertex attribute
	m_geometry->setVertexAttribArray(5, m_relativeHeights);
	m_geometry->setVertexAttribBinding(5, osg::Geometry::BIND_PER_VERTEX);

	// seems to be important so that we won't crash after 683 fence parts
	m_geometry->setUseDisplayList(false);

	// use the shared normal array.
	// polyGeom->setNormalArray(shared_normals.get(), osg::Array::BIND_OVERALL);

	m_drawArrays = new osg::DrawArrays(osg::PrimitiveSet::QUAD_STRIP, 0, 0);
	m_geometry->addPrimitiveSet(m_drawArrays);

	m_geode = new osg::Geode();
	m_geode->addDrawable(m_geometry);

	m_node->addChild(m_geode);
	m_node->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::OFF); 
	m_node->setName("fenceGroup");

	m_radarElementsGroup = new osg::Group();
	m_radarElementsGroup->setNodeMask(CAMERA_MASK_NONE);
	m_node->addChild(m_radarElementsGroup);

	m_bendingActiveUniform = new osg::Uniform("bendingActivated", false);
	m_node->getOrCreateStateSet()->addUniform(m_bendingActiveUniform);

}

void FenceView::updateFadeOutFactor(float fadeOutFactor)
{
	m_fadeOutFactorUniform->set(fadeOutFactor);
}

void FenceView::updateFenceGap(osg::Vec3 lastPosition, osg::Vec3 position)
{
	if (m_coordinates->size() > 1) {
		m_coordinates->at(m_coordinates->size() - 2) = osg::Vec3(position.x(), position.y(), position.z());
		m_coordinates->at(m_coordinates->size() - 1) = osg::Vec3(position.x(), position.y(), position.z() + m_fenceHeight);
		m_relativeHeights->at(m_relativeHeights->size() - 2) = 0.f;
		m_relativeHeights->at(m_relativeHeights->size() - 1) = 1.f;
	}
}

void FenceView::initializeShader()
{
	osg::ref_ptr<osg::StateSet> NodeState = m_node->getOrCreateStateSet();
	
	osg::Uniform* fenceColorU = new osg::Uniform("fenceColor", m_playerColor);
	NodeState->addUniform(fenceColorU);

	osg::Uniform* modelIDU = new osg::Uniform("modelID", GLOW);
	NodeState->addUniform(modelIDU);

	m_fadeOutFactorUniform = new osg::Uniform("fadeOutFactor", 1.f);
	NodeState->addUniform(m_fadeOutFactorUniform);

	NodeState->setMode(GL_BLEND, osg::StateAttribute::ON);
	NodeState->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
	NodeState->setAttributeAndModes(shaders::m_allShaderPrograms[shaders::FENCE], osg::StateAttribute::ON);

	shaders::m_allShaderPrograms[shaders::FENCE]->addBindAttribLocation("a_relHeight", 5);
}

void FenceView::addFencePart(osg::Vec3 lastPosition, osg::Vec3 currentPosition)
{	
	if (m_coordinates->size() == 0)
	{
		m_coordinates->push_back(lastPosition);
		m_coordinates->push_back(osg::Vec3(lastPosition.x(), lastPosition.y(), lastPosition.z() + m_fenceHeight));
		m_relativeHeights->push_back(0.f);
		m_relativeHeights->push_back(1.f);
	}


	// game fence part
	m_coordinates->push_back(currentPosition);
	m_coordinates->push_back(osg::Vec3(currentPosition.x(), currentPosition.y(), currentPosition.z() + m_fenceHeight));
	m_relativeHeights->push_back(0.f);
	m_relativeHeights->push_back(1.f);


	int currentFenceParts = (m_coordinates->size() - 2) / 2;

	// radar fence part
	if (false && currentFenceParts % FENCE_TO_MINIMAP_PARTS_RATIO == 0) // TODO: undo false./translate back in
	{
		osg::ref_ptr<osg::Box> box
			= new osg::Box(osg::Vec3(0, 0, 0), 60, 60, 60);
		osg::ref_ptr<osg::ShapeDrawable> mark_shape = new osg::ShapeDrawable(box);
		mark_shape->setColor(osg::Vec4f(m_playerColor, 1));
		osg::ref_ptr<osg::Geode> mark_node = new osg::Geode;
		mark_node->addDrawable(mark_shape.get());
		//mark_node->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

		// place objects in world space
		osg::Matrixd initialTransform;
		//initialTransform.makeRotate(rotationQuatXY);
		initialTransform *= initialTransform.translate((currentPosition + lastPosition) / 2);

		osg::ref_ptr<osg::MatrixTransform> matrixTransformRadar = new osg::MatrixTransform(initialTransform);
		matrixTransformRadar->addChild(mark_node);

		m_radarElementsGroup->addChild(matrixTransformRadar);
		m_radarFenceBoxes.push_back(matrixTransformRadar);
	}

	// limit
	enforceFencePartsLimit();

	//necessary for network fences, because of unpredictable timings
	m_geometry->dirtyBound();
	// very important to dirty vertex and attributes, as otherwise this can segfault
	// (only expierenced in conjuction with omegalib)
    m_coordinates->dirty();
    m_relativeHeights->dirty();

	m_drawArrays->setCount(m_coordinates->size());

	if (omega::SystemManager::instance()->isMaster())
	{
		m_currentPositionCached = currentPosition;
		m_lastPositionCached = lastPosition;
		m_fenceUpdated = true;
	}



}

void FenceView::removeAllFences()
{
	m_node->removeChild(m_geode);
	for (auto radarFenceBox : m_radarFenceBoxes)
	{
		m_radarElementsGroup->removeChild(radarFenceBox);
	}
	m_radarFenceBoxes.clear();
	initializeFence();
	m_fenceCleared = true;
}

void FenceView::enforceFencePartsLimit()
{
	if (omega::SystemManager::instance()->isMaster())
	 	m_maxFencePartsCached = m_fenceController->getFenceLimit();

	// the quad strip contains two more vertices for the beginning of the fence
	int currentFenceParts = (m_coordinates->size() - 2) / 2;

	if (m_maxFencePartsCached != 0 && currentFenceParts > m_maxFencePartsCached)
	{
		for (int i = 0; i < (currentFenceParts - m_maxFencePartsCached); i++)
		{
			m_coordinates->erase(m_coordinates->begin(), m_coordinates->begin() + 2);
			m_relativeHeights->erase(m_relativeHeights->begin(), m_relativeHeights->begin() + 2);
		}
	}
	// radar fence boxes
	if (m_maxFencePartsCached != 0 && m_radarFenceBoxes.size() > m_maxFencePartsCached / FENCE_TO_MINIMAP_PARTS_RATIO)
	{
		for (int i = 0; i < (m_radarFenceBoxes.size() - m_maxFencePartsCached / FENCE_TO_MINIMAP_PARTS_RATIO); i++)
		{
			m_radarElementsGroup->removeChild(m_radarFenceBoxes.front());
			m_radarFenceBoxes.pop_front();
		}
	}
}

void FenceView::showFencesInRadarForPlayer(const int id)
{
	osg::Node::NodeMask currentMask = m_radarElementsGroup->getNodeMask();
	osg::Node::NodeMask newMask = currentMask | CAMERA_MASK_PLAYER[id];
	m_radarElementsGroup->setNodeMask(newMask);
}

void FenceView::hideFencesInRadarForPlayer(const int id)
{
	osg::Node::NodeMask currentMask = m_radarElementsGroup->getNodeMask();
	osg::Node::NodeMask newMask = currentMask & ~ CAMERA_MASK_PLAYER[id];
	m_radarElementsGroup->setNodeMask(newMask);
}

void FenceView::setBendingActive(bool val)
{
	m_bendingActiveUniform->set(val);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// only run on master
// for each part of each geo of each object of each asset:
// send all the verts, faces, normals, colours, etc
void FenceView::commitSharedData(omega::SharedOStream& out)
{
	// std::cout << "comitting shared data" << std::endl;
	out << m_fenceUpdated << m_lastPositionCached << m_currentPositionCached << m_fenceCleared << m_maxFencePartsCached;

	// clear per frame states
	m_fenceUpdated = false;
	m_fenceCleared = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// only run on slaves!
void FenceView::updateSharedData(omega::SharedIStream& in)
{
	// std::cout << "updating shared data" << std::endl;
	in >> m_fenceUpdated >> m_lastPositionCached >> m_currentPositionCached >> m_fenceCleared >> m_maxFencePartsCached;
	
	if (m_fenceCleared)
		removeAllFences();

	if (m_fenceUpdated)
		addFencePart(m_lastPositionCached, m_currentPositionCached);


	updateFenceGap(m_lastPositionCached, m_currentPositionCached);

}