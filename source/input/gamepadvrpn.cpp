#include "gamepadvrpn.h"
// troen
#include "bikeinputstate.h"
#include "../constants.h"
#include <cmath>



using namespace troen::input;



void GamepadVRPN::reset()
{

}

GamepadVRPN::GamepadVRPN(osg::ref_ptr<BikeInputState> bikeInputState, osg::Vec3 color) : PollingDevice(bikeInputState),
	m_vibrate(false)
{


	// m_writeBuffer[0] = 5; // report number has to be 5
	// m_writeBuffer[1] = 255; // flip LSB to enable/disable rumble
	// m_writeBuffer[4] = 0; // right motor fast/weak
	// m_writeBuffer[5] = 0; // left motor slow/strong
	// m_writeBuffer[6] = 0; // R
	// m_writeBuffer[7] = 255; // G
	// m_writeBuffer[8] = 255; // B
	// m_writeBuffer[9] = 0; // flash on duration
	// m_writeBuffer[10] = 0; // flash off duration

	m_deadzoneX = 0.05f;
	m_deadzoneY = 0.05f;

	//initialise hid api
	// hid_init();

	// if (checkConnection()) {
	// 	setColor(color);
	// }
}

GamepadVRPN::~GamepadVRPN()
{
}

void GamepadVRPN::handleEvent(const omega::Event& event)
{
	if (event.getServiceType() !=  omega::Service::Controller)
		return;






	switch (event.getType())
	{
		case(omega::EventBase::Update) :
		{

			m_leftAnalogLR = event.getAxis(0);
	        m_leftAnalogUD = event.getAxis(1);
	        m_rightAnalogLR = event.getAxis(2);
	        m_rightAnalogUD = event.getAxis(3);
	        m_L2 = event.getAxis(5);
	        m_R2 = event.getAxis(6);

			return;
		}
		case(omega::EventBase::Down):
		{
			if (event.isButtonDown(omega::EventBase::Button1)){
				m_handbrakePressed = true;
				std::cout << "handbrake pressed" << std::endl;
			}

			if (event.isButtonDown(omega::EventBase::Button2))
				m_turboPressed = true;

			return;
		}
		case(omega::EventBase::Up) :
		{
			std::cout << "up" << std::endl;
			//there is a bug, the up containing the correct button is only trigger after another button/joystick was updated
			// this is very dirty hack, which also does not work all the time: when nothing is set, there was 
			// a up event of one of our buttons. guees which one.
			bool anythingset  =false;
			for (int i = 0;i<16;i++)
			{
				if (event.isFlagSet(1<<i)){
					std::cout << "set: " << i << std::endl;
					anythingset = true;
					break;
				}
			}

			// if (!anythingset && m_turboPressed)
			// 	m_turboPressed = false;

			if (!anythingset && m_handbrakePressed)
				m_handbrakePressed = false;

			// if (event.isButtonUp(omega::EventBase::Button2))
			// 	m_turboPressed = false;

			// if(event.isButtonUp(omega::EventBase::Button1)) {
			// 	std::cout << "handbrake released" << std::endl;
			// 	m_handbrakePressed = false;
			// }
			return;
		}
	default:
		return;
	}
}


void GamepadVRPN::setColor(osg::Vec3 color)
{
	// m_writeBuffer[6] = (uchar) color.x();
	// m_writeBuffer[7] = (uchar) color.y();
	// m_writeBuffer[8] = (uchar) color.z();
	// hid_write(_controller, m_writeBuffer, 32);
}

void GamepadVRPN::setVibration(const bool b)
{
	// if (_controller && m_vibrate != b) {
	// 	m_writeBuffer[4] = b ? 255 : 0;
	// 	m_writeBuffer[5] = b ? 255 : 0;
	// 	// hid_write(_controller, m_writeBuffer, 32);
	// 	m_vibrate = b;
	// }
}

//check for events and handle them
void GamepadVRPN::run()
{
	m_pollingEnabled = true;

	while (m_pollingEnabled)
	{
		// _controller = hid_open(VID, PID, m_serialNumber);
		// if (!checkConnection())
		// {
		// 	m_bikeInputState->setAngle(0);
		// 	m_bikeInputState->setAcceleration(0);
		// 	m_bikeInputState->setTurboPressed(false);
		// 	continue;
		// };
		// unsigned char *buffer = buf;
		// get angle value from LEFT_HAT
		float leftStickX = (abs(m_leftAnalogLR) < m_deadzoneX ? 0 : 
				(abs(m_leftAnalogLR) - m_deadzoneX) * (m_leftAnalogLR / abs(m_leftAnalogLR)));

		// get viewingAngle value from RIGHT_HAT
		float rightStickX = (abs(m_rightAnalogLR) < m_deadzoneX ? 0 : 
			(abs(m_rightAnalogLR) - m_deadzoneX) * (m_rightAnalogLR / abs(m_rightAnalogLR)));

		float rightStickY = (abs(m_rightAnalogUD) < m_deadzoneY ? 0 : 
			(abs(m_rightAnalogUD) - m_deadzoneY) * (m_rightAnalogUD / abs(m_rightAnalogUD)));

		float viewingAngle, relativeAngle;
		if (rightStickX != 0.0 || rightStickY != 0.0) {
			relativeAngle = atan(rightStickX / rightStickY);
			viewingAngle = (rightStickY >= 0.f ? rightStickX < 0 ? PI + relativeAngle : -PI + relativeAngle : relativeAngle);
		}
		else {
			viewingAngle = 0.f;
		}

			// std::cout << m_handbrakePressed << std::endl;


		// get acceleration from RIGHT_2 and LEFT_2
		// float rightTrigger = m_R2;
		// float leftTrigger = m_L2;

		m_bikeInputState->setAngle(-leftStickX - leftStickX * m_handbrakePressed * BIKE_HANDBRAKE_FACTOR);
		m_bikeInputState->setTurboPressed(m_turboPressed);
		m_bikeInputState->setAcceleration(m_R2 - m_L2);
		m_bikeInputState->setViewingAngle(viewingAngle);

		this->msleep(POLLING_DELAY_MS);
	}
}

bool GamepadVRPN::checkConnection(){


	return false;
}