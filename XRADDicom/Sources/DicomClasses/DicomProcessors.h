#ifndef __DicomProcessors_h__
#define __DicomProcessors_h__
/*!
	\file
	\date 2018/02/12 13:44
	\author kulberg
*/

#include "Instances/LoadGenericClasses.h"
#include <XRADBasic/Sources/Utils/ProgressIndicatorScheduler.h>
#include <XRADBasic/Sources/Utils/ParallelProcessor.h>

XRAD_BEGIN

namespace Dicom
{
/*!
Классы, предназначенные для обработки иерархически устроенных наборов данных Dicom
Чтобы обработать одну структурную единицу (например, series), необходимо создать класс-
обработчик, наследуемый от соответствующего абстрактного обработчика:

class	DoSomethingWithSeries : public SeriesProcessor
{
	virtual	void Apply(series &data, ProgressProxy pp) override
	{
	// здесь циклы, обходящие внутреннюю структуру series и выполняющие требуемые действия
	}
};

Обработка одной единицы данных.

	Dicom::series	my_series;
	DoSomethingWithSeries	series_processor;
	series_processor->Apply(my_series, GUIProgressProxy());

Иерархическая обработка вложенных структур.

Если нужно обработать одинаковым образом более сложный набор данных, следует создать
объект, выполняющий рекурсивную обработку. Обработчик более мелкой структурной единицы
передается в конструктор более крупной.

	Dicom::study	my_study;
	Dicom::StudyProcessorRecursive	study_processor(make_shared<DoSomethingWithSeries>());
	study_processor->Apply(my_study, GUIProgressProxy());

	Dicom::StudyProcessorRecursive	anonymizer(make_shared<InstanceAnonymizer>(old_study_folder, new_study_folder, anonymized_dataset));
	anonymizer.Apply(study, GUIProgressProxy());


*/

//! Абстрактный класс, обеспечивающий обработку одного Dicom::instance
template<class INSTANCE>
struct InstanceProcessor
{
	typedef	INSTANCE data_t;
// 	typedef	shared_ptr<instance> data_t;
	virtual	void Apply(data_t &data, ProgressProxy pp) = 0;
};

template<class DATA_T, class ELEMENT_PROCESSOR_T>
struct AbstractProcessor
{
	typedef	DATA_T data_t;
//	typedef ELEMENT_T element_t;
	typedef	ELEMENT_PROCESSOR_T	element_processor_t;

	virtual	void Apply(data_t &data, ProgressProxy pp) = 0;
};

//! Абстрактный класс, обеспечивающий обработку одного Dicom::acquisition
template<class ACQUISITION>
struct AcquisitionProcessor : public AbstractProcessor<ACQUISITION, InstanceProcessor<typename ACQUISITION::instance_t>>{};

//! Абстрактный класс, обеспечивающий обработку одного Dicom::stack
template<class STACK>
struct StackProcessor : public AbstractProcessor<STACK, AcquisitionProcessor<typename STACK::acquisition_t>>{};

//! Абстрактный класс, обеспечивающий обработку одной Dicom::series
template<class SERIES>
struct SeriesProcessor : public AbstractProcessor<SERIES, StackProcessor<typename SERIES::stack_t>>{};

//! Абстрактный класс, обеспечивающий обработку одной Dicom::studies
template<class STUDY>
struct StudyProcessor: public AbstractProcessor<STUDY, SeriesProcessor<typename STUDY::series_t>>{};


//! Абстрактный класс, обеспечивающий обработку одного Dicom::patient
template<class PATIENT>
struct PatientProcessor: public AbstractProcessor<PATIENT, StudyProcessor<typename PATIENT::study_t>>{};

//! Абстрактный класс, обеспечивающий обработку одного Dicom::patients
template<class PATIENTS>
struct PatientsProcessor : public AbstractProcessor<PATIENTS, PatientProcessor<typename PATIENTS::patient_t>>{};


//!	Удаление пустых подсписков. необходимо вызывать после каждой обработки
inline void	RemoveEmptySublists(Dicom::acquisition_loader &){}// функция, на которой обрывается рекурсия. по хорошему, ей надо быть от шаблона acquisition<>, но пока не удается

template<class T>
void	RemoveEmptySublists(T &data)
{
	for(auto &d: data) RemoveEmptySublists(d);
	for(auto it = data.begin(); it != data.end();)
	{
		if(it->empty()) it = data.erase(it);
		else ++it;
	}
}



//! Класс, обеспечивающий рекурсивную обработку иерархической структуры
template<class PROC_TYPE>
class ProcessorRecursive : public PROC_TYPE
{
	typedef typename PROC_TYPE processor_t;
	typedef typename processor_t::data_t data_t;
	//typedef typename processor_t::element_t element_t;
	typedef typename processor_t::element_processor_t element_processor_t;
	shared_ptr<element_processor_t>	element_processor;

