#ifndef XRAD__File_QtStringConverters_Test_h
#define XRAD__File_QtStringConverters_Test_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>
#include <XRADBasic/Tests/TestHelpers.h>

XRAD_BEGIN

namespace QtStringConverters_Test
{

//--------------------------------------------------------------

using TestHelpers::ErrorReporter;

/*!
	\brief Проверка преобразований строк, зависящих от Qt
*/
void Test(ErrorReporter *error_reporter);

//--------------------------------------------------------------

} // namespace QtStringConverters_Test

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__File_QtStringConverters_Test_h
