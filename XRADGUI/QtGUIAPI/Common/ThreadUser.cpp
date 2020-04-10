//	file ThreadUser.cpp
//	Created by IRD on 05.2013
//  Version 3.0.2
//--------------------------------------------------------------
#include "pre.h"
#include "ThreadUser.h"

#include "GraphWindow.h"
#include "ImageWindow.h"
#include "PauseDialog.h"
#include "GUIController.h"

namespace XRAD_GUI
{

XRAD_USING


ThreadUser::ThreadUser(GUIController &in_gui_controller) :
	gui_controller(in_gui_controller),
	workthread_is_running(false),
	suspending_window(NULL)
{
}


ThreadUser::~ThreadUser()
{
}



void ThreadUser::run()
{
	try
	{
		workthread_is_running = true;
		gui_controller.xrad_main_call();
		workthread_is_running = false;
	}
	catch(exception &ex)
	{
		emit request_ShowMessage(QMessageBox::Critical, "Unhandled std::exception. Program will be terminated", ex.what());
		workthread_is_running = false;
	}
	catch(...)
	{
		emit request_ShowMessage(QMessageBox::Critical, "Unhandled exception. Program will be terminated", "No information");
		workthread_is_running = false;
	}
}


//--------------------------------------------------------------
//
//	Suspend, resume, sleep
//

void ThreadUser::Suspend(suspend_mode mode)
{
	// вызывается только из рабочего потока
	if(qobject_cast<QThread*>(this) != currentThread())
	{
		return;
	}
	switch(mode)
	{
		case suspend_for_dialog:
			emit request_Dialog();
			break;
		default:
		case suspend_for_data_analyze:
			emit request_InterfacePaused();
	}
	QMutex mutex;
	mutex.lock(); // In Qt5 it must be locked.
	waitCondition.wait(&mutex);
	mutex.unlock();
	if (BreakOnGUIReturn())
	{
		// Выход в отладчик должен срабатывать и в Debug, и в Release.
		XRAD_FORCE_DEBUG_BREAK_FUNCTION();
	}
}

void ThreadUser::Resume()
{
	// вызывается только из GUI потока
	if(suspending_window)
	{
	// если поток был остановлен ради какого-то окна, его закрытие приводит к выходу из сна (см. ThreadGUI::do_ShowDataWindow).
	// если рабочий поток "разбужен" через control-G без закрытия окна, эта зависимость отменяется:
		disconnect(suspending_window, &QDialog::finished, this, &ThreadUser::Resume);
		disconnect(suspending_window, SIGNAL(signal_esc()), 0, 0);
		suspending_window = NULL;
	}
	emit request_InterfaceResumed();
	waitCondition.wakeAll();
}

void ThreadUser::Sleep(double time_in_seconds)
{
	// вызывается только из рабочего потока
	if(qobject_cast<QThread*>(this) != currentThread())
	{
		return;
	}
	// с учетом загрубления задержек внутри функций QThread::_sleep() вводим поправки:
	// задержки менее 5 мс считаем равными нулю
	if(time_in_seconds < 5e-3) return;
	if(time_in_seconds > 0.1)
		msleep(long(time_in_seconds * 1.e3) - 1);
	else
		usleep(long(time_in_seconds * 1.e6) - 1);
}

void ThreadUser::WaitForNonModalDialog()
{
	// TODO: Эта функция вызывается из рабочего потока, но обращается к данным GUI-потока.
	if(!gui_controller.ActiveProgressBar())
	{
		Suspend(suspend_for_dialog);
	}
	ForceUpdateGUI(gui_controller.GUI_update_interval);
}

//
//--------------------------------------------------------------



//--------------------------------------------------------------



void ThreadUser::ForceUpdateGUI_noexcept(const physical_time &update_interval)
{
	//TODO UpdateGUI прокомментировать тщательно.
	static	physical_time previous = clocks(clock());
	physical_time	current = clocks(clock());
	physical_time	delay = current-previous;

	if (delay >= update_interval)
	{
		emit request_UpdateConsole();
		previous = current;
	}

	if(gui_controller.workthread_pause_scheduled)
	{
		suspending_window = NULL;
		Suspend(suspend_for_data_analyze);
	}
}

void ThreadUser::ForceUpdateGUI(const physical_time &update_interval)
{
	//TODO UpdateGUI прокомментировать тщательно.
	ForceUpdateGUI_noexcept(update_interval);

	if(gui_controller.cancel_scheduled)
	{
		gui_controller.cancel_scheduled = false;
		gui_controller.progress_bar_counter->_reset();
		//emit request_EndProgress(gui_controller.progress_bar_counter->_level());
		//TODO request_EndProgress здесь неизящно выглядит.
		// если эта функция вызвана в отсутствие активного индикатора прогресса, то вроде и завершать нечего.
		// внутри есть проверка, но название нелогично.
		// то же см. и по quit_scheduled
		// Работа с прогрессом ведется через объекты, которые в деструкторе завершают прогресс.
		// Принудительное закрытие является лишним и опасным. [АБЕ 2019-01-21]
		throw canceled_operation("Action canceled by user request");
	}

	if(gui_controller.quit_scheduled)
	{
		gui_controller.quit_scheduled = false;
		gui_controller.progress_bar_counter->_reset();
		//emit request_EndProgress(gui_controller.progress_bar_counter->_level());
		throw quit_application("Application quit by user request (via interactive command)", 0);
	}
}



}//namespace XRAD_GUI