	template<class T>
	size_t	complexity(const T&item) { return item.n_instances(); }

	template<>
	size_t	complexity(const cloning_ptr<Dicom::instance >&) { return 1; }

	//! brief Член, определяющий возможность многопоточной обработки. По умолчанию false для обработчиков всех уровней, кроме acquisition
	bool	m_parallel;

	//	Произвольный доступ к элементам списка, нужный для многопоточной обработки.
	//	Считаем, что выигрыш от многопоточности превышает потери от обхода списка
	template<class T> T &nth(vector<T> &data, size_t i){ return data[i]; }
	template<class T> T &nth(list<T> &data, size_t i)
	{
		auto it = data.begin();
		while(i) { ++it; --i; }
		return *it;
	}

public:
	ProcessorRecursive(shared_ptr<element_processor_t> in_pr, bool parallel = false) : element_processor(in_pr), m_parallel(parallel){}

	//! \brief Выполнить обработку списка, затем удалить пустые элементы. Допускается многопоточная обработка
	virtual	void Apply(data_t &data, ProgressProxy pp) override
	{
		if(m_parallel)
		{
			ParallelProcessor	processor;
			processor.init(data.size(), ParallelProcessor::e_auto);

			auto	lambda = [&data, this](size_t frame_no)
			{
				element_processor->Apply(nth(data, frame_no), VoidProgressProxy());
			};
			processor.perform(lambda, L"Processing", pp);
		}
		else
		{
			vector<double>	costs(data.size());
			auto	it = data.begin();
			for(auto &cost : costs) cost = complexity(*(it++));
			ProgressIndicatorScheduler	scheduler(costs);

			RandomProgressBar	progress(pp);
			progress.start("Processing", scheduler.n_steps());
			size_t i = 0;
			for(auto &next_level : data)
			{
				scheduler.run_operation(i++, progress, [&next_level, this](ProgressProxy subpp) {element_processor->Apply(next_level, subpp); });
			}
		}
		RemoveEmptySublists(data);
	}
};

//! Класс, обеспечивающий обработку одного acquisition через последовательную обработку всех instances.
//	Единственный конструктор получает ссылку на объект, унаследованный от AbstractInstanceProcessor.
//	Таким образом, каждый instance (например, срез томограммы) обрабатывается одинаковым способом.
template<class ACQUISITION>
class AcquisitionProcessorRecursive : public ProcessorRecursive<AcquisitionProcessor<ACQUISITION>>
{
	PARENT(ProcessorRecursive<AcquisitionProcessor<ACQUISITION>>);
	typedef ACQUISITION	acquisition_t;
	typedef	typename acquisition_t::instance_t instance_t;
public:
	//! \brief Конструктор содержит необязательный второй булев параметр, определяющий многопоточность обработки.
	AcquisitionProcessorRecursive(shared_ptr<InstanceProcessor<instance_t>> p, bool parallel = true) : parent(p, parallel){}
};

//! Класс, обеспечивающий обработку одного stack через последовательную обработку всех acquisitions.
//	Конструкторы получают либо обработчик acquisition целиком, либо обработчик instance.
//	Во втором случае обработчик acquisition создается автоматически и включает последовательную
//	обработку всех вложенных структур

template<class STACK>
class StackProcessorRecursive : public ProcessorRecursive<StackProcessor<STACK>>
{
	PARENT(ProcessorRecursive<StackProcessor<STACK>>);

