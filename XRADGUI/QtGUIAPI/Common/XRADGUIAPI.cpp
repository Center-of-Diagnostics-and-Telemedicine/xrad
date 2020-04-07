//	file XRADGUIAPI.cpp
//	Created by ACS on 16.02.07
//  Modify by IRD on 05.2013 - create by QT
//  version 1.0.3
//--------------------------------------------------------------

#include "pre_GUI.h"

#include "XRADGUIAPI.h"
#include "SavedSettings.h"
#include <GUIController.h>
#include <XRAD/PlatformSpecific/Qt/Internal/StringConverters_Qt.h>
#include <XRADSystem/System.h>


XRAD_BEGIN


using namespace XRAD_GUI;



template<class T>
T	DetermineDefaultValue(QString fn, QString prompt, GUIValue<T> default_value)
{
	if(default_value.is_stored==saved_default_value)
		return GUILoadParameter(fn, prompt, default_value.value_valid? default_value.value: T());
	if(default_value.value_valid)
		return default_value.value;
	return T();
}


vector<pair<QString, bool*> > ConvertCheckboxList(const vector<pair<wstring, bool*> > &boxes)
{
	vector<pair<QString, bool*> > result(boxes.size());
	for(size_t i = 0; i < boxes.size(); ++i)
	{
		result[i].first = wstring_to_qstring(boxes[i].first);
		result[i].second = boxes[i].second;
	}
	return result;
}


auto	&work_thread()
{
	return *global_gui_controller->work_thread;
}

const auto &update_interval()
{
	return global_gui_controller->GUI_update_interval;
}

auto &progress_bar_counter()
{
	return global_gui_controller->progress_bar_counter;
}

auto progress_update_interval()
{
	return global_gui_controller->progress_update_interval;
}

auto is_progress_active()
{
	return global_gui_controller->IsProgressActive();
}

void api_ShowMessage(const wstring &message, api_message_type wtype, const wstring &type_message)
{
	api_ForceUpdateGUI(sec(0));

	wstring wprompt = type_message;
	const wchar_t *caption = L"";
	QMessageBox::Icon icon = QMessageBox::NoIcon;

	switch(wtype)
	{
		case api_msgFatal:
			caption = L"Fatal error";
			icon = QMessageBox::Critical;
			break;
		case api_msgError:
			caption = L"Error";
			icon = QMessageBox::Critical;
			break;
			//iError(message.c_str());
			return;
		case api_msgWarning:
			caption = L"Warning";
			icon = QMessageBox::Warning;
			break;
		case api_msgInfo:
			caption = L"Information";
			icon = QMessageBox::Information;
			break;
		default:
		case api_msgShow:
			caption = L"Show";
			break;
	}
	if(!wprompt.size())
		wprompt = caption;
	try
	{
		emit work_thread().request_ShowMessage(icon, wstring_to_qstring(wprompt), wstring_to_qstring(message));
	}
	catch(...)
	{
	}

}

//--------------------------------------------------------------

void api_Delay(const physical_time &delay)
{
	api_ForceUpdateGUI(delay);
	work_thread().Sleep(delay.sec());

}

//--------------------------------------------------------------

void api_Pause()
{
	api_ForceUpdateGUI(sec(0));
//	api_ForceUpdateGUI(sec(0));
	emit work_thread().request_Pause();
	work_thread().WaitForNonModalDialog();
}



//--------------------------------------------------------------



size_t api_Decide(const wstring &wprompt, const vector<wstring> &buttons, GUIValue<size_t> gdefault)
{
	api_ForceUpdateGUI(sec(0));
	QString qprompt = wstring_to_qstring(wprompt);
	auto	qbuttons = wstring_list_to_qstring_list(buttons);

	static const QString	fn = "GetRadioButtonDecision";

	api_ForceUpdateGUI(sec(0));
	size_t result(gdefault.value);
	emit work_thread().request_GetRadioButtonDecision(result, qprompt, *&qbuttons, DetermineDefaultValue(fn, qprompt, gdefault));
	work_thread().WaitForNonModalDialog();

	GUISaveParameter(fn, qprompt, result);

	return result;

}

