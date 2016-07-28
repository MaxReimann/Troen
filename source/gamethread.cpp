#include "gamethread.h"

#include "omegascene.h"
#include "troengame.h"
#include <assert.h> 
#include <osgDB/Registry>


using namespace troen;


    GameThread* GameThread::instance;


GameThread::GameThread(QThread* thread) : m_gameThread(thread), m_storedLibPaths("")
{
    if (m_gameThread == NULL) {
        m_gameThread = new QThread(this);
    }

    moveToThread(m_gameThread);
    m_gameThread->start(QThread::HighestPriority);
    GameThread::instance = this;
    m_troenGame = std::make_shared<TroenGame>();
}


// TroenGame* getTroenGame()
// {
//     if (m_troenGame != NULL)
//         return m_troenGame;

//     for (omega::EngineModule* module : omega::ModuleServices::getModules())
//     {
//         // omega::omsg("test m");
//         // omega::omsg(module->getName());
//         if (module->getName().compare("TroenOmegaScene")==0)
//         {
//             m_gameModule = dynamic_cast<TroenOmegaScene*>(module);
//             assert (m_gameModule != NULL);
            

//             std::cout << m_troenGame->getRootNode().get() << std::endl;
//             std::cout << m_troenGame->getSceneNode().get() << std::endl;

            
//             m_gameModule->setRootNode(m_troenGame->getRootNode());
//             m_gameModule->setSceneNode(m_troenGame->getSceneNode());
//             break;
//         }
//     }
//     if (m_gameModule)
//     {
//         m_troenGame = m_gameModule->getTroenGame();
//         return m_troenGame;
//     }
//     else {
//         omega::oerror("TroenOmegaScene Module not found");
//         return NULL;
//     }
// }



void GameThread::prepareAndStartGame(const GameConfig& config){
    m_omegaApp = std::make_shared<omega::Application<TroenOmegaScene>  >("Troen");

    for (auto libPath : osgDB::Registry::instance()->getLibraryFilePathList ()) 
    {
            m_storedLibPaths += libPath + ":";
    }

    omega::DataManager* dataManager = omega::SystemManager::instance()->getDataManager();
    std::cout << OMEGALIB_BIN_DIR << std::endl;
    dataManager->addSource(new omega::FilesystemDataSource(OMEGALIB_BIN_DIR));

    m_gameConfig = new GameConfig(config);


    char* a[] = {"Troen","--log","v"};
    omain(*(m_omegaApp.get()), 3, a);
}


