#pragma once
// qt
#include <QObject>
//bullet
#include <btBulletDynamicsCommon.h>
#include <osg/Vec3>
// troen
#include "../forwarddeclarations.h"
#include "abstractcontroller.h"

namespace troen
{
	class ItemController : public QObject, public AbstractController
	{
		Q_OBJECT
	public:
		ItemController(btVector3 position, std::weak_ptr<PhysicsWorld> world, TroenGame* troenGame, LevelView* view);

		osg::Vec3 getDimensions();

		enum Type
		{
			TURBOSTRIP, HEALTHUP, RADAR, BENDEDVIEWS, COUNT
		};

		void triggerOn(BikeController* bikeController, GameLogic* gamelogic = NULL);

	public slots:
		void hideFencesInRadarForPlayer();
		void deactivateBendedViews();
	private:
		std::shared_ptr<ItemView> m_itemView;
		std::shared_ptr<ItemModel> m_itemModel;
		Type m_type;
		btVector3 m_position;
		int m_id;
		BikeController* m_bikeController;
		GameLogic* m_gamelogic;
		TroenGame* m_troenGame;

		void remove();
		void destroy();
	};
}