size_t api_Decide2(const wstring &wprompt, const wstring &wfirst, const wstring &wsecond, GUIValue<size_t> gdefault)
{
	api_ForceUpdateGUI(sec(0));
	return api_Decide(wprompt, {wfirst,wsecond}, gdefault);
}


//--------------------------------------------------------------

size_t api_GetButtonDecision(const wstring &wprompt, const vector<wstring> &wbuttons,
		GUIValue<size_t> gdefault_button, size_t cancel_button)
{
	QString qprompt = wstring_to_qstring(wprompt);
	api_ForceUpdateGUI(sec(0));

	static const QString	fn = "GetButtonDecision";

	api_ForceUpdateGUI(sec(0));
	size_t result(cancel_button);
	emit work_thread().request_GetButtonDecision(result, qprompt, wstring_list_to_qstring_list(wbuttons), DetermineDefaultValue(fn, qprompt, gdefault_button), cancel_button);//TODO искать не число, а кнопку?
	work_thread().WaitForNonModalDialog();

	if(result != cancel_button) GUISaveParameter(fn, qprompt, result);//коряво пока сделано, чтобы после esc не запоминала кнопку cancel

	return result;

}

//--------------------------------------------------------------

size_t api_GetButtonDecisionMaxButtons()
{
//TODO взять число кнопок из процедуры рисования формы. или вообще снять ограничение
	return 20;
}

//--------------------------------------------------------------

bool api_GetCheckboxDecision(const wstring &wprompt, const vector<pair<wstring, bool*> > &in_boxes)
{
	api_ForceUpdateGUI(sec(0));
	QString qprompt = wstring_to_qstring(wprompt);

	api_ForceUpdateGUI(sec(0));
	emit work_thread().request_GetCheckboxDecision(qprompt, ConvertCheckboxList(in_boxes));
	work_thread().WaitForNonModalDialog();

	return true;
}

//--------------------------------------------------------------

size_t api_GetCheckboxDecisionMaxBoxes()
{
//TODO взять число кнопок из процедуры рисования формы. или вообще снять ограничение
	return 20;
}

//--------------------------------------------------------------

void api_ShowString(const wstring &wtitle, const wstring &text, display_text_dialog_status status)
{
	api_ForceUpdateGUI(sec(0));
	emit work_thread().request_ShowString(wstring_to_qstring(wtitle), wstring_to_qstring(text), status);
	if(status == single_use_window)
	{
		work_thread().WaitForNonModalDialog();
	}
	else if(status == multiple_use_window_paused)
	{
		work_thread().Suspend(ThreadUser::suspend_for_data_analyze);
	}

}

void api_ShowIntegral(const wstring &wtitle, ptrdiff_t value)
{
	api_ForceUpdateGUI(sec(0));
	emit work_thread().request_ShowIntegral(wstring_to_qstring(wtitle), value);
	work_thread().WaitForNonModalDialog();

}

void api_ShowFloating(const wstring &wtitle, double value)
{
	api_ForceUpdateGUI(sec(0));
	emit work_thread().request_ShowFloating(wstring_to_qstring(wtitle), value);
	work_thread().WaitForNonModalDialog();
}

//--------------------------------------------------------------

wstring api_GetString(const wstring &wprompt, const GUIValue<wstring> &default_value, bool /*multiline*/)
{
	static const QString	fn = "GetString";
	api_ForceUpdateGUI(sec(0));

	QString	result;
	QString	qprompt = wstring_to_qstring(wprompt);
	auto	default_string = ConvertGUIValue(default_value, wstring_to_qstring(default_value.value));

	emit work_thread().request_GetString(result, qprompt, DetermineDefaultValue(fn, qprompt, default_string));
	work_thread().WaitForNonModalDialog();

	GUISaveParameter(fn, qprompt, result);

	return qstring_to_wstring(result);

}

