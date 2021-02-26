/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file FlowControl.cpp
//--------------------------------------------------------------
#include "pre.h"

#include "FlowControl.h"
#include "StringEncode.h"

XRAD_BEGIN

//--------------------------------------------------------------

RandomProgressBar::RandomProgressBar(ProgressProxy in_proxy):
	proxy(in_proxy), started(false)
{
	if (!proxy)
	{
		ForceDebugBreak();
		throw invalid_argument("RandomProgressBar::RandomProgressBar(): NULL proxy.");
	}
}

RandomProgressBar::~RandomProgressBar()
{
	end();
}

void	RandomProgressBar::start(const string &prompt, double max_position)
{
	start(convert_to_wstring(prompt), max_position);
}

void	RandomProgressBar::start(const wstring &prompt, double max_position)
{
	if (started)
	{
		ForceDebugBreak();
		throw runtime_error("RandomProgressBar::start(): The progress is already started.");
	}
	proxy->start(prompt, max_position > 0 ? max_position : 1);
	started = true;
}

void RandomProgressBar::set_position(double pos)
{
	if (started)
		proxy->set_position(pos);
}

performance_time_t RandomProgressBar::end()
{
	try
	{
		if (!started)
			return performance_time_t(0);
		started = false;
		return proxy->end();
	}
	catch (...)
	{
		return performance_time_t(0);
	}
}

ProgressProxy RandomProgressBar::subprogress(double start_pos, double end_pos)
{
	if (!started)
		return VoidProgressProxy();
	return proxy->subprogress(start_pos, end_pos);
}

//--------------------------------------------------------------

ProgressBar::ProgressBar(ProgressProxy in_proxy):
	proxy(in_proxy)
{
	if (!proxy)
	{
		ForceDebugBreak();
		throw invalid_argument("ProgressBar::ProgressBar(): NULL proxy.");
	}
}

ProgressBar::~ProgressBar()
{
	end();
}

void ProgressBar::start(const string &prompt, size_t count)
{
	start(convert_to_wstring(prompt), count);
}

void ProgressBar::start(const wstring &prompt, size_t count)
{
	if (started)
	{
		ForceDebugBreak();
		throw runtime_error("ProgressBar::start(): The progress is already started.");
	}
	proxy->start(prompt, count? count: 1);
	started = true;
	counter = 0;
	max_count = count;
}

void ProgressBar::next()
{
	if(started && max_count)
	{
		if (counter < max_count)
			++counter;
		proxy->set_position(counter);
	}
}

performance_time_t ProgressBar::end()
{
	try
	{
		if (!started)
			return performance_time_t(0);
		started = false;
		return proxy->end();
	}
	catch (...)
	{
		return performance_time_t(0);
	}
}

ProgressProxy ProgressBar::substep(double step_min, double step_max)
{
	if (!started)
		return VoidProgressProxy();
	return proxy->subprogress(counter + step_min, counter + step_max);
}

//--------------------------------------------------------------

XRAD_END
