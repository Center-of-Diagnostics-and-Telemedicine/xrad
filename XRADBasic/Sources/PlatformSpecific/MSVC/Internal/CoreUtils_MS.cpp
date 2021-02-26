/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file CoreUtils_MS.cpp
//--------------------------------------------------------------
#include "pre.h"
#include "CoreUtils_MS.h"

#ifdef XRAD_USE_MS_VERSION

#ifndef _MSC_VER
	#error "This file is for MS Visual Studio only"
#endif

#include <windows.h>
#include <stdexcept>

XRAD_BEGIN

//--------------------------------------------------------------

namespace
{

struct PerformanceFrequency
{
	public:
		LARGE_INTEGER	freq;
		BOOL status;
	public:
		PerformanceFrequency()
		{
			// Частота фиксируется в момент загрузки системы и одинакова для всех процессоров.
			// Достаточно получить её один раз.
			status = QueryPerformanceFrequency(&freq);
		}
};

PerformanceFrequency pf;

} // namespace

//--------------------------------------------------------------

double	GetMillisecondCounter_MS()
{
	if(!pf.status)
		throw std::runtime_error("Query performance counter does not work.");

	LARGE_INTEGER	pc;
	QueryPerformanceCounter(&pc);
	return	0.5 + 1000.*double(pc.QuadPart)/double(pf.freq.QuadPart);
}

//--------------------------------------------------------------

XRAD_END

#else // XRAD_USE_MS_VERSION

#include <XRADBasic/Core.h>
XRAD_BEGIN
// To avoid  warning LNK4221: This object file does not define any previously undefined public
// symbols, so it will not be used by any link operation that consumes this library.
// (MSVC 2015)
void xrad__dummy_CoreUtils_MS() {}
XRAD_END

#endif // XRAD_USE_MS_VERSION
