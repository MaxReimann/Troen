#include "troengame.h"
// OSG
#include <osg/LineWidth>

// todo bended:
// #include <osg/BoundingSphere>
// #include <osgViewer/ViewerEventHandlers>
// #include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
// qt
#include <QCoreApplication>
// troen
#include "troengamebuilder.h"
#include "constants.h"
#include "globals.h"
#include "player.h"
#include "gamelogic.h"
#include "sampleosgviewer.h"

#include "controller/levelcontroller.h"
#include "controller/bikecontroller.h"
#include "controller/hudcontroller.h"

#include "model/physicsworld.h"

#include "BendedViews/src/SplineDeformationRendering.h"
#include "view/postprocessing.h"
#include "view/reflection.h"

#include "util/chronotimer.h"
#include "util/gldebugdrawer.h"
#include "sound/audiomanager.h"




#include "network/clientmanager.h"
#include "network/servermanager.h"
#include <thread>
#include <mutex>



using namespace troen;
extern long double g_currentTime;

TroenGame::TroenGame(QThread* thread /*= NULL*/) :
QObject(),
m_gameConfig(NULL),
m_deformationEnd(BENDED_VIEWS_DEACTIVATED)
{

	m_ServerManager = NULL;
	m_ClientManager = NULL;
}




////////////////////////////////////////////////////////////////////////////////
//
// Game Loop
//
////////////////////////////////////////////////////////////////////////////////

void TroenGame::prepareGame(const GameConfig& gameConfig)
{
	std::cout << "prepare game start" << std::endl;
	m_gameConfig = std::make_shared<GameConfig>(gameConfig);
	// m_omegaScene = std::make_shared<OmegaScene>(m_omegaScene);
	

	m_builder = new TroenGameBuilder(this);
	m_builder->build();

	if (m_gameConfig->useDebugView)
		m_sceneNode->addChild(m_physicsWorld->m_debug->getSceneGraph());

	// if (m_gameConfig->fullscreen)
	// 	setupForFullScreen();

	m_gameloopTimer->start();
	m_gameTimer->start();
	m_gameTimer->pause();
	// startGameLoop();

	
}


void TroenGame::handleEvent(const omega::Event &evt)
{
	for (auto player : m_playersWithView)
	{
		player->handleEvent(evt);
	}

	m_gameEventHandler->handleEvent(evt);
}

void TroenGame::stepGameOmega()
{
	static bool firstLoop = true;

	if (firstLoop) 
	{
		if (isNetworking())
		{
			getNetworkManager()->setLocalGameReady();
			getNetworkManager()->waitOnAllPlayers(); //blocking call
		}	
		firstLoop = false;


		// for (auto player : m_playersWithView)
		// {
		// 	player->createCameraManipulator();
		// }

	}

	// auto mconfig = omega::SystemManager::instance()->getMultiInstanceConfig();
	// std::cout << "W;" << mconfig.tilew << "h:" << mconfig.tileh << std::endl;


	// GAME LOOP VARIABLES
	static long double nextTime = m_gameloopTimer->elapsed();
	const double minMillisecondsBetweenFrames = 16.7; // vSync to 60 fps
	const double maxMillisecondsBetweenFrames = 4 * minMillisecondsBetweenFrames + 1;
	static int skippedFrames = 0;
	const int maxSkippedFrames = 4;
	static	bool nearPlaneAdapted = false;

	omega::SystemManager* osys = omega::SystemManager::instance();



	g_gameLoopTime = m_gameloopTimer->elapsed();
	g_gameTime = m_gameTimer->elapsed();

	QCoreApplication::processEvents();

	// are we significantly behind? if yes, "resync", force rendering
	if ((g_gameLoopTime - nextTime) > maxMillisecondsBetweenFrames)
		nextTime = g_gameLoopTime;
	// is it time to render the next frame?
	if (m_gameConfig->testPerformance || g_gameLoopTime >= nextTime)
	{
		// assign the time for the next update
		nextTime += minMillisecondsBetweenFrames;

		// only the master is responsible for updating logic, sounds, etc.
		if (osys->isMaster())
		{
			// LOOP REALLY STARTS HERE:
			m_gameLogic->step(g_gameLoopTime, g_gameTime);

		
			if (!m_gameTimer->paused())
			{
				for (auto player : m_players)
				{
					player->update(g_gameTime);
				}
				m_physicsWorld->stepSimulation(g_gameTime);
				m_levelController->update();
			}

		}

		if (isNetworking())
			getNetworkManager()->update(g_gameTime);

		if (osys->isMaster())
		{
			m_audioManager->Update(g_gameLoopTime / 1000);
			m_audioManager->setMotorSpeed(m_players[0]->bikeController()->speed());
	
			if (m_postProcessing)
				m_postProcessing->setBeat(m_audioManager->getTimeSinceLastBeat());
		}

		// Hack: normalize and use the speed to control the deformation
		float bikeSpeed = m_players[0]->bikeController()->speed();
		float maxSpeed = 400.f;

		handleBending(double(bikeSpeed / maxSpeed));

		// do we have extra time (to draw the frame) or did we skip too many frames already?
		if (g_gameLoopTime < nextTime || (skippedFrames > maxSkippedFrames))
		{
			for (auto player : m_playersWithView)
			{
				player->hudController()->update(
					g_gameLoopTime,
					g_gameTime,
					m_gameConfig->timeLimit,
					m_gameLogic->getGameState(),
					m_players);
			}


			// TODO: find a way to eleminate this workaround
			// doesn't work if it's executed earlier
			if (!nearPlaneAdapted)
			{
				for (auto player : m_playersWithView)
				{
					float znear = 1.0;
					// hack: set the near far maunally, wsince we know the skydome has a radius of 5000
					omega::Engine::instance()->getDefaultCamera()->setNearFarZ(znear, 10000);
					// fixCulling(player->gameView());
				}
			}
			skippedFrames = 0;
		}
		else
			skippedFrames++;
	}
	else // WAIT
	{
		// calculate the time to sleep
		// long double sleepTime = (nextTime - g_gameLoopTime);
		// if (sleepTime > 0)	// sanity check, sleep until nextTime
		// if (!m_gameConfig->testPerformance) m_gameThread->msleep(sleepTime);
	}
}


