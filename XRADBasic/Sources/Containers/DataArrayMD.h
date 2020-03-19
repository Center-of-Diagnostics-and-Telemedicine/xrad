#ifndef __data_array_multidimensional_h
#define __data_array_multidimensional_h

#include "DataArray2D.h"
#include "IndexVector.h"

XRAD_BEGIN

//--------------------------------------------------------------
/*!
	\brief Универсальный многомерный массив

	\par Основные понятия

	В качестве аргумента шаблона A2DT должен использоваться подходящий
	хорошо разработанный двумерный тип (например, из определенных в MathFunction2DTypes.h,
	такой как MathFunction2D).
	Возможно использование базового типа, например,
	DataArray2D, однако этом случае возможности анализа/обработки срезов
	будут минимальными.

	При выборе имен функций/переменных используем следующие имена:
	- row -- одномерный срез, "строка" массива по произвольному направлению;
	- row_type==A2DT::array_type -- тип одномерной строки;
	- slice -- двумерный срез;
	- slice_type==A2DT -- тип двумерного среза;
	- subset -- подмножество с размерностью более 2;
	- subset_type==self: подмножества также являются многомерными массивами.

	Можно было бы свести двумерный и одномерный срезы к частным случаям
	многомерных. Но это не делается следующим по принципиальным соображениям:
	- обращение к "необобщенным" массивам существенно эффективнее
	в вычислительном отношении;
	- "необобщенные" типы включают хорошо разработанные методы
	анализа и обработки, специфические для одно- и двумерного случая,
	для "обобщенных" объектов их пришлось бы переписывать или создавать
	переходные функции, что нерационально.

	\par Инициализация массива

	Происходит по вектору индексов:

	~~~~
	index_vector	dimensions = quick_iv(10,20,30);
	DataArrayMD<GrayImage>	array(dimensions_vector);
	~~~~

	\par Обращение к отдельному элементу

	Происходит по вектору индексов:

	~~~~
	index_vector	iv = quick_iv(1,2,3);
	array.at(iv) = 100;
	~~~~

	Такое обращение допустимо, однако в критических по быстродействия
	участках кода нежелательно, так как разбор индексного вектора требует
	довольно много действий. Наилучший способ -- выделять для работы одно-
	и двумерные подмножества.

	\par Выделение одно- и двумерных подмножеств

	В случае, когда нужно выделить
	подмножество многомерного массива, используется "маска измерения".
	Чтобы выделить одномерное подмножество, объявляем пустой контейнер:

	~~~~
	RealFunction	row;
	~~~~

	И привязываем его к данным, находящимся в нужном нам подмножестве:

	~~~~
	index_vector	row_specification = quick_iv(slice_mask(0), 3, 4);
	array.GetRow(row, row_specification);
	~~~~

	В результате row становится массивом длиной 10 элементов, указывающим на
	данные с индексами (0,3,4)--(9,3,4). `slice_mask(0)` указывает, что данная
	координата будет нулевым измерением во вновь полученном массиве. В одномерном
	массиве есть только нулевая координата, любой другой аргумент, например,
	`slice_mask(i>0)` будет ошибкой.

	Чтобы получить одномерный итератор по одномерному подмножеству,

	~~~~
	iterator it = array.begin(row_specification);
	~~~~

	Для получения двумерного подмножества делаем следующее:

	~~~~
	GrayImage	slice;
	index_vector	slice_specification = quick_iv(2, slice_mask(0), slice_mask(1));
	array.GetSlice(slice, slice_specification);
	~~~~

	`slice` становится двумерным массивом размером 20х30, указывающим на элементы
	исходного массива с индексами (2,0,0)--(2,19,29). Если задать индексы следующим образом:

	~~~~
	index_vector	slice_specification = quick_iv(2, slice_mask(1), slice_mask(0));
	~~~~

	получится транспонированный массив 30х20, указывающий на те же данные. Поскольку
	в двумерном массиве есть только координаты 0 и 1, задание slice_mask(i>1) даст ошибку.

	\par Многомерные подмножества

	Также допустимы подмножества типа `self` с меньшим количеством измерений. Способ задания
	аналогичен. Следует иметь в виду, что они страдают тем же недостатком, что и исходный
	массив -- "тяжеловесность" оператора `at()`. Их следует использовать в тех случаях, когда
	задана функция обработки, например, для трехмерного массива, и нужно выделить аргумент
	для нее из массива с большей размерностью.
*/
template <class A2DT>
class	DataArrayMD : private DataOwner<typename A2DT::value_type>
{
	private:
		PARENT(DataOwner<typename A2DT::value_type>);

