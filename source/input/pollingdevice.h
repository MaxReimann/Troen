#pragma once
// Qt
#include <QThread>
// troen
#include "../forwarddeclarations.h"

#define OMEGA_NO_GL_HEADERS
#include <omega.h>

// OSG
#include <osg/ref_ptr>
#include <osg/Referenced>



namespace troen
{
	namespace input
	{
		/*! The PollingDevice is an abstract class which inherits from QThread. It is used as the base class for controller input and AI.*/
		class PollingDevice : public osg::Referenced, public QThread
		{
		public:
			PollingDevice(osg::ref_ptr<BikeInputState> bikeInputState);
			virtual void run();
			void stop();
			virtual void setVibration(const bool b) { m_vibrationEnabled = b; };
			virtual void handleEvent(const omega::Event &evt) {};
		protected:
			osg::ref_ptr<BikeInputState> m_bikeInputState;

			bool m_pollingEnabled;
			bool m_vibrationEnabled;
		};
	}
}