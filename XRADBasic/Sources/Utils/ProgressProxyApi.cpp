/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file ProgressProxyApi.cpp
//--------------------------------------------------------------
#include "pre.h"

#include "ProgressProxyApi.h"
#include <XRADBasic/Core.h>

XRAD_BEGIN

//--------------------------------------------------------------

std::atomic<bool> ProgressProxyApi::log_enabled = false;

//--------------------------------------------------------------

ProgressProxyApi::ProgressProxyApi(shared_ptr<ProgressApi> api_lock):
	mode(Mode::TopLevel),
	start_position(0),
	interval(1),
	api_lock(api_lock),
	api(api_lock.get())
{
}

//--------------------------------------------------------------

ProgressProxyApi::ProgressProxyApi(double start_pos, double end_pos, double api_pos_norm,
		shared_ptr<ProgressApi> api_lock):
	mode(Mode::Subprogress),
	start_position(start_pos),
	interval(end_pos - start_pos),
	api_position_normalizer(api_pos_norm),
	api_lock(api_lock),
	api(api_lock.get())
{
}

//--------------------------------------------------------------

ProgressProxyApi::~ProgressProxyApi()
{
	if (started)
	{
		try
		{
			end();
		}
		catch (...)
		{
		}
	}
}

//--------------------------------------------------------------

void ProgressProxyApi::EnableLog(bool enable)
{
	log_enabled = enable;
}

//--------------------------------------------------------------

void ProgressProxyApi::start(const wstring &prompt, double count)
{
	if (started)
	{
		ForceDebugBreak();
		throw runtime_error("Progress::start(): called twice.");
	}
	if (!count)
	{
		ForceDebugBreak();
		throw invalid_argument("Progress::start(): invalid step count (zero).");
	}
	m_prompt = prompt;
	switch (mode)
	{
		case Mode::TopLevel:
			api_position_normalizer = count;
			void_progress = api->Started();
			if (!void_progress)
			{
				api->Start(prompt, count);
			}
			break;
		case Mode::Subprogress:
			void_progress = false;
			break;
	}
	started = true;
	ext_position_normalizer = count;
	start_time = GetPerformanceCounterStd();
	if (log_enabled && m_prompt.length())
	{
		printf("%s...\n",
				EnsureType<const char*>(convert_to_string(m_prompt).c_str()));
		fflush(stdout);
	}
	//printf("Progress.start(): [%lf-%lf] * %lf: %s\n",
	//		EnsureType<double>(start_position),
	//		EnsureType<double>(start_position + interval),
	//		EnsureType<double>(api_position_normalizer),
	//		EnsureType<const char*>(convert_to_string(prompt).c_str()));
}

//--------------------------------------------------------------

void ProgressProxyApi::set_position(double pos)
{
	if (!started)
		return;
	if (!in_range(pos, 0, ext_position_normalizer))
	{
		api->ReportOverflow();
	}
	if (void_progress)
		return;
	api->SetPosition(api_position_normalizer * (start_position +
			interval * range(pos, 0, ext_position_normalizer) / ext_position_normalizer));
}

//--------------------------------------------------------------

performance_time_t ProgressProxyApi::end()
{
	if (!started)
		return performance_time_t();

	started = false;

	switch (mode)
	{
		case Mode::TopLevel:
			// Для вызова из деструктора: завершаем прогресс обязательно.
			if (!void_progress)
			{
				api->End();
			}
			break;
		case Mode::Subprogress:
			api->SetPosition(api_position_normalizer * (start_position + interval));
			break;
	}
	performance_time_t process_time = GetPerformanceCounterStd() - start_time;
	if (log_enabled && m_prompt.length())
	{
		printf("%s: %lf sec.\n",
				EnsureType<const char*>(convert_to_string(m_prompt).c_str()),
				EnsureType<double>(chrono::duration_cast<chrono::duration<double>>(process_time).count()));
		fflush(stdout);
	}
	return process_time;
}

//--------------------------------------------------------------

void ProgressProxyApi::update()
{
	api->Update();
}

//--------------------------------------------------------------

shared_ptr<ProgressProxyCore> ProgressProxyApi::subprogress(double start_pos, double end_pos)
{
	if (!started || void_progress)
	{
		return VoidProgressProxy();
		// Сейчас корректно создать подпрогресс до вызова start() невозможно.
		// В будущем возможно определить семантику такой операции.
		// Поэтому не выдаем ошибку.
	}
	double start_pos_norm = range(start_pos/ext_position_normalizer, 0, 1);
	double sub_start_pos = start_position + interval * start_pos_norm;
	double sub_end_pos = start_position +
			interval * range(end_pos/ext_position_normalizer, start_pos_norm, 1);
	return make_shared<ProgressProxyApi>(sub_start_pos, sub_end_pos,
			api_position_normalizer, api_lock);
}

//--------------------------------------------------------------

XRAD_END
