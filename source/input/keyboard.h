#pragma once
// OSG
#include <osgGA/GUIEventHandler>
// troen
#include "pollingdevice.h"
#include "../forwarddeclarations.h"

namespace troen
{
namespace input
{
/*! The keyboard class handles user input which is received via keyboard and is intended to navigate the player.*/
	class Keyboard : public osgGA::GUIEventHandler, public PollingDevice
	{

	public:
		Keyboard(osg::ref_ptr<BikeInputState> bikeInputState, std::vector<osgGA::GUIEventAdapter::KeySymbol> keys);
		virtual bool handle(const osgGA::GUIEventAdapter& eventAdapter, osgGA::GUIActionAdapter&);
		// virtual void accept(osgGA::GUIEventHandlerVisitor& visitor)   { visitor.visit(*this); };
		void run();

	protected:
		osg::ref_ptr<BikeInputState> m_bikeInputState;
		std::vector<osgGA::GUIEventAdapter::KeySymbol> m_keys;
		float m_handbrakePressed;
		bool m_leftPressed, m_upPressed, m_rightPressed, m_downPressed, m_turboPressed;
		float m_angle;
	};
}
}