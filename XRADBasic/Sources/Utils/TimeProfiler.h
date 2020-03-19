#ifndef __time_profiler_h
#define __time_profiler_h

#include <XRADBasic/Core.h>
#include "PhysicalUnits.h"

/*------------------------------------------------------------------------------

использование точного таймера или таймера с clock() определяется значением
переменной USE_EXACT_PERFORMANCE_COUNTER в том же ашнике.

принцип работы. перед началом нужного действия вызываем метод Start(),
после его окончания Stop().

возможны три варианта использования

1) данные о последней операции извлекаются методами LastElapsed(), LastFPS()

2) осуществляется накопление по всем учтенным операциям. это методы
MeanElapsed(), MeanFPS(). это может быть полезно, если исследуемая операция
совсем короткая (менее 1 мс).

3) осуществляется накопление по последним ~20 операциям. (методы SmoothElapsed()
и SmoothFPS()). по сравнению с вариантом 1 облегчается анализ быстродействия,
так как измеренные величины не так быстро мелькают перед глазами


------------------------------------------------------------------------------*/
XRAD_BEGIN



physical_time GetPerformanceCounter();

class	TimeProfiler
{
	double	accumulation_factor;

	int	count;
	// все внутренние расчеты в миллисекундах
	// все возвращаемые вовне величины в PhysicalUnits
	double	t_start, t_end;
	double	dt;
	double	t_elapsed;
	bool	is_running;
	double	smooth_time;

	double	CurrentTime_ms() const;
public:
	TimeProfiler();
	void	Reset();

	physical_time	Elapsed() const { return msec(t_elapsed); }
	physical_time	LastElapsed() const;
	physical_time	MeanElapsed() const;
	physical_time	SmoothElapsed() const;

	physical_frequency	LastFPS() const;
	physical_frequency	MeanFPS() const;
	physical_frequency	SmoothFPS() const;

	void	Start();
	void	Stop();
	int	Count() const;
};



XRAD_END

#endif // __time_profiler_h
