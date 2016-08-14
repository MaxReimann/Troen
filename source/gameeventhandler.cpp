#include "gameeventhandler.h"
// troen
#include "troengame.h"
#include "gamelogic.h"
#include "view/shaders.h"

#ifdef USE_BENDEDVIEWS
	#include "BendedViews/src/SplineDeformationRendering.h"
#endif

using namespace troen;

GameEventHandler::GameEventHandler(TroenGame * game) :
    m_troenGame(game)
{}

void GameEventHandler::attachGameLogic(std::shared_ptr<GameLogic>& gamelogic)
{
    m_gameLogic = gamelogic;
}

 bool GameEventHandler::handleEvent(const omega::Event& event)
 {
	 // if (event.getType() == osgGA::GUIEventAdapter::RESIZE)
	 // {
	 // 	 omega::DisplaySystem* ds = omega::SystemManager::instance()->getDisplaySystem();
  //  		 omega::Vector2i resolution = ds->getDisplayConfig().getCanvasRect().size();

		//  // re setup textures to new size
		//  m_troenGame->resize(resolution[0], resolution[1]);
		//  return false;
	 // }


	 if (event.getType() != omega::Event::Down) {
		 return false;
	 }


	 if (event.isKeyDown('m')) { m_troenGame->switchSoundVolumeEvent(); }
	 else if (event.isKeyDown('c')){  m_gameLogic.lock()->removeAllFences(); }
	 else if (event.isKeyDown('f')){	 m_gameLogic.lock()->toggleFencePartsLimit(); }
	 else if (event.isKeyDown('l')){	 m_troenGame->reloadLevel(); }
	 else if (event.isKeyDown('r')){
		std::cout << "Reloading shaders" << std::endl;
		shaders::reloadShaders();
		#ifdef USE_BENDEDVIEWS
			m_troenGame->getBendedViews()->reloadShaders(); 
		#endif
	 }
	 else if (event.isKeyDown(omega::KC_SHIFT_R) || event.isKeyDown(omega::KC_SHIFT_L)) {
		 m_troenGame->pauseEvent();
	 }
	 else if (event.isKeyDown(127 /*delete*/)){	 m_gameLogic.lock()->restartLevel(); }
#ifdef USE_BENDEDVIEWS
	 else if (event.isKeyDown('1')){	m_troenGame->getBendedViews()->setPreset(0); }
	 else if (event.isKeyDown('2')){	m_troenGame->getBendedViews()->setPreset(1); }
	 else if (event.isKeyDown('3')){	m_troenGame->getBendedViews()->setPreset(2); }
	 else if (event.isKeyDown('4')){	m_troenGame->getBendedViews()->setPreset(3); }
#endif
	 else if (event.isKeyDown(omega::KC_TAB)){	m_troenGame->toggleHUDVisibility(); }
	 else {return false;}

    return true;
}