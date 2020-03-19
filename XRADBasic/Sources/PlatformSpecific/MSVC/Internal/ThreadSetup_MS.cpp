// file ThreadSetup_MS.cpp
//--------------------------------------------------------------
#include "pre.h"
#include "ThreadSetup_MS.h"

#ifdef XRAD_USE_MS_VERSION

#ifndef _MSC_VER
	#error "This file is for MS Visual Studio only"
#endif

#include <windows.h>

XRAD_BEGIN

#ifdef XRAD_MSVC_EHA

//--------------------------------------------------------------

namespace
{

//--------------------------------------------------------------

thread_local bool thread_setup_done = false;

//--------------------------------------------------------------

void se_translator(unsigned int e_code, _EXCEPTION_POINTERS *e_ptr)
{
	string exception_name;
	string details;
	switch (e_code)
	{
		#define se_case(x) case x: exception_name = #x; break;
		//se_case(EXCEPTION_ACCESS_VIOLATION)
		case EXCEPTION_ACCESS_VIOLATION:
			exception_name = "EXCEPTION_ACCESS_VIOLATION";
			if (e_ptr && e_ptr->ExceptionRecord->NumberParameters >= 2)
			{
				const char *op_name = "unknown";
				switch (e_ptr->ExceptionRecord->ExceptionInformation[0])
				{
					case 0:
						op_name = "read";
						break;
					case 1:
						op_name = "write";
						break;
					case 8:
						op_name = "execute";
						break;
				}
				void *addr = reinterpret_cast<void*>(e_ptr->ExceptionRecord->ExceptionInformation[1]);
				details = ssprintf("Operation=%s, Address=%p.",
						op_name,
						addr);
			}
			break;
		se_case(EXCEPTION_ARRAY_BOUNDS_EXCEEDED)
		se_case(EXCEPTION_BREAKPOINT)
		se_case(EXCEPTION_DATATYPE_MISALIGNMENT)
		se_case(EXCEPTION_FLT_DENORMAL_OPERAND)
		se_case(EXCEPTION_FLT_DIVIDE_BY_ZERO)
		se_case(EXCEPTION_FLT_INEXACT_RESULT)
		se_case(EXCEPTION_FLT_INVALID_OPERATION)
		se_case(EXCEPTION_FLT_OVERFLOW)
		se_case(EXCEPTION_FLT_STACK_CHECK)
		se_case(EXCEPTION_FLT_UNDERFLOW)
		se_case(EXCEPTION_GUARD_PAGE)
		se_case(EXCEPTION_ILLEGAL_INSTRUCTION)
		se_case(EXCEPTION_IN_PAGE_ERROR)
		se_case(EXCEPTION_INT_DIVIDE_BY_ZERO)
		se_case(EXCEPTION_INT_OVERFLOW)
		se_case(EXCEPTION_INVALID_DISPOSITION)
		se_case(EXCEPTION_INVALID_HANDLE)
		se_case(EXCEPTION_NONCONTINUABLE_EXCEPTION)
		se_case(EXCEPTION_PRIV_INSTRUCTION)
		se_case(EXCEPTION_SINGLE_STEP)
		se_case(EXCEPTION_STACK_OVERFLOW)
		se_case(STATUS_UNWIND_CONSOLIDATE)
		#undef se_case
		default:
			exception_name = ssprintf("0x%08X",
					EnsureType<unsigned int>(e_code));
			break;
	}
	throw SystemException(ssprintf("System exception %s.%s%s",
			EnsureType<const char*>(exception_name.c_str()),
			details.empty()? "": " ",
			EnsureType<const char*>(details.c_str())));
}

//--------------------------------------------------------------

} // namespace

//--------------------------------------------------------------

ThreadSetup::ThreadSetup()
{
	if (thread_setup_done)
		return;
	old_se_translator = _set_se_translator(se_translator);
#if XRAD_FP_EXCEPTIONS != 0
	old_cw = _controlfp(0, 0);
	// Бросаем исключение: _EM_INVALID|_EM_DENORMAL|_EM_ZERODIVIDE|_EM_OVERFLOW
	// Игнорируем:_EM_UNDERFLOW|_EM_INEXACT
	_controlfp(~(unsigned int)(_EM_INVALID|_EM_DENORMAL|_EM_ZERODIVIDE|_EM_OVERFLOW), _MCW_EM);
#endif
	initialized = true;
	thread_setup_done = true;
}

//--------------------------------------------------------------

ThreadSetup::~ThreadSetup()
{
	if (!initialized)
		return;
#if XRAD_FP_EXCEPTIONS != 0
	_controlfp(old_cw, _MCW_EM);
#endif
	_set_se_translator(old_se_translator);
	thread_setup_done = false;
}

//--------------------------------------------------------------

#endif // XRAD_MSVC_EHA

//--------------------------------------------------------------

namespace internal_ThreadSetup
{

int	option_XRAD_MSVC_EHA()
{
	return 0;
}

int	option_XRAD_FP_EXCEPTIONS()
{
	return 0;
}

} // namespace internal_ThreadSetup

//--------------------------------------------------------------

XRAD_END

#else // XRAD_USE_MS_VERSION

#include <XRADBasic/Core.h>
XRAD_BEGIN
// To avoid  warning LNK4221: This object file does not define any previously undefined public
// symbols, so it will not be used by any link operation that consumes this library.
// (MSVC 2015)
void xrad__dummy_ThreadSetup_MS() {}
XRAD_END

#endif // XRAD_USE_MS_VERSION
