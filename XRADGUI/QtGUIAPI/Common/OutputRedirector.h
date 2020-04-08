// file OutputRedirector.h
//--------------------------------------------------------------
#ifndef __OutputRedirector_h
#define __OutputRedirector_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>

#include "AsyncTextBuffer.h"
#include <cstdio>

namespace XRAD_GUI
{

using namespace std;

//--------------------------------------------------------------

class OutputRedirector
{
	public:
		enum class OutputStream { StdOut, StdErr };

		OutputRedirector(OutputStream output_stream): output_stream(output_stream) {}
		~OutputRedirector();

		bool Start(AsyncTextBuffer *text_buffer);
		void Stop();

		/*!
			\brief Установить файл для вывода, берет указатель во владение (делает ему fclose()).

			\param f Указатель на файл. Если не NULL, использует его для вывода данных. В конце вызывает
				fclose(f). Если NULL, закрывает текущий файл.
		*/
		void SetLogFile(FILE *f);
		FILE *GetLogFile() const { return LogFile; }
	private:
		using HANDLE = ::HANDLE; // Решаем неоднозначность HANDLE из WinAPI и Qt.

		FILE *GetRedirectedStream() const;
		DWORD GetRedirectedStdHandleNumber() const;

		bool CreateStdStreamPipe();
		bool DeleteStdStreamPipe();

		//! \brief Выполняется в потоке обработки pipe
		void ReadPipeLoop(AsyncTextBuffer *text_buffer);
		//! \brief Выполняется в потоке обработки pipe
		void ProcessPipe(AsyncTextBuffer *text_buffer);

		void StartPipeThread(AsyncTextBuffer *text_buffer);
		void StopPipeThread();

		void RedirectStdHandle(AsyncTextBuffer *text_buffer);
		void RestoreStdHandle();

		void RedirectStdStream();
		void RestoreStdStream();
	private:
		const OutputStream output_stream;
		HANDLE SaveStdHandle = INVALID_HANDLE_VALUE;
		HANDLE StdPipeWriteHandle = INVALID_HANDLE_VALUE;
		HANDLE StdPipeReadHandle = INVALID_HANDLE_VALUE;
		volatile bool PipeLoopTerminate = false;
		std::thread PipeThread;
		int SavedCHandle = -1;
		bool Started = false;
		FILE * volatile LogFile = nullptr;
		mutex LogFileMutex;
};

//--------------------------------------------------------------

} // namespace XRAD_GUI

//--------------------------------------------------------------
#endif // __OutputRedirector_h
