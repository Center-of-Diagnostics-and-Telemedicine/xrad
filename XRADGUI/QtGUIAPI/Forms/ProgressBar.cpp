//--------------------------------------------------------------
#include "pre_GUI.h"
#include "ProgressBar.h"
#include "GUIController.h"
#include "ConsoleWindow.h"
#include <XRADBasic/Sources/Utils/TimeProfiler.h>

//--------------------------------------------------------------

namespace XRAD_GUI
{

bool	ProgressBar::geometry_stored = false;
QRect	ProgressBar::progress_geometry;

ProgressBar::ProgressBar(QString prompt, double count) :
	current_value(0), max_value(count > 0? count: 1),
	update_period(msec(10))
{
	if (max_value > numeric_limits<size_t>::max() || max_value < 0.1)
		mode = Mode::Exponential;
	else if (max_value < 0.999 || fabs(max_value - round_n(max_value)) > 0.005)
		mode = Mode::Floating;
	else
		mode = Mode::Integer;
	setupUi(this);
	setWindowTitle(prompt);
	// Прогресс отображается поверх всех окон, не имеет никаких кнопок в заголовке.
	// SetWindowPosition и SetStayOnTop вместе работают не всегда корректно.
	// Если вызвать SetStayOnTop(true) после SetWindowPosition(), окно оказывается не в том месте.
	SetStayOnTop(true);
	SetWindowPosition();

	indicator_quotient = double(n_indicator_divisions) / max_value;

	progressBar->setMaximum(n_indicator_divisions);
	progressBar->setValue(current_value*indicator_quotient);

	time_started = time_updated = GetPerformanceCounter();

	labelDetailedInfo->setText(tr(GenerateDetailedInfo().c_str()));

	//	setAttribute(Qt::WA_DeleteOnClose);

	QObject::connect(buttonQuit, SIGNAL(clicked()), SLOT(QuitClicked()));
	QObject::connect(buttonCancel, SIGNAL(clicked()), SLOT(CancelClicked()));
	QObject::connect(buttonPause, SIGNAL(clicked()), SLOT(PauseClicked()));

	esc_pressed = false;

	installEventFilter(this);
	GUIController::AddGUISettingsChangedTarget(this);
}


ProgressBar::~ProgressBar()
{
	GUIController::RemoveGUISettingsChangedTarget(this);
}



bool ProgressBar::eventFilter(QObject *target, QEvent *event)
{
	if(event->type() == QEvent::KeyPress)
	{
		QKeyEvent *keyEvent = (QKeyEvent *)event;
		// если стрелки влево/вправо и колонок больше, чем одна

		switch(keyEvent->key())
		{
			case Qt::Key_Enter:
				if(esc_pressed)
				{
					// после нажатой esc нажатие enter подтверждает отмену операцию
					CancelClicked();
				}
				break;

			case Qt::Key_Escape://schedule cancel
				if(!esc_pressed)
				{
					if(!WorkthreadPauseScheduled())
						PauseClicked();
					esc_pressed = true;
					buttonCancel->setDefault(true);
				}
				else
				{
					PauseClicked();
				}

				return true;
				break;

			case Qt::Key_Space:
				PauseClicked();
				return true;
				break;
		}
	}

	return QObject::eventFilter(target, event);
}

string	time_string(physical_time t)
{
	if(norma(t.sec()) <= 0.1)
		return ssprintf("%.2f ms", t.msec());
	else if(norma(t.sec()) <= 60)
		return ssprintf("%.2f sec", t.sec());
	else if (norma(t.sec()) > 1000*3600)
		return ssprintf("%.2lg h", t.sec()/3600);
	else
	{
		int	it = int(t.sec());
		if(norma(it) <= 3600)
			return ssprintf("%02d:%02d min", it / 60, it % 60);
		else
			return ssprintf("%02d:%02d:%02d h", it / 3600, (it % 3600) / 60, it % 60);
	}
}

string	ProgressBar::step_string(double current_value, double max_value) const
{
	if(mode == Mode::Exponential)
		return ssprintf("step no = %.6lg (%.6lg)", current_value, max_value);
	else if(mode == Mode::Floating)
		return ssprintf("step no = %.2lf (%.2lf)", current_value, max_value);
	else if(fabs(current_value - round_n(current_value)) > 0.005)
		return ssprintf("step no = %.2lf (%.0lf)", current_value, max_value);
	else
		return ssprintf("step no = %.0lf (%.0lf)", current_value, max_value);
}

string ProgressBar::GenerateDetailedInfo()
{
	if(current_value)
	{
		physical_time	elapsed = GetPerformanceCounter() - time_started;
		physical_time	average = elapsed / current_value;
		physical_frequency	average_speed = current_value / elapsed;
		physical_time	remaining = (max_value - current_value) * average;
		physical_time	full_estimation = elapsed + remaining;

		return ssprintf("%s, time elapsed = %s, time remaining = %s\nfull time = %s, dt = %s/step, average speed = %.3lg steps/sec",
			step_string(current_value, max_value).c_str(),
			time_string(elapsed).c_str(),
			time_string(remaining).c_str(),
			time_string(full_estimation).c_str(),
			time_string(average).c_str(),
			average_speed.Hz());
	}
	else
	{
		return ssprintf("Processed %s", step_string(current_value, max_value).c_str());
	}
}


void ProgressBar::PauseClicked(void)
{
	if(WorkthreadPauseScheduled())
	{
		buttonPause->setText("Pause");
		ScheduleWorkthreadPause(false);
		emit request_ResumeWorkThread();
	}
	else
	{
		buttonPause->setText("Resume");
		ScheduleWorkthreadPause(true);
		emit request_PauseWorkThread();
	}
	//UpdateConsole();

	// если пауза была вызвана по esc, отменяем все последствия
	esc_pressed = false;
	buttonCancel->setDefault(false);
}

void ProgressBar::CancelClicked(void)
{
	StoreGeometry();
	accept();
	ScheduleCurrentOperationCancel();
	emit request_ResumeWorkThread();
	emit request_Cancel();
}

void ProgressBar::QuitClicked(void)
{
	ScheduleApplicationQuit();
	StoreGeometry();
	accept();
	emit request_ResumeWorkThread();
}

void ProgressBar::SetPosition(double position)
{
	current_value = position;
}

void ProgressBar::UpdateIndicator()
{
	if(GetPerformanceCounter() - time_updated >= update_period)
	{
		progressBar->setValue(current_value*indicator_quotient);
		labelDetailedInfo->setText(tr(GenerateDetailedInfo().c_str()));
		time_updated = GetPerformanceCounter();
	}
}

void ProgressBar::StoreGeometry(void)
{
	progress_geometry = geometry();
	geometry_stored = true;
}

void ProgressBar::EndProgress(void)
{
	progressBar->setValue(current_value*indicator_quotient);
	//UpdateConsole();

	StoreGeometry();
	accept();
}

void ProgressBar::SetWindowPosition()
{
	if(geometry_stored)
	{
		setGeometry(progress_geometry);
	}
	else
	{
		QDesktopWidget desktop;
		QRect desktop_rect = desktop.screenGeometry(desktop.primaryScreen()); // or screenGeometry(), depending on your needs
		QRect progress_rect = geometry();
		int	progress_height = progress_rect.bottom() - progress_rect.top();
		int	progress_width = progress_rect.right() - progress_rect.left();

// 		progress_rect.setRight(desktop_rect.right() - WindowGeometry::console_width() - 16); // совпадает с левым краем консоли

		if(desktop.screenCount()>1 && desktop.isVirtualDesktop())			//если экранов более одного, то помещаем индикатор на второй экран
		{
			int	secondary_screen_no = desktop.primaryScreen() == 0 ? 1 : 0;
			QRect secondary_desktop_rect = desktop.screenGeometry(secondary_screen_no);

			progress_rect.setRight(secondary_desktop_rect.left() + WindowGeometry::left_margin() + progress_width);
			progress_rect.setBottom(secondary_desktop_rect.bottom() - WindowGeometry::bottom_margin()); // совпадает с нижним краем консоли
			progress_rect.setTop(progress_rect.bottom() - progress_height);
			progress_rect.setLeft(progress_rect.right() - progress_width);
		}
		else
		{
			progress_rect.setRight(desktop_rect.left() + WindowGeometry::left_margin() + progress_width); // окно помещается в левом нижнем краю основного экрана
			progress_rect.setBottom(desktop_rect.bottom() - WindowGeometry::bottom_margin()); // совпадает с нижним краем консоли
			progress_rect.setTop(progress_rect.bottom() - progress_height);
			progress_rect.setLeft(progress_rect.right() - progress_width);
		}

		setGeometry(progress_rect);
	}
}



ProgressBarManager::ProgressBarManager() :
	progress(NULL)
{
}

ProgressBarManager::~ProgressBarManager()
{
	DestroyObject(progress);
}

void ProgressBarManager::Start(QString prompt, double count)
{
	DestroyObject(progress);
	progress = new ProgressBar(prompt, count);

	QObject::connect(progress, SIGNAL(request_Cancel()), this, SLOT(do_Cancel()));
	QObject::connect(progress, SIGNAL(request_ResumeWorkThread()), global_gui_controller->work_thread, SLOT(Resume()));

	progress->show();
}

void ProgressBarManager::SetPosition(double position)
{
	// 	_next();
	if(progress)
	{
		progress->SetPosition(position);
		progress->UpdateIndicator();
	}
}

void ProgressBarManager::End()
{
	if(progress) progress->EndProgress();
	DestroyObject(progress);
}

void ProgressBarManager::do_Cancel()
{
	// 	progress_increments.resize(0);
	// 	start_times.resize(0);

	if(progress) progress->EndProgress();
	DestroyObject(progress);
}



//	счетчик обращений к индикатору прогресса.
//	легкие функции, которые можно вызывать часто

void ProgressBarCounter::_start()
{
	++level;
}

void ProgressBarCounter::_end()
{
	--level;
}

void ProgressBarCounter::_reset()
{
	level = 0;
}

int ProgressBarCounter::_level()
{
	return int(level);
}



}//namespace XRAD_GUI
