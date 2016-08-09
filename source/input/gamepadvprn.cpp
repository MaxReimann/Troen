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
	m_vibrate(false), m_serialNumber(NULL)
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

	if (checkConnection()) {
		setColor(color);
	}
}

GamepadVRPN::~GamepadVRPN()
{
}

/*
Returns for each type whether it is pressed (1) or not (0).
For {LEFT|RIGHT}_HAT_{X|Y} it returns a value between 0 and 255.
*/
int GamepadVRPN::getValueFromKey(GamepadVRPN::PS4KEY type, unsigned char *buffer){
	return -1;
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
		// unsigned char buf[96];
		// check whether controller is available, if not search for controller
		// if it is still not available do nothing
		if (!_controller){
			// _controller = hid_open(VID, PID, m_serialNumber);
			if (!checkConnection())
			{
				m_bikeInputState->setAngle(0);
				m_bikeInputState->setAcceleration(0);
				m_bikeInputState->setTurboPressed(false);
				continue;
			};
		}

		// unsigned char *buffer = buf;

		// get angle value from LEFT_HAT
		float normLX = (getValueFromKey(GamepadVRPN::PS4KEY::LEFT_HAT_X, buffer) - 128) / 128.f;
		float leftStickX = (abs(normLX) < m_deadzoneX ? 0 : (abs(normLX) - m_deadzoneX) * (normLX / abs(normLX)));

		// get viewingAngle value from RIGHT_HAT
		float normRX = (getValueFromKey(GamepadVRPN::PS4KEY::RIGHT_HAT_X, buffer) - 128) / 128.f;
		float rightStickX = (abs(normRX) < m_deadzoneX ? 0 : (abs(normRX) - m_deadzoneX) * (normRX / abs(normRX)));
		float normRY = (getValueFromKey(GamepadVRPN::PS4KEY::RIGHT_HAT_Y, buffer) - 128) / 128.f;
		float rightStickY = (abs(normRY) < m_deadzoneY ? 0 : (abs(normRY) - m_deadzoneY) * (normRY / abs(normRY)));

		float viewingAngle, relativeAngle;
		if (rightStickX != 0.0 || rightStickY != 0.0) {
			relativeAngle = atan(rightStickX / rightStickY);
			viewingAngle = (rightStickY >= 0.f ? rightStickX < 0 ? PI + relativeAngle : -PI + relativeAngle : relativeAngle);
		}
		else {
			viewingAngle = 0.f;
		}

		float handbrakePressed = getValueFromKey(GamepadVRPN::PS4KEY::ONE_PRESSED, buffer);
		float turboPressed = getValueFromKey(GamepadVRPN::PS4KEY::TWO_PRESSED, buffer);

		// get acceleration from RIGHT_2 and LEFT_2
		float rightTrigger = getValueFromKey(GamepadVRPN::PS4KEY::RIGHT_2, buffer) / 255.f;
		float leftTrigger = getValueFromKey(GamepadVRPN::PS4KEY::LEFT_2, buffer) / 255.f;

		m_bikeInputState->setAngle(-leftStickX - leftStickX * handbrakePressed * BIKE_HANDBRAKE_FACTOR);
		m_bikeInputState->setTurboPressed(turboPressed);
		m_bikeInputState->setAcceleration(rightTrigger - leftTrigger);
		m_bikeInputState->setViewingAngle(viewingAngle);

		this->msleep(POLLING_DELAY_MS);
	}
}

bool GamepadVRPN::checkConnection(){


	return false;
}