//	file XRAD_api.cpp
//	Created by ACS on 16.02.07
//--------------------------------------------------------------
#include "pre.h"
#include "XRADGUI.h"

#include "RasterImageSet.h"
#include <XRADGUIAPI.h>
#include <XRADBasic/Sources/Utils/ProgressProxyApi.h>
#include <ctime>
#include <cstdarg>

XRAD_BEGIN

using namespace XRAD_GUI;

axis_legend::axis_legend(double in_min, double in_step, wstring in_label) : min_value(in_min), step(in_step), label(in_label) {}
axis_legend::axis_legend(double in_min, double in_step, string in_label) : min_value(in_min), step(in_step), label(string_to_wstring(in_label, e_decode_literals)) {}


value_legend::value_legend(double in_min, double in_max, double in_gamma, wstring in_label) : display_range(in_min, in_max), gamma(in_gamma), label(in_label) {}
value_legend::value_legend(double in_min, double in_max, double in_gamma, string in_label) : display_range(in_min, in_max), gamma(in_gamma), label(string_to_wstring(in_label, e_decode_literals)) {}
value_legend::value_legend(const range1_F64& in_display_range, double in_gamma, wstring in_label) : display_range(in_display_range), gamma(in_gamma), label(in_label) {}
value_legend::value_legend(const range1_F64& in_display_range, double in_gamma, string in_label) : display_range(in_display_range), gamma(in_gamma), label(string_to_wstring(in_label, e_decode_literals)) {}


//--------------------------------------------------------------
//
// basic flow control
//
//--------------------------------------------------------------


//--------------------------------------------------------------

void ForceQuit(int exit_code)
{
	api_ForceQuit(exit_code);
}

//--------------------------------------------------------------

void Error(wstring message_string)
{
	//TODO либо сделать кнопку выхода, либо сделать немодальный дилог
	//TODO еще функцию показа исключений
	api_ShowMessage(message_string, api_msgError);
}

void Error(string message_string)
{
	Error(convert_to_wstring(message_string, e_decode_literals));
}
//--------------------------------------------------------------


void Pause(void)
{
	api_Pause();
}

void Delay(const physical_time& delay)
{
	api_Delay(delay);
}


//--------------------------------------------------------------

bool YesOrNo(string prompt, GUIValue<bool> default_choice)
{
	return YesOrNo(convert_to_wstring(prompt), default_choice);
}

bool YesOrNo(wstring prompt, GUIValue<bool> default_choice)
{
	return !api_Decide2(prompt, L"Yes", L"No",
		ConvertGUIValue<size_t>(default_choice, default_choice.value ? 0 : 1));
}

//--------------------------------------------------------------

size_t Decide2(wstring prompt, wstring choice0, wstring choice1, GUIValue<size_t> default_value)
{
	return api_Decide2(prompt, choice0, choice1, default_value);
}

size_t Decide2(string prompt, string choice0, string choice1, GUIValue<size_t> default_value)
{
	return Decide2(convert_to_wstring(prompt), convert_to_wstring(choice0), convert_to_wstring(choice1), default_value);
}

//--------------------------------------------------------------


//--------------------------------------------------------------
size_t Decide(wstring prompt, const vector<wstring>& buttons, GUIValue<size_t> default_button)
{
	if (!buttons.size())
	{
		Error(L"Decide(): no buttons provided.");
		return 0;
	}
	return api_Decide(prompt, buttons, default_button);
}

size_t Decide(string prompt, const vector<string>& buttons, GUIValue<size_t> default_button)
{
	vector<wstring>	wbuttons;
	for (auto button : buttons)
		wbuttons.push_back(convert_to_wstring(button));
	return Decide(convert_to_wstring(prompt), wbuttons, default_button);
}


//--------------------------------------------------------------

