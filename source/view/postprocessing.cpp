#include "postprocessing.h"
//osg
#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/TexGen>
#include <osg/TextureCubeMap>
#include <osg/TexMat>
#include <osg/Material>
#include <osg/Geode>
#include <osgDB/WriteFile>
#include <osg/CullFace>
#include <osg/TexGenNode>
#include <osgUtil/CullVisitor>
#include <osgGA/StateSetManipulator>
#include <osgDB/FileNameUtils>
#include <stdio.h>
// troen
#include "shaders.h"

using namespace troen;

// ugly but convenient global statics for shaders    
static osg::ref_ptr<osg::Uniform> g_nearFarUniform;

PostProcessing::PostProcessing(osg::ref_ptr<osg::Group> rootNode, int width, int height)
:m_root(rootNode), m_width(width), m_height(height), m_sceneNode(new osg::Group())
{
	// init textures, will be recreated when screen size changes
	
	setupTextures(m_width, m_height);

	// create shaders
	shaders::reloadShaders();

	////////////////////////////////////////
	// Multi pass rendering and Ping Pong //
	////////////////////////////////////////

	// 1. gBuffer pass: render color, normal&depth, id buffer
	unsigned int pass = 0;
	m_allCameras.push_back(gBufferPass()); 
	m_root->addChild(m_allCameras[pass++]);
	
	// 2. prepare pass: render id buffer as seeds into PONG texture
	TEXTURE_CONTENT pingPong[] = { PING, PONG };
	// start writing into PONG buffer (pass == 1 )

	m_allCameras.push_back(pingPongPass(pass, COLOR, PONG, shaders::SELECT_GLOW_OBJECTS, -1.0));
	m_root->addChild(m_allCameras[pass++]);
	
	m_allCameras.push_back(pingPongPass(pass, PONG, PING, shaders::HBLUR, -1.0));
	m_root->addChild(m_allCameras[pass++]);
		
	m_allCameras.push_back(pingPongPass(pass, PING, PONG, shaders::VBLUR, -1.0));
	m_root->addChild(m_allCameras[pass++]);

	m_allCameras.push_back(postProcessingPass());
	m_root->addChild(m_allCameras[m_allCameras.size() - 1]);
}

// sets up textures
void PostProcessing::setupTextures(const unsigned int & width, const unsigned int &height)
{
	//////////////////////////////////////////////////////////////////////////
	// 2D textures as render targets
	//////////////////////////////////////////////////////////////////////////

	// store color, normal & Depth, id in textures
	m_fboTextures.resize(TEXTURE_CONTENT_SIZE);
	for (int i = 0; i<m_fboTextures.size(); i++)
	{
		// only create textures on first run
		if (!m_fboTextures[i])  
			m_fboTextures[i] = new osg::Texture2D();
		
		
		if (false && (i == PING || i == PONG)) {
			m_fboTextures[i]->setTextureWidth(width / 2);
			m_fboTextures[i]->setTextureHeight(height / 2);
		} else {
			m_fboTextures[i]->setTextureWidth(width);
			m_fboTextures[i]->setTextureHeight(height);
		}

		// higher resolution
		m_fboTextures[i]->setInternalFormat(GL_RGBA32F_ARB);
		m_fboTextures[i]->setSourceFormat(GL_RGBA);
		m_fboTextures[i]->setSourceType(GL_FLOAT);
		m_fboTextures[i]->setBorderWidth(0);
		m_fboTextures[i]->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
		m_fboTextures[i]->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
		m_fboTextures[i]->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
		m_fboTextures[i]->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);
		m_fboTextures[i]->setBorderColor(osg::Vec4d(0.0, 0.0, 0.0, 1.0));
		m_fboTextures[i]->setResizeNonPowerOfTwoHint(true);

		// important to reflect the change in size
		m_fboTextures[i]->dirtyTextureObject();
	}

	// important to reflect the change in size for the FBO
	if (m_allCameras.size() > 0)
	{
		for (size_t i = 0, iEnd = m_allCameras.size(); i<iEnd; i++)
		{
			m_allCameras[i]->setRenderingCache(0);
		}
	}
}


class NearFarCallback : public osg::NodeCallback
{
	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		traverse(node, nv);

