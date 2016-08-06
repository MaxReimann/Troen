#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgDB/ReadFile>
#include <osg/Geode>
#include <osg/ArgumentParser>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/TexGen>
#include <osg/TexMat>
#include <osg/TextureCubeMap>
#include <osg/Program>
#include <osg/MatrixTransform>
#include <osg/Shader>
#include <osg/io_utils>
#include <osg/Uniform>
#include <osg/Camera>
#include <osg/LightModel>
#include <osg/ClipNode>
#include <osg/ClipPlane>


#define OMEGA_NO_GL_HEADERS
#include <omega.h>
#include <omegaToolkit.h>
#include <omegaOsg/omegaOsg.h>

//troen
#include "reflection.h"
#include "shaders.h"
#include "skydome.h"
#include "../constants.h"
/*
**
*/
using namespace troen;


static osg::ref_ptr<osg::Uniform> g_cameraEyeU = new osg::Uniform("cameraEye", osg::Vec3(0.0, 0.0,0.0));

class FindNamedNode : public osg::NodeVisitor
{
public:
	FindNamedNode(const std::string& name)
		: osg::NodeVisitor( // Traverse all children.
		osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
		_name(name) {
			_node = new osg::Group();
		}

	// This method gets called for every node in the scene
	//   graph. Check each node to see if its name matches
	//   out target. If so, save the node's address.
	virtual void apply(osg::Node& node)
	{
		if (node.getName() == _name)
			_node->addChild(&node);

		// Keep traversing the rest of the scene graph.
		traverse(node);
	}

	osg::Node* getNode() { return _node.get(); }

protected:
	std::string _name;
	osg::ref_ptr<osg::Group> _node;
};



inline osg::Matrix buildOsgMatrix( const omega::Matrix4f& matrix )
{
    return osg::Matrix( matrix( 0, 0 ), matrix( 1, 0 ),
                        matrix( 2, 0 ), matrix( 3, 0 ),
                        matrix( 0, 1 ), matrix( 1, 1 ),
                        matrix( 2, 1 ), matrix( 3, 1 ),
                        matrix( 0, 2 ), matrix( 1, 2 ),
                        matrix( 2, 2 ), matrix( 3, 2 ),
                        matrix( 0, 3 ), matrix( 1, 3 ),
                        matrix( 2, 3 ), matrix( 3, 3 ));
}




class CUpdateCameraCallback : public osg::NodeCallback
{
public:	
	CUpdateCameraCallback() : NodeCallback()
	{
	}

	void setContext(const omega::DrawContext* context)
	{
		m_context = context;
	}

	void operator()(osg::Node *node, osg::NodeVisitor *nv)
	{
		if (nv->getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
		{
			osg::Camera	*camera = dynamic_cast<osg::Camera *>(node->asGroup()->getChild(0));

			if (camera != NULL && m_context != NULL)
			{
				// osg::Matrixd modelView =  buildOsgMatrix( m_omegaCam->getViewTransform().matrix() );
				osg::Matrixd modelView =  buildOsgMatrix( m_context->modelview.matrix() );
				osg::Matrixd projection = buildOsgMatrix(m_context->projection.matrix());
				// osg::Matrixd modelView = buildOsgMatrix(context.modelview.matrix());

				// m_reflectionCamera->setViewMatrix(modelView);
				camera->setProjectionMatrix(projection);
				camera->setViewMatrix(modelView);

				omega::Vector3f pos = m_context->camera->getPosition();
				osg::Vec3 camPos(pos.x(), pos.y(), pos.z());

				g_cameraEyeU->set(camPos);

				// g_cameraEyeU->set(osg::Vec3(0.0, 0.0, 0.0) * m_gameView->getCamera()->getInverseViewMatrix());

				//_m_reflectionCamera->accept(*_cv);
			}

		}
		this->traverse(node, nv);
	}

protected:
	const omega::DrawContext* m_context;
};



Reflection::Reflection(osg::ref_ptr<osg::Group> levelView, osg::ref_ptr<osgViewer::View> gameView, osg::ref_ptr<osg::TextureCubeMap> cubeMap, int playerID )

{
	int texSize = 1024;
	// Set up the reflection camera
	cameraGroup = new osg::Group();
	m_reflectionCamera = new osg::Camera();
	reflectionTransform = new osg::MatrixTransform();



	m_reflectionCamera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	m_reflectionCamera->setRenderOrder(osg::Camera::PRE_RENDER);
	m_reflectionCamera->setClearColor(osg::Vec4(0.0f, 0.f, 0.0f, 0.0f));
	m_reflectionCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	m_reflectionCamera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
	m_reflectionCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	m_reflectionCamera->setViewport(0, 0, texSize, texSize);
	m_reflectionCamera->setClearDepth(1.0);
	m_reflectionCamera->setCullMask(CAMERA_MASK_MAIN);
	m_reflectionCamera->getOrCreateStateSet()->addUniform(new osg::Uniform("isReflecting",true));
	


	m_cameraCallback = new CUpdateCameraCallback();
	cameraGroup->setCullCallback(m_cameraCallback);

	//beware, that vec4(a,b,c,d) is a plane equation with a,b,c the plane normal and d the plane height
	m_ReflectionClipPlane = new osg::ClipPlane(0, osg::Vec4d(0.0, 0.0, 1.0, 0.0));
	m_reflectionClipNode = new osg::ClipNode;
	m_reflectionClipNode->addClipPlane(m_ReflectionClipPlane);


	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D();
	texture->setTextureSize(texSize, texSize);
	texture->setInternalFormat(GL_RGBA);
	texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);


