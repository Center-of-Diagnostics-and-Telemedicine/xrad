/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef SIMPLETEST_QT_H
#define SIMPLETEST_QT_H

#include <XRADGUI/Sources/Internal/std.h>
#include "ui_ConsoleWindow.h"
#include "OutputSubstitution.h"

namespace XRAD_GUI
{

class GUIController;

class ConsoleWindow : public QMainWindow
{
	Q_OBJECT
	public:
		ConsoleWindow(GUIController &in_gui_globals, QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());
		~ConsoleWindow();

	private:
		GUIController &gui_controller;
		Ui::XRADMainWindow ui;
		void closeEvent(QCloseEvent *event);
		unique_ptr<ConsoleRedirector> console_redirector;
		bool eventFilter(QObject *target,QEvent *event);
		void	SetWindowPosition();

	public:
		void	UpdateConsole();

	public:
		enum class ThreadState { Initial = 0, Running, Paused, Dialog, DialogV2, Finished };
		ThreadState GetThreadState() const { return thread_state; }
		void SetThreadState(ThreadState state);

	private:
		ThreadState thread_state = ThreadState::Initial;

	public slots:
		// menu Program
		void command_Quit();
		void command_DebugChecked();
		void menuProgramPriorityShow();

		void command_LogsShowSpecialCharactersChecked();
		void command_LogsWriteToFilesChecked();
		void command_AllowStayOnTopChecked();

		void FinishWorkthread();

		void setup_commands_WorkthreadPaused();
		void setup_commands_WorkthreadResumed();
		void setup_commands_DialogActive();
		void setup_commands_QuitScheduled();

		void command_HelpAbout();

		//void userCommand(const QString  &command);
	private:
		//! \brief Тип uint и числовые значения используются при сериализации, не менять
		enum class ProcessPriority: uint { Default = 0, Normal = 1, Idle = 2 };
#ifdef XRAD_COMPILER_MSC
		DWORD ProcessPriorityToSystemValue(ProcessPriority priority);
		DWORD ProcessPriorityToUIThreadSystemValue(ProcessPriority priority);
#endif // XRAD_COMPILER_MSC
		void LoadProcessPrioritySettings();
		void SetProcessPriority(ProcessPriority priority);

	private:
		ProcessPriority priority_setting = ProcessPriority::Default;
#ifdef XRAD_COMPILER_MSC
		DWORD default_priority = GetPriorityClass(GetCurrentProcess());
#endif // XRAD_COMPILER_MSC
		bool priority_ui_updating = false;

	private:
		void	PutStderrData(QString output_data);
		void	PutStdoutData(QString output_data);
};



struct WindowGeometry
{
	static int console_width(){return 600;}

	// геометрия виджета QWidget::geometry() считается без рамки и заголовка окна.
	// некогда было искать, где эти сведения содержатся, поэтому пока используем константы.
	// потом найти!
	static int top_margin(){return 29;}
	static int bottom_margin(){ return 8; }
	static int right_margin(){ return 8; }
	static int left_margin(){ return 8; }

	static int	window_stair_step(){ return 16; }//шаг в пикселях, с которым вычисляется автоматическое положение нового окна данных на экране
	static int	lowest_window_position(){return 800;}
	static QSize default_graph_window_size(){return QSize(512, 384);}
};



}//namespace XRAD_GUI

#endif // SIMPLETEST_QT_H