	typedef STACK stack_t;
	typedef	typename stack_t::acquisition_t acquisition_t;
	typedef	typename stack_t::instance_t instance_t;

public:
	StackProcessorRecursive(shared_ptr<AcquisitionProcessor<acquisition_t>> &p) : parent(p){}
	StackProcessorRecursive(shared_ptr<InstanceProcessor<instance_t>> &p) : parent(make_shared<AcquisitionProcessorRecursive<acquisition_t>>(p)){}
};


//! Класс, обеспечивающий обработку одной series через последовательную обработку всех stacks.
//	Конструкторы получают либо обработчик stack целиком, либо обработчики всех более простых структур.
//	в иерархии.

template<class SERIES>
class SeriesProcessorRecursive : public ProcessorRecursive<SeriesProcessor<SERIES>>
{
	PARENT(ProcessorRecursive<SeriesProcessor<SERIES>>);

	typedef SERIES series_t;
	typedef	typename series_t::stack_t stack_t;
	typedef	typename series_t::acquisition_t acquisition_t;
	typedef	typename series_t::instance_t instance_t;

public:
	SeriesProcessorRecursive(shared_ptr<StackProcessor<stack_t>> p) : parent(p){}
	SeriesProcessorRecursive(shared_ptr<AcquisitionProcessor<acquisition_t>> p) : parent(make_shared<StackProcessorRecursive<stack_t>>(p)){}
	SeriesProcessorRecursive(shared_ptr<InstanceProcessor<instance_t>> p) : parent(make_shared<StackProcessorRecursive<stack_t>>(p)){}
};


//! Класс, обеспечивающий обработку одного study через последовательную обработку всех series.
//	Конструкторы получают либо обработчик series целиком, либо обработчики всех более простых структур.
//	в иерархии.
template<class STUDY>
class StudyProcessorRecursive : public ProcessorRecursive<StudyProcessor<STUDY>>
{
	PARENT(ProcessorRecursive<StudyProcessor<STUDY>>);

	typedef STUDY study_t;
	typedef	typename study_t::series_t series_t;
	typedef	typename study_t::stack_t stack_t;
	typedef	typename study_t::acquisition_t acquisition_t;
	typedef	typename study_t::instance_t instance_t;

public:
	StudyProcessorRecursive(shared_ptr<SeriesProcessor<series_t>> p) : parent(p){}
 	StudyProcessorRecursive(shared_ptr<StackProcessor<stack_t>> p) : parent(make_shared<SeriesProcessorRecursive<series_t>>(p)){}
 	StudyProcessorRecursive(shared_ptr<AcquisitionProcessor<acquisition_t>> p) : parent(make_shared<SeriesProcessorRecursive<series_t>>(p)){}
 	StudyProcessorRecursive(shared_ptr<InstanceProcessor<instance_t>> p) : parent(make_shared<SeriesProcessorRecursive<series_t>>(p)){}
};


//! Класс, обеспечивающий обработку одного patient через последовательную обработку всех studies.
//	Конструкторы получают либо обработчик series целиком, либо обработчики всех более простых структур.
//	в иерархии.
template<class PATIENT>
class PatientProcessorRecursive : public ProcessorRecursive<PatientProcessor<PATIENT>>
{
	PARENT(ProcessorRecursive<PatientProcessor<PATIENT>>);

	typedef	typename PATIENT patient_t;
	typedef	typename patient_t::study_t study_t;
	typedef	typename patient_t::series_t series_t;
	typedef	typename patient_t::stack_t stack_t;
	typedef	typename patient_t::acquisition_t acquisition_t;
	typedef	typename patient_t::instance_t instance_t;

public:
	PatientProcessorRecursive(shared_ptr<StudyProcessor<study_t>> p) : parent(p){}
	PatientProcessorRecursive(shared_ptr<SeriesProcessor<series_t>> p) : parent(make_shared<StudyProcessorRecursive<study_t>>(p)){}
	PatientProcessorRecursive(shared_ptr<StackProcessor<stack_t>> p) : parent(make_shared<StudyProcessorRecursive<study_t>>(p)){}
	PatientProcessorRecursive(shared_ptr<AcquisitionProcessor<acquisition_t>> p) : parent(make_shared<StudyProcessorRecursive<study_t>>(p)){}
	PatientProcessorRecursive(shared_ptr<InstanceProcessor<instance_t>> p) : parent(make_shared<StudyProcessorRecursive<study_t>>(p)){}
};


//! Класс, обеспечивающий обработку списка пациентов.
//	Конструкторы получают либо обработчик patient целиком, либо обработчики всех более простых структур.
//	в иерархии.
template<class PATIENTS>
class PatientsProcessorRecursive : public ProcessorRecursive<PatientsProcessor<PATIENTS>>
{
	PARENT(ProcessorRecursive<PatientsProcessor<PATIENTS>>);

