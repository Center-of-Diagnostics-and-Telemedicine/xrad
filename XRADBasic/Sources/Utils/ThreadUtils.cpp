/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file ThreadUtils.cpp
//--------------------------------------------------------------
#include "pre.h"

#include "ThreadUtils.h"
#include <sstream>

XRAD_BEGIN

using namespace std;

//--------------------------------------------------------------

string ToString(std::thread::id id)
{
	ostringstream oss;
	oss << id;
	return oss.str();
}

//--------------------------------------------------------------

string DebugThreadIdStr()
{
	return ToString(std::this_thread::get_id());
}

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