ptrdiff_t api_GetIntegral(const wstring &wprompt, GUIValue<ptrdiff_t> default_value, ptrdiff_t min_value, ptrdiff_t max_value, out_of_range_control allow_out_of_range)
{
	api_ForceUpdateGUI(sec(0));
	// allow_out_of_range в функциях, открытых пользователю,задано "говорящим" enum,
	// чтобы при вызове было ясно, что должно произойти.
	// ср.: GetIntegral(..., true(?)) и GetIntegral(..., out_of_range_allowed)
	// внутри реализации в этом нет необходимости, поэтому остается bool
	static const QString	fn = "GetIntegral";

	api_ForceUpdateGUI(sec(0));
	ptrdiff_t result;
	QString	qprompt = wstring_to_qstring(wprompt);
	emit work_thread().request_GetIntegral(result, qprompt, min_value, max_value, DetermineDefaultValue(fn, qprompt, default_value), sizeof(ptrdiff_t), allow_out_of_range);
	work_thread().WaitForNonModalDialog();
	api_ForceUpdateGUI(update_interval());

	GUISaveParameter(fn, qprompt, result);

	return result;
}

//--------------------------------------------------------------

double api_GetFloating(const wstring &wprompt, GUIValue<double> default_value, double min_value, double max_value, out_of_range_control allow_out_of_range)
{
	static const QString	fn = "GetFloating";
	api_ForceUpdateGUI(sec(0));
	double	result;
	QString	qprompt = wstring_to_qstring(wprompt);
	emit work_thread().request_GetFloating(result, qprompt, min_value, max_value, DetermineDefaultValue(fn, qprompt, default_value), allow_out_of_range);
	work_thread().WaitForNonModalDialog();
	api_ForceUpdateGUI(update_interval());

	GUISaveParameter(fn, qprompt, result);

	return result;
}



//--------------------------------------------------------------

//--------------------------------------------------------------



void api_StartProgress(const wstring &wprompt, double count)
{
//TODO некоторые вызовы NextEvent избыточны (с учетом изменений от 06 апреля 2015)
	api_ForceUpdateGUI(sec(0));
	QString qprompt = wstring_to_qstring(wprompt);
	progress_bar_counter()->_start();
	api_ForceUpdateGUI(update_interval());
	emit work_thread().request_StartProgress(qprompt, count);
}

//--------------------------------------------------------------

void api_SetProgressPosition(double position)
{
	//printf("api_SetProgressPosition: %lf\n", EnsureType<double>(position));
	static	physical_time previous = clocks(clock());
	physical_time	current = clocks(clock());
	physical_time	delay = current-previous;

	if(delay >= progress_update_interval())
	{
		emit work_thread().request_SetProgressPosition(position);
		previous = clocks(clock());
	}
	api_ForceUpdateGUI(update_interval());

}

//--------------------------------------------------------------

void api_EndProgress()
{
	//TODO возможно слишком частое обновление при использовании вложенных прогрессов, заменить на update_time
	try
	{
		work_thread().ForceUpdateGUI_noexcept(sec(0));
		progress_bar_counter()->_end();

		emit work_thread().request_EndProgress();
	}
	catch (...)
	{
	}
}

bool api_IsProgressActive()
{
	return is_progress_active();
}

void api_ForceUpdateGUI(const physical_time &update_interval)
{
	work_thread().ForceUpdateGUI(update_interval);

}

void api_ForceQuit(int exit_code)
{
//	TODO здесь добавить отмену quit_scheduled, если есть, иначе они друг другу мешают
	throw quit_application("Application quit by user request (via function call)", exit_code);
}



//--------------------------------------------------------------

