/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//	file XRADGUIAPI.h
//	Created by ACS on 17.02.07

//  Modify by IRD on 05.2013 - create by QT
//  version 1.0.2
//--------------------------------------------------------------
#ifndef XRAD__File_XRADGUIAPI_h
#define XRAD__File_XRADGUIAPI_h
//--------------------------------------------------------------

#include "XRADGUIAPIDefs.h"
#include <XRADGUI/Sources/Core/GUICore.h>
#include <XRADGUI/Sources/GUI/GUIValue.h>
#include <XRADGUI/Sources/GUI/DisplaySampleType.h>
#include <XRADBasic/Sources/Containers/SpaceCoordinates.h>
#include <XRADBasic/Sources/Utils/PhysicalUnits.h>
#include <XRADBasic/Sources/Containers/DataArray.h>
#include <XRADBasic/ContainersAlgebra.h>

#include <string>
#include <vector>

//--------------------------------------------------------------

namespace XRAD_GUI
{

XRAD_USING

//--------------------------------------------------------------

#if 0

все исключения должны ловиться внутри main, так же как и для обычной программы

псевдокод :

int main()
{
	try
	{
		DoSomething();
	}
	catch (...)
	{
		HandleExceptions_NoThrow();
	}
	return 0;
}

bool api_GetExceptionString(string* message);
// обрабатывает специфические для платформы типы исключений
// (например, SysUtils::Exception из Delphi),
// возвращает признак того, что исключение обработано и
// сообщение message заполнено
//
// вопрос по поводу std::exception и прочих "стандартных" (int, string)
// пока открыт, так же как и вопрос об очерёдности вызовов этой функции
// и машинно-независимого фильтра
void api_DebugQuit();

//--------------------------------------------------------------
#endif

enum api_message_type
{
	api_msgFatal = 0,
	api_msgError,
	api_msgWarning,
	api_msgInfo,
	api_msgShow
};

//TODO Нужно точнее уяснить назначение этой функции и прокомментировать прототип
void api_ShowMessage(const wstring& message, api_message_type type,
	const wstring& type_message = wstring());

void api_Delay(const physical_time& delay);
void api_Pause();

//--------------------------------------------------------------

//size_t api_Decide2(string prompt, string first, string second, int default_choice);

size_t api_Decide2(const wstring& prompt, const wstring& first, const wstring& second, GUIValue<size_t> default_choice);
// возвращаемое значение, значение по умолчанию: 0 или 1

//size_t api_GetButtonDecision(string prompt, const vector<string> &buttons,
//		size_t default_button, size_t cancel_button);

size_t api_GetButtonDecision(const wstring& prompt,
	const vector<wstring>& buttons,
	GUIValue<size_t> default_button,
	size_t cancel_button);

size_t api_GetButtonDecisionMaxButtons();

bool	api_GetCheckboxDecision(const wstring& prompt, const vector<pair<wstring, bool*> >& boxes);
size_t	api_GetCheckboxDecisionMaxBoxes();
size_t	api_Decide(const wstring& prompt, const vector<wstring>& buttons, GUIValue<size_t> default_button);


//--------------------------------------------------------------

void api_ShowString(const wstring& title, const wstring& text, XRAD_GUI::display_text_dialog_status status);

void api_ShowIntegral(const wstring& title, ptrdiff_t value);
void api_ShowFloating(const wstring& title, double value);

ptrdiff_t api_GetIntegral(const wstring& prompt, GUIValue<ptrdiff_t> default_value, ptrdiff_t min_value, ptrdiff_t max_value, out_of_range_control allow_out_of_range);
double api_GetFloating(const wstring& prompt, GUIValue<double> default_value, double min_value, double max_value, out_of_range_control allow_out_of_range);
wstring api_GetString(const wstring& prompt, const GUIValue<wstring>& default_value, bool multiline = false);

//--------------------------------------------------------------

void api_StartProgress(const wstring& prompt, double count);
//! \brief Функция не вызывает исключений
void api_EndProgress();
void api_SetProgressPosition(double position);
void api_ForceUpdateGUI(const physical_time& update_interval);
bool api_IsProgressActive();
void api_ForceQuit(int exit_code);

//--------------------------------------------------------------

void api_GetFileName(wstring& wfilename, const wstring& wprompt, const GUIValue<wstring>& gwdefault, const wstring& wtype, XRAD_GUI::file_dialog_mode flag);
void api_GetFolderName(wstring& wfoldername, const wstring& wprompt, const GUIValue<wstring>& gwdefault, const wstring& wtype, XRAD_GUI::file_dialog_mode flag);


//! \brief Закрыть ранее созданное окно одного из типов GraphWindowContainer, ImageWindowContainer, TextWindowContainer
bool	api_CloseDataWindow(DataWindowContainer& dwc);

//! \brief Скрыть ранее созданное окно одного из типов GraphWindowContainer, ImageWindowContainer, TextWindowContainer
bool	api_HideDataWindow(DataWindowContainer& dwc);

bool api_ShowDataWindow(DataWindowContainer& gc, bool is_stopped = true);

//! \brief Сменить заголовок окна одного из типов GraphWindowContainer, ImageWindowContainer, TextWindowContainer
bool	api_SetDataWindowTitle(DataWindowContainer& dwc, const wstring& title);
bool	api_SetPersistent(DataWindowContainer& twc, bool persistent);
bool	api_SetStayOnTop(DataWindowContainer& twc, bool stay_on_top);

//--------------------------------------------------------------

GraphWindowContainer api_CreateGraph(const wstring& title, const wstring& x_label, const wstring& y_label);

// все следующие функции возвращают false в случае неудачи
// (если график удален через пользовательский интерфейс)

bool api_SetupGraphCurve(GraphWindowContainer& gc, size_t curve_no, const DataArray<double>& data_x, const DataArray<double>& data_y, const wstring& curve_name);
bool api_SetGraphLabels(GraphWindowContainer& gc, const wstring& title, const wstring& x_label, const wstring& y_label);
//bool api_SetGraphTitle(GraphWindowContainer &gc, const wstring &title);

bool api_SetGraphScale(GraphWindowContainer& gc, const range2_F64& scale);
bool api_GetGraphScale(const GraphWindowContainer& gc, range2_F64& scale);
bool api_SetGraphStyle(GraphWindowContainer& gc, graph_line_style style, double in_line_width);
bool	api_SaveGraphPicture(GraphWindowContainer& gc, const wstring& filename_with_extension);

//--------------------------------------------------------------

ImageWindowContainer api_CreateRasterImageSet(const wstring& title, size_t vs, size_t hs);
bool	api_AddImageFrames(ImageWindowContainer& risc, size_t n_frames);
bool	api_SetImageAxesScales(ImageWindowContainer&, double in_z0, double in_dz, double in_y0, double in_dy, double in_x0, double in_dx);
bool	api_SetImageDefaultBrightness(ImageWindowContainer&, double in_black, double in_white, double in_gamma);
bool	api_SetupImageFrame(ImageWindowContainer&, int in_frame_no, const void* data, display_sample_type pt);
bool	api_InsertImageFrame(ImageWindowContainer&, int after_frame_no, const void* data, display_sample_type pt);
bool	api_SetImageLabels(ImageWindowContainer&, const wstring& in_title, const wstring& in_z_label, const wstring& in_y_label, const wstring& in_x_label, const wstring& in_value_label);

TextWindowContainer api_CreateTextDisplayer(const wstring& title, bool fixed_width, bool editable);
bool	api_SetFixedWidth(TextWindowContainer&, bool fixed_width);
bool	api_SetEditable(TextWindowContainer&, bool editable);
bool	api_SetFontSize(TextWindowContainer&, double size);
bool	api_SetText(TextWindowContainer&, const wstring& text);
wstring	api_GetText(const TextWindowContainer&);



/*!
	\brief Функции сохранения значения в системном хранилище.
	Имеет реализации только для фиксированного набора значений параметра T
*/
template <class T>
bool api_SaveParameter(const wstring& function_name, const wstring& param_name, const T& value);

/*!
	\brief Функция загрузки значения из системного хранилища.
	Имеет реализации только для фиксированного набора значений параметра T
*/
template <class T>
T api_GetSavedParameter(const wstring& function_name, const wstring& param_name,
	const T& default_value, bool* loaded);



void api_ShowImage(const wstring& title,
	const void* data, display_sample_type pixel_type,
	size_t vs, size_t hs,
	double y0, double dy, const wstring& y_label,
	double x0, double dx, const wstring& x_label,
	double v0, double vmax, double gamma, const wstring& value_label);

//--------------------------------------------------------------

void api_SetVersionInfo(const string& text);



RealFunction2D_F32 api_GetGrayscalePainting(const wstring& title, size_t vsize, size_t hsize);
RealFunction2D_F32 api_GetGrayscalePainting(const wstring& title, RealFunction2D_F32);

ColorImageF32 api_GetColorPainting(const wstring& title, ColorImageF32);
ColorImageF32 api_GetColorPainting(const wstring& title, size_t vsize, size_t hsize);
//--------------------------------------------------------------

} // namespace XRAD_GUI

//--------------------------------------------------------------
#endif // XRAD__File_XRADGUIAPI_h
