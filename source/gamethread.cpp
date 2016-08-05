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


void GameThread::prepareAndStartGame(const GameConfig& config,const CArguments& arguments){
    m_omegaApp = std::make_shared<omega::Application<TroenOmegaScene>  >("Troen");

    for (auto libPath : osgDB::Registry::instance()->getLibraryFilePathList ()) 
    {
            m_storedLibPaths += libPath + ":";
    }

    omega::DataManager* dataManager = omega::SystemManager::instance()->getDataManager();
    std::cout << OMEGALIB_BIN_DIR << std::endl;
    dataManager->addSource(new omega::FilesystemDataSource(OMEGALIB_BIN_DIR));

    m_gameConfig = new GameConfig(config);

    // char* a[] = {"Troen","--log","v"};
    omain(*(m_omegaApp.get()), arguments.argc, arguments.argv);
}