	//osg::ref_ptr<osg::Texture2D>	idTexture = new osg::Texture2D();
	//idTexture->setTextureSize(texSize, texSize);
	m_reflectionCamera->attach((osg::Camera::BufferComponent) osg::Camera::COLOR_BUFFER0, texture);
	//m_reflectionCamera->attach((osg::Camera::BufferComponent) osg::Camera::COLOR_BUFFER1, idTexture);

	m_reflectionCamera->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
	reflectionTransform->setMatrix(osg::Matrix::scale(1.0, 1.0, -1.0));


	cameraGroup->addChild(m_reflectionCamera);
	m_reflectionCamera->addChild(reflectionTransform);
	reflectionTransform->addChild(m_reflectionClipNode);


	// Set reflection textures
	osg::ref_ptr<osg::Node> reflectSurface;
	FindNamedNode findReflecting("floorsNode");
	levelView->accept(findReflecting);
	reflectSurface = static_cast<osg::Group*>(findReflecting.getNode())->getChild(0);

	reflectSurface->getOrCreateStateSet()->setTextureAttributeAndModes(5 + playerID, texture,
		osg::StateAttribute::ON);

	reflectSurface->getOrCreateStateSet()->setAttributeAndModes(shaders::m_allShaderPrograms[shaders::GRID_NOREFLECTION], osg::StateAttribute::OFF);
	reflectSurface->getOrCreateStateSet()->setAttributeAndModes(shaders::m_allShaderPrograms[shaders::GRID], osg::StateAttribute::ON);

	reflectSurface->getOrCreateStateSet()->setTextureAttributeAndModes(2, cubeMap, osg::StateAttribute::ON);

	reflectSurface->getOrCreateStateSet()->addUniform(new osg::Uniform("skyDome", 2));

	reflectSurface->getOrCreateStateSet()->addUniform(g_cameraEyeU);

	//reflectSurface->getOrCreateStateSet()->setTextureAttributeAndModes(3, idTexture, osg::StateAttribute::ON);
	//reflectSurface->getOrCreateStateSet()->addUniform(new osg::Uniform("reflectionAttrib", 3));

}


void Reflection::onRender(omega::Renderer* client, const omega::DrawContext& context, omegaOsg::SceneView* scene)
{
	// osg::Matrixd projection = buildOsgMatrix(context.projection.matrix());
	// osg::Matrixd modelView = buildOsgMatrix(context.modelview.matrix());

	// m_reflectionCamera->setViewMatrix(modelView);
	// m_reflectionCamera->setProjectionMatrix(projection);
	// static_cast<CUpdateCameraCallback*>(m_cameraCallback.get())->setCamera(context.camera);
	static_cast<CUpdateCameraCallback*>(m_cameraCallback.get())->setContext(&context);
}


bool Reflection::addSceneNode(osg::ref_ptr<osg::Group> sceneNode)
{
	reflectionObjectsGroup = new osg::Group();
	char *reflectionNames[] = { "wallsGroup", "obstaclesGroup", "bikeGroup","fenceGroup", "itemGroup"};

	for (auto name : reflectionNames )
	{
		FindNamedNode findReflecting(name);
		sceneNode->accept(findReflecting);
		reflectionObjectsGroup->addChild(findReflecting.getNode());

	}

	m_reflectionClipNode->addChild(reflectionObjectsGroup);

	return true;

}

osg::ref_ptr<osg::Group> Reflection::getReflectionCameraGroup()
{
	return cameraGroup;
}
