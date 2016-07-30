#include "nodefollowcameramanipulator.h"
// OSG
#include <osg/PositionAttitudeTransform>
// troen
#include "../constants.h"
#include "../input/bikeinputstate.h"
#include <osg/Notify>

using namespace troen;



#define OOSG_VEC3(v) osg::Vec3(v[0], v[1], v[2])
#define OSGVEC3_OMEGA(v) omega::Vector3f( v.x() , v.y(), v.z() )
#define P_OSGVEC(v)  std::cout << v.x() << " " << v.y() << " " << v.z() << std::endl;




 NodeFollowCameraManipulator::NodeFollowCameraManipulator() : osgGA::NodeTrackerManipulator(), m_omegaRequest(false) {
 	// setVerticalAxisFixed(true);
 }

osg::Matrixd NodeFollowCameraManipulator::getMatrix() const
{
	NodeTrackerManipulator();
	osg::Vec3d nodeCenter;       
	osg::Quat nodeRotation;
	computeNodeCenterAndRotation(nodeCenter, nodeRotation);
	return osg::Matrixd::translate(0.0, 0.0, _distance)*osg::Matrixd::rotate(_rotation)*osg::Matrixd::rotate(nodeRotation)*osg::Matrix::translate(nodeCenter);
}


osg::Matrixd NodeFollowCameraManipulator::getInverseMatrix() const
{
	osg::Vec3d nodeCenter;
	osg::Quat nodeRotation;
	computeNodeCenterAndRotation(nodeCenter, nodeRotation);
	return osg::Matrixd::translate(-nodeCenter)*osg::Matrixd::rotate(nodeRotation.inverse())*osg::Matrixd::rotate(_rotation.inverse())*osg::Matrixd::translate(0.0, 0.0, -_distance);
}


osg::Matrixd NodeFollowCameraManipulator::omegaGetInverseMatrix()
{
	osg::Vec3d nodeCenter;
	osg::Quat nodeRotation;
	m_omegaRequest = true;
	computeNodeCenterAndRotation(nodeCenter, nodeRotation);
	m_omegaRequest = false;
	return osg::Matrixd::translate(-nodeCenter)*osg::Matrixd::rotate(nodeRotation.inverse())*osg::Matrixd::rotate(_rotation.inverse())*osg::Matrixd::translate(0.0, 0.0, -_distance);
}


void NodeFollowCameraManipulator::setByMatrix(const osg::Matrixd& matrix)
{
	setByInverseMatrix(osg::Matrixd::inverse(matrix));
}

void NodeFollowCameraManipulator::setByInverseMatrix(const osg::Matrixd& matrix)
{
	osg::Vec3d eye, center, up;
	matrix.getLookAt(eye, center, up);

	_center = center; _center.z() = 0.0f;
	if (_node.valid())
		_distance = abs((_node->getBound().center() - eye).z());
	else
		_distance = abs((eye - center).length());
}

void NodeFollowCameraManipulator::setBikeInputState(osg::ref_ptr<input::BikeInputState> bikeInputState)
{
	m_bikeInputState = bikeInputState;
	m_oldPlayerViewingRotation.makeRotate(0, osg::Z_AXIS);
}

// // doc in parent
// void NodeFollowCameraManipulator::getTransformation( osg::Vec3d& eye, osg::Quat& rotation ) const
// {
//     eye = _center - _rotation * osg::Vec3d( 0., 0., -_distance );
//     rotation = _rotation;
// }

// // doc in parent
// void NodeFollowCameraManipulator::getTransformation( osg::Vec3d& eye, osg::Vec3d& center, osg::Vec3d& up ) const
// {
//     center = _center;
//     eye = _center + _rotation * osg::Vec3d( 0., 0., _distance );
//     up = _rotation * osg::Vec3d( 0., 1., 0. );
// }


