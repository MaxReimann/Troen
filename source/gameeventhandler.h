#pragma once
// OSG
#include <osgViewer/ViewerEventHandlers>
// troen
#include "forwarddeclarations.h"
#include <osg/ref_ptr>
#include <osg/Referenced>

#define OMEGA_NO_GL_HEADERS
#include <omega.h>

namespace troen
{

	/*! The GameEventHandler handles keypresses which are independent of the current player. This includes toggling music, pausing/reloading, clearing the fences etc. The necessary actions get delegated to the responsible classes. */
	class GameEventHandler : public osg::Referenced
	{
	public:
		GameEventHandler(TroenGame * game);
		void attachGameLogic(std::shared_ptr<GameLogic>& gamelogic);
		virtual bool handleEvent(const omega::Event& evt);
		
	private:
		TroenGame*					m_troenGame;
		std::weak_ptr<GameLogic>	m_gameLogic;
	};
}