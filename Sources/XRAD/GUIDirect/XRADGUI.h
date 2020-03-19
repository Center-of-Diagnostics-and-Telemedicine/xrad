#ifndef XRADGUI_
#define XRADGUI_

#include <XRADBasic/Core.h>
#include <XRAD/GUICore.h>
#include <XRAD/GUI/Keyboard.h>
#include <XRADBasic/Sources/SampleTypes/ColorSample.h>
#include <XRADBasic/Sources/Containers/SpaceCoordinates.h>
#include <XRADBasic/Sources/Utils/PhysicalUnits.h>
#include <XRADBasic/Sources/Containers/FixedSizeArray.h>
// #include <PixelNormalizers.h>
#include <DisplaySampleType.h>
#include <vector>
#include <string>
#include <memory>
#include <GUIValue.h>

XRAD_BEGIN

//--------------------------------------------------------------
//
//	basic flow control
//
//--------------------------------------------------------------



#define FatalError(message_string) throw runtime_error(convert_to_string(message_string, e_encode_literals));

void ForceQuit(int exit_code); // порождает исключение quit_application
//TODO сделать еще бы обработчик для вызова в блоке catch; написать макро, содержащее этот обработчик.
void Error(string message_string);
void Error(wstring message_string);
/// покаывает сообщение об ошибке и продолжает выполнение
///TODO Error, предусмотреть кнопку cancel (исключение) или continue

/*!
	\brief Стандартная обертка для вызова кода некой операции с обработкой возникающих в нем исключений

	Обработка исключений:
	- quit_application — пробрасывается дальше.
	- canceled_operation — ловится и игнорируется.
	- остальные исключения — выдается Error с текстом сообщения исключения.
*/
template <class F>
void SafeExecute(F f)
{
	try
	{
		f();
	}
	catch (canceled_operation &)
	{
	}
	catch (quit_application &)
	{
		throw;
	}
	catch (...)
	{
		Error(GetExceptionString());
	}
}

/*!
	\brief Реализация ожидания
	\param delay Время задержки. Под Windows точность задания задержки 10 мс с округлением в бо́льшую сторону
	(особенности реализации функций QThread::msleep(), usleep() и т.п.).
*/
void Delay(const physical_time &delay);

//! \brief Пауза рабочего потока с выводом диалогового окна
void Pause();

bool YesOrNo(string prompt, GUIValue<bool> default_choice = saved_default_value);
bool YesOrNo(wstring prompt, GUIValue<bool> default_choice = saved_default_value);

size_t Decide2(string prompt, string choice0, string choice1, GUIValue<size_t> default_value = saved_default_value);
size_t Decide2(wstring prompt, wstring choice0, wstring choice1, GUIValue<size_t> default_value = saved_default_value);

/*!
	\brief Форма с радио-кнопками

	\param prompt Заголовок
	\param buttons Массив кнопок
	\param default_button Кнопка по умолчанию
	\return Номер выбранной кнопки

	Эта функция практически эквивалентна \ref GetButtonDecision(). Отличия интерфейсные.
	В диалоговом окне этой есть штатная кнопка Cancel, кидающая исключение canceled_operation.
*/
size_t Decide(string prompt, const vector<string> &buttons, GUIValue<size_t> default_button = saved_default_value);
size_t Decide(wstring prompt, const vector<wstring> &buttons, GUIValue<size_t> default_button = saved_default_value);



/*!
	\brief Форма с кнопками для выбора действия, базовая функция

	\param prompt Заголовок
	\param buttons Массив кнопок
	\param default_button Кнопка по-умолчанию
	\param cancel_button Кнопка отмены
	\return Номер выбранной кнопки

	\sa
	- GetButtonDecision() с классом Button
	- Decide()
*/
size_t GetButtonDecision(string prompt, const std::vector<string> &buttons, GUIValue<size_t> default_button = saved_default_value);
size_t GetButtonDecision(wstring prompt, const std::vector<wstring> &buttons, GUIValue<size_t> default_button = saved_default_value);

template<class T>
__declspec(deprecated("Use GetButtonDecision() with the Button class")) T GetButtonDecision(wstring prompt, const std::vector<wstring> &buttons, const std::vector<T> &answers)
{
	XRAD_ASSERT_THROW(buttons.size()==answers.size());
	size_t	no = GetButtonDecision(prompt, buttons);
	return answers[no];
}

template<class T>
__declspec(deprecated("Use GetButtonDecision() with the Button class")) T GetButtonDecision(string prompt, const std::vector<string> &buttons, const std::vector<T> &answers)
{
	XRAD_ASSERT_THROW(buttons.size()==answers.size());
	size_t	no = GetButtonDecision(prompt, buttons);
	return answers[no];
}



