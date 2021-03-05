/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//	file PCVisualCSpecific.h
//	Created by KNS on 29.08.13
#ifndef XRAD__File_PCVisualCSpecific_h
#define XRAD__File_PCVisualCSpecific_h
//--------------------------------------------------------------
/*!
	\addtogroup gr_CompilerSpecific
	@{

	\file
	\brief Определения для Visual C++. Этот файл должен включаться до включения стандартных заголовочных файлов
*/
//--------------------------------------------------------------
//
// Отмена некоторых неуместных предупреждений компилятора
//
//--------------------------------------------------------------

// warning C4996: 'fscanf': This function or variable may be unsafe. Consider using fscanf_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
// При отключении C4996 (#pragma warning(disable:4996)) отключаются __deprecated, что нежелательно.
// Поэтому отключаем только _CRT_SECURE_NO_WARNINGS, но, чтобы это работало,
// данный файл должен быть включен прежде заголовочных файлов MSVC.
#ifndef _CRT_SECURE_NO_WARNINGS
	#define _CRT_SECURE_NO_WARNINGS
#endif

// warning C4244: '=' : conversion from 'double' to 'float', possible loss of data
#pragma warning(disable:4244)

//warning C4512: 'class' : assignment operator could not be generated
// с этим предупреждением стоит поработать подробнее (ввести non-copiable класс, наследоваться от него -- поможет навести порядок!)
#pragma warning(disable:4512)

//warning C4127: conditional expression is constant
#pragma warning(disable:4127)

// Forcing value to bool 'true' or 'false' (performance warning)
// Это предупреждение не вполне по делу. См. статьи:
// https://stackoverflow.com/questions/20919650/warning-c4800-int-forcing-value-to-bool-true-or-false-performance-warn
// https://developercommunity.visualstudio.com/content/problem/78259/visual-studio-2017-no-longer-generates-a-c4800-war.html
#pragma warning (disable: 4800)

// warning C4100: 'arg': unreferenced formal parameter
#pragma warning (disable: 4100)

// warning C4458: declaration of 'current_value' hides class member
#pragma warning (disable: 4458)

// warning C4456: declaration of 'cursor' hides previous local declaration
#pragma warning (disable: 4456)

// warning C4238: nonstandard extension used: class rvalue used as lvalue
// Возникает при попытке получить адрес временного (возвращаемого) объекта.
// Форсируем ошибку.
#pragma warning (error: 4238)

// warning C4239: nonstandard extension used: 'initializing': conversion from 'xrad::RealFunction<float,double>' to 'xrad::RealFunctionF32 &'
// Возникает при попытке получить не const ссылку на временный (возвращаемый) объект.
// Форсируем ошибку.
#pragma warning (error: 4239)

// warning C4840: non-portable use of class 'std::basic_string<...>' as an argument to a variadic function
// Возникает при попытке передать в функцию с переменным числом аргументов
// (с многоточием, как printf) структуру или класс, для которых побитовое копирование
// не является допустимым. Передача таких аргументов не является допустимой.
// MSVC 2019 / C++17.
#pragma warning (error: 4840)

// отключаем предупреждения вида  warning C4996: 'std::copy': Function call with parameters that may be unsafe - this call relies on the caller to check that the passed values are correct.
// компилятор MS пытается помешать использованию алгоритмов STL с указателями. взамен предлагает свою структуру checked_array_iterator, но она не переносима
#ifndef _SCL_SECURE_NO_WARNINGS
	#define _SCL_SECURE_NO_WARNINGS
#endif

#if _MSC_VER >= 1920
// Workaround для устранения несовместимости C++17 и Windows 10 SDK.
// Проверено на: MSVC 2019 16.4.4 (_MSC_VER == 1924), Windows 10 SDK 18362.
// https://developercommunity.visualstudio.com/content/problem/93889/error-c2872-byte-ambiguous-symbol.html
// https://github.com/microsoft/STL/issues/204
#ifndef _HAS_STD_BYTE 
	#define _HAS_STD_BYTE 0
#else
	#if _HAS_STD_BYTE != 0
		//TODO
		//#error
	#endif //
#endif
#endif

//--------------------------------------------------------------

#include <intrin.h> // для __debugbreak

//--------------------------------------------------------------
//
// Устранение проблем совместимости
//
//--------------------------------------------------------------

// Некоторые важные заголовки (например windows.h) определяют max, min как макро.
// Это делает невозможным включение заголовков stl, где это же определяется шаблонами.
// Можно отменить назначение опциями препроцессора, но они бывают разные.
// Отменяем напрямую, если оно было задано.

#ifdef max
	#undef max
#endif

#ifdef min
	#undef min
#endif

#define NOMINMAX

//--------------------------------------------------------------
//
// Обязательные определения
//
//--------------------------------------------------------------

//! \brief Компилятор MSVC. Значение константы равно _MSC_VER
#define XRAD_COMPILER_MSC _MSC_VER

// Разрядность
#ifdef _M_X64
	#define XRAD_SIZE_T_BITS 64
#elif defined _M_IX86
	#define XRAD_SIZE_T_BITS 32
#else
	#error "Unknown platform."
#endif

//! \brief Задает порядок байтов: little endian
#define XRAD_ENDIAN XRAD_LITTLE_ENDIAN

//! \brief Задает функцию выхода в отладчик
#define XRAD_FORCE_DEBUG_BREAK_FUNCTION() __debugbreak()


//! \brief Нужно размещать в public-секции итераторов. Сообщает компилятору, что итератор контролирует свои границы во избежание предупреждения C4996
#define XRAD_CheckedIterator typedef void _Unchecked_type;

namespace xrad
{

//! \brief Значение черного цвета для полутонового изображения в Windows
#define	gray_pixel_black 0u

//! \brief Значение белого цвета для полутонового изображения в Windows
#define	gray_pixel_white 255u

//! \brief Тип пикселя полутонового изображения в Windows
typedef	unsigned char	rgb_pixel_component_type;

} // namespace xrad

//--------------------------------------------------------------
//
// Опциональные определения
//
//--------------------------------------------------------------

#define XRAD_USE_MS_VERSION

//--------------------------------------------------------------
//! @} <!-- ^group gr_CompilerSpecific -->
#endif // XRAD__File_PCVisualCSpecific_h
