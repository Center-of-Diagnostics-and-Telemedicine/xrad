/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#include "pre.h"
#include "TimeProfiler.h"

XRAD_BEGIN

physical_time GetPerformanceCounter()
{
	return msec(GetPerformanceCounterMSec());
}

TimeProfiler::TimeProfiler()
{
	accumulation_factor = 0.9;
	Reset();
}

void	TimeProfiler::Reset()
{
	count = 0;
// 	t_start = t_end = 0;
	t_end = t_start = CurrentTime_ms();
	smooth_time = 0;
	t_elapsed = 0;
	dt = 0;
	is_running = false;
}

double	TimeProfiler::CurrentTime_ms() const
{
	return	GetPerformanceCounterMSec();
}


void	TimeProfiler::Start()
{
	if(is_running)
	{
		throw runtime_error("TimeProfiler::Start. Timer is already running");
	}
	t_end = t_start = CurrentTime_ms();
	is_running = true;
}


int	TimeProfiler::Count() const
{
	return count;
}

void	TimeProfiler::Stop()
{
	if(!is_running)
	{
		throw runtime_error("TimeProfiler::Stop. Timer is not started");
	}
	t_end = CurrentTime_ms();

	is_running = false;
	dt = t_end-t_start;
	if(!dt) dt = 1;
	t_elapsed += dt;

	if(count) smooth_time = smooth_time*accumulation_factor + dt*(1.-accumulation_factor);
	else	smooth_time = dt;

	++count;
}


physical_time	TimeProfiler::LastElapsed() const
{
	if(is_running||!count)
	{
		throw runtime_error("TimeProfiler::LastElapsed. Timer is not ready");
	}
	return msec(dt);
}

physical_time	TimeProfiler::MeanElapsed() const
{
	if(is_running||!count)
	{
		throw runtime_error("TimeProfiler::MeanElapsed. Timer is not ready");
	}
	return msec(t_elapsed/count);
}

physical_time	TimeProfiler::SmoothElapsed() const
{
	if(is_running||!count)
	{
		throw runtime_error("TimeProfiler::SmoothElapsed. Timer is not ready");
	}
	return	msec(smooth_time);
}


physical_frequency	TimeProfiler::LastFPS() const
{
	if(is_running||!count)
	{
		throw runtime_error("TimeProfiler::LastFPS. Timer is not ready");
	}
	return KHz(1./dt);
}

physical_frequency	TimeProfiler::MeanFPS() const
{
	if(is_running||!count)
	{
		throw runtime_error("TimeProfiler::MeanFPS. Timer is not ready");
	}
	return KHz(double(count)/t_elapsed);
}

physical_frequency	TimeProfiler::SmoothFPS() const
{
	if(is_running||!count)
	{
		throw runtime_error("TimeProfiler::SmoothFPS. Timer is not ready");
	}
	return KHz(1./smooth_time);
}



XRAD_END
