/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//	Created by IRD on 05.2013
//  Version 3.0.2
//--------------------------------------------------------------
#include "pre.h"
#include "ThreadGUI.h"

#include "GetValueDialog.h"
#include "DecisionDialogs.h"
#include "GUIController.h"
#include "ShowValueDialog.h"
#include "PauseDialog.h"
#include "FileSaveUtils.h"
#include <XRADSystem/System.h>

//--------------------------------------------------------------

//ThreadGUI dialogs_creator;
namespace XRAD_GUI
{



ThreadGUI::ThreadGUI(GUIController& in_gui_globals, QObject* parent) :
	gui_controller(in_gui_globals),
	QObject(parent)
{
	// Вызовы из других потоков
	QObject::connect(this, &self::request_UICallback, this, &self::do_UICallback,
		Qt::BlockingQueuedConnection);
}

ThreadGUI::~ThreadGUI()
{
}

bool ThreadGUI::do_ShowDataWindow(QDialog* object_ptr, bool suspend)
// показывает окно графика или изображения. (к диалоговым формам с кнопками это не относится,)
// вызывается по сигналу из рабочего потока
{
	if (!gui_controller.WidgetExists(object_ptr))
	{
		return false;
	}

	if (suspend)
	{
		// если рабочий поток останавливается ради этого окна, его закрытие должно приводить к выходу из сна
		// если работа продолжена без закрывания окна, эта зависимость отменяется (см. ThreadUser::Resume())
		gui_controller.work_thread->suspending_window = object_ptr;
		QObject::connect(object_ptr, &QDialog::finished,
			gui_controller.work_thread, &ThreadUser::Resume);
		QObject::connect(object_ptr, SIGNAL(signal_esc()), object_ptr, SLOT(close()));
		// по esc закрываем окно. сигнал deleteLater() здесь срабатывал,
		// но приводил к ошибке при завершении прилажения. интересно понять, почему
	}
	object_ptr->show();

	return true;
}

//--------------------------------------------------------------

void ThreadGUI::WorkerUICallback(function<void()>&& action)
{
	if (QThread::currentThread() == thread())
	{
		action();
		return;
	}
	UICallbackParams params;
	params.action = std::move(action);
	emit request_UICallback(&params);
}

void ThreadGUI::do_UICallback(UICallbackParams* params)
{
	try
	{
		if (params->action)
			params->action();
	}
	catch (...)
	{
		fprintf(stderr, "Exception in UICallback:\n%s\n",
			GetExceptionString().c_str());
	}
}

//--------------------------------------------------------------

void ThreadGUI::ExecuteDialog(StayOnTopDialog* dialog)
{
	dialog->SetStayOnTop(true);
	dialog->show();
}

//--------------------------------------------------------------

void ThreadGUI::BeginDialogLoop(const void* id, QDialog* dialog, function<void()>&& end_dialog)
{
	for (auto& v : active_dialogs)
	{
		if (get<0>(v) == id)
			throw invalid_argument("ThreadGUI::BeginDialogLoop: The id is already used.");
	}
	active_dialogs.push_back(make_tuple(id, QPointer<QDialog>(dialog), end_dialog,
		main_window->GetThreadState()));
	main_window->SetThreadState(ConsoleWindow::ThreadState::DialogV2);
}

//--------------------------------------------------------------

void ThreadGUI::EndDialogLoop(const void* id)
{
	for (auto it = active_dialogs.rbegin(); it != active_dialogs.rend(); ++it)
	{
		if (get<0>(*it) == id)
		{
			main_window->SetThreadState(get<ConsoleWindow::ThreadState>(*it));
			active_dialogs.erase(--it.base());
			break;
		}
	}
}

//--------------------------------------------------------------

void ThreadGUI::ExecuteActiveDialog(StayOnTopDialog* dialog)
{
	active_dialog = dialog;
	if (!active_dialog)
		return;
	connect(active_dialog, &QDialog::finished, active_dialog, &QDialog::deleteLater);

	if (gui_controller.ActiveProgressBar())
	{
		active_dialog->exec();
	}
	else
	{
		//диалог отображается поверх всех окон
		active_dialog->SetStayOnTop(true);
		// сигнал finished
		QObject::connect(active_dialog, &QDialog::finished,
			gui_controller.work_thread, &ThreadUser::Resume);

		active_dialog->show();
	}
}

//--------------------------------------------------------------

void ThreadGUI::do_AcceptAndContinue()
{
	bool dialog_processed = false;
	if (active_dialog)
	{
		active_dialog->accept();
		dialog_processed = true;
	}
	else
	{
		if (gui_controller.quit_scheduled)
		{
			for (auto it = active_dialogs.rbegin(); it != active_dialogs.rend(); ++it)
			{
				auto& f = get<2>(*it);
				if (f)
					f();
			}
		}
		else
		{
			if (!active_dialogs.empty())
			{
				auto& dialog = get<1>(active_dialogs.back());
				if (dialog)
				{
					dialog->raise();
					dialog->activateWindow();
				}
				dialog_processed = true;
			}
		}
	}

	if (progress_bar_manager.Active())
	{
		if (gui_controller.quit_scheduled)
		{
			//TODO подумать еще над обнулением счетчиков, не здесь ли?
			gui_controller.progress_bar_counter->_reset();
			do_EndProgress();
		}
		else
		{
			progress_bar_manager.Pause();
		}
		gui_controller.work_thread->Resume();
	}
	else if (!dialog_processed)
	{
		gui_controller.work_thread->Resume();
	}
}

//--------------------------------------------------------------

// вызов формы "Пауза"
void ThreadGUI::do_Pause()
{
	// вызывается в интерфейсном потоке!
	if (progress_bar_manager.Active())
	{
		progress_bar_manager.Pause();
	}
	else
	{
		ExecuteActiveDialog(new PauseDialog);
	}
}



// окно с выводом текста
void	ThreadGUI::do_ShowString(QString title, QString text, display_text_dialog_status status)
{
	if (status == single_use_window)
	{
		ExecuteActiveDialog(new ShowStringDialog(title, text));
	}
	else
	{
		//отображаем объект как окно с данными
		TextWindow* display_dialog = new TextWindow(title, gui_controller);
		display_dialog->SetText(text);
		do_ShowDataWindow(display_dialog, (status == multiple_use_window_paused));
	}
}

// окно с выводом целого числа
void ThreadGUI::do_ShowIntegral(QString title, ptrdiff_t value)
{
	ExecuteActiveDialog(new ShowIntegralDialog(title, value));
}

// окно с выводом числа с плавающей запятой
void ThreadGUI::do_ShowFloating(QString title, double value)
{
	ExecuteActiveDialog(new ShowFloatingDialog(title, value));
}



// вызов окна с сообщением (модальный диалог! для вывода критических ошибок)
void ThreadGUI::do_ShowMessage(QMessageBox::Icon icon, QString title, QString text)
{
	QMessageBox* msgBox = new QMessageBox(icon, title, text, QMessageBox::Ok);
	connect(msgBox, &QDialog::finished, msgBox, &QDialog::deleteLater);
	msgBox->setFixedWidth(512);
	msgBox->exec();
}



// ввод текстового значения
void	ThreadGUI::do_GetString(QString& result, QString prompt, QString default_value)
{
	ExecuteActiveDialog(new GetStringDialog(result, prompt, default_value));
}

// ввод вещественного числа
void	ThreadGUI::do_GetFloating(double& result, QString prompt, double min_value, double max_value, double default_value, bool allow_out_of_range)
{
	ExecuteActiveDialog(new GetFloatingDialog(result, prompt, min_value, max_value, default_value,
		allow_out_of_range));
}

// ввод целого числа
void ThreadGUI::do_GetIntegral(ptrdiff_t& result, QString prompt, ptrdiff_t min_value, ptrdiff_t max_value, ptrdiff_t default_value, size_t width, bool allow_out_of_range)
{
	ExecuteActiveDialog(new GetIntegralDialog(result, prompt, min_value, max_value, default_value,
		width, allow_out_of_range));
}



// динамическая форма с кнопками
void	ThreadGUI::do_GetButtonDecision(size_t& result, const QString& prompt, const vector<QString>& buttons, size_t default_button, size_t cancel_button)
{
	ExecuteActiveDialog(
		new ButtonDecisionDialog(result, prompt, buttons, default_button, cancel_button));
}

// динамическая форма с флажками
void	ThreadGUI::do_GetCheckboxDecision(const QString& prompt, const vector<pair<QString, bool*> >& boxes)
{
	ExecuteActiveDialog(new CheckboxDecisionDialog(prompt, boxes));
}


// динамическая форма с радиокнопками
void ThreadGUI::do_GetRadioButtonDecision(size_t& result, const QString& prompt, const vector<QString>& buttons, size_t default_button)
{
	ExecuteActiveDialog(new RadioButtonDecisionDialog(result, prompt, buttons, default_button));
}



// форма выбора/сохранения файла
bool ThreadGUI::do_GetFileName(file_dialog_mode flag, QString& return_file_name, const QString& prompt, const QString& default_filename, const QString& type)
{
	QString	appname = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
	QSettings settings("XRAD", appname);

	QString currentPath;
	if (default_filename.isEmpty())
	{
		currentPath = GetDefaultPath(flag);
	}
	else
	{
		// Используем разбор строки, а не методы QFileInfo, т.к. QFileInfo в случае отсутствия пути возвращает ".", а не пустую строку.
		int separator_index = default_filename.lastIndexOf(QString(path_separator()));
		if (separator_index == -1)
			currentPath = GetDefaultPath(flag) + QString(path_separator()) + default_filename;
		else
			currentPath = default_filename;
	}

	switch (flag)
	{
		case file_open_dialog:
			return_file_name = QFileDialog::getOpenFileName(0, prompt, currentPath, type);
			break;

		case file_save_dialog:
			return_file_name = QFileDialog::getSaveFileName(0, prompt, currentPath, type);
			break;
		default:
			// этого не должно возникать, но все же проверяем. можно еще было бы исключение бросить
			return false;
	}
	//
	if (!return_file_name.isEmpty())
	{
		SaveDefaultPath(flag, QFileInfo(return_file_name).absolutePath());
		return true;
	}
	else
	{
		return false;
	}
}

bool ThreadGUI::do_GetFolderName(file_dialog_mode flag, QString& return_folder_name, const QString& prompt,
	const QString& default_path)
{
	QString	appname = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
	QSettings settings("XRAD", appname);

	QString currentPath;
	if (default_path.isEmpty())
	{
		currentPath = GetDefaultPath(flag);
	}
	else
	{
		currentPath = default_path;
	}

	return_folder_name = QFileDialog::getExistingDirectory(0, prompt, currentPath, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

	if (!return_folder_name.isEmpty())
	{
		SaveDefaultPath(flag, QFileInfo(return_folder_name).absoluteFilePath());
		return true;
	}
	else
	{
		return false;
	}
}


bool	ThreadGUI::do_SaveGraphPicture(GraphWindow* graph, const QString& filename_with_extension)
{
	try
	{
		graph->SavePicture(filename_with_extension);
		return true;
	}
	catch (...)
	{
		return false;
	}
}


GraphWindow* ThreadGUI::do_CreateGraph(const QString& title, const QString& y_label, const QString& x_label)
{
	return new GraphWindow(title, y_label, x_label, gui_controller);
}

bool	ThreadGUI::do_SetupGraphCurve(GraphWindow* graph, int curve_no, const DataArray<double>& data_y, const DataArray<double>& data_x, const QString& curve_name)
{
	if (!gui_controller.WidgetExists(graph))
	{
		return false;
	}
	graph->SetupCurve(curve_no, data_y, data_x, curve_name);
	return true;
}

bool	ThreadGUI::do_SetupGraphLabels(GraphWindow* graph, const QString& title, const QString& y_label, const QString& x_label)
{
	if (!gui_controller.WidgetExists(graph))
	{
		return false;
	}
	graph->SetGraphLabels(title, y_label, x_label);
	return true;
}

bool	ThreadGUI::do_SetupGraphTitle(GraphWindow* graph, const QString& title)
{
	if (!gui_controller.WidgetExists(graph))
	{
		return false;
	}
	graph->SetWindowTitle(title);
	return true;
}

bool	ThreadGUI::do_SetGraphStyle(GraphWindow* graph, graph_line_style style, double in_line_width)
{
	if (!gui_controller.WidgetExists(graph))
	{
		return false;
	}
	graph->SetLineStyle(style, in_line_width);
	return true;
}

bool ThreadGUI::do_SetGraphScale(GraphWindow* graph, const range2_F64& scale)
{
	if (!gui_controller.WidgetExists(graph))
	{
		return false;
	}
	graph->SetScale(scale);
	return true;
}


bool	ThreadGUI::do_HideDataWindow(QDialog* window)
{
	if (!gui_controller.WidgetExists(window))
	{
		return false;
	}
	try
	{
		window->hide();
		return true;
	}
	catch (...)
	{
		return false;
	}

}


bool	ThreadGUI::do_SetDataWindowTitle(QDialog* window, const QString& title)
{
	if (!gui_controller.WidgetExists(window))
	{
		return false;
	}
	try
	{
		window->setWindowTitle(title);
		return true;
	}
	catch (...)
	{
		return false;
	}
}


bool ThreadGUI::do_CloseDataWindow(QDialog* window)
{
	if (!gui_controller.WidgetExists(window))
	{
		return false;
	}
	try
	{
		delete window;
		return true;
	}
	catch (...)
	{
		return false;
	}
}

bool ThreadGUI::do_GetGraphScale(const GraphWindow* graph, range2_F64& scale)
{
	if (!gui_controller.WidgetExists(graph))
	{
		return false;
	}
	scale = graph->GetScale();
	return true;
}



ImageWindow* ThreadGUI::do_CreateRasterImageSet(const QString& title, int ys, int xs)
{
	return new ImageWindow(title, ys, xs, gui_controller);
}

bool ThreadGUI::do_AddImageFrames(ImageWindow* img, size_t n_frames)
{
	if (!gui_controller.WidgetExists(img)) return false;
	img->AddFrames(n_frames);
	return true;
}

bool ThreadGUI::do_SetupImageFrame(ImageWindow* img, int in_frame_no, const void* data, display_sample_type pt)
{
	if (!gui_controller.WidgetExists(img)) return false;
	img->SetupFrame(in_frame_no, data, pt);
	return true;
}

bool ThreadGUI::do_SetupImageLabels(ImageWindow* img, const QString& title, const QString& z_label, const QString& y_label, const QString& x_label, const QString& value_label)
{
	if (!gui_controller.WidgetExists(img)) return false;
	img->SetImageLabels(title, z_label, y_label, x_label, value_label);
	return true;
}

bool ThreadGUI::do_SetupImageDefaultRanges(ImageWindow* img, double min_value, double max_value, double gamma)
{
	if (!gui_controller.WidgetExists(img)) return false;
	img->SetDefaultBrightness(min_value, max_value, gamma);
	return true;
}

bool ThreadGUI::do_SetImageAxesScales(ImageWindow* img, double z0, double dz, double y0, double dy, double x0, double dx)
{
	if (!gui_controller.WidgetExists(img)) return false;
	img->SetAxesScales(z0, dz, y0, dy, x0, dx);
	return true;
}



//--------------------------------------------------------------



TextWindow* ThreadGUI::do_CreateTextDisplayer(const QString& title)
{
	return new TextWindow(title, gui_controller);
}

bool	ThreadGUI::do_SetFixedWidth(TextWindow* tw, bool fixed_width)
{
	if (!gui_controller.WidgetExists(tw))
	{
		return false;
	}
	tw->SetFixedWidth(fixed_width);
	return true;
}

bool	ThreadGUI::do_SetTitle(TextWindow* tw, const QString& title)
{
	if (!gui_controller.WidgetExists(tw))
	{
		return false;
	}
	tw->SetWindowTitle(title);
	return true;
}

bool	ThreadGUI::do_SetEditable(TextWindow* tw, bool editable)
{
	if (!gui_controller.WidgetExists(tw))
	{
		return false;
	}
	tw->SetEditable(editable);
	return true;
}

bool	ThreadGUI::do_SetFontSize(TextWindow* tw, double size)
{
	if (!gui_controller.WidgetExists(tw))
	{
		return false;
	}
	tw->SetFontSize(size);
	return true;
}

bool ThreadGUI::do_SetPersistent(TextWindow* tw, bool persistent)
{
	if (!gui_controller.WidgetExists(tw))
	{
		return false;
	}
	tw->SetPersistent(persistent);
	return true;
}

bool ThreadGUI::do_SetStayOnTop(TextWindow* tw, bool stay_on_top)
{
	if (!gui_controller.WidgetExists(tw))
	{
		return false;
	}
	tw->SetStayOnTop(stay_on_top);
	return true;
}

bool	ThreadGUI::do_SetText(TextWindow* tw, const QString& text)
{
	if (!gui_controller.WidgetExists(tw))
	{
		return false;
	}
	tw->SetText(text);
	return true;
}

QString	ThreadGUI::do_GetText(const TextWindow* tw)
{
	if (!gui_controller.WidgetExists(tw))
	{
		return "";
	}
	return tw->GetText();
}



//--------------------------------------------------------------



void ThreadGUI::do_StartProgress(QString prompt, double count)
{
	progress_bar_manager.Start(prompt, count);
}

void ThreadGUI::do_EndProgress()
{
	progress_bar_manager.End();
}

void ThreadGUI::do_SetProgressPosition(double position)
{
	progress_bar_manager.SetPosition(position);
}



void ThreadGUI::do_UpdateConsole()
{
	if (!main_window)
		return;
	main_window->UpdateConsole();
}

PainterWindow* ThreadGUI::do_CreatePainterWindow(const QString& title, shared_ptr<QImage> qimg_result)
{
	auto result = new PainterWindow(title, qimg_result, gui_controller);
	return result;
}



}//namespace XRAD_GUI
