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


	std::vector<char*> unprocessedOptions;
	int numPassedOptions = argc;

	bool remoteStart = false;

	std::cout << "Troen started with: ";

    for (int i=0; i < argc; i++)
    {
    	std::cout << argv[i] << " ";
    	if (strcmp(argv[i], "--troen-remote-start") == 0 )
    	{
    		remoteStart = true;
    		numPassedOptions--;
    	} else {
    		unprocessedOptions.push_back(argv[i]);
    	}
    }

   	std::cout << std::endl;


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