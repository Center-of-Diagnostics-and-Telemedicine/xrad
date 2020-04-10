#include "pre.h"
#include "ConsoleWindow.h"

#include "ThreadGUI.h"
#include "GUIController.h"
#include "GUIQtUtils.h"
#include "SavedSettings.h"
#include "WorkflowControl.h"
#include <XRADGUI/Sources/GUI/I18nSupport.h>

//--------------------------------------------------------------

namespace XRAD_GUI
{
XRAD_USING



//TODO следует совместить два механизма паузы и возобновления потоков: через команду меню и через вызов останова диалога
//TODO отдельно рассмотреть, почему пауза в прогрессе не срабатывает

ConsoleWindow::ConsoleWindow(GUIController &in_gui_globals, QWidget *parent, Qt::WindowFlags flags):
	QMainWindow(parent, flags),
	gui_controller(in_gui_globals)
#ifdef XRAD_COMPILER_MSC
	, default_priority(GetPriorityClass(GetCurrentProcess()))
#endif // XRAD_COMPILER_MSC
{
	ui.setupUi(this);

	console_redirector = make_unique<ConsoleRedirector>(
			[this](QString s) { this->PutStdoutData(s); },
			[this](QString s) { this->PutStderrData(s); });

	bool log_to_files = !!GUILoadParameter("GUI", "LogsWriteToFiles", (size_t)0);
	console_redirector->LogToFiles(log_to_files);

	bool show_special_characters = !!GUILoadParameter("GUI", "LogsShowSpecialCharacters", (size_t)0);
	ui.stdout_console->ShowSpecialCharacters(show_special_characters);
	ui.stderr_console->ShowSpecialCharacters(show_special_characters);

#if 0
	QTextCodec *codec = QTextCodec::codecForName("CP1251");
#if QT_VERSION < 0x050000
	QTextCodec::setCodecForTr(codec);
#else
	QTextCodec::setCodecForLocale(codec);
#endif
#endif

	ui.actionPause->setShortcut(QKeySequence("Ctrl+,"));
	ui.actionPause->setShortcutContext(Qt::ApplicationShortcut);

	ui.actionRun->setShortcut(QKeySequence("Ctrl+G"));
	ui.actionRun->setShortcutContext(Qt::ApplicationShortcut);

	ui.actionQuit->setShortcut(QKeySequence("Ctrl+Q"));
	ui.actionQuit->setShortcutContext(Qt::ApplicationShortcut);

	ui.actionDebug->setChecked(gui_controller.work_thread->BreakOnGUIReturn());

	ui.actionLogsShowSpecialCharacters->setChecked(show_special_characters);
	ui.actionLogsWriteToFiles->setChecked(log_to_files);

	ui.actionAllowStayOnTop->setChecked(gui_controller.GetStayOnTopAllowed());

	auto language_group = new QActionGroup(this);
	language_group->addAction(ui.actionLangEn);
	language_group->addAction(ui.actionLangRu);

	auto priority_group = new QActionGroup(this);
	priority_group->addAction(ui.actionPriorityDefault);
	priority_group->addAction(ui.actionPriorityNormal);
	priority_group->addAction(ui.actionPriorityIdle);
	ui.actionPriorityDefault->setChecked(true);

	XRAD_GUI_connect_auto(ui.actionQuit, triggered, this, setup_commands_QuitScheduled);
	XRAD_GUI_connect_auto(ui.actionQuit, triggered, this, FinishWorkthread);
	XRAD_GUI_connect_auto(ui.actionDebug, triggered, this, command_DebugChecked);
	XRAD_GUI_connect_auto(ui.menuPriority, aboutToShow, this, menuProgramPriorityShow);
	XRAD_GUI_connect_auto(ui.actionPause, triggered, gui_controller.dialogs_creator, do_Pause);
	XRAD_GUI_connect_auto(ui.actionRun, triggered,
			gui_controller.dialogs_creator, do_AcceptAndContinue);
	QObject::connect(ui.actionPriorityDefault, &QAction::triggered,
			[this]() { SetProcessPriority(ProcessPriority::Default); });
	QObject::connect(ui.actionPriorityNormal, &QAction::triggered,
			[this]() { SetProcessPriority(ProcessPriority::Normal); });
	QObject::connect(ui.actionPriorityIdle, &QAction::triggered,
			[this]() { SetProcessPriority(ProcessPriority::Idle); });
	XRAD_GUI_connect_auto(ui.actionLogsShowSpecialCharacters, triggered,
			this, command_LogsShowSpecialCharactersChecked);
	XRAD_GUI_connect_auto(ui.actionLogsWriteToFiles, triggered,
			this, command_LogsWriteToFilesChecked);
	XRAD_GUI_connect_auto(ui.actionAllowStayOnTop, triggered,
			this, command_AllowStayOnTopChecked);
	QObject::connect(ui.menuLanguage, &QMenu::aboutToShow,
			[this, language_group]()
			{
				string lang_id = GetLanguageId();
				if (lang_id == GetStdLanguageIdEn())
					ui.actionLangEn->setChecked(true);
				else if (lang_id == GetStdLanguageIdRu())
					ui.actionLangRu->setChecked(true);
				else
				{
					language_group->setExclusive(false);
					ui.actionLangEn->setChecked(false);
					ui.actionLangRu->setChecked(false);
					language_group->setExclusive(true);
				}
			});
	QObject::connect(ui.actionLangEn, &QAction::triggered,
			[]() { SaveLanguageId(GetStdLanguageIdEn()); });
	QObject::connect(ui.actionLangRu, &QAction::triggered,
			[]() { SaveLanguageId(GetStdLanguageIdRu()); });

	XRAD_GUI_connect_auto(gui_controller.work_thread, request_Dialog,
			this, setup_commands_DialogActive);
	//QObject::connect(XRADGUIClass::work_thread, SIGNAL(request_InterfacePaused()),
	//		this, SLOT(command_Quit()));
	XRAD_GUI_connect_auto(gui_controller.work_thread, request_InterfacePaused,
			this, setup_commands_WorkthreadPaused);
	XRAD_GUI_connect_auto(gui_controller.work_thread, request_InterfaceResumed,
			this, setup_commands_WorkthreadResumed);
	XRAD_GUI_connect_auto(gui_controller.work_thread, finished, this, command_Quit);

	QTextCharFormat format;
	format.setForeground(Qt::green);
	ui.stdout_console->output("XRAD GUI is running\n", format);

	installEventFilter(console_redirector.get());

	SetWindowPosition();

	setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint); // hides some buttons
	//	http://doc.crossplatform.ru/qt/4.5.0/qt.html#WindowType-enum

