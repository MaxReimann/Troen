#pragma once
#include "RakNetTime.h"
// OSG
#include <osg/Referenced>
// troen
#include "../forwarddeclarations.h"
#include "LinearMath/btVector3.h"
#include "LinearMath/btQuaternion.h"

namespace troen
{
namespace input
{
/*! The BikeInputState holds the information about which input the player has made. It provides a layer of abstraction for the different input methods (like keyboard, controller etc.). Instead of saving which buttons got pressed, it saves values for acceleration/braking, steering etc.*/
	class BikeInputState : public osg::Referenced
	{
	public:
		enum InputDevice
		{
			KEYBOARD_wasd, KEYBOARD_arrows, GAMEPAD, GAMEPADVRPN, AI, REMOTE_PLAYER
		};

		BikeInputState();
		float getAngle();
		float getAcceleration();
		void setAngle(const float angle);
		void setAcceleration(const float acceleration);
		void setTurboPressed(const bool pressed);
		bool getTurboPressed();
		void setViewingAngle(float angle);
		float getViewingAngle();
		void setRemote(bool isRemote);
		bool isRemote();
		btVector3 getPosition();
		void setPosition(btVector3 position);
		btQuaternion getRotation();
		void setRotation(btQuaternion val);
		btVector3 getLinearVelocity();
		void setLinearVeloctiy(btVector3 val);
		float getAngularVelocity();
		void setAngularVelocityZ(float val);
		void setIsNewPosition(bool value);
		bool isNewPosition();
		void setReceivementTimestamp(RakNet::Time time);
		RakNet::Time getReceivementTime();
	private:
		float m_acceleration;
		float m_angle;
		bool m_turboPressed;
		float m_viewingAngle;
		bool m_isRemote;
		btVector3 m_position;
		btQuaternion m_rotation;
		btVector3 m_linearVeloctiy;
		float m_angularVelocity;
		bool m_isNewPosition;
		RakNet::Time m_receivementTime;
	};
}
}