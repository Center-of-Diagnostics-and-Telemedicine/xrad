#ifndef logger_h__
#define logger_h__
/*!
* \file
* \date 1/12/2018 12:12:47 PM
*
* \author kovbas
*/

#include <XRADSystem/TextFile.h>
#include <mutex>

XRAD_BEGIN

namespace Dicom
{

//! \brief Настроить вывод отладочной информации (в т.ч. DCMTK)
void EnableLogs(bool enable);

class	Logger
{
	public:
		Logger();
		~Logger();
	public:
		void putLogMessage(const string &message);
	private:
		static string current_date_time();
	private:
		text_file_writer log_file;
		std::mutex	m;
};



} //namespace Dicom



XRAD_END

#endif // logger_h__