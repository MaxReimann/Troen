#pragma once
// STD
#include <deque>
// OSG
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/MatrixTransform>
// troen
#include "../forwarddeclarations.h"
#include "abstractview.h"

namespace troen
{
	class RouteView : public AbstractView
	{
	public:
		RouteView(RouteController* fenceController, const osg::Vec3 color, std::shared_ptr<AbstractModel>& model);

		void addFencePart(const osg::Vec3 lastPosition, const osg::Vec3 currentPosition);
		void removeAllFences();
		void enforceFencePartsLimit();
		void updateFenceGap(const osg::Vec3 lastPosition, const osg::Vec3 position);

		void showFencesInRadarForPlayer(const int id);
		void hideFencesInRadarForPlayer(const int id);

		void updateFadeOutFactor(float fadeOutFactor);
		void setBendingActive(bool val);

	private:
		void initializeFence();
		void initializeFenceGap();
		void initializeShader();
		osg::ref_ptr<osg::Geometry>		m_geometry;
		osg::ref_ptr<osg::Vec3Array>	m_coordinates;
		osg::ref_ptr<osg::FloatArray>	m_relativeHeights;
		osg::ref_ptr<osg::DrawArrays>	m_drawArrays;
		osg::ref_ptr<osg::Geode>		m_geode;
		osg::ref_ptr<osg::Group>		m_radarElementsGroup;
		std::deque<osg::ref_ptr<osg::MatrixTransform>>	m_radarFenceBoxes;

		std::weak_ptr<RouteModel>		m_model;

		osg::Uniform*	m_fadeOutFactorUniform;
		osg::Uniform*   m_bendingActiveUniform;

		osg::Vec3 m_playerColor;
		float m_fenceHeight;

		RouteController* m_fenceController;
	};
}