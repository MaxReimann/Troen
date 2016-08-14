#include <reflectionzeug/Object.h>
#include <scriptzeug/ScriptContext.h>

#include "aipy.h"
// troen
#include "bikeinputstate.h"
#include "../constants.h"
#include "../globals.h"
// other
#include <cmath>

// #include <boost/python.h>

#define OMEGA_NO_GL_HEADERS
#include <omega.h>
#include <omegaToolkit.h>
#include <omegaOsg/omegaOsg.h>

// #include <boost/python.hpp>

#include "../controller/bikecontroller.h"

using namespace troen::input;

BOOST_PYTHON_MODULE(cyclops)
{
    // SceneLoader
    class_<AIPy, boost::noncopyable >("AI", no_init)
        PYAPI_GETTER(AIPy, getAcceleration)
        ;

}




AIPy::AIPy(osg::ref_ptr<BikeInputState> bikeInputState, troen::BikeController* bikeController)
	: PollingDevice(bikeInputState)
{
	m_bikeController = bikeController;
	m_bikeInputState = bikeInputState;

	// addFunction("getFreeDistanceInDirection", this, &AIPy::getFreeDistanceInDirection);
	// addFunction("getID", this, &AIPy::getID);
	// addFunction("getHealth", this, &AIPy::getHealth);
	// addFunction("log", this, &AIPy::log);
	// addFunction("setTurbo", this, &AIPy::setTurbo);

	// addProperty<double>("acceleration", this, &AIPy::getAcceleration, &AIPy::setAcceleration);
	// addProperty<double>("angle", this, &AIPy::angle, &AIPy::setAngle);
	// addProperty<double>("distanceToNextObstacle", this, &AIPy::distanceToNextObstacle, &AIPy::setDistanceToNextObstacle);
}

void AIPy::run()
{
	PollingDevice::run();

	m_pollingEnabled = true;


	omega::PythonInterpreter* interp = omega::SystemManager::instance()->getScriptInterpreter();
	// queue command and only execue on master
	
  	QFile scriptFile("scripts/ai.py");

  	if (!scriptFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		std::cout << "could not read ai.py" << std::endl;
	    return;
	}


	interp->queueCommand("import AI", false);

	QTextStream inScript(&scriptFile);
	QString scriptString = inScript.readAll();

		
	while (m_pollingEnabled)
	{
		setDistanceToNextObstacle(m_bikeController->getDistanceToObstacle(0));

		// reflectionzeug::Variant value;
		// value = g_scriptingThread.evaluate("try {     if (typeof move !== 'undefined') { move() } else { player.angle = 0 }      } catch (ex) { ex } ");
		interp->queueCommand(scriptString.toStdString().c_str(), false);

		
		m_bikeInputState->setAcceleration(getAcceleration());
		m_bikeInputState->setAngle(angle());

		QCoreApplication::processEvents();
		this->msleep(POLLING_DELAY_MS * 2);
	}
}