	public:
		typedef	DataArrayMD<A2DT> self;
		typedef DataArrayMD<typename A2DT::invariable> invariable;
		typedef DataArrayMD<typename A2DT::variable> variable;
		typedef ReferenceOwner<self> ref;
		typedef ReferenceOwner<invariable> ref_invariable;
			// про invariable см. подробный комментарий в DataOwner.h
//		typedef	DataArrayMD<A2DT> subset_type;
		typedef DataArrayMD<typename A2DT::invariable> subset_type_invariable;

		typedef A2DT slice_type;
		typedef typename A2DT::invariable slice_type_invariable;
		typedef typename A2DT::variable slice_type_variable;
		typedef typename A2DT::row_type row_type;
		typedef typename A2DT::row_type::invariable row_type_invariable;
//		typedef typename A2DT::row_type::variable row_type_variable;

		typedef ReferenceOwner<slice_type> slice_ref;
		typedef ReferenceOwner<typename slice_type::invariable> slice_ref_invariable;
		typedef ReferenceOwner<row_type> row_ref;
		typedef ReferenceOwner<typename row_type::invariable> row_ref_invariable;

		typedef typename parent::value_type value_type;
		typedef	typename parent::value_type_variable value_type_variable;
		typedef	typename parent::value_type_invariable value_type_invariable;

		typedef index_vector index_type;

		typedef index_vector::iterator index_vector_iterator;
		typedef index_vector::const_iterator const_index_vector_iterator;
		typedef offset_vector::iterator steps_vector_iterator;
		typedef offset_vector::const_iterator const_steps_vector_iterator;

		typedef	typename row_type::iterator row_iterator;
		typedef typename row_type::const_iterator const_row_iterator;
		typedef	typename row_type::reverse_iterator reverse_row_iterator;
		typedef typename row_type::const_reverse_iterator const_reverse_row_iterator;

		//! \name Конструкторы и operator=
		//! @{
	public:
		DataArrayMD();
		DataArrayMD(const index_vector& iv);
		DataArrayMD(const index_vector& iv, const value_type& default_value);

		//! \brief См. \ref pg_CopyContructorOperatorEq
		DataArrayMD(const self &original);
		//! \brief См. \ref pg_MoveOperations
		DataArrayMD(self &&original);
		//! \brief См. \ref pg_CopyContructorOperatorEq
		DataArrayMD &operator=(const self &original);
		//! \brief См. \ref pg_MoveOperations
		DataArrayMD &operator=(self &&original);

		//! \brief См. \ref pg_CopyContructorOperatorEq
		template<class A2DT2>
		DataArrayMD(const DataArrayMD<A2DT2> &original);
		//! \brief См. \ref pg_CopyContructorOperatorEq
		template<class A2DT2>
		DataArrayMD &operator=(const DataArrayMD<A2DT2> &original);
		//! @}

		//! \name Info
		//! @{
	public:
		bool empty() const;
		bool ready() const;

		const index_vector &sizes() const {return m_sizes;}
		const offset_vector &steps() const { return m_steps; }
		const offset_vector steps_raw() const { return m_steps*parent::step(); }

		size_t	sizes(size_t s) const {return m_sizes[s];}
		ptrdiff_t	steps(size_t s) const { return m_steps[s]; }
		ptrdiff_t	steps_raw(size_t s) const { return m_steps[s]*parent::step(); }
		size_t	n_dimensions() const {return m_sizes.size();}
		size_t	element_count() const { size_t result(1); for(auto sz: sizes()) result*=sz; return result; } // количество элементов в массиве
		using parent::element_size;