void TroenGame::shutdown()
{
	
	if (m_gameConfig->fullscreen)
		returnFromFullScreen();

	// SHUTDOWN
	m_builder->destroy();
	delete m_builder;
}


void TroenGame::fixCulling(osg::ref_ptr<osgViewer::View> view)
{
	double fovy, aspect, znear, zfar;
	view->getCamera()->getProjectionMatrixAsPerspective(fovy, aspect, znear, zfar);
	view->getCamera()->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
	znear = 1.0;
	view->getCamera()->setProjectionMatrixAsPerspective(fovy, aspect, znear, zfar);

}

void TroenGame::handleBending(double interpolationSkalar)
{
	m_deformationRendering->setInterpolationSkalar(1.0);

	double currentBending = m_deformationRendering->getDeformationEnd();
	const double targetBending = m_deformationEnd;
	const double bendedStep = (BENDED_VIEWS_DEACTIVATED - BENDED_VIEWS_ACTIVATED) / 300;
	
	if (targetBending == BENDED_VIEWS_ACTIVATED)
	{
		m_levelController->setBendingActive(true);
		for (auto player : m_players)
		{
			player->fenceController()->setBendingActive(true);
		}
		currentBending -= bendedStep;
	}
	else
	{
		currentBending += bendedStep;
		if (currentBending >= BENDED_VIEWS_DEACTIVATED)
		{

			m_levelController->setBendingActive(false);
			for (auto player : m_players)
			{
				player->fenceController()->setBendingActive(false);
			}

		}
	}

#define clamp(l, u, x) \
	((x) < (l) ? (l) : (x) > (u) ? (u) : (x))

	currentBending = clamp(BENDED_VIEWS_ACTIVATED, BENDED_VIEWS_DEACTIVATED, currentBending);

	m_deformationRendering->setDeformationStartEnd(0.05, currentBending);
	m_levelController->setBendingFactor(1.0 - currentBending / BENDED_VIEWS_DEACTIVATED);
	

}

////////////////////////////////////////////////////////////////////////////////
//
// Full Screen Handling
//
////////////////////////////////////////////////////////////////////////////////

void TroenGame::setupForFullScreen()
{
	osg::GraphicsContext::WindowingSystemInterface* wsi =
		osg::GraphicsContext::getWindowingSystemInterface();
	if (!wsi)
	{
		std::cout << "[TroenGame::setupForFullScreen] error ..." << std::endl;
		return;
	}
	wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), m_originalWidth, m_originalHeight);

	//wsi->setScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), 1024, 768);
	this->resize(m_originalWidth, m_originalHeight);
}