size_t GetButtonDecision(string prompt, const std::vector<string>& buttons, GUIValue<size_t> default_button)
{
	std::vector<wstring> wbuttons;
	for (auto button : buttons)
		wbuttons.push_back(convert_to_wstring(button));
	return GetButtonDecision(convert_to_wstring(prompt), wbuttons, default_button);
}

//--------------------------------------------------------------

size_t GetButtonDecision(wstring prompt, const std::vector<wstring>& buttons, GUIValue<size_t> default_button)
{
	size_t n_buttons = buttons.size();
	if (!n_buttons)
	{
		Error(L"GetButtonDecision(): no buttons provided.");
		return 0;
	}
	return api_GetButtonDecision(prompt, buttons, default_button, n_buttons - 1);
}


//--------------------------------------------------------------

namespace
{

	bool GetCheckboxDecisionBig(wstring prompt, const vector<pair<wstring, bool*> >& boxes)
	{
		bool res = false;
		if (boxes.size() <= api_GetCheckboxDecisionMaxBoxes())
			res = api_GetCheckboxDecision(prompt, boxes);
		else
		{
			// переписать с "постраничным" выводом
			res = api_GetCheckboxDecision(prompt, boxes);
		}
		if (res)
		{
			for (size_t i = 0; i < boxes.size(); ++i)
			{
			}
		}
		return res;
	}

} // namespace

//--------------------------------------------------------------


bool GetCheckboxDecision(wstring prompt, const vector<wstring>& texts, const vector<bool*>& values)
{
	XRAD_ASSERT_THROW(texts.size() == values.size() && values.size());
	//
	vector<pair<wstring, bool*> > boxes;
	boxes.reserve(texts.size());
	//
	for (size_t i = 0; i < texts.size(); i++)
	{
		boxes.push_back(make_pair(convert_to_wstring(texts[i]), values[i]));
	}
	//
	return GetCheckboxDecisionBig(convert_to_wstring(prompt), boxes);
}

bool GetCheckboxDecision(string prompt, const vector<string>& texts, const vector<bool*>& values)
{
	vector<wstring> wtexts;
	wtexts.reserve(texts.size());

	for (auto text : texts) wtexts.push_back(convert_to_wstring(text));
	return GetCheckboxDecision(convert_to_wstring(prompt), wtexts, values);
}



wstring GetFileNameRead(wstring prompt, GUIValue<wstring> default_name, wstring filter)
{
	wstring result;
	api_GetFileName(result, prompt, default_name, filter, XRAD_GUI::file_open_dialog);
	return result;
}

string GetFileNameRead(string prompt, GUIValue<string> default_name, string filter)
{
	return convert_to_string(GetFileNameRead(convert_to_wstring(prompt),
		ConvertGUIValue(default_name, convert_to_wstring(default_name.value)),
		convert_to_wstring(filter)));
}

wstring GetFileNameWrite(wstring prompt, GUIValue<wstring> default_name, wstring filter)
{
	wstring result;
	api_GetFileName(result, prompt, default_name, filter, XRAD_GUI::file_save_dialog);
	return result;
}

string GetFileNameWrite(string prompt, GUIValue<string> default_name, string filter)
{
	return convert_to_string(GetFileNameWrite(convert_to_wstring(prompt),
		ConvertGUIValue(default_name, convert_to_wstring(default_name.value)),
		convert_to_wstring(filter)));
}



wstring GetFolderNameRead(wstring prompt, GUIValue<wstring> default_name, wstring filter)
{
	wstring directory_name;
	api_GetFolderName(directory_name, prompt, default_name, filter, XRAD_GUI::file_open_dialog);
	return directory_name;
}

string GetFolderNameRead(string prompt, GUIValue<string> default_name, string filter)
{
	return convert_to_string(GetFolderNameRead(convert_to_wstring(prompt),
		ConvertGUIValue(default_name, convert_to_wstring(default_name.value)),
		convert_to_wstring(filter)));
}