//! \todo Если default_file_name задано, но без пути, извлекать сохраненное значение и брать из него путь. То же для GetFolderName.
void api_GetFileName(wstring &wfilename, const wstring &wprompt, const GUIValue<wstring> &gwdefault, const wstring &wtype, file_dialog_mode flag)
{
	api_ForceUpdateGUI(sec(0));
	QString	qfilename;
	auto	gqdefault = ConvertGUIValue(gwdefault, wstring_to_qstring(gwdefault.value));
	QString qprompt = wstring_to_qstring(wprompt);
	QString	qtype = wstring_to_qstring(wtype);


	bool	result(false);
	try
	{
		static const QString	fn = "GetFileName";
		api_ForceUpdateGUI(sec(0));
		result = emit work_thread().request_GetFileName(flag, qfilename, qprompt, DetermineDefaultValue(fn, qprompt, gqdefault), qtype);
		if(result)
			GUISaveParameter(fn, qprompt, qfilename);
	}
	catch(...)
	{
	}

	if(!result)
	{
		throw canceled_operation("File open canceled");
	}

	//wfilename = qstring_to_wstring(qfilename);
	wfilename = GetPathHumanReadable(qstring_to_wstring(qfilename));
}


void api_GetFolderName(wstring &wfoldername, const wstring &wprompt, const GUIValue<wstring> &gwdefault, const wstring &wtype, file_dialog_mode flag)
{
	api_ForceUpdateGUI(sec(0));
	QString	qfoldername;
	QString qprompt = wstring_to_qstring(wprompt);
	QString	qtype = wstring_to_qstring(wtype);
	auto	gqdefault = ConvertGUIValue(gwdefault, wstring_to_qstring(gwdefault.value));

	bool	result(false);
	try
	{
		static const QString	fn = "GetFolderName";
		api_ForceUpdateGUI(sec(0));
		result = emit work_thread().request_GetFolderName(flag, qfoldername, qprompt, DetermineDefaultValue(fn, qprompt, gqdefault));
		if(result)
			GUISaveParameter(fn, qprompt, qfoldername);
	}
	catch(...)
	{
	}

	if(!result)
	{
		throw canceled_operation("Folder open canceled");
	}

	//wfoldername = qstring_to_wstring(qfoldername);
	wfoldername = GetPathHumanReadable(qstring_to_wstring(qfoldername));
}

//--------------------------------------------------------------
//
//	DataWindowContainer
//

bool api_ShowDataWindow(DataWindowContainer &gc, bool is_stopped)
{
	api_ForceUpdateGUI(sec(0));
	if(IsPointerAValidGUIWidget(gc.window_ptr))
	{
		try
		{
			bool	result = emit work_thread().request_ShowDataWindow(static_cast<QDialog *>(gc.window_ptr), is_stopped);
			if(!result) return false;

			if(is_stopped)
			{
				work_thread().Suspend(ThreadUser::suspend_for_data_analyze);
			}
			return true;
		}
		catch(...)
		{
		}

	}
	return false;
}

bool	api_HideDataWindow(DataWindowContainer &dwc)
{
	if(IsPointerAValidGUIWidget(dwc.window_ptr))
	{
		return emit work_thread().request_HideDataWindow(static_cast<QDialog *>(dwc.window_ptr));
	}
	return false;
}

bool	api_SetDataWindowTitle(DataWindowContainer &dwc, const wstring &title)
{
	if(IsPointerAValidGUIWidget(dwc.window_ptr))
	{
		return emit work_thread().request_SetDataWindowTitle(static_cast<QDialog *>(dwc.window_ptr), wstring_to_qstring(title));
	}
	return false;
}



bool	api_CloseDataWindow(DataWindowContainer &dwc)
{
	if(IsPointerAValidGUIWidget(dwc.window_ptr))
	{
		return emit work_thread().request_CloseDataWindow(static_cast<QDialog *>(dwc.window_ptr));
	}
	return false;
}


