#pragma once
//Qt
#include <QWidget>
#include <QMainWindow>
#include <QStatusBar>
#include <QPushButton>
#include <QColorDialog>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
// troen
#include "forwarddeclarations.h"
#include "troengame.h"
#include "gamethread.h"
#include "qlabel.h"


namespace troen
{
	
	/*! The MainWindow class is responsible for showing a basic game window. It includes GUI elements for settings like the names of players, whether effects like postprocessing or reflections are enabled and if the game should be played via network.*/

	class MainWindow : public QMainWindow
	{
		Q_OBJECT

	public:
		MainWindow (CArguments& arguments, QWidget * parent = NULL);
		virtual ~MainWindow();
		GameConfig getGameConfig();
		GameConfig loadSettingsToConfig();

	signals:
		void startGame(const GameConfig config, const CArguments arguments);


	public slots:
		void updatePlayerInputBoxes();
		void addNetworkPlayer(QString name);
		void changeLevel(int levelID);

	protected:
		virtual bool eventFilter(QObject* object, QEvent* event);
		virtual void childEvent(QChildEvent* e);

	private slots:
		void prepareGameStart();
		void bikeNumberChanged(const int newBikeNumber);
		void chooseColor(const int i);
		void connectNetworking();
		void connectionTypeChanged();
		void showPlayerNameRefused();

	private:
		void loadSettings();
		void saveSettings();
		QString			m_settingsFileName;
		QStatusBar*		m_statusBar;
		QPushButton*	m_gameStartButton;
		QPushButton*  m_connectNetworkButton;
		QSpinBox*		m_bikeNumberSpinBox;
		QVector<QComboBox*>	m_playerComboBoxes;
		QVector<QColor> m_playerColors;
		QVector<QPushButton*> m_colorButtons;
		QVector<QLineEdit*> m_playerNameLineEdits;
		QCheckBox*		m_fullscreenCheckBox;
		QCheckBox*		m_postProcessingCheckBox;
		QCheckBox*		m_testPerformanceCheckBox;
		QCheckBox*		m_debugViewCheckBox;
		QCheckBox*		m_reflectionCheckBox;
		QCheckBox*		m_serverCheckBox;
		QLabel*			m_statusLabel;
		
		std::vector<QCheckBox*> m_ownViewCheckboxes;
		QSpinBox*		m_timeLimitSpinBox;
		QComboBox* m_levelComboBox;

		TroenGame*	m_troenGame;
		std::shared_ptr<GameThread> m_troenGameThread;
		QThread*	m_gameThread;

		double		m_lastTime;
		float		m_fps;
		bool		m_networkingReady;
		QLineEdit* m_connectAdressEdit;
		int m_networkPlayers;
		CArguments* m_arguments;
	};
}