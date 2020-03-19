#ifndef ProgressIndicatorScheduler_h__
#define ProgressIndicatorScheduler_h__

/*!
	\file
	\date 2019/02/05 17:33
	\author kulberg

	\brief
*/

#include <XRADBasic/Core.h>
#include <string>
#include <vector>
#include <utility>

XRAD_BEGIN

//! \brief Cтруктура описывающая этапы обработки (их сложность, название, факт использования в конкретном слуыае)
struct ProgressOperation
{
	double	cost;
	wstring	name;
	bool	used;
	ProgressOperation(wstring n = L"", double c = 1, bool u = true) : name(n), cost(c), used(u){}
};

/*!
	\brief Класс, помогающий задать вычислительную сложность вложенных операций для индикатора прогресса
	\details Пусть некоторое действие состоит из трех поддействий длительностью 1, 10, 5 секунд. Каждое действие
	внутри себя содержит вызов индикатора прогресса от ProgressProxy. При этом одна из операций является необязательной
	Тогда наиболее равномерный индикатор прогресса может быть получен следующим образом:

	\code
		bool	op1_needed = determine_if_it_is_needed();
		ProgressIndicatorScheduler	cc(
		{
			ProgressOperation(1, "op0", true),
			ProgressOperation(10, "op1", op1_needed),
			ProgressOperation(5, "op2", true),
		});
		progress.start("Total progress", cc.n_steps());
		operation0.Perform(cc.operation_boundaries(0));
		if(op1_needed) operation1.Perform(cc.operation_boundaries(1));
		operation2.Perform(cc.operation_boundaries(2));
	\endcode

	Сумма "сложностей" подпрогрессов, задаваемая при создании класса, не обязательно должна быть целым числом.
*/
class ProgressIndicatorScheduler
{
	vector<double>	m_operations_boundaries;

protected:

	vector<ProgressOperation>	m_operations;

	//! \brief Количество шагов, отображаемое на индикаторе прогресса
	const size_t	m_n_displayed_steps;

protected:
	void	ComputeBoundaries();

public:

	//! Количество подпрогрессов
	size_t	size() const { return m_operations.size(); }

	//! \brief Определяет, используется ли операция с номером n
	void	SetUsage(size_t n, bool u);

	//! \brief Инициализация
	//! \param ops Массив, структур описывающий этапы обработки (их сложность, название, использование в конкретном слуыае)
	ProgressIndicatorScheduler(const vector<ProgressOperation> &ops, size_t n_displayed_steps = 100);
	ProgressIndicatorScheduler(const vector<double> &costs, size_t n_displayed_steps = 100);

	double	operation_start(size_t i) const { return  m_operations_boundaries[i]; }
	double	operation_end(size_t i) const { return  m_operations_boundaries[i + 1]; }


	//!	Границы шага (могут быть переданы в качестве аргумента progress::subprogress())
	pair<double, double> operation_boundaries(size_t i) const
	{
		return make_pair(m_operations_boundaries[i], m_operations_boundaries[i+1]);
	}

	//! Число шагов индикатора прогресса. Передается в качестве аргумента progress:start()
	double	n_steps() const { return m_operations_boundaries.back(); }

	//! Сложность шага с номером n
	double	operation_cost(size_t n) const { return m_operations[n].cost; }

	//!	Изменение сложности конкретного шага
	void	set_operation_cost(size_t n, double new_c);

	//! \brief Выполнить одну операцию
	//! \param p Основной индикатор прогресса
	//! \param f лямбда-выражение, единственным аргументом которого должен быть progress_proxy, передаваемый дальнейшему вычислителю
	template<class function>
	void	run_operation(size_t operation_no, RandomProgressBar &p, function f)
	{
		if (m_operations[operation_no].used)
		{
			auto	boundaries = operation_boundaries(operation_no);
			f(p.subprogress(boundaries));
			p.set_position(operation_end(operation_no));
		}
	}


};



XRAD_END

#endif // ProgressIndicatorScheduler_h__