		//! @}

		//! \name Инициализация
		//! @{

	public:
		void	realloc(const index_vector& iv);
		void	realloc(const index_vector& iv, const value_type &default_value){realloc(iv);fill(default_value);}

		void	resize(const index_vector &new_sizes);

		/*!
			\brief Изменение порядка следования измерений (многомерное транспонирование)

			\param iv Аргумент должен содержать только значения типа slice_mask.

			Работает следующим образом.
			Пусть дана функция f(x,y,z,t). Результат вызова с различными iv:

			~~~~
			iv = (#0,#1,#2,#3) => f(x,y,z,t)
			iv = (#1,#0,#2,#3) => f(y,x,z,t)
			iv = (#2,#3,#1,#0) => f(z,t,y,x)
			~~~~

			#n значит slice_mask(n).
		*/
		void	reorder_dimensions(const index_vector& iv);

		//! \brief Заполнение значением по умолчанию
		void	fill(const value_type &x);

		template<class A2DT2>
		void	MakeCopy(const DataArrayMD<A2DT2> &original);
		template<class A2DT2, class functor>
		void	MakeCopy(const DataArrayMD<A2DT2> &original, const functor &f);

		template<class A2DT2>
		void	CopyData(const DataArrayMD<A2DT2> &original);
		template<class A2DT2, class functor>
		void	CopyData(const DataArrayMD<A2DT2> &original, const functor &f);
		//TODO в отличие от двумерных и одномерных здесь CopyData не для разноразмерных массивов. возможно, потом придется это доделать

		// void	CopyData(const value_type *, const index_vector &, int );
		//TODO сделать функции импорта из указателя такие же, как в одномерном и двумерном массиве

		void UseData(value_type *new_data, const index_vector& in_sizes, ptrdiff_t in_data_step);
		//! \todo Сделать поддержку отрицательных шагов внутри DataArrayMD.
		void UseData(value_type *new_data, const index_vector& in_sizes, const offset_vector& in_steps);
		void UseData(parent &new_data, const index_vector& in_sizes, const index_vector& in_steps);

		//! \brief Создать ссылку на элементы существующего контейнера, полностью повторяя его свойства
		//!
		//! Может использоваться для создания const ссылок.
		template<class A2DT2>
		void UseData(DataArrayMD<A2DT2> &new_data);

		//! \brief Создать ссылку на элементы существующего контейнера, полностью повторяя его свойства
		//!
		//! Используется для создания const ссылок, тип данных текущего класса должен быть const.
		template<class A2DT2>
		void UseData(const DataArrayMD<A2DT2> &new_data);

		template<class A2DT2>
		void UseDataFragment(DataArrayMD<A2DT2> &original, const index_vector &p0, const index_vector &p1);
		template<class A2DT2>
		void UseDataFragment(const DataArrayMD<A2DT2> &original, const index_vector &p0, const index_vector &p1);

		//! \brief Переместить данные из original (без копирования). Контейнер original становится пустым
		void MoveData(DataArrayMD &original);

		//! \brief Получить ссылку на подмножество элементов существующего контейнера
		//! в некотором диапазоне.
		//! Предполагается, что для ArrayMD реализована move-семантика
		template <class ArrayMD = self>
		ArrayMD GetDataFragment(const index_vector &p0, const index_vector &p1);

		//! \brief Получить ссылку на подмножество элементов существующего контейнера
		//! в некотором диапазоне.
		//! Предполагается, что для ArrayMD реализована move-семантика
		template <class ArrayMD = invariable>
		ArrayMD GetDataFragment(const index_vector &p0, const index_vector &p1) const;

		//! @}

		//! \name Access: Доступ к элементу напрямую по индексам. Неоптимальный способ с точки зрения быстродействия
		//!
		//! Работать следует только через выделение одно- и двумерных срезов.
		//! @{

		value_type	&at(const index_vector& index);
		const value_type &at(const index_vector& index) const;

		//! @}