void NodeFollowCameraManipulator::computeNodeCenterAndRotation(osg::Vec3d& nodeCenter, osg::Quat& nodeRotation) const
{
	osg::Matrixd localToWorld, worldToLocal;

	computeNodeLocalToWorld(localToWorld);
	computeNodeWorldToLocal(worldToLocal);

	// center
	osg::NodePath nodePath;
	if (_trackNodePath.getNodePath(nodePath) && !nodePath.empty())
	{
		nodeCenter = osg::Vec3d(nodePath.back()->getBound().center())*localToWorld + CAMERA_POSITION_OFFSET;
	}
	else
	{
		nodeCenter = osg::Vec3d(0.0f, 0.0f, 0.0f)*localToWorld;
	}

	// std::cout << "node Center:" << std::endl;
	// P_OSGVEC(nodeCenter);

	// rotation
	osg::Matrixd coordinateFrame = getCoordinateFrame(nodeCenter);
	osg::Matrixd localToFrame(localToWorld*osg::Matrixd::inverse(coordinateFrame));

	osg::Quat nodeYawRelToFrame, nodePitchRelToFrame, nodeRollRelToFrame;
	osg::Quat rotationOfFrame;

	double yaw = atan2(-localToFrame(0, 1), localToFrame(0, 0));
	nodeYawRelToFrame.makeRotate(-yaw + CAMERA_ROTATION_OFFSET, osg::Z_AXIS);
	
	double roll = atan2(-localToFrame(0, 2), sqrt(pow(localToFrame(1, 2), 2) + pow(localToFrame(2, 2), 2)));
	nodeRollRelToFrame.makeRotate(roll / CAMERA_TILT_FACTOR,osg::Y_AXIS);

	osg::Quat playerViewingRotation;
	float angle = m_bikeInputState->getViewingAngle();
	playerViewingRotation.makeRotate(angle, osg::Z_AXIS);

	// do spherical-linear interpolation (slerp) between the old and new viewing direction
	playerViewingRotation.slerp(BIKE_VIEWING_ANGLE_DAMPENING_TERM, m_oldPlayerViewingRotation, playerViewingRotation);
	m_oldPlayerViewingRotation = playerViewingRotation;

	// jd: camera pitch rotation not wanted so far, maybe useful for later
	//double pitch = atan2(localToFrame(1, 2), localToFrame(2, 2));
	//nodePitchRelToFrame.makeRotate(pitch, osg::X_AXIS);
	
	rotationOfFrame = coordinateFrame.getRotate();
	nodeRotation = playerViewingRotation*nodeRollRelToFrame*nodeYawRelToFrame*rotationOfFrame;
}

void NodeFollowCameraManipulator::setEventAdapter(EventAdapter *eventAdapter)
{
	_eventAdapter = eventAdapter;
	_eventAdapter->configOptions->fixVerticalAxis = getVerticalAxisFixed();
	_eventAdapter->configOptions->flags = _flags;
}




bool NodeFollowCameraManipulator::handleEvent(const omega::Event &event) {
	if(event.isProcessed()) return false;

	osg::ref_ptr<osgGA::GUIEventAdapter> ea = _eventAdapter->bridge(event);
	setNewOptions();
	

    switch( ea->getEventType() )
    {
        case osgGA::GUIEventAdapter::MOVE:
            return handleMouseMove( event );
            break;

        case osgGA::GUIEventAdapter::DRAG:
            return handleMouseDrag( event );
            break;

        case osgGA::GUIEventAdapter::PUSH:
            return handleMousePush( event );
            break;

        case osgGA::GUIEventAdapter::RELEASE:
            return handleMouseRelease( event );
            break;

        case osgGA::GUIEventAdapter::KEYDOWN:
            return handleKeyDown( event );
            break;

        case osgGA::GUIEventAdapter::KEYUP:
            return handleKeyUp( event );
            break;

        case osgGA::GUIEventAdapter::SCROLL:
            if( _flags & PROCESS_MOUSE_WHEEL )
            return handleMouseWheel(event);
            else
            return false;

        default:
            return false;
    }
    return true;
}

bool NodeFollowCameraManipulator::handleMousePush(const omega::Event &event) {
	flushMouseEventStack();
	addMouseEvent( event );

	_thrown = false;
	return true;
}


bool NodeFollowCameraManipulator::handleKeyDown(const omega::Event &event) {
   osg::ref_ptr<osgGA::GUIEventAdapter> ea = _eventAdapter->bridge(event);
   // if( ea->getKey() == osgGA::GUIEventAdapter::KEY_Space )
   //  {
   //      flushMouseEventStack();
   //      _thrown = false;
   //      home(0);
   //      return true;
   //  }

    return false;
}

bool NodeFollowCameraManipulator::handleKeyUp(const omega::Event &event) {
	return true;
}

void NodeFollowCameraManipulator::addMouseEvent(const omega::Event &event){
    // osg guieventdapaters of parent class
	_ga_t1 = _ga_t0;
	_ga_t0 = _eventAdapter->bridge(event);
}


