/*!
	\file
	\date 2015-01-12 20:16
	\author kns
*/
#include "pre.h"
#include "PerformanceCounter.h"

#if !defined(XRAD_USE_PREFER_STD_VERSION) && defined(XRAD_USE_MS_VERSION)
	#include <XRADBasic/Sources/PlatformSpecific/MSVC/Internal/CoreUtils_MS.h>
#endif

XRAD_BEGIN

double GetPerformanceCounterMSec()
{
#if !defined(XRAD_USE_PREFER_STD_VERSION) && defined(XRAD_USE_MS_VERSION)
	// Эта версия (реализована через QueryPerformanceCounter) быстрее версии со steady_clock
	// приблизительно в 2 раза (MSVC 2015, x64, Win10.1909 x64).
	return GetMillisecondCounter_MS();
#else
	using namespace std::chrono;

	// Используем steady_clock, а не high_resolution_clock в связи с тем, что последний может быть
	// немонотонным в некоторых системах. См. примечания:
	// https://en.cppreference.com/w/cpp/chrono/high_resolution_clock

	// Используем оптимизированную версию следующего кода:
	// return EnsureType<double>(duration_cast<duration<double, ratio<1, 1000>>>(
	//   steady_clock::now().time_since_epoch()).count());
	// При использовании double можно заменить последовательность умножения и деления на константы
	// одной операцией умножения на константу.
	// За счет исключения операции деления получаем ускорение примерно на 10%
	// (MSVC 2015, x64, Win10.1909 x64).

	auto t = steady_clock::now().time_since_epoch();
	constexpr double mul = 1e3 * double(decltype(t)::period::num) / double(decltype(t)::period::den);
	return double(t.count()) * mul;
#endif
}

performance_time_t GetPerformanceCounterStd()
{
#if !defined(XRAD_USE_PREFER_STD_VERSION) && defined(XRAD_USE_MS_VERSION)
	// Эта версия (реализована через QueryPerformanceCounter) быстрее версии со steady_clock
	// приблизительно в 2 раза (MSVC 2015, x64, Win10.1909 x64).
	return performance_time_t(0.001*GetMillisecondCounter_MS());
#else
	// Используем steady_clock, а не high_resolution_clock в связи с тем, что последний может быть
	// немонотонным в некоторых системах. См. примечания:
	// https://en.cppreference.com/w/cpp/chrono/high_resolution_clock
	return chrono::steady_clock::now().time_since_epoch();
#endif
}

XRAD_END
