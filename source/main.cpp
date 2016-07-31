// STD
#include <iostream>
// Qt
#include <QApplication>
#include <QCommandLineParser>
//troen
#include "mainwindow.h"
#include "troengame.h"

#define EXTERN
#include "globals.h"


#include <stdio.h>
#include <string.h>
#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"
#include "RakNetTypes.h"  // MessageID

//#include "network\networkmanager.h"



using namespace troen;

/*! \file The main file is the entry point of the c++ application and sets up the Qt app (see mainwindow.cpp).*/

int main(int argc, char* argv[])
{


	int result = -1;

	// register meta types
	qRegisterMetaType<troen::GameConfig>("GameConfig");
	qRegisterMetaType<troen::CArguments>("CArguments");

	// setup application settings
	QApplication::setApplicationName("Troen");
	QApplication application(argc, argv);


	// QCommandLineParser parser;
 //    parser.setApplicationDescription("Troen: An action packed 3D-remake of the classic Armagetron featuring polished graphics, customizable maps, powerups, scriptable AI, multiplayer and many more!");
 //    parser.addHelpOption();
 //    parser.addVersionOption();

 //    parser.addOptions({
 //        // A boolean option with multiple names (-f, --force)
 //        {"troen-remote-start",
 //            QCoreApplication::translate("main", "Remote rendering client startup. Does not open qt window")},
 //    });



    // // Process the actual command line arguments given by the user
    // parser.process(application);

    // bool remoteStart = parser.isSet("troen-remote-start");

	std::vector<char*> unprocessedOptions;
	int numPassedOptions = argc;

	bool remoteStart = false;

    for (int i=0; i < argc; i++)
    {
    	if (strcmp(argv[i], "--troen-remote-start") == 0 )
    	{
    		remoteStart = true;
    		numPassedOptions--;
    	} else {
    		unprocessedOptions.push_back(argv[i]);
    	}
    }


    CArguments args;
    args.argc = numPassedOptions;
    args.argv = unprocessedOptions.data();

	troen::MainWindow mainWindow(args);
    
    if (!remoteStart){
		mainWindow.show();
		result = application.exec();
    } else   {
    	// create GameThread and Game
		std::shared_ptr<GameThread> troenGameThread = std::make_shared<GameThread>(new QThread());
		GameConfig gameConfig(mainWindow.loadSettingsToConfig());

		// m_troenGame = new TroenGame(m_gameThread);
		troenGameThread->prepareAndStartGame(gameConfig, args);
    }


	// Clean Up
	// delete mainWindow;
	// delete application;

	return result;
}