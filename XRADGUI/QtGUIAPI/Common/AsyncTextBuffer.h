/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file AsyncTextBuffer.h
//--------------------------------------------------------------
#ifndef XRAD__File_AsyncTextBuffer_h
#define XRAD__File_AsyncTextBuffer_h
//--------------------------------------------------------------

#include <functional>
#include <vector>
#include <mutex>

namespace XRAD_GUI
{

using namespace std;

//--------------------------------------------------------------

class AsyncTextBuffer
{
	public:
		/*!
			\param buffer_ready_mt Потокобезопасная функция обратного вызова, будет вызываться
			при появлении в буфере достаточного количества данных для вывода
		*/
		AsyncTextBuffer(function<void ()> buffer_ready_mt,
				size_t buffer_size = 16384);
	public:
		//! \brief Потокобезопасное извлечение данных из буфера
		string ReadBufferMT();
		//! \brief Потокобезопасное добавление данных в буфер
		bool AppendBufferMT(char c);
		//! \brief Потокобезопасная функция принудительной выдачи запроса на чтение буфера
		void ReadyMT();
	private:
		size_t buffer_size;
		vector<char> buffer;
		volatile char *buffer_data;
		volatile size_t read_pos;
		volatile size_t write_pos;
		mutex buffer_mutex;
		function<void ()> buffer_ready_mt;
};

//--------------------------------------------------------------

} // namespace XRAD_GUI

//--------------------------------------------------------------
#endif // XRAD__File_AsyncTextBuffer_h