	typedef typename PATIENTS patients_t;
	typedef typename patients_t::patient_t patient_t;
	typedef typename patients_t::study_t study_t;
	typedef	typename patients_t::series_t series_t;
	typedef	typename patients_t::stack_t stack_t;
	typedef	typename patients_t::acquisition_t acquisition_t;
	typedef	typename patients_t::instance_t instance_t;

public:
	PatientsProcessorRecursive(shared_ptr<PatientProcessor<patient_t>> p) : parent(p){}
	PatientsProcessorRecursive(shared_ptr<StudyProcessor<study_t>> p) : parent(make_shared<PatientProcessorRecursive<patient_t>>(p)){}
	PatientsProcessorRecursive(shared_ptr<SeriesProcessor<series_t>> p) : parent(make_shared<PatientProcessorRecursive<patient_t>>(p)){}
	PatientsProcessorRecursive(shared_ptr<StackProcessor<stack_t>> p) : parent(make_shared<PatientProcessorRecursive<patient_t>>(p)){}
	PatientsProcessorRecursive(shared_ptr<AcquisitionProcessor<acquisition_t>> p) : parent(make_shared<PatientProcessorRecursive<patient_t>>(p)){}
	PatientsProcessorRecursive(shared_ptr<InstanceProcessor<instance_t>> p) : parent(make_shared<PatientProcessorRecursive<patient_t>>(p)){}
};



//--------------------------------------------------------------
//
//--------------------------------------------------------------



//! \brief Абстрактный класс, обеспечивающий обработку одного элемента данных Dicom
//! с использованием контекста
template<class Datum, class Context>
struct AbstractProcessorCtx
{
	using data_t = Datum;
	using context_t = Context;

	virtual	void Apply(const context_t &context, data_t &data, ProgressProxy pp) = 0;
};

//--------------------------------------------------------------

/*!
	\brief Функция конкатенации контекста в ProcessorRecursiveCtx для контекста типа std::tuple

	- Первым аргументом передается родительский контекст.
	- Вторым аргументом передается добавок PDatum. PDatum реально является указателем на некоторый
		loader.
*/
template <class... Args, class PDatum>
auto ContextCat(const std::tuple<Args...> &context, const PDatum &data)
{
	return std::tuple_cat(context, tuple<PDatum>(data));
}

//--------------------------------------------------------------

//! \brief Вспомогательная структура для ContextCat_t
template <class Context, class Datum>
struct ContextCatType
{
	using type = decltype(ContextCat(std::declval<Context>(), std::declval<const Datum*>()));
};

//! \brief Тип результата конкатенации к контексту _указателя_ на Datum
template <class Context, class Datum>
using ContextCat_t = typename ContextCatType<Context, Datum>::type;

//--------------------------------------------------------------

/*!
	\brief Класс, обеспечивающий рекурсивную обработку иерархической структуры с поддержкой контекста

	См. ProcessorRecursive.

	\par Работа с контекстом

	Внешний контекст определяется типом PROC_TYPE::context_t, он передается снаружи в функцию Apply().
	Функция Apply() добавляет к глобальному контексту указатель на обрабатываемый контейнер данных,
	на параметр data, который передан на вход в Apply(), получает некий current_context_t.
	Полученный контекст передается при обработке каждого элемента в метод Apply() обрабтчика элементов
	ElementProcessor. Тип контекста ElementProcessor должен соответствовать типу current_context_t.

	Из контекста обрабтчик acquisition_loader, к примеру, может получить информацию о
	пациенте, study и т.п.

	Для добавления данных к контексту используется функция ContextCat. Имеющаяся реализация
	поддерживает тип контекста std::tuple, данные добавляет в конец списка агрументов tuple.

	Для использования структур, отличных от tuple, нужно перегрузить функцию ContextCat для
	нужного типа контекста.
*/
template<class PROC_TYPE, class ElementProcessor>
class ProcessorRecursiveCtx : public PROC_TYPE
{
public:
	using processor_t = typename PROC_TYPE;
	using data_t = typename processor_t::data_t;
	using context_t = typename processor_t::context_t;
	using element_processor_t = ElementProcessor;

public:
	ProcessorRecursiveCtx(const shared_ptr<element_processor_t> &in_pr, bool parallel = false) :
		element_processor(in_pr),
		m_parallel(parallel)
	{
	}