bool api_SetPersistent(DataWindowContainer &dwc, bool persistent)
{
	if(!IsPointerAValidGUIWidget(dwc.window_ptr)) return false;
	try
	{
		bool result = work_thread().request_SetPersistent(static_cast<TextWindow*>(dwc.window_ptr), persistent);
		if(result)
		{
			api_ForceUpdateGUI(sec(0));
		}
		return result;
	}
	catch(...)
	{
	}

	return false;
}

bool api_SetStayOnTop(DataWindowContainer &dwc, bool stay_on_top)
{
	if(!IsPointerAValidGUIWidget(dwc.window_ptr)) return false;

	try
	{
		bool result = work_thread().request_SetStayOnTop(static_cast<TextWindow*>(dwc.window_ptr), stay_on_top);
		if(result)
			api_ForceUpdateGUI(sec(0));
		return result;
	}
	catch(...)
	{
	}
	return false;
}


//--------------------------------------------------------------
//
//	GraphWindowContainer
//

GraphWindowContainer api_CreateGraph(const wstring &wtitle, const wstring &wy_label, const wstring &wx_label)
{
	api_ForceUpdateGUI(sec(0));
	GraphWindowContainer	gc;
	gc.window_ptr = emit work_thread().request_CreateGraph(wstring_to_qstring(wtitle), wstring_to_qstring(wy_label), wstring_to_qstring(wx_label));

	return gc;
}

bool api_SetupGraphCurve(GraphWindowContainer &gc, size_t curve_no, const DataArray<double> &data_y, const DataArray<double> &data_x, const wstring& wcurve_name)
{
	api_ForceUpdateGUI(sec(0));
//	int	cn = (curve_no==graph_set_new_graph()) ? -1 : int(curve_no);
	if(IsPointerAValidGUIWidget(gc.window_ptr))
	{
		try
		{
			bool result = emit work_thread().request_SetupGraphCurve(static_cast<GraphWindow *>(gc.window_ptr), int(curve_no), data_y, data_x, wstring_to_qstring(wcurve_name));
			while(!static_cast<GraphWindow *>(gc.window_ptr)->AllCurvesCompleted())
			{
			// не даст ничего более делать в этом потоке, пока GUI поток не закончил формирование текущей кривой
			}
			return result;
		}
		catch(...)
		{
			return false;
		}
	}
	else return false;
}

bool api_SetGraphLabels(GraphWindowContainer &gc, const wstring &wtitle, const wstring &wy_label, const wstring &wx_label)
{
	api_ForceUpdateGUI(sec(0));
	if(IsPointerAValidGUIWidget(gc.window_ptr))
	{
		try
		{
			bool result = emit work_thread().request_SetupGraphLabels(static_cast<GraphWindow *>(gc.window_ptr), wstring_to_qstring(wtitle), wstring_to_qstring(wy_label), wstring_to_qstring(wx_label));
			api_ForceUpdateGUI(sec(0));
			return result;
		}
		catch(...)
		{
			return false;
		}
	}
	else return false;
}


bool api_SetGraphScale(GraphWindowContainer &gc, const range2_F64 &scale)
{
	api_ForceUpdateGUI(sec(0));
	if(IsPointerAValidGUIWidget(gc.window_ptr))
	{
		try
		{
	//		return work_thread->SetGraphScale(graph_ptr, scale);
			return emit work_thread().request_SetGraphScale(static_cast<GraphWindow *>(gc.window_ptr), scale);
		}
		catch(...)
		{
			return false;
		}
	}
	else return false;
}

bool api_GetGraphScale(const GraphWindowContainer &gc, range2_F64 &scale)
{
	api_ForceUpdateGUI(sec(0));
	if(IsPointerAValidGUIWidget(gc.window_ptr))
	{
		try
		{
	//		return work_thread->GetGraphScale(graph_ptr, scale);
			return emit work_thread().request_GetGraphScale(static_cast<const GraphWindow *>(gc.window_ptr), scale);
		}
		catch(...)
		{
			return false;
		}
	}
	else return false;
}