wstring GetFolderNameWrite(wstring prompt, GUIValue<wstring> default_name, wstring filter)
{
	wstring directory_name;
	api_GetFolderName(directory_name, prompt, default_name, filter, XRAD_GUI::file_save_dialog);
	return directory_name;
}

string GetFolderNameWrite(string prompt, GUIValue<string> default_name, string filter)
{
	return convert_to_string(GetFolderNameWrite(convert_to_wstring(prompt),
		ConvertGUIValue(default_name, convert_to_wstring(default_name.value)),
		convert_to_wstring(filter)));
}

//--------------------------------------------------------------





//--------------------------------------------------------------
//
//	progress
//
//--------------------------------------------------------------



//--------------------------------------------------------------

void ForceUpdateGUI(const physical_time& update_interval)
{
	api_ForceUpdateGUI(update_interval);
}

bool IsProgressActive()
{
	return api_IsProgressActive();
}

//--------------------------------------------------------------

namespace
{

	class GUIProgressApi : public ProgressApi
	{
	public:
		virtual void Start(const wstring& message, double count) override
		{
			api_StartProgress(message, count);
			started = true;
		}
		virtual void End() override
		{
			started = false;
			api_EndProgress();
		}
		virtual bool Started() const override
		{
			return started;
		}
		virtual void SetPosition(double position) override
		{
			api_SetProgressPosition(position);
		}
		virtual void Update() override
		{
			xrad::ForceUpdateGUI();
		}
		virtual void ReportOverflow() override
		{
			if (overflow_reported)
				return;
			fprintf(stderr, "Progress: position overflow in set_position().\n");
			overflow_reported = true;
		}
	private:
		static bool started;
		static bool overflow_reported;
	};

	bool GUIProgressApi::started = false;
	bool GUIProgressApi::overflow_reported = false;

} // namespace

ProgressProxy GUIProgressProxy()
{
	return make_shared<ProgressProxyApi>(make_shared<GUIProgressApi>());
}

//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------


//--------------------------------------------------------------
//
//	simple I/O
//
//--------------------------------------------------------------


void ShowText(string prompt, string text, bool bIsStopped)
{
	ShowText(convert_to_wstring(prompt), convert_to_wstring(text), bIsStopped);
}

void ShowText(wstring prompt, wstring text, bool bIsStopped)
{
	using namespace XRAD_GUI;
	display_text_dialog_status status = bIsStopped ? multiple_use_window_paused : multiple_use_window_silent;
	api_ShowString(prompt, text, status);
}

void ShowString(string prompt, string text)
{
	ShowString(convert_to_wstring(prompt), convert_to_wstring(text));
}

void ShowString(wstring prompt, wstring text)
{
	api_ShowString(prompt, text, XRAD_GUI::single_use_window);
}



//--------------------------------------------------------------

void ShowSigned(wstring prompt, ptrdiff_t value)
{
	api_ShowIntegral(prompt, value);
}

void ShowSigned(string prompt, ptrdiff_t value)
{
	ShowSigned(convert_to_wstring(prompt), value);
}

void ShowUnsigned(wstring prompt, size_t value)
{
	api_ShowIntegral(prompt, value);
}

void ShowUnsigned(string prompt, size_t value)
{
	ShowUnsigned(convert_to_wstring(prompt), value);
}


//--------------------------------------------------------------

void ShowFloating(wstring prompt, double value)
{
	api_ShowFloating(prompt, value);
}

void ShowFloating(string prompt, double value)
{
	ShowFloating(convert_to_wstring(prompt), value);
}

//--------------------------------------------------------------

string GetString(string prompt, GUIValue<string> default_string)
{
	return convert_to_string(GetString(convert_to_wstring(prompt),
		ConvertGUIValue(default_string, convert_to_wstring(default_string.value))));
}

wstring GetString(wstring prompt, GUIValue<wstring> default_string)
{
	return api_GetString(prompt, default_string, false);
}

//--------------------------------------------------------------