	SetThreadState(ThreadState::Running);
	LoadProcessPrioritySettings();
}

void ConsoleWindow::SetWindowPosition()
{
	QDesktopWidget desktop;
	QRect console_rect = desktop.screenGeometry(desktop.primaryScreen()); // or screenGeometry(), depending on your needs
	console_rect.setLeft(console_rect.right() - WindowGeometry::console_width()); // правая часть первичного экрана

	if(desktop.screenCount() > 1 && desktop.isVirtualDesktop())
	{
		//если экранов более одного...
		int	secondary_screen_no = desktop.primaryScreen() == 0 ? 1:0;
		if(desktop.screen(secondary_screen_no))
		{
			//...и если экран с заданным номером существует, то занимаем под консоль весь вторичный экран
			console_rect = desktop.screenGeometry(secondary_screen_no);
			console_rect.setLeft(console_rect.left() + 8);
		}
	}

	console_rect.setTop(console_rect.top() + WindowGeometry::top_margin());
	console_rect.setBottom(console_rect.bottom() - WindowGeometry::bottom_margin());
	console_rect.setRight(console_rect.right() - WindowGeometry::right_margin());

	setGeometry(console_rect);
}

ConsoleWindow::~ConsoleWindow()
{
}

void ConsoleWindow::SetThreadState(ThreadState state)
{
	switch (state)
	{
		case ThreadState::Initial:
			ui.actionPause->setEnabled(false);
			ui.actionRun->setEnabled(false);
			ui.actionQuit->setEnabled(true);
			break;
		default:
		case ThreadState::Finished:
			ui.actionPause->setEnabled(false);
			ui.actionRun->setEnabled(false);
			ui.actionQuit->setEnabled(false);
			break;
		case ThreadState::Running:
			ui.actionPause->setEnabled(true);
			ui.actionRun->setEnabled(false);
			ui.actionQuit->setEnabled(true);
			break;
		case ThreadState::Paused:
			ui.actionPause->setEnabled(false);
			ui.actionRun->setEnabled(true);
			ui.actionQuit->setEnabled(true);
			break;
		case ThreadState::Dialog:
			ui.actionPause->setEnabled(false);
			ui.actionRun->setEnabled(false);
			ui.actionQuit->setEnabled(true);
			break;
		case ThreadState::DialogV2:
			ui.actionPause->setEnabled(false);
			ui.actionRun->setEnabled(true);
			ui.actionQuit->setEnabled(true);
			break;
	}
}

