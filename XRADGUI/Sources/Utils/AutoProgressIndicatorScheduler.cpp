/*!
	\file
	\date 2019/02/07
	\author kulberg
*/
#include "pre.h"
#include "AutoProgressIndicatorScheduler.h"
#include <XRADGUI/Sources/GUI/XRADGUI.h>

XRAD_BEGIN



AutoProgressIndicatorScheduler::AutoProgressIndicatorScheduler(const wstring &task_name, const vector<ProgressOperation> &ops, size_t n_displayed_steps) :
	parent(ops, n_displayed_steps),
	m_registered(ops.size(), false),
	m_profilers(ops.size()),
	m_process_name(task_name),
	m_updated_values(ops.size(), 0),
	auto_tune_speed(0.5)
{
	try
	{
		load_saved_costs();
	}
	catch (...) {}
}

AutoProgressIndicatorScheduler::~AutoProgressIndicatorScheduler()
{
	try
	{
		save_registered_costs();
	}
	catch (...) {}
}

void AutoProgressIndicatorScheduler::load_saved_costs()
{
	vector<double>	new_costs(size(), 0);
	vector<bool>	loaded(size(), false);
	double	loaded_costs_new_sum(0);
	double	loaded_costs_old_sum(0);

	// сумма загруженных порций должна быть равна сумме прежних порций на этих позициях
	for (size_t i = 0; i < size(); ++i)
	{
		double	v = GetSavedParameter(ProcessName(), m_operations[i].name, m_operations[i].cost);
		loaded_costs_old_sum += m_operations[i].cost;
		if (is_number(v))
		{
			loaded_costs_old_sum += m_operations[i].cost;
			loaded_costs_new_sum += v;
			new_costs[i] = v;
			loaded[i] = true;
		}
	}

	double	factor = loaded_costs_old_sum ? loaded_costs_old_sum / loaded_costs_new_sum : 1;

	for (size_t i = 0; i < size(); ++i)
	{
		if (loaded[i])
		{
			m_operations[i].cost = new_costs[i] * factor;
		}
	}

	ComputeBoundaries();
}



void AutoProgressIndicatorScheduler::register_cost(size_t i, physical_time c)
{
	m_updated_values[i] = c.msec();
	m_registered[i] = true;
}



void AutoProgressIndicatorScheduler::save_registered_costs()
{
	double	old_updated_costs(0);
	double	new_updated_costs(0);
	double	old_total_costs(0);

	//	в начале сумма всех з

	for (size_t i = 0; i < size(); ++i)
	{
		reset_not_number(m_operations[i].cost);

		if (m_registered[i])
		{
			reset_not_number(m_updated_values[i]);

			old_updated_costs += m_operations[i].cost;
			new_updated_costs += m_updated_values[i];
		}
		old_total_costs += m_operations[i].cost;
	}

	double	factor;						// Поправка, сохраняющая удельный вес обновленных операций в общем количестве операций
	if (!old_total_costs) factor = 1;	// Если суммарная цена операций неизвестна, определяем заново на основании обновленных значений
	else if (!old_updated_costs && new_updated_costs) factor = 0.01*old_total_costs / new_updated_costs;
	// Если до обновления цена обновляемых операций была равна 0, а теперь выросла,
	// делаем ее очень маленькой ненулевой, чтобы сохранить пропорцию между ними
	else if (!new_updated_costs) factor = 0;
	// Если после обновления цена операций обнулилась, принимаем это
	else factor = old_updated_costs / new_updated_costs;
	// Еначе сохраняем суммарную долю цены обновляемых операций в общем множестве операций

	for (size_t i = 0; i < size(); ++i)
	{
		// для обновленных действий меняем их вес, необновленные остаются.
		double	new_value = m_registered[i] ?
			m_updated_values[i] * factor :
			m_operations[i].cost;

		m_operations[i].cost = mix(m_operations[i].cost, new_value, auto_tune_speed);

	}

	//	нормируем результат так, чтобы сумма всех сохраняемых цен была равна n_displayed_steps
	double	accumulator(0);
	for (size_t i = 0; i < size(); ++i)
	{
		accumulator += m_operations[i].cost;
	}


	for (size_t i = 0; i < size(); ++i)
	{
		double	new_value = double(m_n_displayed_steps)*m_operations[i].cost / accumulator;

		if (is_number(new_value))
		{
			SaveParameter(ProcessName(), m_operations[i].name, new_value);
		}
	}
}

void AutoProgressIndicatorScheduler::begin_operation_profiling(size_t n)
{
	if (m_profilers[n]) throw logic_error("AutoProgressIndicatorScheduler::profile_begin, profiler initialized repeatedely");
	m_profilers[n].reset(new TimeProfiler);
	m_profilers[n]->Start();
}

void AutoProgressIndicatorScheduler::end_operation_profiling(size_t n)
{
	if (!m_profilers[n]) throw logic_error("AutoProgressIndicatorScheduler::profile_begin, profiler not initialized");
	m_profilers[n]->Stop();
	register_cost(n, m_profilers[n]->LastElapsed());
	m_profilers[n].reset();
}

XRAD_END