ptrdiff_t GetSigned(wstring prompt, GUIValue<ptrdiff_t> default_value, ptrdiff_t min_value, ptrdiff_t max_value, out_of_range_control allow_out_of_range)
{
	return api_GetIntegral(prompt, default_value, min_value, max_value, allow_out_of_range);
}

size_t GetUnsigned(wstring prompt, GUIValue<size_t> default_value, size_t min_value, size_t max_value, out_of_range_control allow_out_of_range)
{
	return (size_t)api_GetIntegral(prompt, ConvertGUIValue(default_value, (ptrdiff_t)default_value.value),
		min_value, max_value, allow_out_of_range);
}

ptrdiff_t GetSigned(string prompt, GUIValue<ptrdiff_t> default_value, ptrdiff_t min_value, ptrdiff_t max_value, out_of_range_control allow_out_of_range)
{
	return GetSigned(convert_to_wstring(prompt), default_value, min_value, max_value, allow_out_of_range);
}

size_t GetUnsigned(string prompt, GUIValue<size_t> default_value, size_t min_value, size_t max_value, out_of_range_control allow_out_of_range)
{
	return GetUnsigned(convert_to_wstring(prompt), default_value, min_value, max_value, allow_out_of_range);
}

//--------------------------------------------------------------

double GetFloating(wstring prompt, GUIValue<double> default_value, double min_value, double max_value, out_of_range_control allow_out_of_range)
{
	return api_GetFloating(prompt, default_value, min_value, max_value, allow_out_of_range);
}

double GetFloating(string prompt, GUIValue<double> default_value, double min_value, double max_value, out_of_range_control allow_out_of_range)
{
	return GetFloating(convert_to_wstring(prompt), default_value, min_value, max_value, allow_out_of_range);
}


//--------------------------------------------------------------

//--------------------------------------------------------------
//todo (Kovbas) Для всех окон отображения данных нужно сделать функции закрытия окна из кода программы. (график, изображение, текст).



//--------------------------------------------------------------


template<>
void DisplayImage<uint8_t>(wstring name, const uint8_t* data,
	size_t vs, size_t hs,
	const axis_legend& ylegend,
	const axis_legend& xlegend,
	const value_legend& value_legend)
{
	if (!vs || !hs) Error(ssprintf("DisplayImage(%s,%d,%d), invalid dimensions.", name.c_str(), vs, hs));
	api_ShowImage(name, data, gray_sample_ui8,
		vs, hs,
		ylegend.min_value, ylegend.step, ylegend.label,
		xlegend.min_value, xlegend.step, xlegend.label,
		value_legend.display_range.p1(), value_legend.display_range.p2(), value_legend.gamma, value_legend.label);
}

template<>
void DisplayImage<int16_t>(wstring name, const int16_t* data,
	size_t vs, size_t hs,
	const axis_legend& ylegend,
	const axis_legend& xlegend,
	const value_legend& value_legend)
{
	if (!vs || !hs) Error(ssprintf("DisplayImage(%s,%d,%d), invalid dimensions.", name.c_str(), vs, hs));
	api_ShowImage(name, data, gray_sample_i16,
		vs, hs,
		ylegend.min_value, ylegend.step, ylegend.label,
		xlegend.min_value, xlegend.step, xlegend.label,
		value_legend.display_range.p1(), value_legend.display_range.p2(), value_legend.gamma, value_legend.label);
}

template<>
void DisplayImage<float>(wstring name, const float* data,
	size_t vs, size_t hs,
	const axis_legend& ylegend,
	const axis_legend& xlegend,
	const value_legend& value_legend)
{
	if (!vs || !hs) Error(ssprintf("DisplayImage(%s,%d,%d), invalid dimensions.", name.c_str(), vs, hs));
	api_ShowImage(name, data, gray_sample_f32,
		vs, hs,
		ylegend.min_value, ylegend.step, ylegend.label,
		xlegend.min_value, xlegend.step, xlegend.label,
		value_legend.display_range.p1(), value_legend.display_range.p2(), value_legend.gamma, value_legend.label);
}