/*!
	\brief Данные кнопки для GetButtonDecision()

	\tparam T Тип связанных с кнопкой данных. Должен поддерживать копирование. Может быть ссылкой.

	Для удобства использования шаблона сделаны вспомогательные функции MakeButton().
*/
template <class T>
class Button
{
	public:
		Button(const wstring &caption, T id): caption(caption), id(id), is_default(false),
				is_cancel(false) {}

		//! \brief Установить признак кнопки по умолчанию.
		//! Задание значения true здесь имеет приоритет над заданием кнопки по умолчанию
		//! в параметре GetButtonDecision()
		Button &SetDefault(bool flag = true) { is_default = flag; return *this; }

		//! \brief Установить признак кнопки отмены
		Button &SetCancel(bool flag = true) { is_cancel = flag; return *this; }
	public:
		wstring caption;
		T id;
		//! \brief Кнопка является кнопкой по умолчанию
		bool is_default;
		//! \brief Результат нажатия кнопки следует трактовать как отмену (Cancel)
		bool is_cancel;
};

//! \brief Создать кнопку для GetButtonDecision()
template <class T>
inline Button<T> MakeButton(const wstring &caption, T id) { return Button<T>(caption, id); }

//! \brief Создать кнопку для GetButtonDecision
template <class T>
inline Button<T> MakeButton(const string &caption, T id) { return Button<T>(convert_to_wstring(caption), id); }



//! \brief Диалог выбора варианта. Возвращает id выбранной кнопки
//!
//! Признак is_default == true у кнопки имеет приоритет над значением default_button_value.
template <class T, class V = GUIValue<T>>
T Decide(const wstring &prompt, const vector<Button<T>> &buttons, const V &default_button_value = saved_default_value)
{
	GUIValue<T> default_button(default_button_value);
	GUIValue<size_t> legacy_default_button = default_button.is_stored;
	bool default_button_found = false;
	vector<wstring> texts;
	for (size_t i = 0; i < buttons.size(); ++i)
	{
		auto &b = buttons[i];
		texts.push_back(b.caption);
		if (!default_button_found)
		{
			if (b.is_default)
			{
				legacy_default_button = MakeGUIValue(i, default_button.is_stored);
				default_button_found = true;
			}
			else if (default_button.value_valid &&
					try_equals<false>(b.id, default_button.value)) // не все типы T могут иметь operator==
			{
				legacy_default_button = MakeGUIValue(i, default_button.is_stored);
				// default_button_found не устанавливаем, чтобы у флага b.is_default был приоритет
			}
		}
	}
	size_t answer = Decide(prompt, texts, legacy_default_button);
	return buttons[answer].id;
}

//! \brief Диалог выбора варианта. Возвращает id выбранной кнопки
template <class T, class V = GUIValue<T>>
T Decide(const wstring &prompt, std::initializer_list<Button<T>> buttons, const V &default_button = saved_default_value)
{
	return Decide(prompt, vector<Button<T>>(buttons), default_button);
}



//! \brief Диалог выбора варианта (кнопки). Возвращает id выбранной кнопки
//!
//! Признак is_default == true у кнопки имеет приоритет над значением default_button_value.
template <class T, class V = GUIValue<T>>
T GetButtonDecision(const wstring &prompt, const vector<Button<T>> &buttons, const V &default_button_value = saved_default_value)
{
	GUIValue<T> default_button(default_button_value);
	GUIValue<size_t> legacy_default_button = default_button.is_stored;
	bool default_button_found = false;
	vector<wstring> texts;
	for (size_t i = 0; i < buttons.size(); ++i)
	{
		auto &b = buttons[i];
		texts.push_back(b.caption);
		if (!default_button_found)
		{
			if (b.is_default)
			{
				legacy_default_button = MakeGUIValue(i, default_button.is_stored);
				default_button_found = true;
			}
			else if (default_button.value_valid &&
					try_equals<false>(b.id, default_button.value)) // не все типы T могут иметь operator==
			{
				legacy_default_button = MakeGUIValue(i, default_button.is_stored);
				// default_button_found не устанавливаем, чтобы у флага b.is_default был приоритет
			}
		}
	}
	size_t answer = GetButtonDecision(prompt, texts, legacy_default_button);
	return buttons[answer].id;
}

//! \brief Диалог выбора варианта (кнопки). Возвращает id выбранной кнопки
template <class T, class V = GUIValue<T>>
T GetButtonDecision(const string &prompt, const vector<Button<T>> &buttons, const V &default_button = saved_default_value)
{
	return GetButtonDecision(convert_to_wstring(prompt), buttons, default_button);
}

