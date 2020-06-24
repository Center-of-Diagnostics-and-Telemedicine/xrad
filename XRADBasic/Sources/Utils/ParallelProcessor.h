#ifndef ParallelProcessor_h__
#define ParallelProcessor_h__

/*!
	\file
	\date 2018/12/28 12:05
	\author kulberg
	\brief  Класс, обеспечивающий разбиение больших циклов на порции для многопроцессорной обработки с индикатором прогресса


*/

#include <XRADBasic/Core.h>
#include <omp.h>
#include <map>
#include <mutex>

XRAD_BEGIN

/*!
	\brief  Класс, обеспечивающий разбиение больших циклов на порции для многопроцессорной обработки с индикатором прогресса

	\details
	Пусть необходимо выполнить поэлементную обработку некоторого массива:

	\code
	ProgressProxy	pproxy;
	ProgressBar	progress(pproxy);
	size_t	large_number;
	vector<some_class>	array_to_process(large_number);
	void	very_hard_operation(some_class &);

	progress.start("processing", array_to_process.size());
	for(size_t i = 0; i < array_to_process.size(); ++i
	{
		very_hard_operation(array_to_process[i]);
		++progress;
	};
	progress.end();
	\endcode

	Код выполняется в однопоточном режиме.
	Использование omp напрямую недопустимо, т.к. приведет к некорректной работе индикатора прогресса
	(++progress будет вызываться из различных потоков, что не предусмотрено классом ProgressBar)

	\code
	progress.start("processing", array_to_process.size());
	#pragma omp parallel for schedule (guided) // Вычисления выполнятся, но индикатор прогресса будет работать неправильно
	for(size_t i = 0; i < array_to_process.size(); ++i
	{
		very_hard_operation(array_to_process[i]);
		++progress;
	};
	progress.end();
	\endcode

	Таким образом, весь код с индикаторами прогресса либо выполняется в один поток, либо требует переписывания
	с разбиением на порции вручную.
	В предлагаемом решении большие циклы автоматически делятся на блоки, каждый из которых может безопасно обрабатываться
	в многопоточном режиме. Индикатор прогресса меняется только из одного потока после того, как работа всех потоков для одного
	блока завершена.

	Пример использования:

	\code
	ParallelProcessor	proc(array_to_process.size());
	auto	one_cycle_action = [proc, option, &array_to_process, very_hard_operation](size_t piece, size_t thread_no)
	{
		size_t	i = proc.absolute_index(piece, thread_no);
		very_hard_operation(array_to_process[i]);
	};

	proc.perform(one_cycle_action, "processing", pproxy);
	\endcode

*/
class ParallelProcessor
{
public:

	//! \brief Режимы работы процессора
	enum mode_t
	{
		//! \brief В конфигурации Release многопоточная обработка включена, в Debug выключена, но сохранено разбиение на порции
		e_auto,
		//! \brief Многопоточная обработка включена всегда
		e_force_parallel,
		//! \brief Многопоточная обработка выключена, но сохранено разбиение на порции
		e_force_plain_portions,
		//! \brief Многопоточная обработка выключена, разбиение на порции выключено (размер части всегда равен 1)
		e_force_plain
	};

	//! \brief Режимы обработки ошибок
	enum error_process_mode_t
	{
		//!	После возникновения ошибки обработка прекращается сразу, как только это возможно
		skip_rest,
		//!	После возникновения ошибки отменяются только одно действие, вызвавшее ошибку, остальные выполняются
		skip_nothing
	};

private:

	//! \brief Общее число шагов обработки (например, размер обрабатываемого массива).
	size_t	m_n_total_steps;

	//! \brief Количество потоков. По умолчанию равно числу логических ядер процессора
	size_t	m_n_threads;

	//! \brief Общее количество действий на один поток
	size_t m_n_total_actions_per_thread;

	//! \brief Количество действий, выполняемых одним потоком за один раз. По умолчанию равно 1
	//! \details Если действие, переданное функции perform, занимает мало процессорного времени,
	//! накладные расходы по созданию потоков могут уничтожить прирост производительности. Поэтому для
	//! "легких" действий следует увеличивать этот параметр. Оптимальное значение определяется только
	//!	экспереминтальным путем (увеличение его, помимо выигрыша скорости, может привести к заметной
	//! "дискретности" движения индикатора прогресса).
	size_t	m_n_actions_per_thread;

	//! \brief Количество шагов, отображаемых на индикаторе прогресса
	size_t	m_n_pieces;

	mode_t m_mode;

	error_process_mode_t	m_error_process_mode = skip_rest;

	//! \brief Вспомогательная величина, позволяет избежать обращений к макро в теле функции
	#if defined(XRAD_DEBUG)
		const	bool	debug = true;
	#else
		const	bool	debug = false;
	#endif