		/*!
			\name Access: Извлечение подмножеств меньшей размерности в преждесозданный контейнер

			~~~~
			DataArrayMD<>	mdarray;
			index_vector	vector;
			DataArrayMD<>::slice_type	slice;
			mdarray.GetSlice(slice, vector);
			for(size_t i=0;i<slice.vsize();++i)slice[i].fill(i);
			~~~~

			Оптимальный способ для многократного доступа к заданному подмножеству.
			Недостаток -- требуется объявлять отдельную переменную для такого
			контейнера, загромождает код.

			@{
		*/
		//! \brief Выделение одномерного среза
		void	GetRow(row_type &row, const index_vector& iv);
		//! \brief Выделение одномерного среза
		void	GetRow(typename row_type::invariable &row, const index_vector& iv) const;

		//! \brief Получить ссылку на одномерный срез существующего контейнера.
		//! Предполагается, что для Array реализована move-семантика
		template <class Array = typename row_type::ref>
		Array GetRow(const index_vector &index);

		//! \brief Получить ссылку на константный одномерный срез существующего контейнера.
		//! Array должен иметь константный тип данных.
		//! Предполагается, что для Array реализована move-семантика
		template <class Array = typename row_type::invariable::ref>
		Array GetRow(const index_vector &index) const;

		//! \brief Выделение двумерного среза
		void	GetSlice(slice_type &slice, const index_vector& iv);
		//! \brief Выделение двумерного среза
		void	GetSlice(typename slice_type::invariable &slice, const index_vector& iv) const;

		//! \brief Получить ссылку на двумерный срез существующего контейнера.
		//! Предполагается, что для Array2D реализована move-семантика
		template <class Array2D = typename slice_type::ref>
		Array2D GetSlice(const index_vector &index);

		//! \brief Получить ссылку на константный двумерный срез существующего контейнера.
		//! Array2D должен иметь константный тип данных.
		//! Предполагается, что для Array2D реализована move-семантика
		template <class Array2D = typename slice_type::invariable::ref>
		Array2D GetSlice(const index_vector &index) const;

		//! \brief Выделение многомерного подмножества
		template<class A2DT2>
		void	GetSubset(DataArrayMD<A2DT2> &subset, const index_vector& iv);

		//! \brief Получение многомерного константного подмножества.
		//! Подразумевается, что A2DT2 задает тип с константными данными.
		//! Если это не так, при компиляции реализации метода будет ошибка
		template<class A2DT2>
		void	GetSubset(DataArrayMD<A2DT2> &subset, const index_vector& iv) const;

		//! \brief Получить ссылку на многомерный срез существующего контейнера.
		//! Предполагается, что для ArrayMD реализована move-семантика
		template <class ArrayMD = ref>
		ArrayMD GetSubset(const index_vector &index);

		//! \brief Получить ссылку на константный многомерный срез существующего контейнера.
		//! ArrayMD должен иметь константный тип данных.
		//! Предполагается, что для ArrayMD реализована move-семантика
		template <class ArrayMD = typename invariable::ref>
		ArrayMD GetSubset(const index_vector &index) const;

		//! @}

		/*!
			\name Access: Извлечение компонент данных (для комплексных, RGB и т.п.)

			См. \ref pg_ComponentSelector.

			@{
		*/
		//! \brief Получить ссылку на компоненту данных контейнера.
		//! См. \ref pg_ComponentSelector
		template<class A2DT2, class ComponentSelector>
		void GetDataComponent(DataArrayMD<A2DT2> &component, ComponentSelector component_selector);

		//! \brief Получить ссылку на компоненту данных контейнера (данные константные).
		//! См. \ref pg_ComponentSelector
		template<class A2DT2, class ComponentSelector>
		void GetDataComponent(DataArrayMD<A2DT2> &component,
				ComponentSelector component_selector) const;

	private:
		//! \brief Вспомогательный тип для задания параметра шаблона по умолчанию
		struct default_type_argument;