template<>
void DisplayImage<ColorPixel>(wstring name, const ColorPixel* data,
	size_t vs, size_t hs,
	const axis_legend& ylegend,
	const axis_legend& xlegend,
	const value_legend& value_legend)
{
	if (!vs || !hs) Error(ssprintf("DisplayImage(%s,%d,%d), invalid dimensions.", name.c_str(), vs, hs));
	api_ShowImage(name, data, rgba_sample_ui8,
		vs, hs,
		ylegend.min_value, ylegend.step, ylegend.label,
		xlegend.min_value, xlegend.step, xlegend.label,
		value_legend.display_range.p1(), value_legend.display_range.p2(), value_legend.gamma, value_legend.label);
}

template<>
void DisplayImage<ColorSampleF32>(wstring name, const ColorSampleF32* data,
	size_t vs, size_t hs,
	const axis_legend& ylegend,
	const axis_legend& xlegend,
	const value_legend& vlegend)
{
	if (!vs || !hs) Error(ssprintf(L"DisplayImage(%Ls,%d,%d), invalid dimensions.", name.c_str(), vs, hs));
	api_ShowImage(name, data, rgb_sample_f32,
		vs, hs,
		ylegend.min_value, ylegend.step, ylegend.label,
		xlegend.min_value, xlegend.step, xlegend.label,
		vlegend.display_range.p1(), vlegend.display_range.p2(), vlegend.gamma, vlegend.label);
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
bool SaveParameter(const wstring& function_name, const wstring& param_name, const T& param)
{
	return api_SaveParameter(function_name, param_name, param);
}

// Явное инстанциирование шаблонной функции для набора значений параметра T
#define instantiate(T) \
	template bool SaveParameter(const wstring &function_name, const wstring &param_name, \
			const T &param);

instantiate(wstring)
instantiate(string)
instantiate(bool)
instantiate(int)
instantiate(unsigned int)
instantiate(long)
instantiate(unsigned long)
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
T GetSavedParameter(const wstring& function_name, const wstring& param_name, const T& default_value,
	bool* loaded)
{
	return api_GetSavedParameter(function_name, param_name, default_value, loaded);
}

// Явное инстанциирование шаблонной функции для набора значений параметра T
#define instantiate(T) \
	template T GetSavedParameter(const wstring &function_name, const wstring &param_name, \
			const T &default_value, bool *loaded);

instantiate(wstring)
instantiate(string)
instantiate(bool)
instantiate(int)
instantiate(unsigned int)
instantiate(long)
instantiate(unsigned long)
instantiate(long long)
instantiate(unsigned long long)
instantiate(double)
instantiate(float)

#undef instantiate



template<>
void DisplayImage<complexF32>(wstring name, const complexF32* data,
	size_t vs, size_t hs,
	const axis_legend& ylegend,
	const axis_legend& xlegend,
	const value_legend& vlegend)
{
	if (!vs || !hs) Error(ssprintf(L"DisplayImage(%Ls,%d,%d), invalid dimensions.", name.c_str(), vs, hs));
	api_ShowImage(name, data, complex_sample_f32,
		vs, hs,
		ylegend.min_value, ylegend.step, ylegend.label,
		xlegend.min_value, xlegend.step, xlegend.label,
		vlegend.display_range.p1(), vlegend.display_range.p2(), vlegend.gamma, vlegend.label);
}

//--------------------------------------------------------------

void SetVersionInfo(const string& text)
{
	api_SetVersionInfo(text);
}

RealFunction2D_F32	GetPainting(const wstring& title, size_t vsize, size_t hsize)
{
	RealFunction2D_F32 result = api_GetPainting(title, vsize, hsize);
	return result;
}


//--------------------------------------------------------------
XRAD_END
