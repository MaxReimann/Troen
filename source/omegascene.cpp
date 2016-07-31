#include "omegascene.h"
#include <osgDB/WriteFile>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include "gamethread.h"
#include "troengame.h"
#include <omegaOsg/omegaOsg/SceneView.h>
#include <iostream>
#include <cstdlib>

using namespace troen;



class RenderPassListener : public omegaOsg::IOsgRenderPassListener
{
public:
    virtual void onFrameFinished(omega::Renderer* client, const omega::DrawContext& context, omegaOsg::SceneView* scene){

        //fix culling
        double fovy, aspect, znear, zfar;
        scene->getCamera()->getProjectionMatrixAsPerspective(fovy, aspect, znear, zfar);
        scene->getCamera()->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
        znear = 1.0;
        scene->getCamera()->setProjectionMatrixAsPerspective(fovy, aspect, znear, zfar);
    }
};


///////////////////////////////////////////////////////////////////////////////
void TroenOmegaScene::initialize()
{
    std::cout << "[TroenOmegaScene::initialize]" << std::endl;

    registerCorrectPath();

    myRenderPassListener = std::make_shared<RenderPassListener>();

    m_troenGame = GameThread::getInstance()->getTroenGame();

    m_troenGame->prepareGame(*GameThread::getInstance()->getGameConfig());


    // Load osg object
    if(omega::SystemManager::settingExists("config/scene"))
    {
        omega::Setting& sscene = omega::SystemManager::settingLookup("config/scene");
        // sModelName = omega::Config::getStringValue("filename", sscene, sModelName);
        // sModelSize = omega::Config::getFloatValue("size", sscene, sModelSize);
    }


    // Create an omegalib scene node and attach the root osg node to it. This is used to interact with the 
    // osg object through omegalib interactors.
    omegaOsg::OsgSceneObject* oso = new omegaOsg::OsgSceneObject(m_troenGame->getPlayerNode());
    // const AlignedBox3* bbox = oso->getBoundingBox();
    myOsg->setRootNode(oso->getTransformedNode());

    // Resize the entire scene
    mySceneNode = omega::SceneNode::create("osgRoot");
    mySceneNode->addComponent(oso);

    myOsg->setRenderPassListener(myRenderPassListener.get());
    myOsg->setAutoNearFar(true);
    // disable culling to make sure reflected geoemtry is drawn
    getEngine()->getDefaultCamera()->setCullingEnabled(false);
    // myRoot->setScale(0.05f, 0.05f, 0.05f);


    // osgDB::writeNodeFile(*(m_scene).get(), "saved.osg");


    // Set the osg node as the root node
    // myOsg->setRootNode();

    // root->addChild(ls);
}

///////////////////////////////////////////////////////////////////////////////
void TroenOmegaScene::update(const omega::UpdateContext& context) 
{
    m_troenGame->stepGameOmega();

    // updateOmegaCamera(m_troenGame->getViewCamera());
}


void TroenOmegaScene::updateOmegaCamera(const osg::Camera* cam)
{
    // osg::Vec3d eye, center, up;
    // // _getTransformation(eye, center, up);

    // cam->getViewMatrixAsLookAt(eye, center, up); 

    // omega::Vector3f oPosVec(eye.x(), eye.y(), eye.z());
    // omega::Vector3f oUpVec(up.x(), up.y(), up.z());
    // omega::Vector3f oCenterVec(center.x(), center.y(), center.z());

    
    // //order is important here, setting lookat before position 
    // // will result in choppy camera rotation
    // oCenterVec = getEngine()->getDefaultCamera()->convertWorldToLocalPosition(oCenterVec);
    // getEngine()->getDefaultCamera()->setPosition(oPosVec);
    // getEngine()->getDefaultCamera()->lookAt(oCenterVec, oUpVec);
}


void TroenOmegaScene::registerCorrectPath()
{
    osgDB::Registry* reg = osgDB::Registry::instance();
    std::string libPath = GameThread::getInstance()->getStoredLibPaths();

    reg->setLibraryFilePathList(libPath);
    reg->removeReaderWriter(reg->getReaderWriterForExtension("tga"));
}


void TroenOmegaScene::handleEvent(const omega::Event& evt)
{
    m_troenGame->handleEvent(evt);
}