		//! \brief Вспомогательный шаблон для определения типа многомерного массива с компонентой
		//! по умолчанию
		template <class ArrayMD, class ValueType>
		struct default_array
		{
			using type = ArrayMD;
		};
		//! \brief Вспомогательный шаблон для определения типа многомерного массива с компонентой
		//! по умолчанию (специализация для типа по умолчанию)
		template <class ValueType>
		struct default_array<default_type_argument, ValueType>
		{
			using type = DataArrayMD<DataArray2D<DataArray<std::remove_reference_t<ValueType>>>>;
		};
		//! \brief Вспомогательный шаблон для определения типа многомерного массива с компонентой
		//! по умолчанию
		template <class ArrayMD, class ValueType>
		using default_array_t = typename default_array<ArrayMD, ValueType>::type;

	public:
		//! \brief Получить ссылку на компоненту данных контейнера.
		//! Предполагается, что для ArrayMD реализована move-семантика.
		//! См. \ref pg_ComponentSelector
		template <class ArrayMD = default_type_argument, class ComponentSelector>
		auto GetDataComponent(ComponentSelector component_selector) ->
				default_array_t<ArrayMD,
						decltype(*std::declval<ComponentSelector>()(std::declval<value_type*>()))>;

		//! \brief Получить ссылку на компоненту данных контейнера.
		//! ArrayMD должен иметь константный тип данных.
		//! Предполагается, что для ArrayMD реализована move-семантика.
		//! См. \ref pg_ComponentSelector
		template<class ArrayMD = default_type_argument, class ComponentSelector>
		auto GetDataComponent(ComponentSelector component_selector) const ->
				default_array_t<ArrayMD,
						decltype(*std::declval<ComponentSelector>()(std::declval<value_type_invariable*>()))>;

		//! @}

		/*!
			\name Извлечение подмножеств меньшей размерности в ссылочный контейнер

			Оптимально для разового доступа к нужному подмножеству. Например:

			~~~~
			DataArrayMD<>	mdarray;
			index_vector	vector;
			slice(mdarray(vector)).fill(0)
			~~~~

			Для поэлементного доступа недопустимо.

			@{
		*/
		slice_ref slice(const index_vector& iv);
		slice_ref_invariable slice(const index_vector& iv) const;

		row_ref row(const index_vector& iv);
		row_ref_invariable row(const index_vector& iv) const;

		ref subset(const index_vector& iv);
		ref_invariable subset(const index_vector& iv) const;

		//! @}

	private:
		//size_t	m_dimensions_number;
		index_vector	m_sizes;
		offset_vector	m_steps;

	private:
		//! \name Вспомогательные функции разбора индексного вектора
		//! @{

		//! \brief Смещение до элемента с заданным индекс-вектором
		size_t	ElementOffset(const index_vector& index) const;
		//! \brief Параметры выделения одномерного подмножества
		void	GetRowIndex(size_t &row_index, const index_vector &iv) const;
		//! \brief Параметры выделения двумерного подмножества;
		void	GetSliceIndices(size_t &slice_index_0, size_t &slice_index_1, const index_vector &iv) const;
		//! \brief Параметры выделения многомерного подмножества
		void	GetSubsetIndices(index_vector &subset_indices, const index_vector &iv) const;

		//! @}

		/*!
			\brief По заданному индексному вектору размеров инициализирует внутренние
			вектора размеров и шагов

			Самым важным здесь является процедура построения вектора шагов:
			наименьший шаг соответствует самому последнему измерению.
			Самый быстрый доступ через итератор возможен по нему.
			Здесь тот же принцип, что и при создании двумерного массива.
		*/
		void	GenerateSizesAndSteps(const index_vector& index);

		//! \brief Проверка, соответствует ли разметка массива самому массиву.
		//! В частности, нет ли неудаленных данных при удаленной разметке
		void	CheckDataIntegrity() const;
};

//--------------------------------------------------------------

// template<class A2DT>
// void	swap(DataArrayMD<A2DT> &arr1, DataArrayMD<A2DT> &arr2);

//--------------------------------------------------------------

