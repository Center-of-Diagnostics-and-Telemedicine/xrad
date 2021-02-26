/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file OutputRedirector.cpp
//--------------------------------------------------------------
#include "pre.h"
#include "OutputRedirector.h"

#ifdef XRAD_USE_GUI_MS_WIN32_VERSION
#if defined(XRAD_COMPILER_MSC) && _MSC_VER >= 1900
#include <io.h>
#endif
#endif // XRAD_USE_GUI_MS_WIN32_VERSION

namespace XRAD_GUI
{

//--------------------------------------------------------------

OutputRedirector::~OutputRedirector()
{
	if (Started)
		Stop();
	if (LogFile) // Mutex не нужен, т.к. поток pipe остановлен.
	{
		fprintf(LogFile, "Log closed.\n");
		fclose(LogFile);
		LogFile = nullptr;
	}
}

//--------------------------------------------------------------

bool OutputRedirector::Start(AsyncTextBuffer *text_buffer)
{
	if (Started)
		throw runtime_error("OutputRedirector::Start: already started.");
#if defined(XRAD_USE_GUI_MS_WIN32_VERSION)
	if (!CreateStdStreamPipe())
		return false;
	StartPipeThread(text_buffer);
	RedirectStdHandle(text_buffer);
	RedirectStdStream();
#endif // XRAD_USE_GUI_MS_WIN32_VERSION
	Started = true;
	return true;
}

//--------------------------------------------------------------

void OutputRedirector::Stop()
{
	if (!Started)
		return;
#if defined(XRAD_USE_GUI_MS_WIN32_VERSION)
	RestoreStdStream();
	RestoreStdHandle();
	StopPipeThread();
	DeleteStdStreamPipe();
#endif // XRAD_USE_GUI_MS_WIN32_VERSION
	Started = false;
}

//--------------------------------------------------------------

void OutputRedirector::SetLogFile(FILE *f)
{
	lock_guard<mutex> lock(LogFileMutex);
	if (LogFile)
	{
		fprintf(LogFile, "Log closed.\n");
		fclose(LogFile);
	}
	LogFile = f;
	if (LogFile)
	{
		fprintf(LogFile, "Log opened.\n");
	}
}

//--------------------------------------------------------------

#if defined(XRAD_USE_GUI_MS_WIN32_VERSION)

//--------------------------------------------------------------

FILE *OutputRedirector::GetRedirectedStream() const
{
	switch (output_stream)
	{
		case OutputStream::StdOut:
			return stdout;
		case OutputStream::StdErr:
			return stderr;
		default:
			throw invalid_argument("OutputRedirector: invalid output_stream.");
	}
}

//--------------------------------------------------------------

DWORD OutputRedirector::GetRedirectedStdHandleNumber() const
{
	switch (output_stream)
	{
		case OutputStream::StdOut:
			return STD_OUTPUT_HANDLE;
		case OutputStream::StdErr:
			return STD_ERROR_HANDLE;
		default:
			throw invalid_argument("OutputRedirector: invalid output_stream.");
	}
}

//--------------------------------------------------------------

bool OutputRedirector::CreateStdStreamPipe()
{
	if (!CreatePipe(&StdPipeReadHandle, &StdPipeWriteHandle, 0, 0))
		return false;
	return true;
}

//--------------------------------------------------------------

bool OutputRedirector::DeleteStdStreamPipe()
{
	CloseHandle(StdPipeReadHandle); // TODO ??? hang if writing thread is terminated
	CloseHandle(StdPipeWriteHandle);
	StdPipeWriteHandle = INVALID_HANDLE_VALUE;
	StdPipeReadHandle = INVALID_HANDLE_VALUE;
	return true;
}

//--------------------------------------------------------------

void OutputRedirector::ReadPipeLoop(AsyncTextBuffer *text_buffer)
{
	for(;;)
	{
		DWORD dwRead = 0;
		char chBuf[1];
		int res = ReadFile(StdPipeReadHandle, chBuf, 1, &dwRead, 0);
		// ReadFile из pipe может прочитать 0 байт, если был вызов WriteFile с нулевой длиной,
		// и это не ошибка.
		if (dwRead)
		{
			{
				lock_guard<mutex> lock(LogFileMutex);
				FILE *log_file = LogFile;
				if (log_file)
				{
					// В лог-файл пишем данные без какой-либо обработки, как есть в бинарном виде.
					fwrite(&chBuf, 1, 1, log_file);
					int c = chBuf[0];
					if (c == '\n' || c == '\r')
						fflush(log_file);
				}
			}
			if (!text_buffer->AppendBufferMT(chBuf[0]))
			{
				// TODO: Сейчас символ выкидывается.
				// Надо ввести condition_variable, через который подождать чтения буфера в течение
				// какого-то времени. Ждать бесконечно долго нельзя, т.к., если в pipe пишет тот же поток,
				// который извлекает данные из буфера, возникнет взаимная блокировка.
				// При закрытии pipe нужно, для вывода потока из режима ожидания, не забыть установить
				// и condition_variable.
				// Если чтения из буфера не дождались, нужно установить некоторый флаг, чтобы
				// в следующий раз сразу выкидывать символ, ничего не ожидая.
				// Флаг сбрасывать при чтении из буфера.
				// TODO: Возможно, логичнее выкидывать символы из начала буфера.
				// Сейчас это можно сделать относительно просто, внутри AsyncTextBuffer,
				// т.к. операции записи и чтения производятся под одним мьютексом.
				// TODO: При выкидывании символа желательно учитывать особенности кодировки,
				// чтобы не возникали недопустимые последовательности байтов, кодирующих символы Юникода.
				// Сделать конвертацию в UTF-32 на стадии добавления символов в буфер.
			}
		}
		if (PipeLoopTerminate)
			break;
		if (!res)
			break;
	}
	text_buffer->ReadyMT();
}

//--------------------------------------------------------------

void OutputRedirector::ProcessPipe(AsyncTextBuffer *text_buffer)
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
	xrad::ThreadSetup ts; (void)ts;
	try
	{
		ReadPipeLoop(text_buffer);
	}
	catch (...)
	{
	}
}

