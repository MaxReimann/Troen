#pragma once
// STD
#include <utility>
// OSG
#include <osg/ref_ptr>
#include <osgGA/NodeTrackerManipulator>
//bullet
#include <btBulletDynamicsCommon.h>
// troen
#include "../forwarddeclarations.h"
#include "abstractcontroller.h"
#include "../input/bikeinputstate.h"
#include "../input/keyboard.h"

namespace troen
{
	class BikeController : public AbstractController
	{
	public:
		BikeController(
			Player * player,
			const input::BikeInputState::InputDevice& inputDevice,
			const btTransform initialPosition,
			ResourcePool *resourcePool);
		~BikeController();

		void killThread();

		typedef enum enum_BIKESTATE
		{
			DRIVING,
			RESPAWN,
			RESPAWN_PART_2,
			WAITING_FOR_GAMESTART,
			WAITING
		} BIKESTATE;

		//
		// initialization & communication
		//
		void attachTrackingCamera(std::shared_ptr<HUDController>& hudController);
        void attachTrackingCamera (NodeFollowCameraManipulator* manipulator);
		void attachWorld(std::shared_ptr<PhysicsWorld> world);
		void attachGameView(osg::ref_ptr<osgViewer::View> gameView);

		void addUniformsToPlayerNode();

		//
		// logic events
		//
		void updateModel(const long double gameTime);
		void updateView(const btTransform &worldTrans);
		void setState(const BIKESTATE newState, const double respawnTime = -1);
		void moveBikeToPosition(btTransform position);
		void registerCollision(const btScalar impulse);
		void rememberFenceCollision(FenceController* fence);
		void activateTurbo();
		void updateUniforms();
		void reset();
		void handleEvent(const omega::Event& evt);

		//
		// getters & setters & attributes
		//
		virtual osg::ref_ptr<osg::Group> getViewNode();
		Player * player()		{ return m_player; };
		osg::ref_ptr<input::Keyboard> keyboardHandler()
		{
			return m_keyboardHandler;
		};
		float speed()			{ return m_speed; };
		BIKESTATE state()		{ return m_state; };
		double respawnTime()	{ return m_respawnTime; };
		std::pair<float, FenceController*> lastFenceCollision() { return m_lastFenceCollision; };

		bool turboInitiated()	{ return m_turboInitiated; };
		bool hasKeyboardHandler() { return m_keyboardHandler != NULL; };
		

		std::shared_ptr<BikeModel> getModel();
		std::shared_ptr<input::RemotePlayer> getRemote() const { return m_remote; }
		float getDistanceToObstacle(double angle);
		bool isFalling();

	private:
		std::shared_ptr<BikeView> m_bikeView;
		std::shared_ptr<BikeModel> m_bikeModel;

		//
		// field of view methods
		//
		void updateFov(const double speed);
		void setFovy(const float newFovy);
		float getFovy();
		float computeFovyDelta(const float speed, const float currentFovy);

		//
		// input
		//
		void initializeInput(const input::BikeInputState::InputDevice inputDevice);
		void setInputState(osg::ref_ptr<input::BikeInputState> bikeInputState);
		void initializeWASD(osg::ref_ptr<input::BikeInputState> bikeInputState);
		void initializeArrows(osg::ref_ptr<input::BikeInputState> bikeInputState);
		void initializeGamepadVRPN(osg::ref_ptr<input::BikeInputState> bikeInputState);
		void initializeAI(osg::ref_ptr<input::BikeInputState> bikeInputState);
		void initializeAIPy(osg::ref_ptr<input::BikeInputState> bikeInputState);
		void initializeGamepad(osg::ref_ptr<input::BikeInputState> bikeInputState);
		void initializeRemote(osg::ref_ptr<input::BikeInputState> bikeInputState);



		void updateNetworkFence(btTransform transform);
		long double getTimeFactor();

		//
		// communication links
		//
		Player * m_player;
		osg::ref_ptr<osgViewer::View>		m_gameView;
		osg::ref_ptr<input::Keyboard>	m_keyboardHandler;
		input::PollingDevice* m_pollingThread;

		osg::ref_ptr<input::BikeInputState> m_bikeInputState;
		std::shared_ptr<input::RemotePlayer> m_remote;

		//not smart_ptr: not actually owned by this object but by omega::enginemodule
		ManipulatorController* m_cameraController;
		


		// the following attributes only exist if the player
		// has a corresponding gameview
		osg::Uniform*	m_timeOfCollisionUniform;
		osg::Uniform*	m_velocityUniform;
		osg::Uniform*	m_timeFactorUniform;
		osg::Uniform*	m_healthUniform;

		//
		// behaviour attributes
		//
		btTransform	m_initialTransform;
		BIKESTATE	m_state;
		float		m_speed;
		bool		m_turboInitiated;
		double		m_respawnTime;
		std::shared_ptr<PhysicsWorld> m_world;
		float		m_timeOfLastCollision;
		std::pair<float, FenceController*> m_lastFenceCollision;
		input::BikeInputState::InputDevice m_inputDevice;
	};
}