/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file ConsoleProgress.cpp
//--------------------------------------------------------------
#include "pre.h"

#include "ConsoleProgress.h"
#include <XRADBasic/Sources/Utils/ProgressProxyApi.h>

XRAD_BEGIN

//--------------------------------------------------------------

namespace
{

class ConsoleProgressApi: public ProgressApi
{
	public:
		ConsoleProgressApi() {}
		virtual ~ConsoleProgressApi() {}
	public:
		static bool EnablePercent(bool enabled)
		{
			bool prev_value = percent_enabled;
			percent_enabled = enabled;
			return prev_value;
		}
	public:
		virtual void Start(const wstring &in_prompt, double in_count) override
		{
			prompt = convert_to_string(in_prompt);
			max_count = in_count;
			position = 0;
			if (percent_enabled)
			{
				printf("\n%s\n0%%        ", prompt.c_str());
				fflush(stdout);
			}
			else
			{
				printf("%s\n", prompt.c_str());
			}
			started = true;
		}
		virtual void End() override
		{
			started = false;
			if (percent_enabled)
			{
				printf("\r%s Done        \n", prompt.c_str());
				fflush(stdout);
			}
			else
			{
				printf("%s Done\n", prompt.c_str());
			}
		}
		virtual bool Started() const override
		{
			return started;
		}
		virtual void SetPosition(double pos) override
		{
			position = pos;
			if (percent_enabled)
			{
				printf("\r%lf%%        ", EnsureType<double>(100*double(pos)/max_count));
				fflush(stdout);
			}
		}
		virtual void Update() override
		{
		}
		virtual void ReportOverflow() override
		{
			if (overflow_reported)
				return;
			fprintf(stderr, "Progress: position overflow in set_position().\n");
			fflush(stdout);
			overflow_reported = true;
		}
	private:
		double position = 0;
		double	max_count = 0;
		string	prompt;
	private:
		static bool started;
		static bool overflow_reported;
		static bool percent_enabled;
};

bool ConsoleProgressApi::started = false;
bool ConsoleProgressApi::overflow_reported = false;
bool ConsoleProgressApi::percent_enabled = true;

} // namespace

//--------------------------------------------------------------

ProgressProxy ConsoleProgressProxy()
{
	return make_shared<ProgressProxyApi>(make_shared<ConsoleProgressApi>());
}

//--------------------------------------------------------------

bool ConsoleProgressProxyEnablePercent(bool enable)
{
	return ConsoleProgressApi::EnablePercent(enable);
}

//--------------------------------------------------------------

XRAD_END
