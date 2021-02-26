/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file BasicUtils.cpp
//--------------------------------------------------------------
#include "pre.h"

#include "BasicUtils.h"
#include "String.h"

XRAD_BEGIN

//--------------------------------------------------------------


string format_assert_message(const string &condition, const string &func, const string &message)
{
	ForceDebugBreak();
	auto result = "Assertion failed. Function: \"" + func + "\". Condition: \"" + condition + "\".";
	if(message.size())
		result += "\n" + message;
	return result;
}


//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
