// file ThreadSetup_MS.h
//--------------------------------------------------------------
#ifndef __ThreadSetup_MS_h
#define __ThreadSetup_MS_h
/*!
	\file
	В этом файле используются макросы:
	\verbatim
	XRAD_MSVC_EHA
	XRAD_FP_EXCEPTIONS
	\endverbatim

	Макрос `XRAD_MSVC_EHA` задается в property sheet `XRAD-EHA.props`.
	Задавать его вручную не рекомендуется.

	Макрос `XRAD_FP_EXCEPTIONS = 0 | 1` задается автоматически в зависимости от _DEBUG.
*/
//--------------------------------------------------------------

#include <XRADBasic/Sources/Core/Config.h>
#include <XRADBasic/Sources/Core/BasicMacros.h>

#ifdef XRAD_USE_MS_VERSION

#ifndef _MSC_VER
	#error "This file is for MS Visual Studio only"
#endif

#include <stdexcept>

XRAD_BEGIN

//--------------------------------------------------------------

class SystemException: public std::runtime_error
{
	public:
		SystemException(const std::string &message): std::runtime_error(message) {}
};

//--------------------------------------------------------------

#ifdef XRAD_MSVC_EHA

#ifndef XRAD_FP_EXCEPTIONS
#ifdef _DEBUG
	#define XRAD_FP_EXCEPTIONS 1
#else
	#define XRAD_FP_EXCEPTIONS 0
#endif
#endif

#if XRAD_FP_EXCEPTIONS != 0
#pragma float_control(precise, on)
#pragma fenv_access(on)
#pragma float_control(except, on)
#endif

#endif // XRAD_MSVC_EHA

//--------------------------------------------------------------

class ThreadSetup
{
	public:
		ThreadSetup(const ThreadSetup&) = delete;
		ThreadSetup &operator=(const ThreadSetup&) = delete;
#ifdef XRAD_MSVC_EHA
	public:
		ThreadSetup();
		~ThreadSetup();
		// Можно реализовать move-операции.
	private:
		bool initialized = false;
		_se_translator_function old_se_translator = nullptr;
#if XRAD_FP_EXCEPTIONS != 0
		unsigned int old_cw = 0;
#endif
#else
	public:
		ThreadSetup() = default;
#endif
};

//--------------------------------------------------------------

namespace internal_ThreadSetup
{

#ifdef XRAD_MSVC_EHA
#define option_XRAD_MSVC_EHA option_XRAD_MSVC_EHA_True
#else
#define option_XRAD_MSVC_EHA option_XRAD_MSVC_EHA_False
#endif

int	option_XRAD_MSVC_EHA();

#if XRAD_FP_EXCEPTIONS != 0
#define option_XRAD_FP_EXCEPTIONS option_XRAD_FP_EXCEPTIONS_True
#else
#define option_XRAD_FP_EXCEPTIONS option_XRAD_FP_EXCEPTIONS_False
#endif

int	option_XRAD_FP_EXCEPTIONS();

//! \brief См. комментарий к internal_ContainerChecksControl
inline int InitCompilerOptions()
{
	return option_XRAD_MSVC_EHA() + option_XRAD_FP_EXCEPTIONS();
}

} // namespace internal_ThreadSetup

namespace
{
	//! \brief Назначение этого вызова: см. комментарий к internal_ContainerChecksControl::InitFlowControl()
	//!
	//! Объявляется в неименованном namespace, чтобы не было конфликтов при линковке
	int	internal_ThreadSetup_InitCompilerOptions = internal_ThreadSetup::InitCompilerOptions();
}

//--------------------------------------------------------------

XRAD_END

#endif // XRAD_USE_MS_VERSION

//--------------------------------------------------------------
#endif // __ThreadSetup_MS_h
