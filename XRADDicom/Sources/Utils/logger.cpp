/*!
	\file
	\date 1/12/2018 12:12:47 PM
	\author kovbas

	\brief Здесь предполагается реализация логгера для XRADDicom
*/
#include "pre.h"
#include "logger.h"

#include <dcmtk/config/osconfig.h>
#include <dcmtk/oflog/consap.h>
#include <dcmtk/oflog/nullap.h>

XRAD_BEGIN

namespace Dicom
{

void EnableLogs(bool enable)
{
	if (enable)
	{
		// Включаем логи DCMTK: копия кода из OFLog_init() (dcmtk: oflog.cc)
		// Шаблон вывода: loglevel_prefix: message\n
		const char *pattern = "%P: %m%n";
		OFunique_ptr<dcmtk::log4cplus::Layout> layout(new dcmtk::log4cplus::PatternLayout(pattern));
		// logToStdErr = true, immediateFlush = true
		dcmtk::log4cplus::SharedAppenderPtr console(new dcmtk::log4cplus::ConsoleAppender(
				OFTrue, OFTrue));
		dcmtk::log4cplus::Logger rootLogger = dcmtk::log4cplus::Logger::getRoot();
		console->setLayout(OFmove(layout));
		rootLogger.addAppender(console);
		rootLogger.setLogLevel(dcmtk::log4cplus::INFO_LOG_LEVEL);
	}
	else
	{
		// Отключаем логи DCMTK.
		// https://support.dcmtk.org/redmine/projects/dcmtk/wiki/Howto_LogProgram
		dcmtk::log4cplus::SharedAppenderPtr nullapp(new dcmtk::log4cplus::NullAppender());
		dcmtk::log4cplus::Logger log = dcmtk::log4cplus::Logger::getRoot();
		log.removeAllAppenders();
		log.addAppender(nullapp);
	}
}

Logger::Logger()
{
	// Запись лог-файла отключена. Прежде чем включить обратно, нужно решить вопросы:
	// 1. Нужна возможность настраивать вывод информации (разрешить создание файла,
	// вести вывод в stdout/stderr) из main(). Нужно расширить имеющуюся DisableLogs().
	// 2. Лог-файл должен создаваться в папке %TEMP%.
	// 3. Подумать об объединении с системой логов DCMTK.
#if 0
	wstring	filename = WGetApplicationDirectory() + wstring(L"/XRADDicom-DCMTKAccess.log");
	try
	{
		log_file.open_create(filename, text_encoding::utf16_le);
		//			log_file.open(filename.c_str(), L"a+");
	}
	catch (...)
	{
	}
	if (log_file.is_open())
	{
		log_file.printf_("-------------------------------------------------------\n%s\tDCMTK Access logging started\n", current_date_time().c_str());
		log_file.flush();
	}
#endif
}

Logger::~Logger()
{
#if 0
	if (log_file.is_open())
	{
		log_file.printf_("\n%s\tDCMTK Access logging finished\n-------------------------------------------------------\n", current_date_time().c_str());
		log_file.flush();
	}
#endif
}

string Logger::current_date_time()
{
	std::stringstream ss;
	auto curTime = std::time(nullptr);
	auto tm = *std::localtime(&curTime);
	ss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");

	return ss.str();
}


void Logger::putLogMessage(const string &message)
{
#if 0
	if (!log_file.is_open()) return;

	m.lock();

	string msg = current_date_time() + "\t-\t" + message + "\n";
	log_file.printf_(msg.c_str());
	log_file.flush();
	m.unlock();
#endif
}



} //namespace Dicom

XRAD_END