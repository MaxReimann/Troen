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



MyOsgModule* MyOsgModule::m_instance = NULL;

void MyRenderPassListener::onFrameFinished(omega::Renderer* client, const omega::DrawContext& context, omegaOsg::SceneView* scene){


    for (auto listener : m_game->getRenderPassListeners())
    {
        listener->onRender(client, context, scene);
    }

}

MyRenderPassListener::MyRenderPassListener(TroenGame* game) : m_game(game) {}


//////////////////////////////////////////////////////////////////////////////

void MyOsgModule::initializeRenderer(omega::Renderer* r)
{
    omegaOsg::OsgModule::initializeRenderer(r);
}


MyOsgModule* MyOsgModule::instance() 
{ 
    if(m_instance == NULL)
    {
        m_instance = new MyOsgModule();
        omega::ModuleServices::addModule(m_instance);
        m_instance->doInitialize(omega::Engine::instance());
    }
    return m_instance; 
}



///////////////////////////////////////////////////////////////////////////////

TroenOmegaScene::TroenOmegaScene() : omega::EngineModule("TroenOmegaScene")
{
    std::cout << "[TroenOmegaScene] instantiate" << std::endl;
    enableSharedData(); 
}



void TroenOmegaScene::initialize()
{
    std::cout << "[TroenOmegaScene::initialize]" << std::endl;

    myOsg = MyOsgModule::instance();

    registerCorrectPath();


    m_troenGame = GameThread::getInstance()->getTroenGame();

    m_troenGame->prepareGame(*GameThread::getInstance()->getGameConfig());

    myRenderPassListener = std::make_shared<MyRenderPassListener>(m_troenGame);

    // Create an omegalib scene node and attach the root osg node to it. This is used to interact with the 
    // osg object through omegalib interactors.
    omegaOsg::OsgSceneObject* oso = new omegaOsg::OsgSceneObject(m_troenGame->getPlayerNode());
    // const AlignedBox3* bbox = oso->getBoundingBox();
    myOsg->setRootNode(oso->getTransformedNode());

    // Resize the entire scene
    mySceneNode = omega::SceneNode::create("osgRoot");
    mySceneNode->addComponent(oso);

    myOsg->setRenderPassListener(myRenderPassListener.get());
    myOsg->setAutoNearFar(false);
    std::cout << omega::SystemManager::instance()->getHostnameAndPort() << " omegascene initialized" << std::endl; 
}

///////////////////////////////////////////////////////////////////////////////
void TroenOmegaScene::update(const omega::UpdateContext& context) 
{
    m_troenGame->stepGameOmega();
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