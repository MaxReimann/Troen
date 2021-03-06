#pragma once

// STD includes we need everywhere
#include <memory>
#include <iostream>
// #include <boost/shared_ptr.hpp>
// #include <boost/weak_ptr.hpp>

// This file contains forward declarations of classes

// Qt forward declarations

// OSG forward declarations
namespace osgViewer
{
	class Viewer;
	class CompositeViewer;
	class View;
	class StatsHandler;
}

namespace osgQt
{
	class GraphicsWindowQt;
}

namespace osg
{
	class Camera;
	class Node;
	class Group;
	class Geode;
	class GraphicsContext;
	class ShapeDrawable;
	class Vec3f;
	class Image;
}

namespace osgText
{
	class Text;
}


namespace osgGA
{
	class CameraManipulator;
	class NodeTrackerManipulator;
}

// Bullet
class btDiscreteDynamicsWorld;
class btSequentialImpulseConstraintSolver;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btRigidBody;
class btCollisionObject;
class btCollisionShape;
class btPersistentManifold;
class btVector3;
class btTransform;

namespace scriptzeug {
	class ScriptEnvironment;
}

// own classes
namespace troen
{
	class TroenGame;
	class TroenOmegaScene;
	class SharedDataListener;
	class GameThread;
	class TroenGameBuilder;
	class SampleOSGViewer;
	class GameEventHandler;
	class GameLogic;
	class GameConfig;
	class CArguments;
	class Player;

	class BikeModel;
	class LevelModel;
	class FenceModel;
	class AbstractModel;
	class PhysicsWorld;
	class ItemModel;

	class BikeController;
	class LevelController;
	class FenceController;
	class AbstractController;
	class HUDController;
	class ItemController;


	class BikeView;
	class LevelView;
	class FenceView;
	class AbstractView;
	class HUDView;
	class ItemView;
	class SkyDome;
	class PostProcessing;
	class NodeFollowCameraManipulator;
	class ManipulatorController;
	class Reflection;
	class ResourcePool;
#ifdef USE_BENDEDVIEWS
	class SplineDeformationRendering;
#endif


	namespace util
	{
		class ChronoTimer;
		class GLDebugDrawer;
	}
	namespace input
	{
		class Keyboard;
		class Gamepad;
		class BikeInputState;
		class PollingDevice;
		class AI;
		class RemotePlayer;
	}

	namespace sound
	{
		class AudioManager;
	}
	
	namespace networking
	{
		struct bikeUpdateMessage;
		class NetworkPlayerInfo;
		class NetworkManager;
		class ClientManager;
		class ServerManager;
	}
}