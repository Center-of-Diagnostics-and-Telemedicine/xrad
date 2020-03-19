#ifndef PerformanceCounter_h__
#define PerformanceCounter_h__
/*!
	\file
	\date 2015-01-12 20:17
	\author kns
*/

#include "Config.h"
#include "BasicMacros.h"
#include <chrono>

XRAD_BEGIN

//! \brief Получить значение системного счетчика времени в миллисекундах
double GetPerformanceCounterMSec();

//! \brief Тип данных счетчика времени. Время в секундах
using performance_time_t = chrono::duration<double>;

//! \brief Получить значение системного счетчика времени
performance_time_t GetPerformanceCounterStd();

XRAD_END

#endif // PerformanceCounter_h__