		osgUtil::CullVisitor * cv = dynamic_cast<osgUtil::CullVisitor*> (nv);
		if (cv)
		{
			double n = cv->getCalculatedNearPlane();
			double f = cv->getCalculatedFarPlane();

			osg::Matrixd m = *cv->getProjectionMatrix();
			cv->clampProjectionMatrix(m, n, f);

			if (n != m_oldNear || f != m_oldFar)
			{
				m_oldNear = n;
				m_oldFar = f;
				g_nearFarUniform->set(osg::Vec2(n, f));
			}
		}
	}

private:
	double m_oldNear;
	double m_oldFar;
};

// create gbuffer creation camera
osg::ref_ptr<osg::Camera> PostProcessing::gBufferPass()
{

	osg::ref_ptr<osg::Camera> cam = new osg::Camera();
	// output textures
	cam->attach((osg::Camera::BufferComponent)(osg::Camera::COLOR_BUFFER0 + COLOR), m_fboTextures[COLOR]);
	cam->attach((osg::Camera::BufferComponent)(osg::Camera::COLOR_BUFFER0 + NORMALDEPTH), m_fboTextures[NORMALDEPTH]);
	cam->attach((osg::Camera::BufferComponent)(osg::Camera::COLOR_BUFFER0 + ID), m_fboTextures[ID]);

	// Configure fboCamera to draw fullscreen textured quad
	// black clear color
	cam->setClearColor(osg::Vec4(0.0, 0.0, 0.5, 1.0));
	cam->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

	cam->setReferenceFrame(osg::Camera::RELATIVE_RF);
	cam->setRenderOrder(osg::Camera::PRE_RENDER, 0);

	// need to know about near far changes for correct depth 
	cam->setCullCallback(new NearFarCallback());
	cam->addChild(m_sceneNode);

	// attach shader program
	cam->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
	cam->getOrCreateStateSet()->addUniform(new osg::Uniform("colorTex", COLOR));
	cam->getOrCreateStateSet()->setAttributeAndModes(shaders::m_allShaderPrograms[shaders::GBUFFER], osg::StateAttribute::ON);
	cam->getOrCreateStateSet()->setTextureAttributeAndModes(COLOR, m_fboTextures[COLOR], osg::StateAttribute::ON);
	cam->getOrCreateStateSet()->setTextureAttributeAndModes(NORMALDEPTH, m_fboTextures[NORMALDEPTH], osg::StateAttribute::ON);
	cam->getOrCreateStateSet()->setTextureAttributeAndModes(ID, m_fboTextures[ID], osg::StateAttribute::ON);

	g_nearFarUniform = new osg::Uniform("nearFar", osg::Vec2(0.0, 1.0));
	cam->getOrCreateStateSet()->addUniform(g_nearFarUniform);

	return cam;
}

class timeUpdate : public osg::Uniform::Callback
{
public:
	virtual void operator()
		(osg::Uniform* uniform, osg::NodeVisitor* nv)
	{
		float time = nv->getFrameStamp()->getReferenceTime();
		uniform->set(time);
	}
};


// create skeleton creation camera 
osg::ref_ptr<osg::Camera> PostProcessing::pingPongPass(int order, TEXTURE_CONTENT inputTexture, TEXTURE_CONTENT outputTexture, int shader, int step)
{
	osg::ref_ptr<osg::Camera> camera(new osg::Camera());

	// output textures	
	camera->attach((osg::Camera::BufferComponent) (osg::Camera::COLOR_BUFFER0), m_fboTextures[outputTexture]);

	// Configure fboCamera to draw fullscreen textured quad
	camera->setClearColor(osg::Vec4(0.0, float(float(order) / float(m_allCameras.size())), 0.0, 1.0));
	camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

	camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
	camera->setRenderOrder(osg::Camera::PRE_RENDER, order);

	// geometry
	osg::Geode* geode(new osg::Geode());
	geode->addDrawable(osg::createTexturedQuadGeometry(osg::Vec3(-1, -1, 0), osg::Vec3(2, 0, 0), osg::Vec3(0, 2, 0)));
	geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	camera->addChild(geode);

	// attach shader program
	osg::ref_ptr<osg::StateSet>	state = camera->getOrCreateStateSet();
	state->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);

	state->setAttributeAndModes(shaders::m_allShaderPrograms[shader], osg::StateAttribute::ON);

	// add sampler textures	
	state->addUniform(new osg::Uniform("inputLayer", inputTexture));
	state->addUniform(new osg::Uniform("idLayer", ID));
	if (step != -1) state->addUniform(new osg::Uniform("currentStep", step));


	// add time uniform
	osg::Uniform* timeU = new osg::Uniform("time", 0.f);
	state->addUniform(timeU);
	timeU->setUpdateCallback(new timeUpdate());

	state->setTextureAttributeAndModes(inputTexture, m_fboTextures[inputTexture], osg::StateAttribute::ON);
	state->setTextureAttributeAndModes(ID, m_fboTextures[ID], osg::StateAttribute::ON);

	return camera.release();
}


