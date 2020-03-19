// file StringConverters_Qt_Test.h
//--------------------------------------------------------------
#ifndef __StringConverters_Qt_Test_h
#define __StringConverters_Qt_Test_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>
#include "TestHelpers.h"

XRAD_BEGIN

namespace StringConverters_Qt_Test
{

//--------------------------------------------------------------

using TestHelpers::ErrorReporter;

/*!
	\brief Проверка преобразований строк, зависящих от Qt
*/
void Test(ErrorReporter *error_reporter);

//--------------------------------------------------------------

} // namespace StringConverters_MS_Test

XRAD_END

//--------------------------------------------------------------
#endif // __StringConverters_Qt_Test_h
