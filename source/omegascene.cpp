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

TroenOmegaScene::TroenOmegaScene() : omega::EngineModule("TroenOmegaScene")
{
    std::cout << "[TroenOmegaScene] instantiate" << std::endl;
    enableSharedData(); 
}



void TroenOmegaScene::initialize()
{
    std::cout << "[TroenOmegaScene::initialize]" << std::endl;

    myOsg = omegaOsg::OsgModule::instance();

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


void TroenOmegaScene::commitSharedData(omega::SharedOStream& out)
{
    m_troenGame->commitSharedData(out);
}


void TroenOmegaScene::updateSharedData(omega::SharedIStream& in)
{
    m_troenGame->updateSharedData(in);
}