	//! \brief Выполнить обработку списка. Пустые элементы не удаляются.
	//! Допускается многопоточная обработка
	virtual	void Apply(const context_t &context, data_t &data, ProgressProxy pp) override
	{
		auto current_context = ContextCat(context, const_cast<std::add_const_t<data_t*>>(&data));
		if(m_parallel)
		{
			ParallelProcessor	processor;
			processor.init(data.size(), ParallelProcessor::e_auto);

			auto	lambda = [&current_context, &data, this](size_t frame_no)
			{
				element_processor->Apply(current_context, nth(data, frame_no), VoidProgressProxy());
			};
			processor.perform(lambda, L"Processing", pp);
		}
		else
		{
			vector<double>	costs(data.size());
			auto	it = data.begin();
			for(auto &cost : costs) cost = complexity(*(it++));
			ProgressIndicatorScheduler	scheduler(costs);

			RandomProgressBar	progress(pp);
			progress.start("Processing", scheduler.n_steps());
			size_t i = 0;
			for(auto &next_level : data)
			{
				scheduler.run_operation(i++, progress,
						[&current_context, &next_level, this](ProgressProxy subpp)
						{
							element_processor->Apply(current_context, next_level, subpp);
						});
			}
		}
	}

	//! \brief Выполнить обработку списка с пустым внешним контекстом
	void Apply(data_t &data, ProgressProxy pp)
	{
		Apply(context_t(), data, pp);
	}

private:
	template<class T>
	size_t	complexity(const T&item) { return item.n_instances(); }

	template<>
	size_t	complexity(const cloning_ptr<Dicom::instance >&) { return 1; }