void ConsoleWindow::setup_commands_WorkthreadPaused()
{
	SetThreadState(ThreadState::Paused);
}

void ConsoleWindow::setup_commands_DialogActive()
{
	SetThreadState(ThreadState::Dialog);
}


void ConsoleWindow::setup_commands_WorkthreadResumed()
{
	SetThreadState(ThreadState::Running);
}

void ConsoleWindow::setup_commands_QuitScheduled()
{
	SetThreadState(ThreadState::Finished);
}

void ConsoleWindow::command_Quit()
{
	close();
}

void ConsoleWindow::command_DebugChecked()
{
	bool checked = ui.actionDebug->isChecked();
	gui_controller.work_thread->SetBreakOnGUIReturn(checked);
}

void ConsoleWindow::menuProgramPriorityShow()
{
#ifdef XRAD_COMPILER_MSC
	if (priority_setting == ProcessPriority::Default)
		return;
	DWORD actual_priority = GetPriorityClass(GetCurrentProcess());
	if (actual_priority == ProcessPriorityToSystemValue(priority_setting))
		return;
	priority_ui_updating = true;
	ui.actionPriorityDefault->setChecked(true);
	priority_ui_updating = false;
	// TODO: #else
#endif // XRAD_COMPILER_MSC
}

#ifdef XRAD_COMPILER_MSC
DWORD ConsoleWindow::ProcessPriorityToSystemValue(ProcessPriority priority)
{
	switch (priority)
	{
		default:
		case ProcessPriority::Default: return 0;
		case ProcessPriority::Normal: return NORMAL_PRIORITY_CLASS;
		case ProcessPriority::Idle: return IDLE_PRIORITY_CLASS;
	}
}

DWORD ConsoleWindow::ProcessPriorityToUIThreadSystemValue(ProcessPriority priority)
{
	switch (priority)
	{
		default:
		case ProcessPriority::Default: return THREAD_PRIORITY_NORMAL;
		case ProcessPriority::Normal: return THREAD_PRIORITY_NORMAL;
		// При понижении приоритета процесса стараемся сохранить абсолютное значение
		// приоритета GUI-потока.
		// NORMAL_PRIORITY_CLASS, THREAD_PRIORITY_NORMAL => 8
		// IDLE_PRIORITY_CLASS, THREAD_PRIORITY_HIGHEST => 6 (точно значение 8 не получить)
		// См. Scheduling Priorities
		// (https://docs.microsoft.com/ru-ru/windows/desktop/ProcThread/scheduling-priorities).
		case ProcessPriority::Idle: return THREAD_PRIORITY_HIGHEST;
	}
}
#endif // XRAD_COMPILER_MSC

void ConsoleWindow::LoadProcessPrioritySettings()
{
	bool loaded = false;
	auto priority_uint = GUILoadParameter<std::underlying_type_t<ProcessPriority>>(
			"GUI", "ProcessPriority",
			static_cast<std::underlying_type_t<ProcessPriority>>(priority_setting), &loaded);
	if (!loaded)
		return;
	auto priority = priority_setting;
	priority_ui_updating = true;
	switch ((ProcessPriority)priority_uint)
	{
		case ProcessPriority::Default:
			priority = ProcessPriority::Default;
			ui.actionPriorityDefault->setChecked(true);
			break;
		case ProcessPriority::Normal:
			priority = ProcessPriority::Normal;
			ui.actionPriorityNormal->setChecked(true);
			break;
		case ProcessPriority::Idle:
			priority = ProcessPriority::Idle;
			ui.actionPriorityIdle->setChecked(true);
			break;
	}
	priority_ui_updating = false;
	if (priority == priority_setting)
		return;
	SetProcessPriority(priority);
}

