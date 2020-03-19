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
		virtual void Start(const wstring &in_prompt, double in_count) override
		{
			prompt = convert_to_string(in_prompt);
			max_count = in_count;
			position = 0;
			printf("\n%s\n0%%        ", prompt.c_str());
			fflush(stdout);
			started = true;
		}
		virtual void End() override
		{
			started = false;
			printf("\r%s Done        \n", prompt.c_str());
			fflush(stdout);
		}
		virtual bool Started() const override
		{
			return started;
		}
		virtual void SetPosition(double pos) override
		{
			position = pos;
			printf("\r%lf%%        ", EnsureType<double>(100*double(pos)/max_count));
			fflush(stdout);
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
};

bool ConsoleProgressApi::started = false;
bool ConsoleProgressApi::overflow_reported = false;

} // namespace

//--------------------------------------------------------------

ProgressProxy ConsoleProgressProxy()
{
	return make_shared<ProgressProxyApi>(make_shared<ConsoleProgressApi>());
}

//--------------------------------------------------------------

XRAD_END
