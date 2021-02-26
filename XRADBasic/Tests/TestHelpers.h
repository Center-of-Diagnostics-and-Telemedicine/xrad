/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file TestHelpers.h
//--------------------------------------------------------------
#ifndef XRAD__File_TestHelpers_h
#define XRAD__File_TestHelpers_h
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
#endif // XRAD__File_TestHelpers_h