void TroenGame::returnFromFullScreen()
{
	osg::GraphicsContext::WindowingSystemInterface* wsi =
		osg::GraphicsContext::getWindowingSystemInterface();
	if (!wsi)
	{
		std::cout << "[TroenGame::returnFromFullScreen] error ..." << std::endl;
		return;
	}
	wsi->setScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), m_originalWidth, m_originalHeight);
}



void TroenGame::switchSoundVolumeEvent()
{
	if (omega::SystemManager::instance()->isMaster())
		m_audioManager->SetMasterVolume(1 - m_audioManager->GetMasterVolume());
}

void TroenGame::pauseEvent()
{
	if (m_gameLogic->getGameState() == GameLogic::GAMESTATE::GAME_START)
		return;
	else if (!m_gameTimer->paused())
		pauseSimulation();
	else
		unpauseSimulation();
}

void TroenGame::pauseSimulation()
{
	if (!m_gameTimer->paused()) m_gameTimer->pause();
}

void TroenGame::unpauseSimulation()
{
	if (m_gameTimer->paused()) m_gameTimer->start();
}

void TroenGame::resize(int width, int height){
	if (m_postProcessing){
		m_postProcessing->setupTextures(width, height);
	}

	for (auto player : m_playersWithView)
	{
		player->hudController()->resize(width, height);
	}
}


////////////////////////////////////////////////////////////////////////////////
//
// Networking
//
////////////////////////////////////////////////////////////////////////////////

//setup the network connection
std::string TroenGame::setupServer(std::vector<QString> playerNames)
{

		std::cout << "[TroenGame::initialize] networking Server..." << std::endl;
		m_ServerManager = std::make_shared<networking::ServerManager>(this, playerNames);
		m_ServerManager->openServer();
		return std::string("ok");
}

std::string TroenGame::setupClient(QString playerName, std::string connectAddr)
{
		std::cout << "[TroenGame::initialize] networking Client..." << std::endl;
		m_ClientManager = std::make_shared<networking::ClientManager>(this, playerName);
		m_ClientManager->openClient(connectAddr);
		return std::string("ok");
}



bool TroenGame::synchronizeGameStart(GameConfig config)
{
	getNetworkManager()->synchronizeGameStart(config);
	return true;
}


bool TroenGame::isNetworking()
{
	if (getNetworkManager() != NULL)
	{
		if (getNetworkManager()->isValidSession())
			return true;
	}
	return false;
}

std::shared_ptr<networking::NetworkManager> TroenGame::getNetworkManager()
{
	if (m_ClientManager != NULL)
		return static_cast<std::shared_ptr<networking::NetworkManager> >(m_ClientManager);
	else if (m_ServerManager != NULL)
		return static_cast<std::shared_ptr<networking::NetworkManager> >(m_ServerManager);
	else
		return NULL;
}


// equalizer screen sync

void TroenGame::registerSharedListener(SharedDataListener* listener)
{
	m_sharedDataListeners.push_back(listener);
}

void TroenGame::registerRenderPassListener(RenderPassListener* listener)
{
	m_renderPassListeners.push_back(listener);
}

void TroenGame::commitSharedData(omega::SharedOStream& out)
{
    for (auto listener : m_sharedDataListeners)
    {
    	listener->commitSharedData(out);
    }
}


void TroenGame::updateSharedData(omega::SharedIStream& in)
{
    for (auto listener : m_sharedDataListeners)
    {
    	listener->updateSharedData(in);
    }
}




/////// utility //////////////
void TroenGame::reloadLevel()
{
	m_levelController->reload();
	m_builder->setupReflections();
}


void TroenGame::toggleHUDVisibility()
{
	m_players[0]->hudController()->toggleVisibility();
}

osg::Camera* TroenGame::getViewCamera()
{
	for (auto player : players())
    {
	    if (player->hasGameView())
	    {
	        return player->gameView()->getCamera();
	    }
    }
}


osg::ref_ptr<osg::Group> TroenGame::getBikeNode(){
	 return m_playersWithView[0]->bikeController()->getViewNode(); 
}
osg::ref_ptr<osg::Group> TroenGame::getLevelNode(){
	return m_levelController->getViewNode(); 
}

osg::ref_ptr<osg::Group> TroenGame::getPlayerNode(){
	return m_playersWithView[0]->playerNode();
}