	size_t	absolute_index(size_t piece_no, size_t thread_no, size_t subthread_no) const
	{
		//	изменения в этой функции должны быть симметрично учтены в функции thread_no
		// Этот вариант обеспечивает более равномерную загрузку потоков:
		return (piece_no*m_n_actions_per_thread + subthread_no)*m_n_threads + thread_no;
		// Такой вариант использовался ранее (ср. комментарий в thread_no):
		//return (piece_no*m_n_threads + thread_no)*m_n_actions_per_thread + subthread_no;
	}

	void	select_indices_util(size_t in_n_actions_per_thread, size_t in_n_threads)
	{
		if (m_n_total_steps <= in_n_threads)
		{
			m_n_threads = m_n_total_steps;
			m_n_actions_per_thread = 1;
			m_n_pieces = 1;
			return;
		}
		m_n_threads = in_n_threads;
		m_n_total_actions_per_thread = (m_n_total_steps + m_n_threads - 1) / m_n_threads;
		if (!in_n_actions_per_thread)
			in_n_actions_per_thread = 1;
		if (in_n_actions_per_thread >= m_n_total_actions_per_thread)
		{
			m_n_actions_per_thread = m_n_total_actions_per_thread;
			m_n_pieces = 1;
			return;
		}
		m_n_actions_per_thread = in_n_actions_per_thread;
		m_n_pieces = (m_n_total_actions_per_thread + m_n_actions_per_thread - 1) /
				m_n_actions_per_thread;
	}

	//! \brief Список ошибок обработки. Хранит информацию о линейном номере шага
	//! и сообщение исключения
	//!
	//!	После вызова perform() в случае успешной обработки errors.HasErrors()==false.
	using error_list_t = ThreadErrorCollectorEx<size_t>;

	template<class T>
	void	perform_one_action(T &one_step_action, size_t i, error_list_t &err) const
	{
		if(i < m_n_total_steps)
		{
			// если нужно попытаться выполнить все шаги обработки, проверка исключений на каждом элементарном шаге
			if(m_error_process_mode==skip_nothing)
			{
				try
				{
					one_step_action(i);
				}
				catch(...)
				{
					err.CatchException(i);
				}
			}
			// Иначе только один раз за время существования потока
			else
			{
				one_step_action(i);
			}
		}
	}


	template<class T>
	void	perform_one_thread(T &one_step_action, size_t piece, size_t thread_no, error_list_t &err) const
	{
		size_t	i(0);
		try
		{
			for (size_t subthread_no = 0; subthread_no < m_n_actions_per_thread; ++subthread_no)
			{
				// В случае skip_nothing исключение ловится в perform_one_action, поэтому здесь нужно
				// проверять признак err.HasSpecialErrors().
				// В случае не skip_nothing цикл будет прерван исключением.
				if(m_error_process_mode==skip_nothing && err.HasSpecialErrors())
					break;
				i = absolute_index(piece, thread_no, subthread_no);
				perform_one_action(one_step_action, i, err);
			}
		}
		catch (...)
		{
			err.CatchException(i);
		}
	}

	bool need_break(const error_list_t &err) const
	{
		if(m_error_process_mode==skip_nothing)
			return err.HasSpecialErrors();
		return err.HasErrors();
	}

public:
	//! \name Инициализация
	//! @{

	//!	\param in_n_stps Обязательный параметр, общее число шагов, которые необходимо выполнить
	//! \param in_n_actions_per_thread Необязательный параметр. Позволяет вручную установить нагрузку на один поток
	//!	\param in_mode Необязательный параметр. Позволяет принудительно включить или выключить многопоточную обработку
	//!	\param in_n_threads Необязательный параметр. Позволяет принудительно установить число потоков
	void	init(size_t in_n_steps, mode_t in_mode = e_auto, size_t in_n_actions_per_thread = 0, size_t in_n_threads = omp_get_max_threads())
	{
		//TODO добавить проверку корректности аргументов, бросать исключение. Специально ради этого запретил инициализирующий конструктор

		// Возможна инициализация для пустого набора данных. Обработки все равно не будет,
		// но возникают ошибки в процедуре инициализации многопоточного режима.
		// Чтобы не усложнять инициализацию, просто отключаем многопоточный режим
		m_mode = in_n_steps ? in_mode : e_force_plain;
		m_n_total_steps = in_n_steps;

		if(m_mode==e_force_plain)
		{
			// в этом случае все аргументы, определяющие многопоточную обработку, игнорируются
			m_n_threads = 1;
			m_n_actions_per_thread = 1;
			m_n_pieces = m_n_total_steps;
		}
		else
		{
			if(in_n_actions_per_thread) select_indices_util(in_n_actions_per_thread, in_n_threads);
			else
			do
			{
				// увеличиваем нагрузку на поток, пока число шагов прогресса не опустится до 8 и ниже
				select_indices_util(++in_n_actions_per_thread, in_n_threads);
			} while(m_n_pieces > 8);
		}
	}

