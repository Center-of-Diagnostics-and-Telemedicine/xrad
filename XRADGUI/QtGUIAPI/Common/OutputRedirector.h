// file OutputRedirector.h
//--------------------------------------------------------------
#ifndef XRAD__File_OutputRedirector_h
#define XRAD__File_OutputRedirector_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>

#include "AsyncTextBuffer.h"
#include <XRADGUI/Sources/Core/GUIConfig.h>
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
#if defined(XRAD_USE_GUI_MS_WIN32_VERSION)
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
#endif // XRAD_USE_GUI_MS_WIN32_VERSION
	private:
		const OutputStream output_stream;
#if defined(XRAD_USE_GUI_MS_WIN32_VERSION)
		HANDLE SaveStdHandle = INVALID_HANDLE_VALUE;
		HANDLE StdPipeWriteHandle = INVALID_HANDLE_VALUE;
		HANDLE StdPipeReadHandle = INVALID_HANDLE_VALUE;
		volatile bool PipeLoopTerminate = false;
		std::thread PipeThread;
		int SavedCHandle = -1;
#endif // XRAD_USE_GUI_MS_WIN32_VERSION
		bool Started = false;
		FILE * volatile LogFile = nullptr;
		mutex LogFileMutex;
};

//--------------------------------------------------------------

} // namespace XRAD_GUI

//--------------------------------------------------------------
#endif // XRAD__File_OutputRedirector_h