// create post processing pass to put it all together
osg::ref_ptr<osg::Camera> PostProcessing::postProcessingPass()
{
	osg::ref_ptr<osg::Camera> postRenderCamera(new osg::Camera());

	// input textures
	postRenderCamera->attach((osg::Camera::BufferComponent) (osg::Camera::COLOR_BUFFER0 + COLOR), m_fboTextures[COLOR]);
	postRenderCamera->attach((osg::Camera::BufferComponent) (osg::Camera::COLOR_BUFFER0 + NORMALDEPTH), m_fboTextures[NORMALDEPTH]);
	postRenderCamera->attach((osg::Camera::BufferComponent) (osg::Camera::COLOR_BUFFER0 + ID), m_fboTextures[ID]);
	postRenderCamera->attach((osg::Camera::BufferComponent) (osg::Camera::COLOR_BUFFER0 + PONG), m_fboTextures[PONG]);

	// configure postRenderCamera to draw fullscreen textured quad
	postRenderCamera->setClearColor(osg::Vec4(0.0, 0.5, 0.0, 1)); // should never see this.
	postRenderCamera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
	postRenderCamera->setRenderOrder(osg::Camera::POST_RENDER);

	// geometry
	osg::Geode* geode(new osg::Geode());
	geode->addDrawable(osg::createTexturedQuadGeometry(osg::Vec3(-1, -1, 0), osg::Vec3(2, 0, 0), osg::Vec3(0, 2, 0)));
	/*geode->getOrCreateStateSet()->setTextureAttributeAndModes(COLOR, m_fboTextures[COLOR], osg::StateAttribute::ON);
	geode->getOrCreateStateSet()->setTextureAttributeAndModes(NORMALDEPTH, m_fboTextures[NORMALDEPTH], osg::StateAttribute::ON);
	geode->getOrCreateStateSet()->setTextureAttributeAndModes(ID, m_fboTextures[ID], osg::StateAttribute::ON);
	geode->getOrCreateStateSet()->setTextureAttributeAndModes(PONG, m_fboTextures[PONG], osg::StateAttribute::ON);*/
	geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	postRenderCamera->addChild(geode);

	// attach shader program
	osg::ref_ptr<osg::StateSet>	state = postRenderCamera->getOrCreateStateSet();
	state->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);

	state->setAttributeAndModes(shaders::m_allShaderPrograms[shaders::POST_PROCESSING], osg::StateAttribute::ON);

	// add samplers	
	state->addUniform(new osg::Uniform("sceneLayer", COLOR));
	state->addUniform(new osg::Uniform("normalDepthLayer", NORMALDEPTH));
	state->addUniform(new osg::Uniform("idLayer", ID));
	state->addUniform(new osg::Uniform("pongLayer", PONG));

	state->setTextureAttributeAndModes(COLOR, m_fboTextures[COLOR], osg::StateAttribute::ON);
	state->setTextureAttributeAndModes(NORMALDEPTH, m_fboTextures[NORMALDEPTH], osg::StateAttribute::ON);
	state->setTextureAttributeAndModes(ID, m_fboTextures[ID], osg::StateAttribute::ON);
	state->setTextureAttributeAndModes(PONG, m_fboTextures[PONG], osg::StateAttribute::ON);

	return postRenderCamera;
}

bool PostProcessing::handleGuiEvents(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&, osg::Object*, osg::NodeVisitor*)
{
	if (ea.getEventType() == osgGA::GUIEventAdapter::RESIZE)
	{
		// re setup textures to new size
		setupTextures(ea.getWindowWidth(), ea.getWindowHeight());
		return true;
	}
	else return false;
}