/// Handles GUIEventAdapter::RELEASE event.
bool NodeFollowCameraManipulator::handleMouseRelease(const omega::Event &event){
	osg::ref_ptr<osgGA::GUIEventAdapter> ea = _eventAdapter->bridge(event);
    // no mouse button is pressed
    if( ea->getButtonMask() == 0  )
    {

        double timeSinceLastRecordEvent = _ga_t0.valid() ? (event.getTimestamp() - _ga_t0->getTime()) : DBL_MAX;
        if( timeSinceLastRecordEvent > 0.02 )
            flushMouseEventStack();

        if( isMouseMoving() )
        {
            if( performMovement() && _allowThrow )
            {
                _thrown = true;
            }

            return true;
        }
    }

    flushMouseEventStack();
    addMouseEvent( event );
    performMovement();

    _thrown = false;

    return true;
}


bool NodeFollowCameraManipulator::handleMouseWheel(const omega::Event &event) {
	    // int wheel = event.getExtraDataInt(0);
	osg::ref_ptr<osgGA::GUIEventAdapter> ea = _eventAdapter->bridge(event);
	    osgGA::GUIEventAdapter::ScrollingMotion sm = ea->getScrollingMotion();

	// if( ((wheel < 0 && _wheelZoomFactor > 0.)) ||
	//         ((wheel > 0  && _wheelZoomFactor < 0.)) )
	//     {    

    // handle centering
    if( _flags & SET_CENTER_ON_WHEEL_FORWARD_MOVEMENT )
	{
        if( ((sm == osgGA::GUIEventAdapter::SCROLL_DOWN && _wheelZoomFactor > 0.)) ||
            ((sm == osgGA::GUIEventAdapter::SCROLL_UP   && _wheelZoomFactor < 0.)) )
        {

	        if( getAnimationTime() <= 0. )
	        {
	            // center by mouse intersection (no animation)
	            setCenterByMousePointerIntersection( event );
	        }

	    }
	}
    // mouse scroll up event
    if (sm == osgGA::GUIEventAdapter::SCROLL_UP)
    {
        // perform zoom
        zoomModel( _wheelZoomFactor, true );
        return true;
    } else {
        zoomModel( -_wheelZoomFactor, true );
        return true;
    }
}


bool NodeFollowCameraManipulator::handleMouseDrag(const omega::Event &event){
	addMouseEvent(event);
	performMovement();

	return true;
}


void NodeFollowCameraManipulator::setNewOptions(){
	setVerticalAxisFixed(_eventAdapter->configOptions->fixVerticalAxis);
	_flags = _eventAdapter->configOptions->flags;
}


/// The method processes events for manipulation based on relative mouse movement (mouse delta).
bool NodeFollowCameraManipulator::handleMouseDeltaMovement( const omega::Event &event )
{
	osg::ref_ptr<osgGA::GUIEventAdapter> ea = _eventAdapter->bridge(event);
    
    float dx = ea->getX() - _mouseCenterX;
    float dy = ea->getY() - _mouseCenterY;

    if( dx == 0.f && dy == 0.f )
        return false;

    addMouseEvent( event );
    

    _mouseCenterX = (ea->getXmin() + ea->getXmax()) / 2.0f;
	_mouseCenterY = (ea->getYmin() + ea->getYmax()) / 2.0f;

    return performMouseDeltaMovement( dx, dy );
}



/** The method sends a ray into the scene and the point of the closest intersection
    is used to set a new center for the manipulator. For Orbit-style manipulators,
    the orbiting center is set. For FirstPerson-style manipulators, view is pointed
    towards the center.*/
