#include "hudcontroller.h"
// std
#include <chrono>
#include <regex>
// troen
#include "../constants.h"
#include "../globals.h"
#include "../gamelogic.h"
#include "../player.h"
#include "../controller/bikecontroller.h"
#include "../view/hudview.h"

namespace
{
	const std::vector<std::string> killMessages
	{
	"PLAYER_X\njust hit your fence hard",
	"You just fenced\nPLAYER_X"
	};
	const std::vector<std::string> selfKillMessages
	{
		"That was your own fence ...",
		"You just hit your own fence ..."
	};
	const std::vector<std::string> diedMessages
	{
		"PLAYER_X\njust hit a wall",
		"PLAYER_X\njust crashed!"
	};
	const std::string diedOnFenceMessage("PLAYER_X\njust crashed into PLAYER_Y's fence");
	const std::string diedOnFallMessage("PLAYER_X\njust fell into the abyss");
}

using namespace troen;

HUDController::HUDController(const int id,
	const std::vector<std::shared_ptr<Player> >& players) :
AbstractController(),
m_player(players[id])
{
	m_view = m_HUDView = std::make_shared<HUDView>(id, players);

	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	m_randomGenerator.seed(seed);
}

void HUDController::resize(const int width, const int height)
{
	m_HUDView->resize(width, height);
}

void HUDController::attachSceneToRadarCamera(osg::Group* scene)
{
	m_HUDView->attachSceneToRadarCamera(scene);
}

void HUDController::update(
	const long double currentGameloopTime,
	const long double currentGameTime,
	const int timeLimit,
	const GameLogic::GAMESTATE gameState,
	const std::vector<std::shared_ptr<Player> >& players)
{
	std::shared_ptr<HUDView> hudview = m_HUDView;
	std::shared_ptr<Player> player = m_player.lock();

	hudview->setSpeedText(player->bikeController()->speed());
    hudview->updateRadarCamera();
	hudview->setTimeText(currentGameTime, timeLimit);
	hudview->setHealthText(100 * player->health() / BIKE_DEFAULT_HEALTH);
	hudview->setPointsText(player->points());
	for (auto player : players)
	{
		hudview->setKillCountText(player->id(), player->name(), player->killCount());
	}

	//
	// Countdown
	//
	BikeController::BIKESTATE bikeState = player->bikeController()->state();
	if (gameState == GameLogic::GAMESTATE::GAME_OVER)
	{
		hudview->setCountdownText("GameOver");
	}
	else if (bikeState == BikeController::BIKESTATE::RESPAWN || bikeState == BikeController::BIKESTATE::RESPAWN_PART_2)
	{
		double respawnTime = player->bikeController()->respawnTime();
		hudview->setCountdownText((int)(respawnTime - currentGameTime + RESPAWN_DURATION) / 1000 + 1);
	}
	else if (bikeState == BikeController::BIKESTATE::WAITING_FOR_GAMESTART)
	{
		double respawnTime = player->bikeController()->respawnTime();
		hudview->setCountdownText((int)(respawnTime - currentGameloopTime + GAME_START_COUNTDOWN_DURATION) / 1000 + 1);
	}
	else
	{
		hudview->setCountdownText(-1);
	}

	//
	// ingame messages
	//
	while (!m_ingameMessages.empty() && m_ingameMessages.front()->endTime < currentGameTime)
	{
		m_ingameMessages.pop_front();
	}
	hudview->updateIngameMessageTexts(m_ingameMessages);
}

void HUDController::setTrackNode(osg::Node* trackNode)
{
    m_HUDView->setTrackNode(trackNode);
}


void HUDController::addKillMessage(Player* player)
{
	std::shared_ptr<IngameMessage> message = std::make_shared<IngameMessage>();

	std::uniform_int_distribution<int> distribution(0, killMessages.size() - 1);
	int n = distribution(m_randomGenerator);

	std::string text = killMessages[n];
	std::regex reg("PLAYER_X");
	text = std::regex_replace(text, reg, player->name());

	message->text = text;
	message->color = osg::Vec4(player->color(), 1);
	message->endTime = g_gameTime + RESPAWN_DURATION;

	m_ingameMessages.push_back(message);
}

void HUDController::addSelfKillMessage()
{
	std::shared_ptr<IngameMessage> message = std::make_shared<IngameMessage>();

	std::uniform_int_distribution<int> distribution(0, selfKillMessages.size() - 1);
	int n = distribution(m_randomGenerator);

	message->text = selfKillMessages[n];
	message->color = osg::Vec4(m_player.lock()->color(),1);
	message->endTime = g_gameTime + RESPAWN_DURATION;

	m_ingameMessages.push_back(message);
}

void HUDController::addDiedMessage(Player* player)
{
	std::shared_ptr<IngameMessage> message = std::make_shared<IngameMessage>();

	std::uniform_int_distribution<int> distribution(0, diedMessages.size() - 1);
	int n = distribution(m_randomGenerator);

	std::string text = diedMessages[n];
	std::regex reg("PLAYER_X");
	text = std::regex_replace(text, reg, player->name());

	message->text = text;
	message->color = osg::Vec4(player->color(), 1);
	message->endTime = g_gameTime + RESPAWN_DURATION;

	m_ingameMessages.push_back(message);
}

void HUDController::addDiedOnFenceMessage(Player* bikePlayer, Player* fencePlayer)
{
	std::shared_ptr<IngameMessage> message = std::make_shared<IngameMessage>();

	std::string text = diedOnFenceMessage;
	std::regex regx("PLAYER_X");
	text = std::regex_replace(text, regx, bikePlayer->name());
	std::regex regy("PLAYER_Y");
	text = std::regex_replace(text, regy, fencePlayer->name());

	message->text = text;
	message->color = osg::Vec4(bikePlayer->color(), 1);
	message->endTime = g_gameTime + RESPAWN_DURATION;

	m_ingameMessages.push_back(message);
}

void HUDController::addDiedOnFallMessage(Player* deadPlayer)
{
	std::shared_ptr<IngameMessage> message = std::make_shared<IngameMessage>();

	std::string text = diedOnFallMessage;
	std::regex regx("PLAYER_X");
	text = std::regex_replace(text, regx, deadPlayer->name());

	message->text = text;
	message->color = osg::Vec4(deadPlayer->color(), 1);
	message->endTime = g_gameTime + RESPAWN_DURATION;

	m_ingameMessages.push_back(message);
}

void HUDController::toggleVisibility()
{
	m_HUDView->toggleVisibility();
}