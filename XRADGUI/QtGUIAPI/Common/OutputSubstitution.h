#ifndef OutputSubstitution_h__
#define OutputSubstitution_h__
/*!
	\file
	\date 2015-01-01 18:05
	\author kns
*/
//--------------------------------------------------------------

#include "AsyncTextBuffer.h"

namespace XRAD_GUI
{
XRAD_USING

//--------------------------------------------------------------

class ConsoleRedirector: public QObject
{
	Q_OBJECT

	private:
		bool eventFilter(QObject *target,QEvent *event);

	public:
		ConsoleRedirector(
				function<void (QString)> process_stdout,
				function<void (QString)> process_stderr);
		~ConsoleRedirector();

		void LogToFiles(bool flag);

		void DoStdoutReady();
		void DoStderrReady();

	signals:
		void signal_stdout_ready();
		void signal_stderr_ready();

	private:
		struct Internal;
		unique_ptr<Internal> internal;
		QTimer timer;
		bool log_to_files = false;
};

//--------------------------------------------------------------

}//namespace XRAD_GUI

#endif //OutputSubstitution_h__