bool NodeFollowCameraManipulator::setCenterByMousePointerIntersection( const omega::Event &event)
{
	
	osg::ref_ptr<osgGA::GUIEventAdapter> ea = _eventAdapter->bridge(event);
    // prepare variables
    float x = ( ea->getX() - ea->getXmin() ) / ( ea->getXmax() - ea->getXmin() );
    float y = ( ea->getY() - ea->getYmin() ) / ( ea->getYmax() - ea->getYmin() );

    omega::SceneNode* scene = omega::Engine::instance()->getScene();
    omega::DisplaySystem* ds = omega::SystemManager::instance()->getDisplaySystem();
	omega::Ray r;
	bool res = ds->getViewRayFromEvent(event, r);

   	omega::Vector3f hitPoint;
    bool hit = scene->hit(r, &hitPoint, omega::SceneNode::HitBest);

    if (hit)
	{
		// omsg("intersection");
	}
	else
	{
		// omsg("no intersection");
		return false;
	}

    // get current transformation
    osg::Vec3d eye, oldCenter, up;
    getTransformation( eye, oldCenter, up );

    // new center
    osg::Vec3d newCenter = OOSG_VEC3(hitPoint);

    // make vertical axis correction
    if( getVerticalAxisFixed() )
    {

        osg::CoordinateFrame coordinateFrame = getCoordinateFrame( newCenter );
        osg::Vec3d localUp = getUpVector( coordinateFrame );

        fixVerticalAxis( newCenter - eye, up, up, localUp, true );

    }

    // set the new center
    setTransformation( eye, newCenter, up );

   _mouseCenterX = (ea->getXmin() + ea->getXmax()) / 2.0f;
   _mouseCenterY = (ea->getYmin() + ea->getYmax()) / 2.0f;

    return true;
}


void NodeFollowCameraManipulator::updateOmegaCamera(omega::Camera *cam){
	// return;
    osg::Vec3d eye, center, up, unused;

    // call same method, that osg internally uses for its camera updates
    osg::Matrixd invMatrix = getInverseMatrix();
	invMatrix.getLookAt(eye, center, up);

    // getTransformation(eye, center, up);
    
    // P_OSGVEC(center)
    // P_OSGVEC(up)
    // 

    // std::cout << cam->getParent() << std::endl;


    omega::Vector3f oPosVec(eye.x(), eye.y(), eye.z());
    omega::Vector3f oUpVec(up.x(), up.y(), up.z());
    // omega::Vector3f oUpVec(0,0,1);
    omega::Vector3f oCenterVec(center.x(), center.y(), center.z());

    //order is important here, setting lookat before position 
    // will result in choppy camera rotation
   	osg::NodePath nodePath;
   	 getTrackNodePath().getNodePath(nodePath);

    osg::Matrixd localToWorld = osg::computeLocalToWorld(nodePath, true);
    osg::Vec3d worldCenter = _center * localToWorld;

	// GameThread::getInstance()->getTroenGame()->getBikeNode()
    std::cout << "nodefollow center" << std::endl;
	P_OSGVEC(worldCenter);

    oCenterVec = OSGVEC3_OMEGA(worldCenter);// cam->convertWorldToLocalPosition(oCenterVec);// omega::Vector3f(20, 20, 0.584348);   //cam->convertWorldToLocalPosition(omega::Vector3f( worldCenter.x(), worldCenter.y(), worldCenter.z())); //omega::Vector3f( worldCenter.x(), worldCenter.y(), worldCenter.z()); // 
    cam->setPosition(oPosVec);
    cam->lookAt(oCenterVec, oUpVec);     
}

// ============================================================================

osg::ref_ptr<osgGA::GUIEventAdapter> EventAdapter::bridge(const omega::Event &event){
    osg::ref_ptr<osgGA::GUIEventAdapter> osgNewEvent = new osgGA::GUIEventAdapter;

    this->preMapping();

    int buttonMask = this->mapButton(event);
    osgNewEvent->setButtonMask(buttonMask);


    osgGA::GUIEventAdapter::EventType eventType = this->mapEventType(event);
    osgNewEvent->setEventType(eventType);


    omega::Vector2f xy = this->mapXY(event);
    osgNewEvent->setX(xy.x());
    osgNewEvent->setY(xy.y());


    omega::Vector4f inputRange = this->setInputRange(event);
    osgNewEvent->setInputRange(inputRange[0], inputRange[1], inputRange[2], inputRange[3]);

    osgGA::GUIEventAdapter::KeySymbol key = this->mapKeySymbol(event);
    if (key != osgGA::GUIEventAdapter::KEY_F35)
        osgNewEvent->setKey(key);


    if (eventType == osgGA::GUIEventAdapter::SCROLL)
    {
        osgGA::GUIEventAdapter::ScrollingMotion scrollingMotion = this->mapScrollingMotion(event);
        osgNewEvent->setScrollingMotion(scrollingMotion);        
    }

    // time in seconds
    osgNewEvent->setTime( event.getTimestamp() / 1000.0 );


    this->postMapping();

    return osgNewEvent;
}


