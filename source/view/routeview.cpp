#include "routeview.h"
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
#include "../model/routemodel.h"
#include "../controller/routecontroller.h"
#include "osg/Vec3"

using namespace troen;


RouteView::RouteView(RouteController* routeController, osg::Vec3 color, std::shared_ptr<AbstractModel>& model) :
AbstractView(),
m_model(std::static_pointer_cast<RouteModel>(model)),
m_playerColor(color),
m_fenceController(routeController)
{
	initializeRoute();
	initializeShader();
}

void RouteView::initializeRoute()
{
	m_routeWidth = ROUTE_WIDTH_VIEW;

	m_coordinates = new osg::Vec3Array();
	m_coordinates->setDataVariance(osg::Object::DYNAMIC);

	m_relativeWidth = new osg::FloatArray();
	m_relativeWidth->setDataVariance(osg::Object::DYNAMIC);

	// this value could need adaption; will avoid time-intensive array resizing
	m_coordinates->reserveArray(10000);
	m_relativeWidth->reserveArray(10000);

	m_geometry = new osg::Geometry();
	m_geometry->setVertexArray(m_coordinates);

	// set the relative height between 0 and 1 as an additional vertex attribute
	m_geometry->setVertexAttribArray(5, m_relativeWidth);
	m_geometry->setVertexAttribBinding(5, osg::Geometry::BIND_PER_VERTEX);

	// seems to be important so that we won't crash after 683 fence parts
	m_geometry->setUseDisplayList(false);

	// use the shared normal array.
	// polyGeom->setNormalArray(shared_normals.get(), osg::Array::BIND_OVERALL);

	m_drawArrays = new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 0); 
	m_geometry->addPrimitiveSet(m_drawArrays);

	m_geode = new osg::Geode();
	m_geode->addDrawable(m_geometry);

	m_node->addChild(m_geode);
	m_node->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
	m_node->setName("fenceGroup");

	m_radarElementsGroup = new osg::Group();
	m_radarElementsGroup->setNodeMask(CAMERA_MASK_NONE);
	m_node->addChild(m_radarElementsGroup);


}

void RouteView::updateFadeOutFactor(float fadeOutFactor)
{
	m_fadeOutFactorUniform->set(fadeOutFactor);
}

void RouteView::updateFenceGap(osg::Vec3 lastPosition, osg::Vec3 position)
{
	if (m_coordinates->size() > 1) {
		m_coordinates->at(m_coordinates->size() - 2) = osg::Vec3(position.x() - m_routeWidth / 2.f, position.y(), position.z() + ROUTE_HOVER_HEIGHT);
		m_coordinates->at(m_coordinates->size() - 1) = osg::Vec3(position.x() + m_routeWidth / 2.f, position.y(), position.z() + ROUTE_HOVER_HEIGHT);
		m_relativeWidth->at(m_relativeWidth->size() - 2) = 0.f;
		m_relativeWidth->at(m_relativeWidth->size() - 1) = 1.f;
	}
}

void RouteView::initializeShader()
{
	osg::ref_ptr<osg::StateSet> nodeState = m_node->getOrCreateStateSet();
	
	osg::Uniform* fenceColorU = new osg::Uniform("fenceColor", m_playerColor);
	nodeState->addUniform(fenceColorU);

	osg::Uniform* modelIDU = new osg::Uniform("modelID", GLOW);
	nodeState->addUniform(modelIDU);

	m_fadeOutFactorUniform = new osg::Uniform("fadeOutFactor", 1.f);
	nodeState->addUniform(m_fadeOutFactorUniform);

	m_playerPositionUniform = new osg::Uniform("playerPosition", osg::Vec3(0.0, 0.0, 0.0));
	nodeState->addUniform(m_playerPositionUniform);

	nodeState->setMode(GL_BLEND, osg::StateAttribute::ON);
	nodeState->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
	nodeState->setAttributeAndModes(shaders::m_allShaderPrograms[shaders::FENCE], osg::StateAttribute::ON);

	shaders::m_allShaderPrograms[shaders::FENCE]->addBindAttribLocation("a_relWidth", 5);
}

std::vector<osg::Vec3> RouteView::subdivide(std::vector<osg::Vec3>& input, int level)
{
	typedef std::vector<osg::Vec3> v3Array;

	v3Array points = v3Array(input);
	for (int j = 0; j < level; j++)
	{
		v3Array subdivided = v3Array();

		for (int i = 1; i < points.size() - 1; i++)
		{
			osg::Vec3 p_1 = (points[i - 1] + points[i]) / 2.f;
			osg::Vec3 p_2 = (points[i] + points[i + 1]) / 2.f;
			osg::Vec3 p_i_new = (points[i] + (p_1 + p_2) / 2.f) / 2.f;

			subdivided.push_back(p_1);
			subdivided.push_back(p_i_new);
		}
		points.clear();
		points.push_back(input[0]);
		points.insert(points.end(), subdivided.begin(), subdivided.end());
		points.push_back(input.back());

	}

	return points;
}

