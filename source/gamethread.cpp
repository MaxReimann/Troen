#include "gamethread.h"

#include "omegascene.h"
#include "troengame.h"
#include <assert.h> 


using namespace troen;

GameThread::GameThread(QThread* thread) : m_gameThread(thread)
{
    if (m_gameThread == NULL) {
        m_gameThread = new QThread(this);
    }

    moveToThread(m_gameThread);
    m_gameThread->start(QThread::HighestPriority);
}

void GameThread::prepareAndStartGame(const GameConfig& config){
    m_troenGame = std::make_shared<TroenGame>();
    m_omegaApp = std::make_shared<omega::Application<TroenOmegaScene>  >("Troen");
    // m_troenGame->prepareGame(config);


    for (omega::EngineModule* module : omega::ModuleServices::getModules())
    {
        if (module->getName().compare("TroenOmegaScene")==0)
        {
            m_gameModule = dynamic_cast<TroenOmegaScene*>(module);
            assert (m_gameModule != NULL);
            
            m_gameModule->setRootNode(m_troenGame->getRootNode());
            m_gameModule->setSceneNode(m_troenGame->getSceneNode());
            break;
        }
    }

    if (!m_gameModule)
        std::cout << "TroenOmegaScene Module not found" << std::endl;
    

    omega::DataManager* dataManager = omega::SystemManager::instance()->getDataManager();
    std::cout << OMEGALIB_BIN_DIR << std::endl;
    dataManager->addSource(new omega::FilesystemDataSource(OMEGALIB_BIN_DIR));




    char* a[] = {"Troen",};
    omain(*(m_omegaApp.get()), 1, a);

}