//! \brief Диалог выбора варианта (кнопки). Возвращает id выбранной кнопки
template <class T, class V = GUIValue<T>>
T GetButtonDecision(const wstring &prompt, std::initializer_list<Button<T>> buttons, const V &default_button = saved_default_value)
{
	return GetButtonDecision(prompt, vector<Button<T>>(buttons), default_button);
}

//! \brief Диалог выбора варианта (кнопки). Возвращает id выбранной кнопки
template <class T, class V = GUIValue<T>>
T GetButtonDecision(const string &prompt, std::initializer_list<Button<T>> buttons, const V &default_button = saved_default_value)
{
	return GetButtonDecision(prompt, vector<Button<T>>(buttons), default_button);
}



/*!	\brief Форма с флажками для задания набора бинарных параметров
	\param prompt Заголовок
	\param boxesCount Количество флажков
	\param boxes Массив флажков
	\param [in,out] values  Значения на каждом флажке (отмечено/не отмечено)
	\return 0-отмена, 1-дальше
*/
bool GetCheckboxDecision(string prompt, const vector<string> &texts, const vector<bool *> &values);
bool GetCheckboxDecision(wstring prompt, const vector<wstring> &texts, const vector<bool *> &values);



//
//! \name Индикатор прогресса
//! @{

//! \brief Создание GUI индикатора прогресса
ProgressProxy GUIProgressProxy();

//! \brief Класс для GUI-прогресса с произвольными шагами
class GUIRandomProgressBar: public RandomProgressBar
{
	public:
		GUIRandomProgressBar(): RandomProgressBar(GUIProgressProxy()) {}
};

//! \brief Класс для GUI-прогресса с произвольными шагами
class GUIProgressBar: public ProgressBar
{
	public:
		GUIProgressBar(): ProgressBar(GUIProgressProxy()) {}
};

//!	\brief сообщает, активен ли в настоящее время индикатор прогресса
bool IsProgressActive();

//! @}

//! \brief	Принудительно обновляет все окна графического интерфейса. Нужно, если автоматическое обновление срабатывает реже, чем надо.
void ForceUpdateGUI(const physical_time &update_interval = sec(0.1));


//
//--------------------------------------------------------------





//--------------------------------------------------------------
//
//	simple I/O
//
//--------------------------------------------------------------

//	отображает окно с текстом, которое можно не закрывать
//	(таким же образом, как ранее обрабатывались окна с графикой)
//TODO название не слишком удачное, объединить с ShowString, добавив параметр (может быть)
//TODO или сделать разницу не только в глаголе, но и в существительном (например, DisplayText)?
void ShowText(wstring prompt, wstring text, bool bIsStopped = true);
template<class S1, class S2> void ShowText(const S1 &prompt, const S2 &value, bool bIsStopped = true){ ShowText(convert_to_wstring(prompt), convert_to_wstring(value), bIsStopped);}

//	отображает значения во временном окне, которое закрывается кнопкой ОК
void ShowString( wstring prompt, wstring value);
template<class S1, class S2> void ShowString(const S1 &prompt, const S2 &value){ ShowString(convert_to_wstring(prompt), convert_to_wstring(value)); }

void ShowUnsigned(wstring prompt, size_t value);
void ShowSigned(wstring prompt, ptrdiff_t value);
void ShowFloating( wstring prompt, double value);

template<class S> void ShowUnsigned(const S &prompt, size_t value){ ShowUnsigned(convert_to_wstring(prompt), value); }
template<class S> void ShowSigned(const S &prompt, ptrdiff_t value){ ShowSigned(convert_to_wstring(prompt), value); }
template<class S> void ShowFloating( const S &prompt, double value){ ShowFloating(convert_to_wstring(prompt), value); }

//! \name Функции пользовательского ввода
//! \{

/*!	\brief Ввод текстового значения
	\param prompt Заголовок окна
	\param default_value Текст сообщения по умолчанию
	\return Текст
*/

string GetString( string prompt, GUIValue<string> default_value = saved_default_value);
wstring GetString( wstring prompt, GUIValue<wstring> default_value = saved_default_value);

/*!	\brief Ввод целого числа
	\param prompt Заголовок окна
	\param min_value Минимальное значение
	\param max_value Максимальное значение
	\param default_value Значение по умолчанию
	\return Целое число
*/

size_t GetUnsigned( string prompt, GUIValue<size_t> default_value, size_t min_value, size_t max_value, out_of_range_control allow_out_of_range = out_of_range_prohibited);
size_t GetUnsigned( wstring prompt, GUIValue<size_t> default_value, size_t min_value, size_t max_value, out_of_range_control allow_out_of_range = out_of_range_prohibited);
ptrdiff_t GetSigned( string prompt, GUIValue<ptrdiff_t> default_value, ptrdiff_t min_value, ptrdiff_t max_value, out_of_range_control allow_out_of_range = out_of_range_prohibited);
ptrdiff_t GetSigned( wstring prompt, GUIValue<ptrdiff_t> default_value, ptrdiff_t min_value, ptrdiff_t max_value, out_of_range_control allow_out_of_range = out_of_range_prohibited);