void ConsoleWindow::SetProcessPriority(ProcessPriority priority)
{
#ifdef XRAD_COMPILER_MSC
	if (priority_ui_updating)
		return;
	switch (priority)
	{
		case ProcessPriority::Default:
			SetPriorityClass(GetCurrentProcess(), default_priority);
			break;
		case ProcessPriority::Normal:
		case ProcessPriority::Idle:
			SetPriorityClass(GetCurrentProcess(), ProcessPriorityToSystemValue(priority));
			break;
	}
	// При понижении приоритета процесса абсолютный приоритет GUI-потока стараемся оставить
	// на уровне процесса с нормальным приоритетом.
	// Сделать наоборот, изменить приоритет потоков обработки, не меняя класса приориета всего
	// процесса, не получается простым способом,
	// поскольку поток обработки может использовать OMP, который создает дополнительные потоки.
	// Штатного API для задания приоритетов потокам OMP на данный момент [2019-03-15] нет.
	SetThreadPriority(GetCurrentThread(), ProcessPriorityToUIThreadSystemValue(priority));
	priority_setting = priority;
	GUISaveParameter("GUI", "ProcessPriority", (std::underlying_type_t<ProcessPriority>)(priority));
	// TODO: #else
#endif // XRAD_COMPILER_MSC
}

void ConsoleWindow::command_LogsShowSpecialCharactersChecked()
{
	bool checked = ui.actionLogsShowSpecialCharacters->isChecked();
	GUISaveParameter("GUI", "LogsShowSpecialCharacters", (size_t)(checked? 1: 0));
	ui.stdout_console->ShowSpecialCharacters(checked);
	ui.stderr_console->ShowSpecialCharacters(checked);
}

void ConsoleWindow::command_LogsWriteToFilesChecked()
{
	bool checked = ui.actionLogsWriteToFiles->isChecked();
	GUISaveParameter("GUI", "LogsWriteToFiles", (size_t)(checked? 1: 0));
	console_redirector->LogToFiles(checked);
}

void ConsoleWindow::command_AllowStayOnTopChecked()
{
	bool checked = ui.actionAllowStayOnTop->isChecked();
	gui_controller.SetStayOnTopAllowed(checked);
}

void ConsoleWindow::FinishWorkthread()
{
	if(gui_controller.work_thread->IsRunning())
	{
		ScheduleApplicationQuit();
		// завершаем диалоговые формы
		gui_controller.dialogs_creator->do_AcceptAndContinue();
	}
}

void ConsoleWindow::closeEvent(QCloseEvent *event)
{
	// завершаем рабочий поток, если он еще не закрыт
	FinishWorkthread();
	// закрываем все окна с данными
	gui_controller.FinishDialogs();

	if(gui_controller.work_thread->IsRunning())
	{
		gui_controller.dialogs_creator->do_ShowMessage(QMessageBox::Critical, "Fatal error", "Workthread wasn't finilized properly");
	}

	QMainWindow::closeEvent(event);
}

bool ConsoleWindow::eventFilter(QObject *target, QEvent *event)
{
	//	return QObject::eventFilter(target,event);
	return QMainWindow::eventFilter(target, event);
}

void	ConsoleWindow::PutStderrData(QString output_data)
{
	QTextCharFormat format;
	format.setForeground(Qt::red);
	ui.stderr_console->output(output_data, format);
}

void	ConsoleWindow::PutStdoutData(QString output_data)
{
	QTextCharFormat format;
	format.setForeground(Qt::green);
	ui.stdout_console->output(output_data, format);
}

void ConsoleWindow::UpdateConsole()
{
	if (!console_redirector)
		return;
	console_redirector->DoStdoutReady();
	console_redirector->DoStderrReady();
}

}//namespace XRAD_GUI
