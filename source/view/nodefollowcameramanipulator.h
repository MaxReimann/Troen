#pragma once
// OSG
#include <osgGA/NodeTrackerManipulator>
#include <osg/Matrixd>
// troen
#include "../forwarddeclarations.h"
#include "omega.h"

#define OMEGA_NO_GL_HEADERS
#include <omega.h>

namespace troen
{




    class ConfigurableOptions {
    public:
    	ConfigurableOptions() {};
    	bool fixVerticalAxis;
    	int flags;
    };

	class EventAdapter {
		public:
			EventAdapter() {
				configOptions = std::make_shared<ConfigurableOptions>();
			}
			virtual osg::ref_ptr<osgGA::GUIEventAdapter> bridge(const omega::Event &event);
			virtual int mapButton(const omega::Event &event) = 0;
			virtual omega::Vector2f mapXY(const omega::Event &event) = 0;
			virtual omega::Vector4f setInputRange(const omega::Event &event) = 0;
			virtual osgGA::GUIEventAdapter::ScrollingMotion mapScrollingMotion(const omega::Event &event) = 0;
			virtual osgGA::GUIEventAdapter::EventType mapEventType(const omega::Event &event) = 0;
			virtual osgGA::GUIEventAdapter::KeySymbol mapKeySymbol(const omega::Event &event) = 0;


			virtual void preMapping() {}
			virtual void postMapping() {}

			std::shared_ptr<ConfigurableOptions> configOptions;
	};
	

	class MouseAdapter : public EventAdapter {
		public:
			MouseAdapter() : EventAdapter() {}
			// virtual osg::ref_ptr<osgGA::GUIEventAdapter> bridge(Event *event);
			virtual int mapButton(const omega::Event &event);
			virtual omega::Vector2f mapXY(const omega::Event &event);
			virtual omega::Vector4f setInputRange(const omega::Event &event);
			virtual osgGA::GUIEventAdapter::ScrollingMotion mapScrollingMotion(const omega::Event &event);
			virtual osgGA::GUIEventAdapter::EventType mapEventType(const omega::Event &event);
			virtual osgGA::GUIEventAdapter::KeySymbol mapKeySymbol(const omega::Event &event);
	};


	/*! The NodeFollowCameraManipulator is a osg::NodeTrackerManipulator and is used to point the camera always to the bike of the current player.*/
	class NodeFollowCameraManipulator : public osgGA::NodeTrackerManipulator
	{
	public:
		NodeFollowCameraManipulator() : osgGA::NodeTrackerManipulator() {}

		virtual osg::Matrixd getMatrix() const;
		virtual osg::Matrixd getInverseMatrix() const;
		virtual void setByMatrix(const osg::Matrixd& matrix);
		virtual void setByInverseMatrix(const osg::Matrixd& matrix);

		virtual void computeNodeCenterAndRotation(osg::Vec3d& nodeCenter, osg::Quat& nodeRotation) const;

		virtual void setBikeInputState(osg::ref_ptr<input::BikeInputState> bikeInputState);
		
		bool handleEvent(const omega::Event &event);
		bool handleMousePush(const omega::Event &event);
		bool handleKeyDown(const omega::Event &event);
		bool handleKeyUp(const omega::Event &event);
		void addMouseEvent(const omega::Event &event);
		bool handleMouseRelease(const omega::Event &event);
		bool handleMouseWheel(const omega::Event &event);
		bool handleMouseDrag(const omega::Event &event);
		bool handleMouseMove(const omega::Event &event) {return false;}
		void setNewOptions();
		bool handleMouseDeltaMovement( const omega::Event &event );
		bool setCenterByMousePointerIntersection( const omega::Event &event);

		void setEventAdapter(EventAdapter *eventAdapter);

		void updateOmegaCamera(omega::Camera *cam);

		// virtual void getTransformation( osg::Vec3d& eye, osg::Quat& rotation ) const;
		// virtual void getTransformation( osg::Vec3d& eye, osg::Vec3d& center, osg::Vec3d& up ) const;



	protected:
		osg::Vec3 rollPitchYaw(float x, float y, float z, float w) const;
		osg::ref_ptr<input::BikeInputState> m_bikeInputState;
		mutable osg::Quat m_oldPlayerViewingRotation;

		EventAdapter* _eventAdapter;

		
	};




	class ManipulatorController: public omega::CameraController
    {
    public:
    	static ManipulatorController* create();


		ManipulatorController(omega::Camera *cam = NULL);

		// ~ManipulatorController();

		// void onEvent(const omega::Event* event);
		//sets the manipulator to the node, if node is NULL, the manipualtor is set to the root scene node
		void setManipulator(NodeFollowCameraManipulator* manipulator, osg::Node* node = NULL);
		void setEventAdapter(EventAdapter *eventAdapter);
		// void setManipulator(NodeTrackerManipulator* manipulator);

		// events should be only handled by callng onEvent (explicit callback)
		// this enables the python subclasses to process events
		virtual void handleEvent(const omega::Event& evt);
		virtual void update(const omega::UpdateContext& context);

	protected:
		NodeFollowCameraManipulator* myManipulator;
		omega::Camera *myCamera;
    };



}