//--------------------------------------------------------------

void OutputRedirector::StartPipeThread(AsyncTextBuffer *text_buffer)
{
	PipeThread = std::thread([this, text_buffer]() { this->ProcessPipe(text_buffer); });
}

//--------------------------------------------------------------

void OutputRedirector::StopPipeThread()
{
	PipeLoopTerminate = true;
	// Пишем сообщение, чтобы поток вышел из ожидания.
	DWORD num_written = 0;
	WriteFile(StdPipeWriteHandle, "\n", 1, &num_written, nullptr);
	PipeThread.join();
}

//--------------------------------------------------------------

void OutputRedirector::RedirectStdHandle(AsyncTextBuffer *text_buffer)
{
	DWORD std_handle_number = GetRedirectedStdHandleNumber();
	SaveStdHandle = GetStdHandle(std_handle_number);
	if (!SetStdHandle(std_handle_number, StdPipeWriteHandle) ||
			GetStdHandle(std_handle_number) != StdPipeWriteHandle)
	{
		// При определенных обстоятельствах SetStdHandle(STD_OUTPUT_HANDLE, value) не меняет
		// значение handle потока вывода, при этом никаких признаков ошибки не возвращается.
		// Ошибка происходит про невыясненным причинам. Ошибка не возникает, если приложение
		// запускается из Visual Studio, из командной строки cmd.exe и, как пишут, из taskbar.
		// Ошибка возникает при запуске из проводника.
		//
		// См.:
		// - Is SetStdHandle(STD_OUTPUT_HANDLE, ..) broken under windows 7 ??
		// https://social.msdn.microsoft.com/Forums/vstudio/en-US/a111b4c6-c491-4586-8fcb-2ad67bfbbae8/is-setstdhandlestdoutputhandle-broken-under-windows-7-?forum=vcgeneral
		// - SetStdHandle behaves strangely in Windows 7
		// https://social.msdn.microsoft.com/Forums/windowsdesktop/en-us/299c9401-c9c0-4425-ab78-6df04340aa84/setstdhandle-behaves-strangely-in-windows-7?forum=windowsgeneraldevelopmentissues
		// - http://support.microsoft.com/kb/105305/en-us (недоступна/404 2019-10-29)
		//
		// В качестве решения предлагается вызывать AllocConsole() перед вызовом
		// GetStdHandle/SetStdHandle, потом это окно прятать (ShowWindow(GetConsoleWindow(), SW_HIDE)).
		// Побочный эффект: появляется консольное окно, потом исчезает.
		// Это решение пока не применяем.
#ifdef XRAD_DEBUG
		for (char c: string("RedirectStdHandle() failed.\n"))
			text_buffer->AppendBufferMT(c);
#endif
		return;
	}
#ifdef XRAD_DEBUG
	for (char c: string("RedirectStdHandle() OK.\n"))
		text_buffer->AppendBufferMT(c);
#endif
}

//--------------------------------------------------------------

void OutputRedirector::RestoreStdHandle()
{
	DWORD std_handle_number = GetRedirectedStdHandleNumber();
	SetStdHandle(std_handle_number, SaveStdHandle);
}

//--------------------------------------------------------------

// Процедура перенаправления стандартных потоков различается в разных версиях MSVC.
#if defined(XRAD_COMPILER_MSC) && _MSC_VER >= 1900

void OutputRedirector::RedirectStdStream()
{
	FILE *redirected_stream = GetRedirectedStream();
	int c_handle = _open_osfhandle((intptr_t)StdPipeWriteHandle, 0);
	if( c_handle == -1)
		return;
	int redirected_fileno = _fileno(redirected_stream);
	if( redirected_fileno == -2)
	{
		freopen("nul", "wt", redirected_stream);
		redirected_fileno = _fileno(redirected_stream);
	}
	else if( redirected_fileno != -1)
	{
		SavedCHandle = _dup(redirected_fileno);
	}
	_dup2(c_handle, redirected_fileno);
	// Отключаем буферизацию (чтобы не было необходимости вызывать fflush после каждой записи).
	setbuf(redirected_stream, nullptr);
}

void OutputRedirector::RestoreStdStream()
{
	if( SavedCHandle == -1)
		return;
	FILE *redirected_stream = GetRedirectedStream();
	int redirected_fileno = _fileno(redirected_stream);
	_dup2(SavedCHandle, redirected_fileno);
	SavedCHandle = -1;
}

#else
#error This old MSC compiler version is not supported.
#endif

//--------------------------------------------------------------

#endif // XRAD_USE_GUI_MS_WIN32_VERSION

//--------------------------------------------------------------

} // namespace XRAD_GUI
