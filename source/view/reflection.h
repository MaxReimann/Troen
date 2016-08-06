#pragma once
// OSG
#include <osg/ref_ptr>
#include <osg/Group>
#include <osg/ClipPlane>
#include <osg/ClipNode>

// STD
#include <vector>
// troen
#include "../forwarddeclarations.h"

#include "../omegascene.h"

namespace troen
{
	
	/*! The reflection class is responsible for rendering the scene into a texture and mapping this texture to the level floor.*/
	class Reflection : public RenderPassListener
	{
	public:
		Reflection(osg::ref_ptr<osg::Group> reflectSurface, osg::ref_ptr<osgViewer::View> gameView, osg::ref_ptr<osg::TextureCubeMap> cubeMap, int playerID);

		bool addSceneNode(osg::ref_ptr<osg::Group> sceneNode);
		void setReflectMode(bool mode);

		virtual osg::ref_ptr<osg::Group> getReflectionCameraGroup();

		virtual void onRender(omega::Renderer* client, const omega::DrawContext& context, omegaOsg::SceneView* scene);

	protected:
		osg::ref_ptr<osg::Group> m_node;

		osg::ref_ptr<osg::Group>			cameraGroup;
		osg::ref_ptr<osg::Camera>			m_reflectionCamera;
		osg::ref_ptr<osg::MatrixTransform>	reflectionTransform;
		osg::ref_ptr<osg::Group>			reflectionObjectsGroup;
		osg::ref_ptr<osg::ClipNode>			m_reflectionClipNode;

		osg::ref_ptr<osg::ClipPlane> m_ReflectionClipPlane;
		osg::ref_ptr<osg::NodeCallback> m_cameraCallback;

	};
}