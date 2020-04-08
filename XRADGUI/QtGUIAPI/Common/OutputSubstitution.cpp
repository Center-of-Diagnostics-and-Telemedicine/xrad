#include "pre.h"
#include "OutputSubstitution.h"

#include "OutputRedirector.h"
#include <XRADGUI/Sources/PlatformSpecific/Qt/Internal/StringConverters_Qt.h>
#include <XRADSystem/System.h>
#include <io.h>
#include <fcntl.h>

namespace XRAD_GUI
{

XRAD_USING
using namespace Qt;

//--------------------------------------------------------------

struct ConsoleRedirector::Internal
{
	Internal(function<void (QString)> process_stdout,
				function<void (QString)> process_stderr,
				function<void ()> stdout_ready_mt,
				function<void ()> stderr_ready_mt):
		process_stdout(process_stdout),
		process_stderr(process_stderr),
		stdout_buffer(stdout_ready_mt),
		stdout_redirector(OutputRedirector::OutputStream::StdOut),
		stderr_buffer(stderr_ready_mt),
		stderr_redirector(OutputRedirector::OutputStream::StdErr)
	{}

	function<void (QString)> process_stdout;
	function<void (QString)> process_stderr;
	AsyncTextBuffer stdout_buffer;
	OutputRedirector stdout_redirector;
	AsyncTextBuffer stderr_buffer;
	OutputRedirector stderr_redirector;
};

//--------------------------------------------------------------

ConsoleRedirector::ConsoleRedirector(
		function<void (QString)> process_stdout,
		function<void (QString)> process_stderr):
	internal(make_unique<Internal>(
			process_stdout, process_stderr,
			[this]() { emit this->signal_stdout_ready(); },
			[this]() { emit this->signal_stderr_ready(); }))
{
	connect(this, &ConsoleRedirector::signal_stdout_ready,
			this, &ConsoleRedirector::DoStdoutReady, QueuedConnection);
	connect(this, &ConsoleRedirector::signal_stderr_ready,
			this, &ConsoleRedirector::DoStderrReady, QueuedConnection);
	if (!internal->stdout_redirector.Start(&internal->stdout_buffer))
	{
		throw runtime_error("Stdout redirection failed.");
	}
	if (!internal->stderr_redirector.Start(&internal->stderr_buffer))
	{
		throw runtime_error("Stderr redirection failed.");
	}

	connect(&timer, &QTimer::timeout, this, &ConsoleRedirector::DoStdoutReady);
	connect(&timer, &QTimer::timeout, this, &ConsoleRedirector::DoStderrReady);
	timer.setInterval(200);
	timer.start();
}

//--------------------------------------------------------------

ConsoleRedirector::~ConsoleRedirector()
{
	internal->stdout_redirector.Stop();
	internal->stderr_redirector.Stop();
	timer.stop();
}

//--------------------------------------------------------------

namespace
{

string GetLogPath()
{
	return qstring_to_string(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
}

FILE *OpenNewLogFile(const string &app_name, const string &datetime, const string &suffix,
		int number,
		string *o_filename, int *o_number)
{
	// Важно!
	// Формат имени файла должен удовлетворять условиям:
	// - должен быть префикс, по которому можно идентифицировать файлы данного приложения
	//   (см. ClearOldLogFiles(), GetDateTimeStringForLogFileName());
	// - при алфавитной сортировке старые файлы должны быть в начале списка.

	string path = GetLogPath();
	string filename = ssprintf("%s/%s-%s-%s.log", path.c_str(), app_name.c_str(),
			datetime.c_str(), suffix.c_str());
	for (int n = 0; n < 100; ++n)
	{
		// Create a new file. Fail if the file already exists.
		// Note: In C++17, use (_w)fopen(filename, "wbx").
		int fd = _wopen(convert_to_wstring(filename).c_str(),
				_O_CREAT | _O_EXCL | _O_WRONLY | _O_BINARY,
				_S_IREAD | _S_IWRITE);
		if (fd != -1)
		{
			if (FILE *f = _fdopen(fd, "wb"))
			{
				try
				{
					*o_filename = std::move(filename);
					*o_number = n;
					return f;
				}
				catch (...)
				{
					fclose(f);
					throw;
				}
			}
		}
		filename = ssprintf("%s/%s-%s[%i]-%s.log", path.c_str(), app_name.c_str(),
				datetime.c_str(), n + 1, suffix.c_str());
	}
	return nullptr;
}

void ClearOldLogFiles(const string &app_name)
{
	constexpr size_t FileCountToKeep = 100;

	string path = GetLogPath();
	wstring filename_prefix = CmpNormalizeFilename(convert_to_wstring(ssprintf("%s/%s-",
			path.c_str(), app_name.c_str())));
	wstring filename_suffix = CmpNormalizeFilename(L".log");
	vector<wstring> filenames = GetDirectoryFiles(convert_to_wstring(path), L"", false);
	vector<wstring> filenames_sorted;
	for (wstring &f: filenames)
	{
		auto f_norm = CmpNormalizeFilename(f);
		if (f_norm.length() > filename_prefix.length() &&
				f_norm.length() > filename_suffix.length() &&
				!wcsncmp(f_norm.c_str(), filename_prefix.c_str(), filename_prefix.length()) &&
				!wcscmp(f_norm.c_str() + f_norm.length() - filename_suffix.length(),
						filename_suffix.c_str()))
		{
			filenames_sorted.push_back(f_norm);
		}
	}
	sort(filenames_sorted.begin(), filenames_sorted.end());

	if (filenames_sorted.size() <= FileCountToKeep)
		return;
	for (size_t i = 0; i < filenames_sorted.size() - FileCountToKeep; ++i)
	{
		DeleteFileW(filenames_sorted[i].c_str());
	}
}

//! \brief Строка с датой и временем для OpenNewLogFile()
string GetDateTimeStringForLogFileName()
{
	time_t t = time(nullptr);
	// Формат RFC3339 не подходит из-за использования символов, недопустимых в имени файла.
	// Используем более простой формат.
	const size_t buf_size = 1024;
	char buf[buf_size];
	size_t len = strftime(buf, buf_size, "%Y%m%d-%H%M%S%z", localtime(&t));
	string datetime;
	if (len)
		return string(buf, len);

	// В случае ошибки strftime.
	// По крайней мере дата должна совпадать с тем, что пишем с помощью strftime.
	const tm *s_time = gmtime(&t);
	return ssprintf("%04i%02i%02i-%02i%02i%02iZ",
			EnsureType<int>(s_time->tm_year + 1900),
			EnsureType<int>(s_time->tm_mon),
			EnsureType<int>(s_time->tm_mday),
			EnsureType<int>(s_time->tm_hour),
			EnsureType<int>(s_time->tm_min),
			EnsureType<int>(s_time->tm_sec));
}

} // namespace

//--------------------------------------------------------------

void ConsoleRedirector::LogToFiles(bool flag)
{
	if (log_to_files == flag)
		return;
	log_to_files = flag;
	if (flag)
	{
		string app_name = qstring_to_string(qApp->applicationName());

		time_t t = time(nullptr);
		// Формат RFC3339 не подходит из-за использования символов, недопустимых в имени файла.
		// Используем более простой формат.
		const size_t buf_size = 1024;
		char buf[buf_size];
		size_t len = strftime(buf, buf_size, "%Y%m%d-%H%M%S%z", localtime(&t));
		string datetime;
		if (len)
		{
			datetime = buf;
		}
		else
		{
			const tm *s_time = gmtime(&t);
			datetime = ssprintf("%04i%02i%02i-%02i%02i%02iZ",
					EnsureType<int>(s_time->tm_year + 1900),
					EnsureType<int>(s_time->tm_mon),
					EnsureType<int>(s_time->tm_mday),
					EnsureType<int>(s_time->tm_hour),
					EnsureType<int>(s_time->tm_min),
					EnsureType<int>(s_time->tm_sec));
		}

		int n = 0;
		string stdout_filename;
		FILE *f_stdout = OpenNewLogFile(app_name, datetime, "out", n, &stdout_filename, &n);
		if (f_stdout)
		{
			internal->stdout_redirector.SetLogFile(f_stdout);
			fprintf(stdout, "Stdout redirected to \"%s\".\n", stdout_filename.c_str());
			fflush(stdout);
		}
		else
		{
			fprintf(stdout, "Stdout redirection to file failed.\n");
			fflush(stdout);
		}
		// n: продолжаем с индекса, полученного при открытии лога stdout.
		string stderr_filename;
		FILE *f_stderr = OpenNewLogFile(app_name, datetime, "err", n, &stderr_filename, &n);
		if (f_stderr)
		{
			internal->stderr_redirector.SetLogFile(f_stderr);
			fprintf(stderr, "Stderr redirected to \"%s\".\n", stderr_filename.c_str());
			fflush(stderr);
		}
		else
		{
			fprintf(stderr, "Stderr redirection to file failed.\n");
			fflush(stderr);
		}

		ClearOldLogFiles(app_name);
	}
	else
	{
		if (internal->stdout_redirector.GetLogFile())
		{
			fprintf(stdout, "Stdout file closed.\n");
			fflush(stdout);
			internal->stdout_redirector.SetLogFile(nullptr);
		}
		if (internal->stderr_redirector.GetLogFile())
		{
			fprintf(stderr, "Stderr file closed.\n");
			fflush(stderr);
			internal->stderr_redirector.SetLogFile(nullptr);
		}
	}
}

//--------------------------------------------------------------

void ConsoleRedirector::DoStdoutReady()
{
	string s_text = internal->stdout_buffer.ReadBufferMT();
	if (!s_text.length())
		return;
	QString q_text = wstring_to_qstring(convert_to_wstring(s_text));
	internal->process_stdout(q_text);
}

//--------------------------------------------------------------

void ConsoleRedirector::DoStderrReady()
{
	string s_text = internal->stderr_buffer.ReadBufferMT();
	if (!s_text.length())
		return;
	QString q_text = wstring_to_qstring(convert_to_wstring(s_text));
	internal->process_stderr(q_text);
}

//--------------------------------------------------------------

namespace
{
constexpr performance_time_t update_interval = 200ms;
} // namespace

//--------------------------------------------------------------

bool ConsoleRedirector::eventFilter(QObject *target, QEvent *event)
{
	if (event->type() != QEvent::None)
	{
		static performance_time_t previous = GetPerformanceCounterStd();
		auto current = GetPerformanceCounterStd();
		auto delay = current-previous;

		if (delay >= update_interval)
		{
			DoStdoutReady();
			DoStderrReady();
			previous = current;
		}
	}
	return QObject::eventFilter(target, event);
}

//--------------------------------------------------------------

}//namespace XRAD_GUI