/*!	\brief Ввод вещественного числа
	\param prompt Заголовок окна
	\param min_value Минимальное значение
	\param max_value Максимальное значение
	\param default_value Значение по умолчанию
	\return Вещественное число
*/
double GetFloating( string prompt, GUIValue<double> default_value, double min_value, double max_value, out_of_range_control allow_out_of_range = out_of_range_prohibited);
double GetFloating( wstring prompt, GUIValue<double> default_value, double min_value, double max_value, out_of_range_control allow_out_of_range = out_of_range_prohibited);
//! \} //Функции пользовательского ввода



//--------------------------------------------------------------
//
//	filenames
//
//--------------------------------------------------------------



/*! \brief Формы выбора/сохранения файла
	\param prompt Заголовок
	\param filter Тип файлов
	\return написать
*/


// string GetExistingDirectoryName(string prompt, string filter = "*.*");
// wstring GetExistingDirectoryName(wstring prompt, wstring filter = L"*.*");

string GetFolderNameWrite(string prompt, GUIValue<string> default_name = saved_default_value, string filter = "*.*");
wstring GetFolderNameWrite(wstring prompt, GUIValue<wstring> default_name = saved_default_value, wstring filter = L"*.*");

string GetFolderNameRead(string prompt, GUIValue<string> default_name = saved_default_value, string filter = "*.*");//TODO default value
wstring GetFolderNameRead(wstring prompt, GUIValue<wstring> default_name = saved_default_value, wstring filter = L"*.*");

string GetFileNameRead(string prompt, GUIValue<string> default_name = saved_default_value, string filter = "*.*");
wstring GetFileNameRead(wstring prompt, GUIValue<wstring> default_name = saved_default_value, wstring filter = L"*.*");

string GetFileNameWrite(string prompt, GUIValue<string> default_name = saved_default_value, string filter = "*.*");
wstring GetFileNameWrite(wstring prompt, GUIValue<wstring> default_name = saved_default_value, wstring filter = L"*.*");

/*!
	\brief Функции сохранения значения в системном хранилище.
	Имеет реализации только для фиксированного набора значений параметра T

	\sa GUISaveParameter.
*/
template <class T>
bool SaveParameter(const wstring &function_name, const wstring &param_name, const T &param);

/*!
	\brief Функция загрузки значения из системного хранилища. Имеет реализации
	только для фиксированного набора значений параметра T

	\sa GUILoadParameter.
*/
template <class T>
T GetSavedParameter(const wstring &function_name, const wstring &param_name,
		const T &default_value, bool *loaded = nullptr);



//--------------------------------------------------------------
//
// вывод растровых изображений
// unsigned char *data: диапазон значений от 0 до 255 вкл., 0 - black, 255 - white
//
//--------------------------------------------------------------



//	частные случаи сделаны через частичную реализацию шаблона
//	сделаны реализации для SAMPLE_T = uint8_t, ColorPixel, int16_t, uint16_t, float, complexF32
//	добавить в первую очередь еще: int16_t, float

template<class SAMPLE_T>
void DisplayImage(wstring name, const SAMPLE_T* data,
		size_t vs, size_t hs,
		const axis_legend &y_axis_legend,
		const axis_legend &x_axis_legend,
		const value_legend &vlegend);

template<class SAMPLE_T>
void DisplayImage(string name, const SAMPLE_T* data,
		size_t vs, size_t hs,
		const axis_legend &y_axis_legend,
		const axis_legend &x_axis_legend,
		const value_legend &vlegend)
		{
		DisplayImage(string_to_wstring(name, e_decode_literals), data,
			vs, hs, y_axis_legend, x_axis_legend, vlegend);
		}


//--------------------------------------------------------------
//
// TODO запрос области интереса (доделать применительно к RasterAnimations)
//
//--------------------------------------------------------------



//double_range2 GetROI( const char *prompt, const double_range2 &default_value, const double_range2 &max_bounds);
//double_range3 GetROI( const char *prompt, const double_range3 &default_value, const double_range3 &max_bounds);



#define __XRAD_INTERFACE_FUNCTIONS
// по этой константе можно узнать, доступны ли интерфейсные функции
// в конкретном проекте
//	TODO константа устарела?

XRAD_END


#include <XRAD/PlatformSpecific/MSVC/PC_XRADGUILink.h>


#endif //XRADGUI_
