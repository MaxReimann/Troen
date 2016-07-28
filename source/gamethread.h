#pragma once
#include "forwarddeclarations.h"

// Qt
#include <QThread>
#include <QMetaType>
#include <QColor>


#define OMEGA_NO_GL_HEADERS
#include <omega.h>
// #include <omegaToolkit.h>
// #include <omegaOsg/omegaOsg.h>
#undef foreach //conflict of boost::foearch with qt::foreach



namespace troen
{

    class GameThread : public QObject
    {
        Q_OBJECT
        static GameThread* instance;
    public:

        GameThread(QThread* thread = NULL);

        TroenGame* getTroenGame() {return m_troenGame.get();}
        GameConfig* getGameConfig() { return m_gameConfig;}
        std::string getStoredLibPaths() { return m_storedLibPaths; }

        static GameThread* getInstance() {return GameThread::instance;}


    public slots:
        void prepareAndStartGame(const GameConfig& config);

    protected:
        // Startup Options
        QThread* m_gameThread;
        std::shared_ptr<TroenGame> m_troenGame; 

        std::shared_ptr<omega::Application<TroenOmegaScene> > m_omegaApp;
        GameConfig* m_gameConfig;
        std::string m_storedLibPaths;

    };

}