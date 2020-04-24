// file TestHelpers.h
//--------------------------------------------------------------
#ifndef __TestHelpers_h
#define __TestHelpers_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>

XRAD_BEGIN

namespace TestHelpers
{

class ErrorReporter
{
	public:
		virtual void ReportError(const string &error_message) = 0;
};

} // TestHelpers

XRAD_END

//--------------------------------------------------------------
#endif // __TestHelpers_h