	ParallelProcessor() : m_n_total_steps(0), m_n_threads(1), m_n_total_actions_per_thread(1),
			m_n_actions_per_thread(1), m_n_pieces(0), m_mode(e_auto){}
	//ParallelProcessor(size_t in_n_steps, mode_t in_mode = e_auto, size_t in_n_actions_per_thread = 0, size_t in_n_threads = omp_get_num_procs()){ init(in_n_steps, in_mode, in_n_actions_per_thread, in_n_threads); }
	//! @}


	size_t	thread_no(size_t absolute_step_no) const
	{
	//	изменения в этой функции должны быть симметрично учтены в функции absolute_index
		return absolute_step_no % m_n_threads;
	//	Такой вариант использовался ранее (ср. комментарий в absolute_index):
	// 	return (absolute_step_no/m_n_actions_per_thread)%m_n_threads;
	}
	size_t	n_threads() const { return m_n_threads; }
	mode_t	mode() const { return m_mode; }


	void	set_error_process_mode(error_process_mode_t epm){m_error_process_mode = epm;}

	//! \brief Основной цикл обработки
	//!
	//! \param one_cycle_action функция, описывающая один шаг обработки. Единственным параметром является номер шага.
	//!	Обычно это лямбда-функция, захватывающая какой-либо контейнер в области видимости.
	//! \param message Сообщение, отображаемое в окне индикатора прогресса (отображается только для прогрессов нулевого уровня)
	//! \param pproxy Ссылка на индикатор прогресса
	//! \return Возвращает время, затраченное на обработку полного цикла
	template<class T> performance_time_t perform(T &one_step_action, const wstring &message,
			const ProgressProxy &pproxy) const
	{
		map<size_t, wstring> errors;
		auto result = perform(one_step_action, message, pproxy, errors);
		if(!errors.empty() && m_error_process_mode!=skip_nothing)
		{
			// Записываем сообщения об ошибках в порядке возрастания номера шага.
			wstring err_message(message);
			for(auto &e: errors)
			{
				wstring step_name;
				if (e.first != (size_t)-1)
					step_name = ssprintf(L"step %zu", EnsureType<size_t>(e.first));
				else
					step_name = L"[...]";
				err_message += ssprintf(L"\n%ls: %ls",
						EnsureType<const wchar_t*>(step_name.c_str()),
						EnsureType<const wchar_t*>(e.second.c_str()));
			}
			throw runtime_error(convert_to_string(err_message).c_str());
		}
		return result;
	}

	template<class T> performance_time_t perform(T &one_step_action, const wstring &message,
			const ProgressProxy &pproxy,
			map<size_t, wstring> &errors) const
	{
		// Пустой набор данных возможен.
		if(!m_n_pieces)
			return performance_time_t(0);

		error_list_t	err(convert_to_string(message));

		RandomProgressBar	progress(pproxy);
		progress.start(message, m_n_pieces);

		bool	parallel =
			(m_mode==e_force_parallel) ? true :
			(m_mode==e_force_plain || m_mode==e_force_plain_portions || m_n_threads==1) ? false :
			(debug==true) ? false:
			true;

		for(size_t piece = 0; piece < m_n_pieces; ++piece)
		{
			if (need_break(err))
				break;
			if(parallel)
			{
				#pragma omp parallel for schedule (guided)
				for(ptrdiff_t thread_no = 0; thread_no < ptrdiff_t(m_n_threads); ++thread_no)
				{
					if (need_break(err))
					{
#ifdef XRAD_COMPILER_MSC
						break;
#else
						continue; // Реализация OMP в GCC 9.2.1 не поддерживает break.
#endif
					}
					ThreadSetup ts; (void)ts;
					perform_one_thread(one_step_action, piece, thread_no, err);
				}
			}
			else
			{
				for(ptrdiff_t thread_no = 0; thread_no < ptrdiff_t(m_n_threads); ++thread_no)
				{
					if (need_break(err))
						break;
					perform_one_thread(one_step_action, piece, thread_no, err);
				}
			}
			progress.set_position(piece);
		}

		err.ThrowIfSpecialExceptions();
		err.ProcessErrors([&errors](const string &message, size_t step)
				{
					errors[step] = convert_to_wstring(message);
				});
		if (err.ErrorCount() > errors.size())
		{
			errors[size_t(-1)] = ssprintf(L"An unknown error occurred %zu times.",
					EnsureType<size_t>(err.ErrorCount()-errors.size()));
		}
		return progress.end();
	}
};


XRAD_END

#endif // ParallelProcessor_h__
