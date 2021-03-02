/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file ThreadUtils.h
//--------------------------------------------------------------
#ifndef XRAD__ThreadUtils_h
#define XRAD__ThreadUtils_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>
#include <string>
#include <thread>

XRAD_BEGIN

using namespace std;

//--------------------------------------------------------------

string ToString(std::thread::id id);

string DebugThreadIdStr();

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__ThreadUtils_h
