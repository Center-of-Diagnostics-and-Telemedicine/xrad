//--------------------------------------------------------------
#ifndef XRAD__File_GUIConfig_h
#define XRAD__File_GUIConfig_h
/*!
	\file
	\brief Настройка конфигурации XRADGUI
*/
//--------------------------------------------------------------

#include <XRADBasic/Core.h>

//--------------------------------------------------------------

#if defined(XRAD_COMPILER_MSC)

// Конфигурация для MSVC

#define XRAD_USE_KEYBOARD_WIN32_VERSION



#elif defined(XRAD_COMPILER_GNUC)

// Конфигурация для GCC



#else
#error Unknown compiler.
#endif

//--------------------------------------------------------------
#endif // XRAD__File_GUIConfig_h
