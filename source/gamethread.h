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
    public:

        GameThread(QThread* thread = NULL);

        TroenGame* getTroenGame() {return m_troenGame.get();}


    public slots:
        void prepareAndStartGame(const GameConfig& config);

    protected:
        // Startup Options
        QThread* m_gameThread;

        std::shared_ptr<TroenGame> m_troenGame;
        std::shared_ptr<omega::Application<TroenOmegaScene> > m_omegaApp;
        TroenOmegaScene* m_gameModule;
    };

}