void RouteView::addFencePart(osg::Vec3 lastPosition, osg::Vec3 currentPosition)
{


	osg::Vec2 ncurrentDirection = toVec2((currentPosition - lastPosition));
	ncurrentDirection.normalize();

	osg::Vec3 sideDirection = osg::Vec3(ncurrentDirection, 0.0) ^ osg::Vec3(0.0, 0.0, 1.0);
	sideDirection.normalize();
	osg::Vec3 sideVec = sideDirection * (m_routeWidth / 2.f);

	osg::Vec2 currentDirection = toVec2((currentPosition - lastPosition));

	osg::Vec3 lP = currentPosition + sideVec  + osg::Vec3(0.0,0.0,0.1f);
	osg::Vec3 rP = currentPosition - sideVec + osg::Vec3(0.0, 0.0, 0.1f);
	osg::Vec3 lastlP = lastPosition + sideVec + osg::Vec3(0.0, 0.0, 0.1f);
	osg::Vec3 lastrP = lastPosition - sideVec + osg::Vec3(0.0, 0.0, 0.1f);
	osg::Vec3 up = osg::Vec3(0.0, 0.0, ROUTE_HOVER_HEIGHT);

	osg::Vec3 cube[] = { lastlP, lastrP, rP, lP,
		lastrP, lastrP + up, rP + up, rP,
		lastlP, lastlP + up, lP + up, lP,
		lastlP + up, lastrP + up, rP + up, lP + up };
	float relWidths[] = { 
		0.f, 1.f, 1.f, 0.f,
		0.f, 1.f, 1.f, 0.f,
		0.f, 1.f, 1.f, 0.f,
		0.f, 1.f, 1.f, 0.f };

	if (m_coordinates->size() == 0)
	{
		

		m_coordinates->push_back(lP - osg::Vec3(currentDirection,0.0));
		m_coordinates->push_back(rP - osg::Vec3(currentDirection, 0.0));
		m_coordinates->push_back(rP - osg::Vec3(currentDirection, ROUTE_HOVER_HEIGHT));
		m_coordinates->push_back(lP - osg::Vec3(currentDirection, ROUTE_HOVER_HEIGHT));
		m_relativeWidth->push_back(0.f);
		m_relativeWidth->push_back(1.f);

	}

	// game fence part
	//m_coordinates->push_back(lP);
	//m_coordinates->push_back(rP);
	for (osg::Vec3 v : cube)
		m_coordinates->push_back(v);
	for (float f : relWidths)
		m_relativeWidth->push_back(f);

	//m_relativeWidth->push_back(0.f);
	//m_relativeWidth->push_back(1.f);


	int currentFenceParts = (m_coordinates->size() - 2) / 2;

	// radar fence part
	if (currentFenceParts % FENCE_TO_MINIMAP_PARTS_RATIO == 0)
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
	//enforceFencePartsLimit();

	//necessary for network fences, because of unpredictable timings
	m_geometry->dirtyBound();
	m_drawArrays->setCount(m_coordinates->size());
}

void RouteView::removeAllFences()
{
	m_node->removeChild(m_geode);
	for (auto radarFenceBox : m_radarFenceBoxes)
	{
		m_radarElementsGroup->removeChild(radarFenceBox);
	}
	m_radarFenceBoxes.clear();
	initializeRoute();
}

void RouteView::enforceFencePartsLimit()
{
	int maxFenceParts = m_fenceController->getFenceLimit();

	// the quad strip contains two more vertices for the beginning of the fence
	int currentFenceParts = (m_coordinates->size() - 2) / 2;

	if (maxFenceParts != 0 && currentFenceParts > maxFenceParts)
	{
		for (int i = 0; i < (currentFenceParts - maxFenceParts); i++)
		{
			m_coordinates->erase(m_coordinates->begin(), m_coordinates->begin() + 2);
			m_relativeWidth->erase(m_relativeWidth->begin(), m_relativeWidth->begin() + 2);
		}
	}
	// radar fence boxes
	if (maxFenceParts != 0 && m_radarFenceBoxes.size() > maxFenceParts / FENCE_TO_MINIMAP_PARTS_RATIO)
	{
		for (int i = 0; i < (m_radarFenceBoxes.size() - maxFenceParts / FENCE_TO_MINIMAP_PARTS_RATIO); i++)
		{
			m_radarElementsGroup->removeChild(m_radarFenceBoxes.front());
			m_radarFenceBoxes.pop_front();
		}
	}
}

void RouteView::showFencesInRadarForPlayer(const int id)
{
	osg::Node::NodeMask currentMask = m_radarElementsGroup->getNodeMask();
	osg::Node::NodeMask newMask = currentMask | CAMERA_MASK_PLAYER[id];
	m_radarElementsGroup->setNodeMask(newMask);
}

void RouteView::hideFencesInRadarForPlayer(const int id)
{
	osg::Node::NodeMask currentMask = m_radarElementsGroup->getNodeMask();
	osg::Node::NodeMask newMask = currentMask & ~ CAMERA_MASK_PLAYER[id];
	m_radarElementsGroup->setNodeMask(newMask);
}

