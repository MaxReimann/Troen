#include "omegascene.h"
#include <osgDB/WriteFile>
#include "gamethread.h"
#include "troengame.h"

using namespace troen;



///////////////////////////////////////////////////////////////////////////////
void TroenOmegaScene::initialize()
{
    std::cout << "[TroenOmegaScene::initialize]" << std::endl;
    // The node containing the scene
    // osg::Node* node = NULL;

    // The root node (we attach lights and other global state properties here)
    // Set the root to be a lightsource to attach a light to it to illuminate the scene
    // osg::Group* root = new osg::Group();


    // Load osg object
    if(omega::SystemManager::settingExists("config/scene"))
    {
        omega::Setting& sscene = omega::SystemManager::settingLookup("config/scene");
        // sModelName = omega::Config::getStringValue("filename", sscene, sModelName);
        // sModelSize = omega::Config::getFloatValue("size", sscene, sModelSize);
    }

    m_troenGame = GameThread::getInstance()->getTroenGame();




    // osgDB::writeNodeFile(*(m_scene).get(), "saved.osg");

    // Create an omegalib scene node and attach the osg node to it. This is used to interact with the 
    // osg object through omegalib interactors.
    omegaOsg::OsgSceneObject* oso = new omegaOsg::OsgSceneObject(m_troenGame->getSceneNode());
    mySceneNode = new omega::SceneNode(getEngine());
    mySceneNode->addComponent(oso);
    getEngine()->getScene()->addChild(mySceneNode);



    getEngine()->getDefaultCamera()->focusOn(getEngine()->getScene());

    // Set the interactor style used to manipulate meshes.
    if(omega::SystemManager::settingExists("config/interactor"))
    {
        omega::Setting& sinteractor = omega::SystemManager::settingLookup("config/interactor");
        myInteractor = omegaToolkit::ToolkitUtils::createInteractor(sinteractor);
        if(myInteractor != NULL)
        {
            omega::ModuleServices::addModule(myInteractor);
        }
    }

    if(myInteractor != NULL)
    {
        myInteractor->setSceneNode(mySceneNode);
    }

    // Set the osg node as the root node
    myOsg->setRootNode(m_troenGame->getRootNode());

    // root->addChild(ls);
}

///////////////////////////////////////////////////////////////////////////////
void TroenOmegaScene::update(const omega::UpdateContext& context) 
{
}