	// Произвольный доступ к элементам списка, нужный для многопоточной обработки.
	// Считаем, что выигрыш от многопоточности превышает потери от обхода списка.
	template<class T> std::add_const_t<T> &nth(const vector<T> &data, size_t i)
	{
		return data[i];
	}
	template<class T> T &nth(vector<T> &data, size_t i)
	{
		return data[i];
	}
	template<class T> std::add_const_t<T> &nth(const list<T> &data, size_t i)
	{
		auto it = data.begin();
		std::advance(it, i);
		return *it;
	}
	template<class T> T &nth(list<T> &data, size_t i)
	{
		auto it = data.begin();
		std::advance(it, i);
		return *it;
	}

private:
	shared_ptr<element_processor_t> element_processor;
	//! \brief Флаг, определяющий возможность многопоточной обработки.
	//! По умолчанию false для обработчиков всех уровней, кроме acquisition
	bool m_parallel;
};

//--------------------------------------------------------------

//! \brief Вспомогательное определение
template <class Datum, class DatumElement, class Context>
using ElementProcessorRecursiveCtx = ProcessorRecursiveCtx<
		AbstractProcessorCtx<Datum, Context>,
		AbstractProcessorCtx<DatumElement, ContextCat_t<Context, Datum>>>;

//--------------------------------------------------------------

//! \brief Шаблон, определяющий, имеет ли класс T элементы для обработки
//! (T = patients_loader и т.п.). По умолчанию не имеет
template <class T, class = void>
struct IsCompound: public std::false_type {};

//! \brief Специализация для классов, имеющих определение типа value_type (vector, list и т.п.):
//! элементы для обработки есть
template <class T>
struct IsCompound<T, std::void_t<typename T::value_type>>: public std::true_type {};

//! \brief Шаблон класса, обеспечивающего обработку контейнера. Имеет две специализации
//! по параметру Compound. Используется через DataProcessorRecursiveCtx
template<class ACQUISITION, class Context, bool Compound>
class DataProcessorRecursiveCtxClass;

//--------------------------------------------------------------

/*!
	\brief Класс, обеспечивающий обработку контейнера данных через последовательную обработку всех
	его вложенных структур с использованием контекста

	\par Пример

	- Процессор: `DataProcessorRecursiveCtx &lt; patients_loader, tuple &lt;&gt; &gt;`.
	- Обработчик элемента: наследник `DataProcessorRecursiveCtx &lt;
		patients_loader, tuple &lt; &gt; &gt;::ProcessorType_t &lt; acquisition_loader &gt;`.
		Наследник должен переопределить абстрактный метод Apply() класса AbstractProcessorCtx.
	- Контекст обработчика `acquisition_loader` будет иметь тип:
		`tuple &lt; const patients_loader*, const patient_loader*, ..., const stack_loader* gt;`.

	\sa ProcessorRecursiveCtx.
*/
template<class Data, class Context>
using DataProcessorRecursiveCtx = DataProcessorRecursiveCtxClass<Data, Context,
		IsCompound<typename Data::value_type>::value>;

//--------------------------------------------------------------

//! \brief Класс, обеспечивающий обработку одного контейнера Data через последовательную обработку
//! всех его вложенных структур. Элементы контейнера сами не являются контейнерами.
//! Используется через DataProcessorRecursiveCtx
template<class Data, class Context>
class DataProcessorRecursiveCtxClass<Data, Context, false> :
	public ElementProcessorRecursiveCtx<Data, copy_const_t<Data, typename Data::value_type>, Context>
{
	PARENT(ElementProcessorRecursiveCtx<Data, copy_const_t<Data, typename Data::value_type>, Context>);

public:
	using typename parent::data_t;
	using typename parent::context_t;
	using element_t = copy_const_t<data_t, typename data_t::value_type>;
	using element_context_t = ContextCat_t<context_t, data_t>;

	template <class D>
	struct ProcessorType;

	template <>
	struct ProcessorType<element_t>
	{
		using type = AbstractProcessorCtx<element_t, element_context_t>;
	};

	template <class D>
	using ProcessorType_t = typename ProcessorType<D>::type;

public:
	//! \brief Конструктор от обработчика элементов
	DataProcessorRecursiveCtxClass(const shared_ptr<ProcessorType_t<element_t>> &p) :
		parent(p, true)
	{}
};

//--------------------------------------------------------------

/*!
	\brief Класс, обеспечивающий обработку одного контейнера Data через последовательную обработку
	всех его вложенных структур. Элементы контейнера сами тоже являются контейнерами.
	Используется через DataProcessorRecursiveCtx

	\note
	Многопоточность на уровне, отличном от acquisition_loader, не будет эффективно работать,
	если её реализовать простым флажком, передаваемым в ProcessorRecursiveCtx.
	Количество элементов на этих уровнях может быть мало по сравнению с количеством доступных потоков.
	Для эффективной работы нужно организовать "сбор" элементов нижнего уровня, и собранные элементы
	обрабатывать в многопоточном режиме.
*/
template<class Data, class Context>
class DataProcessorRecursiveCtxClass<Data, Context, true> :
	public ElementProcessorRecursiveCtx<Data, copy_const_t<Data, typename Data::value_type>, Context>
{
	PARENT(ElementProcessorRecursiveCtx<Data, copy_const_t<Data, typename Data::value_type>, Context>);

public:
	using typename parent::data_t;
	using typename parent::context_t;
	using element_t = copy_const_t<data_t, typename data_t::value_type>;
	using element_context_t = ContextCat_t<context_t, data_t>;
	using element_processor_recursive_t = DataProcessorRecursiveCtx<element_t, element_context_t>;

	template <class D>
	struct ProcessorType: public element_processor_recursive_t::template ProcessorType<D> {};

	template <>
	struct ProcessorType<element_t>
	{
		using type = AbstractProcessorCtx<element_t, element_context_t>;
	};

	template <class D>
	using ProcessorType_t = typename ProcessorType<D>::type;

public:
	//! \brief Конструктор от обработчика элементов, непосредственно содержащихся в контейнере Data
	DataProcessorRecursiveCtxClass(const shared_ptr<ProcessorType_t<element_t>> &p) :
		parent(p)
	{}

	//! \brief Конструктор от обработчика элементов, содержащихся в элементах контейнера Data
	template <class Proc,
			class = std::enable_if_t<!std::is_base_of<ProcessorType_t<element_t>, Proc>::value>>
	DataProcessorRecursiveCtxClass(const shared_ptr<Proc> &p) :
		parent(make_shared<element_processor_recursive_t>(p))
	{}
};

//--------------------------------------------------------------

}//namespace Dicom

XRAD_END

#endif // __DicomProcessors_h__
