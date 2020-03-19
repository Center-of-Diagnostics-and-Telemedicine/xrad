﻿// file StringConverters_MS_Test.h
//--------------------------------------------------------------
#ifndef __StringConverters_MS_Test_h
#define __StringConverters_MS_Test_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>

#ifdef XRAD_USE_MS_VERSION

#ifndef _MSC_VER
	#error "This file is for MS Visual Studio only"
#endif

#include "TestHelpers.h"

XRAD_BEGIN

namespace StringConverters_MS_Test
{

//--------------------------------------------------------------

using TestHelpers::ErrorReporter;

/*!
	\brief Проверка преобразований строк, зависящих от платформы MSWindows

	\note
	Внимание! Этот тест должен запускаться в однопоточном режиме, другие потоки не должны
	вызывать функции перекодировки строк, зависящие от платформы MSWindows.
	Во время теста переопределяется кодовая страница для string.
*/
void Test(ErrorReporter *error_reporter);

//--------------------------------------------------------------

} // namespace StringConverters_MS_Test

XRAD_END

#endif // XRAD_USE_MS_VERSION

//--------------------------------------------------------------
#endif // __StringConverters_MS_Test_h
