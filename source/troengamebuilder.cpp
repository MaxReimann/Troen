#include "troengamebuilder.h"
// OSG

#include <osg/LineWidth>
#include <osgUtil/Optimizer>
// troen
#include "constants.h"
#include "globals.h"
#include "player.h"
#include "gamelogic.h"
#include "sampleosgviewer.h"
#include "gameeventhandler.h"

#include "controller/levelcontroller.h"
#include "controller/bikecontroller.h"
#include "controller/fencecontroller.h"
#include "controller/levelcontroller.h"
#include "controller/hudcontroller.h"

#include "model/physicsworld.h"
#include "model/abstractmodel.h"

#include "view/shaders.h"
#include "view/postprocessing.h"
#include "view/reflection.h"
#ifdef USE_BENDEDVIEWS
#include "BendedViews/src/SplineDeformationRendering.h"
#endif

#include "util/chronotimer.h"
#include "util/gldebugdrawer.h"
#include "sound/audiomanager.h"
#include "input/gamepadvrpn.h"
#ifdef WIN32
#include "input/gamepad.h"
#endif

#include <thread>

using namespace troen;

TroenGameBuilder::TroenGameBuilder(TroenGame * game) :
t(game) {};


bool TroenGameBuilder::build()
{


	omega::SystemManager* osys = omega::SystemManager::instance();
	t->m_rootNode = new osg::Group;
	t->m_sceneNode = new osg::Group;

	osg::DisplaySettings::instance()->setNumMultiSamples(NUM_MULTISAMPLES);
	std::cout << "[TroenGame::build] building game ..." << std::endl;

	////////////////////////////////////////////////////////////////////////////////
	//
	// timers & shaders
	//
	////////////////////////////////////////////////////////////////////////////////
	std::cout << "[TroenGame::build] timer & shaders ..." << std::endl;
	t->m_gameloopTimer = std::make_shared<util::ChronoTimer>(false, true);
	t->m_gameTimer = std::make_shared<util::ChronoTimer>(false, true);
	shaders::reloadShaders();

	////////////////////////////////////////////////////////////////////////////////
	//
	// Sound
	//
	////////////////////////////////////////////////////////////////////////////////
	if (osys->isMaster()) //dont playback sounds on childs, causes crashes 
	{
		std::cout << "[TroenGame::build] sound ..." << std::endl;
		t->m_audioManager = std::shared_ptr<sound::AudioManager>(new sound::AudioManager);
		t->m_audioManager->LoadSFX("data/sound/explosion.wav");
		t->m_audioManager->LoadSong("data/sound/theGameHasChanged.mp3");
		t->m_audioManager->LoadEngineSound();
		t->m_audioManager->SetSongsVolume(0.5);

		t->m_audioManager->PlaySong("data/sound/theGameHasChanged.mp3");
		t->m_audioManager->PlayEngineSound();
		t->m_audioManager->SetMasterVolume(0.f);
	}

	////////////////////////////////////////////////////////////////////////////////
	//
	// Event Handlers
	//
	////////////////////////////////////////////////////////////////////////////////
	t->m_skyDome = new SkyDome;
	{ // event handlers
		t->m_statsHandler = new osgViewer::StatsHandler;
		t->m_statsHandler->setKeyEventTogglesOnScreenStats(osgGA::GUIEventAdapter::KEY_T);
		t->m_statsHandler->setKeyEventPrintsOutStats(osgGA::GUIEventAdapter::KEY_P);
		t->m_statsHandler->setKeyEventToggleVSync(osgGA::GUIEventAdapter::KEY_V);

		t->m_gameEventHandler = new GameEventHandler(t);
	}

	////////////////////////////////////////////////////////////////////////////////
	//
	// Controllers (Level, Player, Hud)
	//
	////////////////////////////////////////////////////////////////////////////////
	std::cout << "[TroenGame::build] controllers (models & views) ..." << std::endl;
	{ // controllers
		t->m_levelController = std::make_shared<LevelController>(t, t->m_gameConfig->levelName);
		std::cout << omega::SystemManager::instance()->getHostnameAndPort() 
			<< " config: num players: " << t->m_gameConfig->numberOfPlayers << std::endl;

		for (int i = 0; i < t->m_gameConfig->numberOfPlayers; i++)
		{
			std::shared_ptr<Player> player = std::make_shared<Player>(t, t->m_gameConfig, i);
			t->m_players.push_back(player);
			std::cout << omega::SystemManager::instance()->getHostnameAndPort() << " player ownView: " << t->m_gameConfig->ownView[i] << std::endl;
			if (t->m_gameConfig->ownView[i])
			{
    			std::cout << omega::SystemManager::instance()->getHostnameAndPort() << " adding player with view" << std::endl;
				t->m_playersWithView.push_back(player);
			}
		}
		for (auto player : t->m_players)
		{
			player->createHUDController(t->m_players);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	//
	// Game Logic
	//
	////////////////////////////////////////////////////////////////////////////////
	std::cout << "[TroenGameBuilder::build] gameLogic ..." << std::endl;
	t->m_gameLogic = std::make_shared<GameLogic>(t,t->m_gameConfig->timeLimit);
	t->m_gameEventHandler->attachGameLogic(t->m_gameLogic);

	////////////////////////////////////////////////////////////////////////////////
	//
	// PostProcessing, SceneGraph, Input, PhysicsWorld in individual methods
	//
	////////////////////////////////////////////////////////////////////////////////
	std::cout << "[TroenGameBuilder::build] postprocessing & scenegraph ..." << std::endl;
	composeSceneGraph();
	std::cout << "[TroenGameBuilder::build] input ..." << std::endl;
	buildInput();
	std::cout << "[TroenGameBuilder::build] physics ..." << std::endl;
	buildPhysicsWorld();
	t->m_physicsWorld->stepSimulation(0);

	std::cout << "[TroenGameBuilder::build] successfully built TroenGame !" << std::endl;
	return true;
}

bool TroenGameBuilder::composeSceneGraph()
{
	if (t->m_gameConfig->usePostProcessing)
	{
		// viewport of all windows has to be equal since only
		// the first is taken for PostProcessing Texture Sizes
		// osg::Viewport * viewport =
		// 	t->m_players[0]->gameView()->getCamera()->getViewport();

		auto pixSize = omega::SystemManager::instance()->getDisplaySystem()->getDisplayConfig().tileResolution;

		t->m_postProcessing = std::make_shared<PostProcessing>(t->m_rootNode, pixSize.x(), pixSize.y());
		t->registerRenderPassListener(t->m_postProcessing.get());

		// t->m_sceneWithSkyboxNode = t->m_rootNode;
		t->m_sceneWithSkyboxNode = t->m_postProcessing->getSceneNode();

		//explicit call, to enable glow from start
		t->resize(pixSize.x(), pixSize.y());
	}
	else
	{
		t->m_sceneWithSkyboxNode = t->m_rootNode;
	}

	for (auto player : t->m_playersWithView)
	{
			player->playerNode()->addChild(t->m_rootNode);
	}

	t->m_skyDome->getOrCreateStateSet()->setRenderBinDetails(-1, "RenderBin");
	t->m_sceneWithSkyboxNode->addChild(t->m_skyDome.get());
	t->m_sceneWithSkyboxNode->addChild(t->m_sceneNode);

	t->m_sceneNode->addChild(t->m_levelController->getViewNode());
	//t->m_sceneNode->addChild(t->m_sunLightSource.get());

	for (auto player : t->m_players)
	{
		t->m_sceneNode->addChild(player->bikeController()->getViewNode());
		t->m_sceneNode->addChild(player->fenceController()->getViewNode());
	}

	t->m_sceneNode->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::ON);

	if (t->m_gameConfig->useReflection)
	{
		// sceneNode has to be added to reflection after adding all (non hud) objects
		setupReflections();
	}

	for (auto player : t->m_playersWithView)
	{
		osg::Group * node = player->hudController()->getViewNode();
		osg::Group * playerNode = player->playerNode();
		// playerNode->addChild(node); //TODO: comment back in
	}


	if (t->m_gameConfig->usePostProcessing)
		t->m_rootNode->addChild(t->m_sceneWithSkyboxNode);


	// TODO: comment back in
	// osg::ref_ptr<osg::Group> radarScene = new osg::Group;

	// for (auto player : t->m_players)
	// {
	// 	radarScene->addChild(player->bikeController()->getViewNode());
	// 	radarScene->addChild(player->fenceController()->getViewNode());
	// }
	// radarScene->addChild(t->m_levelController->getViewNode());

#ifdef USE_BENDEDVIEWS
 	const osg::BoundingSphere& bs = t->m_sceneNode->getBound();
 	// todo: the magic number (0.25) can be used to control the length of the deformation and must be possibly adjusted after the scene graph tweaks
 	float radius = LEVEL_SIZE / 5;
	radius = 1071;
 	double nearD = 0.1;
	std::cout << "radius  " << radius << std::endl;
 	t->m_deformationRendering = new SplineDeformationRendering(t->m_sceneNode);
 	t->m_deformationRendering->setDeformationStartEnd(nearD, radius);
 	t->m_deformationRendering->setPreset(4);
#endif

 	//TODO: comment back in
	// for (auto player : t->m_playersWithView)
	// {
	// 	player->hudController()->attachSceneToRadarCamera(radarScene);
	// }

 	//TODO: comment back in
	// disbled optimizer for now, takes a lot of time to execute
	// std::cout << "[TroenGameBuilder::composeSceneGraph] starting Optimizer" << std::endl;
	// osgUtil::Optimizer optimizer;
	// optimizer.optimize(t->m_rootNode, 		optimizer.TRISTRIP_GEOMETRY | optimizer.OPTIMIZE_TEXTURE_SETTINGS |
	// 	optimizer.VERTEX_POSTTRANSFORM | optimizer.INDEX_MESH);
	// std::cout << "[TroenGameBuilder::composeSceneGraph] done optimizing" << std::endl;


	return true;
}

bool TroenGameBuilder::setupReflections()
{
	for (auto player : t->m_playersWithView)
	{
		player->setupReflections(t, t->m_sceneNode);
	}

	return true;
}

bool TroenGameBuilder::buildInput()
{
	for (auto player : t->m_players)
	{
		// attach all keyboard handlers to all gameViews
		// if (player->bikeController()->hasKeyboardHandler())
		// {
		// 	for (auto otherPlayer : t->m_playersWithView)
		// 	{
		// 		otherPlayer->gameView()->addEventHandler(player->bikeController()->keyboardHandler());
		// 	}
		// }
	}
	return true;
}

bool TroenGameBuilder::buildPhysicsWorld()
{
	t->m_physicsWorld = std::make_shared<PhysicsWorld>(t->m_gameLogic, t->m_gameConfig->useDebugView);
	t->m_levelController->attachWorld(t->m_physicsWorld);
	t->m_levelController->addRigidBodiesToWorld();

	// attach world
	for (auto player : t->m_players)
	{
		player->bikeController()->attachWorld(t->m_physicsWorld);
		player->fenceController()->attachWorld(t->m_physicsWorld);
	}

	return true;
}


////////////////////////////////////////////////////////////////////////////////
//
// Shutdown
//
////////////////////////////////////////////////////////////////////////////////

bool TroenGameBuilder::destroy()
{
	// clean up in reverse order from initialization

	// AIs

	for (auto player : t->m_players) {
		player->bikeController()->killThread();
	}

	t->m_gameloopTimer.reset();
	t->m_gameTimer.reset();

#ifdef WIN32
	input::Gamepad::clearPorts();
#endif
	input::GamepadVRPN::reset();
	t->m_statsHandler = NULL;

	t->m_physicsWorld.reset();
	t->m_gameLogic.reset();

	t->m_rootNode = NULL;
	t->m_levelController.reset();

	for (auto player : t->m_players)
    {
		player.reset();
	}
    for (auto player : t->m_playersWithView)
	{
		player.reset();
	}
	t->m_players.clear();
	t->m_playersWithView.clear();

	if (omega::SystemManager::instance()->isMaster()){
		t->m_audioManager->StopSFXs();
		t->m_audioManager->StopSongs();
		t->m_audioManager.reset();
	}

	shaders::m_allShaderPrograms.clear();

	std::cout << "[TroenGameBuilder::destroy] shutdown complete " << std::endl;
	return true;
}