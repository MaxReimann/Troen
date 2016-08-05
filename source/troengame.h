#pragma once
// Qt
#include <QThread>
#include <QMetaType>
#include <QColor>
// OSG
#include <osg/ref_ptr>
#include <osg/ShapeDrawable>
#include <osgGA/GUIEventAdapter>
#include <osg/Group>
#include <osgViewer/ViewerEventHandlers>
// troen
//order is important, has to be imported before windows.h
#include "network/networkmanager.h"
#include "forwarddeclarations.h"
#include "constants.h"
#include "gameeventhandler.h"
#include "resourcepool.h"
#include "view/skydome.h"

// #define OMEGA_NO_GL_HEADERS
// #include <omega.h>

// #include <omegaToolkit.h>
// #include <omegaOsg/omegaOsg.h>
#include "omegascene.h"


#define MAX_BIKES 6
namespace troen
{

	/*! The TroenGame class contains the logic for the game loop and basic functionality such as pausing the game, resizing the game window or switching to fullscreen.*/
	class GameConfig
	{
	public:
		// GameConfig(GameConfig& config) : numberOfPlayers(config.numberOfPlayers), timeLimit(config.timeLimit),
		// 	playerInputTypes(config.playerInputTypes), 

		int numberOfPlayers;
		int timeLimit;
		int* playerInputTypes;
		QColor* playerColors;
		QString* playerNames;
		bool fullscreen;
		bool usePostProcessing;
		bool useDebugView;
		bool testPerformance;
		bool useReflection;
		bool ownView[MAX_BIKES];
		std::string levelName;
        // TODO: add Destructor to avoid memory leak
	};


	class CArguments
	{
	public:
		int argc;
		char** argv;
	};
}

Q_DECLARE_METATYPE(troen::GameConfig)
Q_DECLARE_METATYPE(troen::CArguments)

namespace troen
{
	class TroenGame : public QObject
	{
		Q_OBJECT

		friend class TroenGameBuilder;
		friend class GameLogic;

	public:
		TroenGame(QThread* thread = NULL);
		//
		// getters
		//
		osg::ref_ptr<GameEventHandler> gameEventHandler()
		{
			return m_gameEventHandler;
		};
		osg::ref_ptr<osgViewer::StatsHandler> statsHandler()
		{
			return m_statsHandler;
		};
		std::shared_ptr<LevelController> levelController()
		{
			return m_levelController;
		};
		std::vector<std::shared_ptr<Player> > players()
			{ return m_players; };
		osg::ref_ptr<SkyDome> skyDome()
			{ return m_skyDome; };
		ResourcePool* resourcePool(){ return &m_resourcePool; };
		void prepareGame(const GameConfig& config);
		//
		// Events
		//
		void switchSoundVolumeEvent();
		void pauseEvent();
		void pauseSimulation();
		void unpauseSimulation();
		void resize(const int width, const int height);
		void reloadLevel();


		std::shared_ptr<networking::ClientManager> getClientManager() { return m_ClientManager; }
		std::shared_ptr<networking::ServerManager> getServerManager() { return m_ServerManager; }
		std::shared_ptr<networking::NetworkManager> getNetworkManager();

		std::vector<RenderPassListener*>& getRenderPassListeners() { return m_renderPassListeners;}


		void commitSharedData(omega::SharedOStream& out);
        void updateSharedData(omega::SharedIStream& in);

        void registerSharedListener(SharedDataListener* listener);
        void registerRenderPassListener(RenderPassListener* listener);



		SplineDeformationRendering* getBendedViews() {
			return m_deformationRendering;
		}

		void enableBendedViews() { m_deformationEnd = BENDED_VIEWS_ACTIVATED; }
		void disableBendedViews() { m_deformationEnd = BENDED_VIEWS_DEACTIVATED; }
		
		double m_deformationEnd;

		void toggleHUDVisibility();

		osg::ref_ptr<osg::Group> getRootNode() { return m_rootNode; }
		osg::ref_ptr<osg::Group> getSceneNode() { return m_sceneNode; }
		osg::ref_ptr<osg::Group> getBikeNode();
		osg::ref_ptr<osg::Group> getLevelNode();
		osg::ref_ptr<osg::Group> getPlayerNode();


		void shutdown();
		void stepGameOmega();

		osg::Camera* getViewCamera();

		void handleEvent(const omega::Event& evt);



	public slots:
		// void prepareAndStartGame(const GameConfig& config);
		bool synchronizeGameStart(GameConfig config);
		bool isNetworking();
		std::string setupClient(QString playerName, std::string connectAddr = "127.0.0.1");
		std::string setupServer(std::vector<QString> playerNames);

	private:
		TroenGameBuilder *m_builder;
		//
		// Game Loop
		//
		void startGameLoop();
		void fixCulling(osg::ref_ptr<osgViewer::View> view);
		void handleBending(double interpolationSkalar);

		//
		// fullscreen handling
		//
		void setupForFullScreen();
		void returnFromFullScreen();
		uint m_originalWidth;
		uint m_originalHeight;

		//
		// OSG Components
		//
		osg::ref_ptr<GameEventHandler>		m_gameEventHandler;
		osg::ref_ptr<osg::Group>			m_rootNode;
		osg::ref_ptr<SkyDome>               m_skyDome;
		osg::ref_ptr<osgViewer::StatsHandler> m_statsHandler;
		std::shared_ptr<PostProcessing>		m_postProcessing;
		osg::ref_ptr<osg::Group>			m_sceneNode;
		osg::ref_ptr<osg::Group>			m_sceneWithSkyboxNode;

		//
		// Game Components
		//
		std::shared_ptr<GameConfig>							m_gameConfig;
		std::shared_ptr<LevelController>					m_levelController;
		std::vector<std::shared_ptr<Player> >				m_players;
		std::vector<std::shared_ptr<Player> >				m_playersWithView;
		std::shared_ptr<util::ChronoTimer>					m_gameloopTimer;
		std::shared_ptr<util::ChronoTimer>					m_gameTimer;
		std::shared_ptr<PhysicsWorld>						m_physicsWorld;
		std::shared_ptr<GameLogic>							m_gameLogic;
		std::shared_ptr<sound::AudioManager>				m_audioManager;
		std::shared_ptr<networking::ServerManager>  		m_ServerManager;
		std::shared_ptr<networking::ClientManager>  		m_ClientManager;
		// std::shared_ptr<omega::Application<TroenOmegaScene> >	m_omegaApp;

		std::vector<SharedDataListener*> 					m_sharedDataListeners;
		std::vector<RenderPassListener*> 		m_renderPassListeners;


		ResourcePool m_resourcePool;

		// BendedViews
		SplineDeformationRendering* m_deformationRendering;

	};
}