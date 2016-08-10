#pragma once
// OSG
#include <osg/Vec3>
// troen
#include "pollingdevice.h"
#include "../forwarddeclarations.h"

// #include <hidapi.h>

namespace troen
{
namespace input
{
/*! The GamepadVRPN class is responsible for receiving input from a Playstation 4 controller. Additionally, it can control the vibration and LED of the controller.*/
	class GamepadVRPN : public PollingDevice
	{
	public:
		GamepadVRPN(osg::ref_ptr<BikeInputState> bikeInputState, osg::Vec3 color);
		~GamepadVRPN();

		void run() override;
		bool checkConnection();
		void setColor(osg::Vec3 color);
		void setVibration(const bool b) override;

		virtual void handleEvent(const omega::Event& event);
		// hid_device *_controller = NULL;
		bool _controller = false; //dummy

		// static hid_device_info* allHidDevices;
		static void reset();

	private:

		float m_deadzoneX, m_deadzoneY;

		bool m_vibrate = false;
		float m_leftAnalogLR = 0.0 ;
		float m_leftAnalogUD = 0.0 ;
		float m_rightAnalogLR = 0.0;
		float m_rightAnalogUD = 0.0;
		float m_L2 = 0.0;
		float m_R2 = 0.0;

		bool m_handbrakePressed = false;
		bool m_turboPressed = false;
	};
}
}