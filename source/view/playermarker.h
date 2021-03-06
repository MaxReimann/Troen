#pragma once
// OSG
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/PositionAttitudeTransform>
#include <osg/Material>
// troen
#include "easemotion.h"
#include "../forwarddeclarations.h"
#include "abstractview.h"

namespace troen
{

	/*! PlayerMarkers are displayed above the players to make it easier to spot enemies which are far away. The screen size of the PlayerMarkers is always constant (no matter how far away they are).*/
	class PlayerMarker : public AbstractView
	{
	public:
		PlayerMarker(const osg::Vec3 color);
		osg::ref_ptr<osg::Group> getNode();

	private:
		osg::ref_ptr<osg::MatrixTransform> m_matrixTransform;
		osg::Vec3 m_playerColor;
		osg::ref_ptr<osg::Group> m_node;
		osg::ref_ptr<osg::Uniform> m_markerColorUniform;
	};


	class FadeInOutCallback : public osg::NodeCallback
	{
		public:
			FadeInOutCallback(osg::ref_ptr<osg::Uniform> colorU, osg::Vec3 color, osg::MatrixTransform *markerTransform);
			
			virtual void operator()( osg::Node* node, osg::NodeVisitor* nv );
		
		protected:
			osg::ref_ptr<osgAnimation::InOutCubicMotion> _motion;
			osg::ref_ptr<osg::MatrixTransform> _markerTransform;
			osg::ref_ptr<osg::Uniform> m_colorUniform;
			float _lastDistance;
			int _fadingState;
			osg::Vec3 m_color;
	};

}