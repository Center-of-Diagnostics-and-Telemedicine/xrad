/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file StringEncodeTest.h
//--------------------------------------------------------------
#ifndef XRAD__File_StringEncodeTest_h
#define XRAD__File_StringEncodeTest_h
//--------------------------------------------------------------

#include <XRADBasic/Sources/Core/StringEncode.h>
#include "TestHelpers.h"

XRAD_BEGIN

namespace StringEncodeTest
{

//--------------------------------------------------------------

using TestHelpers::ErrorReporter;

void Test(ErrorReporter *error_reporter);

//--------------------------------------------------------------

} // namespace StringEncodeTest

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__File_StringEncodeTest_h