bool	api_SetGraphStyle(GraphWindowContainer &gc, graph_line_style style, double in_line_width)
{
	api_ForceUpdateGUI(sec(0));
	if(IsPointerAValidGUIWidget(gc.window_ptr))
	{
		try
		{
	// 		return work_thread->SetGraphStyle(graph_ptr, style);
			return emit work_thread().request_SetGraphStyle(static_cast<GraphWindow *>(gc.window_ptr), style, in_line_width);
		}
		catch(...)
		{
			return false;
		}
	}
	else return false;
}

bool	api_SaveGraphPicture(GraphWindowContainer &gc, const wstring &filename_with_extension)
{
	api_ForceUpdateGUI(sec(0));
	if(IsPointerAValidGUIWidget(gc.window_ptr))
	{
		try
		{
			return work_thread().request_SaveGraphPicture(static_cast<GraphWindow *>(gc.window_ptr), wstring_to_qstring(filename_with_extension));
		}
		catch(...)
		{
			return false;
		}
	}
	else return false;
}

//--------------------------------------------------------------
//
//	ImageWindowContainer
//


ImageWindowContainer api_CreateRasterImageSet(const wstring &wtitle, size_t vs, size_t hs)
{
	ImageWindowContainer	risc;
	try
	{
		risc.window_ptr = emit work_thread().request_CreateRasterImageSet(wstring_to_qstring(wtitle), int(vs), int(hs));
	}
	catch(...)
	{
	}
	return risc;
}

bool	api_AddImageFrames(ImageWindowContainer &risc, size_t n_frames)
{
	api_ForceUpdateGUI(sec(0));
	if(IsPointerAValidGUIWidget(risc.window_ptr))
	{
		try
		{
			emit work_thread().request_AddImageFrames(static_cast<ImageWindow*>(risc.window_ptr),n_frames);
			return true;
		}
		catch(...)
		{
			return false;
		}
	}
	else return false;
}


bool	api_SetImageAxesScales(ImageWindowContainer &risc, double z0, double dz, double y0, double dy, double x0, double dx)
{
	api_ForceUpdateGUI(sec(0));
	if(IsPointerAValidGUIWidget(risc.window_ptr))
	{
		try
		{
			emit work_thread().request_SetImageAxesScales(static_cast<ImageWindow*>(risc.window_ptr), z0, dz, y0, dy, x0, dx);
			return true;
		}
		catch(...)
		{
			return false;
		}
	}
	else return false;
}

bool	api_SetImageDefaultBrightness(ImageWindowContainer &risc, double black, double white, double gamma)
{
	api_ForceUpdateGUI(sec(0));
	if(IsPointerAValidGUIWidget(risc.window_ptr))
	{
		try
		{
		//TODO разобраться, где передавать void, а где указатель на правильный тип
			emit work_thread().request_SetupImageDefaultRanges(static_cast<ImageWindow*>(risc.window_ptr), black, white, gamma);
			return true;
		}
		catch(...)
		{
			return false;
		}
	}
	else return false;
}

bool	api_SetupImageFrame(ImageWindowContainer &risc, int frame_no, const void* data, display_sample_type pt)
{
	api_ForceUpdateGUI(sec(0));
	if(IsPointerAValidGUIWidget(risc.window_ptr))
	{
		try
		{
			emit work_thread().request_SetupImageFrame(static_cast<ImageWindow*>(risc.window_ptr), frame_no, data, pt);
			return true;
		}
		catch(...)
		{
			return false;
		}
	}
	else return false;
}

