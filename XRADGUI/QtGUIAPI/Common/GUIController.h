//	file XRADGUIFunction.h
//	Created by IRD on 05.2013
//  Version 3.0.2
//--------------------------------------------------------------
#ifndef XRADGUIFunctionH
#define XRADGUIFunctionH

#include "ThreadGUI.h"
#include "ThreadUser.h"
#include "ConsoleWindow.h"
#include <set>

namespace XRAD_GUI
{

//#error продолжить уничтожение static

struct GUIController
{
	GUIController(int *in_result_ptr, int in_argc, char *in_argv[]);
	~GUIController();

	void	xrad_main_call();

	ThreadUser* work_thread;//!< указатель на рабочий поток
	//		static	QThread *interface_thread;// пока не инициализируется
	ConsoleWindow	*main_window;//!< указатель на главное окно интерфейса
	QApplication	*xrad_application;
	ThreadGUI *dialogs_creator;

	static	ProgressBarCounter *progress_bar_counter;

	bool	workthread_pause_scheduled;
	bool	cancel_scheduled;
	bool	quit_scheduled;

	const physical_time	GUI_update_interval;
	const physical_time	progress_update_interval;

	int argc;
	char **argv;
	int* result_ptr;

	bool IsProgressActive();
	bool	ActiveProgressBar();

	inline bool	IsPauseScheduled(){ return workthread_pause_scheduled; }
	inline bool	IsQuitScheduled(){ return quit_scheduled; }

	static bool GetStayOnTopAllowed();
	static void SetStayOnTopAllowed(bool allow);
	static void AddGUISettingsChangedTarget(IGUISettingsChanged *target);
	static void RemoveGUISettingsChangedTarget(IGUISettingsChanged *target);
	static void NotifyGUISettingsChanged();

	static bool StayOnTopAllowed;
	static set<IGUISettingsChanged*> GUISettingsChangedTargets;

	void InitDialogs();
	void AddWidget(QWidget *);
	bool WidgetExists(const QWidget *);
	void RemoveWidget(QWidget *);

	void FinishDialogs(void);

//	static void	Initialize(int *in_result_ptr, int in_argc, char *in_argv[]);
// 	static void	Finalize();
	int	Run();

	static void InitLogFile();

	private:
		vector<QWidget*> widgets;
};

bool IsPointerAValidGUIWidget(const void *);
extern shared_ptr<GUIController> global_gui_controller;

}//namespace XRAD_GUI

#endif //XRADGUIFunctionH
