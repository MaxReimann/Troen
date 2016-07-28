#include "omegascene.h"
#include <osgDB/WriteFile>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include "gamethread.h"
#include "troengame.h"
#include <iostream>
#include <cstdlib>

using namespace troen;



///////////////////////////////////////////////////////////////////////////////
void TroenOmegaScene::initialize()
{
    std::cout << "[TroenOmegaScene::initialize]" << std::endl;

    registerCorrectPath();

    // The node containing the scene
    // osg::Node* node = NULL;

    // The root node (we attach lights and other global state properties here)
    // Set the root to be a lightsource to attach a light to it to illuminate the scene
    // osg::Group* root = new osg::Group();



    m_troenGame = GameThread::getInstance()->getTroenGame();

    

    m_troenGame->prepareGame(*GameThread::getInstance()->getGameConfig());


    // Load osg object
    if(omega::SystemManager::settingExists("config/scene"))
    {
        omega::Setting& sscene = omega::SystemManager::settingLookup("config/scene");
        // sModelName = omega::Config::getStringValue("filename", sscene, sModelName);
        // sModelSize = omega::Config::getFloatValue("size", sscene, sModelSize);
    }


    getEngine()->getDefaultCamera()->setPosition(omega::Vector3f(0,40,180));
    getEngine()->getDefaultCamera()->lookAt(omega::Vector3f(0,0,0), omega::Vector3f(0,0,1));

    
    // osgDB::writeNodeFile(*(m_scene).get(), "saved.osg");

    // Create an omegalib scene node and attach the osg node to it. This is used to interact with the 
    // osg object through omegalib interactors.
    // omegaOsg::OsgSceneObject* oso = new omegaOsg::OsgSceneObject(m_troenGame->getBikeNode());
    // mySceneNode = new omega::SceneNode(getEngine());
    // mySceneNode->addComponent(oso);
    // getEngine()->getScene()->addChild(mySceneNode);


    // Set the osg node as the root node
    myOsg->setRootNode(m_troenGame->getPlayerNode());

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
    osg::Vec3d eye, center, up;
    // _getTransformation(eye, center, up);

    cam->getViewMatrixAsLookAt(eye, center, up); 

    omega::Vector3f oPosVec(eye.x(), eye.y(), eye.z());
    omega::Vector3f oUpVec(up.x(), up.y(), up.z());
    omega::Vector3f oCenterVec(center.x(), center.y(), center.z());

    
    //order is important here, setting lookat before position 
    // will result in choppy camera rotation
    getEngine()->getDefaultCamera()->setPosition(oPosVec);
    getEngine()->getDefaultCamera()->lookAt(oCenterVec, oUpVec);
}


void TroenOmegaScene::registerCorrectPath()
{
    std::string execPath = OMEGALIB_BIN_DIR ;
    // String execName;
    // omega::StringUtils::splitFilename(execPath, execName, execPath);
    osgDB::Registry* reg = osgDB::Registry::instance();
    // std::string existingPath = "";
    // for (auto st : reg->getLibraryFilePathList())
    // {   
    //     existingPath += st + ":";
    // }


    int vmaj = OPENSCENEGRAPH_MAJOR_VERSION;
    int vmin = OPENSCENEGRAPH_MINOR_VERSION;
    int vp = OPENSCENEGRAPH_PATCH_VERSION;

    // omega::ostr("%1%.%2%.%3%",  
    std::string osgv = std::to_string(vmaj) + "." + std::to_string(vmin) + "."  + std::to_string(vp);
    // std::cout << osgv << std::endl;

    
    std::string libPath = GameThread::getInstance()->getStoredLibPaths();

    // std::cout << "OSG Plugin Path(s): " << libPath << std::endl;
    reg->closeAllLibraries ();
    reg->setLibraryFilePathList(libPath);
    osgDB::Registry::instance()->removeReaderWriter(osgDB::Registry::instance()->getReaderWriterForExtension("tga"));
}