bool	api_InsertImageFrame(ImageWindowContainer &risc, int after_frame_no, const void* data, display_sample_type pt)
{
	api_ForceUpdateGUI(sec(0));
	if(IsPointerAValidGUIWidget(risc.window_ptr))
	{
		try
		{
		//TODO доделать чтобы вставляла
			emit work_thread().request_SetupImageFrame(static_cast<ImageWindow*>(risc.window_ptr), -1, data, pt);
			return true;
		}
		catch(...)
		{
			return false;
		}
	}
	else return false;
}

bool	api_SetImageLabels(ImageWindowContainer &risc, const wstring &wtitle, const wstring &wz_label, const wstring &wy_label, const wstring &wx_label, const wstring &wvalue_label)
{
	api_ForceUpdateGUI(sec(0));
	if(IsPointerAValidGUIWidget(risc.window_ptr))
	{
		try
		{
			emit work_thread().request_SetupImageLabels(static_cast<ImageWindow*>(risc.window_ptr),
				wstring_to_qstring(wtitle),
				wstring_to_qstring(wz_label),
				wstring_to_qstring(wy_label),
				wstring_to_qstring(wx_label),
				wstring_to_qstring(wvalue_label));
			return true;
		}
		catch(...)
		{
			return false;
		}
	}
	else return false;
}


//--------------------------------------------------------------
//
//	Text display window
//


TextWindowContainer api_CreateTextDisplayer(const wstring &wtitle, bool fixed_width, bool editable)
{
	TextWindowContainer	twc;
	try
	{
		twc.window_ptr = emit work_thread().request_CreateTextDisplayer(wstring_to_qstring(wtitle));
		api_SetFixedWidth(twc, fixed_width);
		api_SetEditable(twc, editable);
		return twc;
	}
	catch(...)
	{
		return twc;
	}
}


bool	api_SetFixedWidth(TextWindowContainer &twc, bool fixed_width)
{
	if(IsPointerAValidGUIWidget(twc.window_ptr))
	{
		try
		{
			bool result = work_thread().request_SetFixedWidth(static_cast<TextWindow*>(twc.window_ptr), fixed_width);
			if(result) api_ForceUpdateGUI(sec(0));
			return result;
		}
		catch(...)
		{
		}
	}
	return false;
}

bool	api_SetEditable(TextWindowContainer &twc, bool editable)
{
	if(IsPointerAValidGUIWidget(twc.window_ptr))
	{

		try
		{
			bool result = work_thread().request_SetEditable(static_cast<TextWindow*>(twc.window_ptr), editable);
			if(result) api_ForceUpdateGUI(sec(0));
			return result;
		}
		catch(...)
		{
		}
	}
	return false;
}

bool	api_SetFontSize(TextWindowContainer &twc, double size)
{
	if(IsPointerAValidGUIWidget(twc.window_ptr))
	{
		try
		{
			bool result = work_thread().request_SetFontSize(static_cast<TextWindow*>(twc.window_ptr), size);
			if(result) api_ForceUpdateGUI(sec(0));
			return result;
		}
		catch(...)
		{
		}
	}
	return false;
}

bool	api_SetText(TextWindowContainer &twc, const wstring &text)
{
	if(IsPointerAValidGUIWidget(twc.window_ptr))
	{
		try
		{
			bool result = work_thread().request_SetText(static_cast<TextWindow*>(twc.window_ptr), wstring_to_qstring(text));
			if(result) api_ForceUpdateGUI(sec(0));
			return result;
		}
		catch(...)
		{
		}

	}
	return false;
}

wstring	api_GetText(const TextWindowContainer &twc)
{
	if(IsPointerAValidGUIWidget(twc.window_ptr))
	{
		try
		{
			return qstring_to_wstring(work_thread().request_GetText(static_cast<TextWindow*>(twc.window_ptr)));
		}
		catch(...)
		{
		}
	}
	return L"";
}

//--------------------------------------------------------------

