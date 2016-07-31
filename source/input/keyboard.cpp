#include "keyboard.h"
// troen
#include "bikeinputstate.h"
#include "../view/shaders.h"
#include "../constants.h"

using namespace troen::input;

Keyboard::Keyboard(osg::ref_ptr<BikeInputState> bikeInputState, std::vector<omega::KeyCode> keys) :
PollingDevice(bikeInputState),
m_bikeInputState(bikeInputState),
m_keys(keys),
m_handbrakePressed(false),
m_leftPressed(false), m_upPressed(false), 
m_rightPressed(false), m_downPressed(false), m_turboPressed(false), m_angle(0)
{}


void Keyboard::handleEvent(const omega::Event& event)
{
	switch (event.getType())
	{
		case(omega::Event::Down) :
		{
			if (event.isKeyDown(m_keys[0]))
			{
				m_upPressed = true;
				return;
			}
			else if ( event.isKeyDown(m_keys[1]) )
			{
				m_leftPressed = true;
				return;
			}
			else if ( event.isKeyDown(m_keys[2]) )
			{
				m_downPressed = true;
				return;
			}
			else if ( event.isKeyDown(m_keys[3]) )
			{
				m_rightPressed = true;
				return;
			}
			else if ( event.isKeyDown(m_keys[4]) )
			{
				m_handbrakePressed = 1.0;
				return;
			}
			else if ( event.isKeyDown(m_keys[5]) )
			{
				m_turboPressed = true;
				return;
			}
			return;
		}
		case(omega::Event::Up) :
		{
			if ( event.isKeyUp(m_keys[0]) )
			{
				m_upPressed = false;
				return;
			}
			else if ( event.isKeyUp(m_keys[1]) )
			{
				m_leftPressed = false;
				return;
			}
			else if ( event.isKeyUp(m_keys[2]) )
			{
				m_downPressed = false;
				return;
			}
			else if ( event.isKeyUp(m_keys[3]) )
			{
				m_rightPressed = false;
				return;
			}
			else if ( event.isKeyUp(m_keys[4]) )
			{
				m_handbrakePressed = 0.0;
				return;
			}
			else if ( event.isKeyUp(m_keys[5]) )
			{
				m_turboPressed = false;
				return;
			}
			return;
		}
	default:
		return;
	}
}


void Keyboard::run()
{
	m_pollingEnabled = true;

	while (m_pollingEnabled)
	{
		// save angle to enable smooth direction changes
		if (m_leftPressed)
			m_angle = mi(1.0, m_angle + 0.05);
		else if (m_angle > 0)
			m_angle = 0;
		if (m_rightPressed)
			m_angle = ma(-1.0, m_angle - 0.05);
		else if (m_angle < 0)
			m_angle = 0;



		
		float leftAngle = m_leftPressed ? m_angle + m_handbrakePressed * BIKE_HANDBRAKE_FACTOR : 0.0;
		float rightAngle = m_rightPressed ? m_angle - m_handbrakePressed * BIKE_HANDBRAKE_FACTOR : 0.0;
		m_bikeInputState->setAngle(leftAngle + rightAngle);

		m_bikeInputState->setTurboPressed(m_turboPressed);

		float acceleration = m_upPressed ? 1.0 : 0.0 - m_downPressed ? -1.0 : 0.0;
		m_bikeInputState->setAcceleration(acceleration);

		this->msleep(POLLING_DELAY_MS);
	}
}