omega::Vector2f MouseAdapter::mapXY(const omega::Event &event) {
    return omega::Vector2f(event.getPosition().x(), event.getPosition().y());
}

osgGA::GUIEventAdapter::KeySymbol MouseAdapter::mapKeySymbol(const omega::Event &event){
    // only handle home (spacebar for now)
    if (event.isKeyDown(omega::KC_HOME)) //omicron has no space bar key ??
        return osgGA::GUIEventAdapter::KEY_Space;
    else
        return osgGA::GUIEventAdapter::KEY_F35; // Theres no void event so use this useless key
}


omega::Vector4f MouseAdapter::setInputRange(const omega::Event &event){
    omega::DisplaySystem* ds = omega::SystemManager::instance()->getDisplaySystem();
    omega::Vector2i resolution = ds->getDisplayConfig().getCanvasRect().size();

    // osgNewEvent->setInputRange(0, 0, static_cast<float>(resolution[0]), static_cast<float>(resolution[1]) );
    return omega::Vector4f(0,0, static_cast<float>(resolution[0]), static_cast<float>(resolution[1]) );
}

osgGA::GUIEventAdapter::EventType MouseAdapter::mapEventType(const omega::Event &event)
{
    switch( event.getType() )
    {

        case omega::Event::Move:
            if ( event.isFlagSet(omega::Event::Right) ||
                 event.isFlagSet(omega::Event::Left) ||
                 event.isFlagSet(omega::Event::Middle))
                return osgGA::GUIEventAdapter::DRAG;
            else 
                return osgGA::GUIEventAdapter::MOVE;

        case omega::Event::Up:
            if (event.getServiceType() == omega::Service::Pointer)
                return osgGA::GUIEventAdapter::RELEASE;
            else 
                return osgGA::GUIEventAdapter::KEYUP;

        case omega::Event::Down:
            if (event.getServiceType() == omega::Service::Pointer)
                return osgGA::GUIEventAdapter::PUSH;
            else 
                return osgGA::GUIEventAdapter::KEYDOWN;
        
        case omega::Event::Zoom:
            return osgGA::GUIEventAdapter::SCROLL;

        default:
            return osgGA::GUIEventAdapter::NONE;
    }
}



int MouseAdapter::mapButton(const omega::Event &event) {
    int buttonMask = 0;

    if (event.isFlagSet(omega::Event::Left))
        buttonMask |= osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON;
    if (event.isFlagSet(omega::Event::Middle))
        buttonMask |= osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON;
    if (event.isFlagSet(omega::Event::Right))
        buttonMask |= osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON;


    return buttonMask;
}


osgGA::GUIEventAdapter::ScrollingMotion MouseAdapter::mapScrollingMotion(const omega::Event &event) {
    int wheel = event.getExtraDataInt(0);
    // mouse scroll up event
    if (wheel > 0)
        return osgGA::GUIEventAdapter::SCROLL_UP;
    else
        return osgGA::GUIEventAdapter::SCROLL_DOWN;
}


//---------------------------------------------------------------------------------------------------


ManipulatorController::ManipulatorController(omega::Camera *cam) : omega::CameraController("ManipulatorController") 
{
	if (cam == NULL)
        myCamera = omega::Engine::instance()->getDefaultCamera();
    else
        myCamera = cam;

    myCamera->setController(this);
}


void ManipulatorController::handleEvent(const omega::Event& event)
{
    if (myManipulator==NULL)
        return;

    myManipulator->handleEvent(event);
}



void ManipulatorController::update(const omega::UpdateContext& context)
{
    if(!isEnabled() || myManipulator==NULL) return;



    if (myCamera == NULL) {
    	std::cout << "warning: camera in manipulatorcontroller is null. skipping update" << std::endl;
    	return;
    }


    myManipulator->updateOmegaCamera(myCamera);
}



void ManipulatorController::setManipulator(NodeFollowCameraManipulator* manipulator){
    myManipulator = manipulator;


    // myManipulator->setNode(node);
    
    // default, can be overwritten by setEventAdapter
    MouseAdapter* adapter = new MouseAdapter;
    myManipulator->setEventAdapter(adapter);
}


void ManipulatorController::setEventAdapter(EventAdapter* adapter){
    myManipulator->setEventAdapter(adapter);
}


ManipulatorController* ManipulatorController::create(){
    return new ManipulatorController();
}

// ManipulatorController::~ManipulatorController(){
// 	throw std::exception();

// }