template<class A2DT>
DataArrayMD<A2DT>	zero_value(const DataArrayMD<A2DT> &datum)
{
	index_vector	access_v(datum.sizes());
	access_v.fill(0);
	return DataArrayMD<A2DT>(datum.sizes(), zero_value(datum.at(access_v)));
}

template<class A2DT>
void	make_zero(DataArrayMD<A2DT> &datum)
{
	index_vector	access_v(datum.sizes());
	access_v.fill(0);
	make_zero(datum.at(access_v));
	datum.fill(datum.at(access_v));
}


check_container_md(DataArrayMD, DataArray2D<DataArray<double> >)



//--------------------------------------------------------------
// Копирование из массива в массив (не всё еще сделано)
//--------------------------------------------------------------

template<class A2DT, class A2DT2>
void MakeCopy(DataArrayMD<A2DT> &destination, const DataArrayMD<A2DT2> &original);

template<class A2DT, class A2DT2>
void CopyData(DataArrayMD<A2DT> &destination, const DataArrayMD<A2DT2> &original);

template<class A2DT, class A2DT2, class functor>
void MakeCopy(DataArrayMD<A2DT> &destination, const DataArrayMD<A2DT2> &original, const functor &f);

//--------------------------------------------------------------

/*!
	\brief Применение функтора `R f(T)` ко всем элементам массива

	Для каждого элемента массива выполняется `data[i] = f(data[i])`.
*/
template <class A2DT, class F>
void ApplyFunction(DataArrayMD<A2DT> &array, const F &function);

/*!
	\brief Применение функтора `void f(T &)` ко всем элементам массива

	Для каждого элемента массива выполняется `f(data[i])`.

	Может использоваться для модификации элементов.
*/
template <class A2DT, class F>
void ApplyAction(DataArrayMD<A2DT> &array, const F &function);

//--------------------------------------------------------------



//! \brief Вспомогательные функции, генерируют сообщение об ошибке
namespace MDAAuxiliaries
{
	// Делаем шаблон со специализациями, а не перегруженные функции,
	// чтобы требовать точное соответствие типа.
	template<class T>
	const char *index_string_number_format();

	template<>
	inline const char *index_string_number_format<size_t>()
	{
		return "%3.1zu";
	}

	template<>
	inline const char *index_string_number_format<ptrdiff_t>()
	{
		return "%3.1ti";
	}

	template<class IV>
	string	index_string(const IV &iv)
	{
		string	result;

		result += ssprintf("(n_dimensions=%zu | ", size_t(iv.size()));

		for(size_t i = 0; i < iv.size(); ++i)
		{
			if(i)
				result += string(", ");

			if(is_slice_mask(iv[i]))
				result += ssprintf("#%zu#", (size_t)(dimension_no(iv[i])));
			else
				result += ssprintf(index_string_number_format<std::decay_t<decltype(iv[i])>>(), iv[i]);
		}
		result += string(")");
		return result;
	}

	template<class IV1, class IV2>
	string	index_compare_string(const IV1 &invalid, const IV2 &valid)
	{
		return string("\n") + index_string(invalid) + string(" -- index vector\n") + index_string(valid) + string(" -- array sizes");
	}

	template<class IV1, class IV2>
	string	size_compare_string(const IV1 &size_1, const IV2 &size_2)
	{
		return string("\n") + index_string(size_1) + string(" -- array 1 size\n") + index_string(size_2) + string(" -- array 2 size");
	}

	template<class IV>
	size_t	index_vector_volume(const IV &iv)
	{
		//TODO следует эту функцию уточнить на предмет отрицательных значений, они должны не просто умножаться
		if(iv.size())
		{
			size_t result = 1;
			for(typename IV::const_iterator it = iv.begin(), ie = iv.end(); it<ie; ++it)
			{
				if(*it<0) return slice_mask(0);
				result *= *it;
			}
			return result;
		}
		else return 0;
	}

} //namespace MDAAuxiliaries

//--------------------------------------------------------------

XRAD_END

#include "DataArrayMD.hh"

//--------------------------------------------------------------
#endif //__data_array_multidimensional_h