/*! \brief Вывод растрового изображения
	\param title Заголовок
	\param data Массив с данными
	\param hs Ширина картинки, пиксели
	\param vs Высота картинки, пиксели
	\param x0 Начальное значение x
	\param dx Шаг по координате x
	\param x_label Наименование координаты x
	\param y0 Начальное значение y
	\param dy Шаг по координате y
	\param y_label Наименование координаты y
*/
void api_ShowImage(const wstring &wtitle,
		const void* data, display_sample_type pixel_type,
		size_t vs, size_t hs,
		double y0, double dy, const wstring &y_label,
		double x0, double dx, const wstring &x_label,
		double v0, double vmax, double gamma, const wstring &value_label)
{
	api_ForceUpdateGUI(sec(0));

	try
	{
		ImageWindow *image_ptr = emit work_thread().request_CreateRasterImageSet(wstring_to_qstring(wtitle), int(vs), int(hs));
		emit work_thread().request_SetImageAxesScales(image_ptr, 0, 1, y0, dy, x0, dx);
		//	image_ptr->SetRanges(x0, dx, y0, dy);//эту вроде получается прямо в своем потоке делать, без emit. а вообще надо внимательнее посмотреть, где оно нужно, а где нет.
		emit work_thread().request_SetupImageLabels(image_ptr, wstring_to_qstring(wtitle),
															   "None",
															   wstring_to_qstring(y_label),
															   wstring_to_qstring(x_label),
															   wstring_to_qstring(value_label));
		emit work_thread().request_SetupImageDefaultRanges(image_ptr, v0, vmax, gamma);
		emit work_thread().request_SetupImageFrame(image_ptr, -1, data, pixel_type);
		emit work_thread().request_ShowDataWindow(static_cast<ImageWindow*>(image_ptr), true);

		work_thread().Suspend(ThreadUser::suspend_for_data_analyze);
	}
	catch(...)
	{
	}

}



/*!
	\details
	Доступны реализации для типов T:
	- wstring
	- string
	- bool
	- int
	- unsigned int
	- long long
	- unsigned long long
	- double
	- float

	При появлении новых типов аргументов следует добавлять соответствующие реализации путем
	явного инстанциирования шаблона.
*/
template <class T>
bool api_SaveParameter(const wstring &function_name, const wstring &param_name, const T &value)
{
	try
	{
		GUISaveParameter(function_name, param_name, value);
		return true;
	}
	catch(...)
	{
		return false;
	}
}

// Явное инстанциирование шаблонной функции для набора значений параметра T
#define instantiate(T) \
	template bool api_SaveParameter(const wstring &function_name, const wstring &param_name, \
			const T &value);

instantiate(wstring)
instantiate(string)
instantiate(bool)
instantiate(int)
instantiate(unsigned int)
instantiate(long long)
instantiate(unsigned long long)
instantiate(double)
instantiate(float)

#undef instantiate



/*!
	\details
	Доступны реализации для типов T:
	- wstring
	- string
	- bool
	- int
	- unsigned int
	- long long
	- unsigned long long
	- double
	- float

	При появлении новых типов аргументов следует добавлять соответствующие реализации путем
	явного инстанциирования шаблона.
*/
template <class T>
T api_GetSavedParameter(const wstring &function_name, const wstring &param_name,
		const T &default_value, bool *loaded)
{
	try
	{
		return GUILoadParameter(function_name, param_name, default_value, loaded);
	}
	catch(...)
	{
		return default_value;
	}
}

// Явное инстанциирование шаблонной функции для набора значений параметра T
#define instantiate(T) \
	template T api_GetSavedParameter(const wstring &function_name, const wstring &param_name, \
			const T &default_value, bool *loaded);

instantiate(wstring)
instantiate(string)
instantiate(bool)
instantiate(int)
instantiate(unsigned int)
instantiate(long long)
instantiate(unsigned long long)
instantiate(double)
instantiate(float)

#undef instantiate



XRAD_END
