#ifndef AutoProgressIndicatorScheduler_h__
#define AutoProgressIndicatorScheduler_h__

/*!
\file
\date 2019/02/07
\author kulberg

\brief
*/

#include <XRADBasic/Sources/Utils/ProgressIndicatorScheduler.h>
#include <XRADBasic/Sources/Utils/TimeProfiler.h>

XRAD_BEGIN


/*!
\brief Класс, позволяющий менять отображаемую вычислительную сложность операций в зависимости от
времени, действительно затраченного на каждую операцию.


\code
bool	filter_rasp = determine_if_it_is_needed();

AutoProgressIndicatorScheduler 	scheduler(
L"StudyAnonymizer::Apply",
{
ProgressOperation(L"load", 100, true),
ProgressOperation(L"rasp", 500, filter_rasp),
ProgressOperation(L"edit", 1, true),
ProgressOperation(L"anonym", 1, true),
ProgressOperation(L"save", 150, true)
}
);

scheduler.LoadSavedCosts();

ProgressBar	progress(pproxy);

progress.start("Processing item", scheduler.n_steps());
scheduler.UpdateCost(0, loader.Apply(study, progress.subprogress(scheduler.operation_boundaries(0))).msec());

if(filter_rasp)
{
scheduler.UpdateCost(1, RASP.Apply(study, progress.subprogress(scheduler.operation_boundaries(1))).msec());
}

scheduler.UpdateCost(2, editor.Apply(study, progress.subprogress(scheduler.operation_boundaries(2))).msec());
scheduler.UpdateCost(3, anonymizer.Apply(study, progress.subprogress(scheduler.operation_boundaries(3))).msec());
scheduler.UpdateCost(4, saver.Apply(study, progress.subprogress(scheduler.operation_boundaries(4))).msec());

scheduler.SaveUpdatedCosts();

}
\endcode

*/

class AutoProgressIndicatorScheduler : public ProgressIndicatorScheduler
{
	PARENT(ProgressIndicatorScheduler);

	vector<bool>	m_registered;
	vector<double>	m_updated_values;
	wstring	m_process_name;
	wstring	ProcessName() const { return L"AutoProgressIndicatorScheduler/" + m_process_name; }
	//!	\brief Параметр, определяющий быстроту автоподстройки прогресса. Может принимать значения от 0 до 1
	//	0 означает, что автоподстройки нет совсем, 1 -- то, что параметры последнего вызова используются
	//	сразу без накопления.
	double	auto_tune_speed;

	vector<unique_ptr<TimeProfiler>>	m_profilers;

	//!	\brief Загружает ранее сохраненные относительные сложности операций
	void	load_saved_costs();
	//!	\brief Запоминает обновленные операции. Пропорции для не обновлявшихся действий сохраняются неизменными
	void	save_registered_costs();

public:
	AutoProgressIndicatorScheduler(const wstring &task_name, const vector<ProgressOperation> &ops, size_t n_displayed_steps = 100);
	~AutoProgressIndicatorScheduler();

	void	SetAutoTuneSpeed(double s) { auto_tune_speed = s; }

	//!	\brief Задает новую относительную сложность операции номер n.
	//! \param cost Экспериментально определенная сложность операции номер n
	void	register_cost(size_t n, physical_time cost);


	//! \brief Действия, между которыми следует вызвать операцию номер n. Время выполнения регистрируется автоматически
	void	begin_operation_profiling(size_t n);
	void	end_operation_profiling(size_t n);

	//! \brief Выполнить одну операцию и зарегистрировать ее стоимость
	//! \param p Основной индикатор прогресса
	//! \param f лямбда-выражение, единственным аргументом которого должен быть progress_proxy, передаваемый дальнейшему вычислителю
	template<class function>
	void	run_and_register_cost(size_t operation_no, RandomProgressBar &p, function f)
	{
		if (m_operations[operation_no].used)
		{
			begin_operation_profiling(operation_no);
			auto	boundaries = operation_boundaries(operation_no);
			f(p.subprogress(boundaries));
			end_operation_profiling(operation_no);
			p.set_position(operation_end(operation_no));
		}
	}
};


XRAD_END

#endif //  AutoProgressIndicatorScheduler_h__
