//	file DataArray.h
//	Created by ACS on 09.06.03
//--------------------------------------------------------------
#ifndef __data_array_h
#define __data_array_h
//--------------------------------------------------------------

#include "DataOwner.h"
#include "ReferenceOwner.h"
#include "ContainerCheck.h"
#include <XRADBasic/Sources/Core/NumberTraits.h>

#include <type_traits>

XRAD_BEGIN

//--------------------------------------------------------------

/*!
	\brief Одномерный массив

	Отличие от DataOwner:

	Объекты этого класса принципиально одномерные.
	При создании штатными конструкторами данные размещаются в памяти непрерывно.
	При помощи DataOwner::UseData() можно создать ссылку на внешние данные с произвольным шагом.
	По ним можно итерировать, их можно создавать и ими пользоваться (аналогичные
	методы и конструкторы DataOwner сделаны protected).

	\sa pg_MoveOperations.
*/
template <class T>
class DataArray : public DataOwner<T>
{
		PARENT(DataOwner<T>);
	public:
		typedef typename parent::value_type value_type;
		typedef	typename parent::value_type_variable value_type_variable;
		typedef	typename parent::value_type_invariable value_type_invariable;

		typedef DataArray<value_type> self;
		typedef DataArray<value_type_variable> variable;
		typedef DataArray<value_type_invariable> invariable;

		typedef ReferenceOwner<self>	ref;
		typedef ReferenceOwner<invariable> ref_invariable;
			// про invariable см. подробный комментарий в DataOwner.h

	protected:
		using parent::owner_move;

	public:
		//
		//	Создание объектов
		//
		DataArray() = default;
		explicit DataArray(size_t s) : DataOwner<T>(s){}
		DataArray(size_t s, const value_type &v) : parent(s){ fill(v); }

		//! \brief Вызывает ошибку компиляции при инстанциировании.
		//! Используется для защиты от использования чисел с плавающей точкой в качестве
		//! размера в конструкторе от (size_t s, const value_type &v).
		template <class T2, class = std::enable_if_t<std::is_floating_point<std::decay_t<T2>>::value>>
		DataArray(T2, const value_type &)
		{
			static_assert(!std::is_floating_point<std::decay_t<T2>>::value, "Floating point values are not allowed for DataArray size.");
		}

		//! \brief См. \ref pg_CopyContructorOperatorEq
		DataArray(const self &f){ MakeCopy(f); }

		//! \brief См. \ref pg_MoveOperations
		DataArray(self &&f) { owner_move(f); }

		//! \brief См. \ref pg_CopyContructorOperatorEq
		template<class T2> DataArray(const DataArray<T2> &f){ MakeCopy(f); }

		/*!
			\brief Конструктор от списков значений

			Не следует делать конструктор от initializer_list шаблонным:

			~~~~
			template<class T2> DataArray(const std::initializer_list<T2> &l).
			~~~~

			В этом случае перестают распознаваться списки, в которых смешаны аргументы разных типов,
			например {1, 2, 3.1}.
		*/
		DataArray(const std::initializer_list<value_type> &l) : parent(l.size()){ std::copy(l.begin(), l.end(), begin()); }

		/*!
			\brief Конструктор от диапазона итераторов

			Возможна путаница с конструктором от (size_t, value_type).
			Поэтому шаблон включается (std::enable_if), только если IT — не целочисленный тип.
		*/
		template<class IT, class = std::enable_if_t<!std::is_arithmetic<std::decay_t<IT>>::value>>
		DataArray(const IT &first, const IT &last)
		{
			realloc(last - first);
			std::copy(first, last, begin());
		}

		/*!
			\brief Конструктор от итератора и количества элементов

			Возможна путаница с конструктором от (size_t, value_type).
			Поэтому шаблон включается (std::enable_if), только если IT — не целочисленный тип.
		*/
		template<class IT, class = std::enable_if_t<!std::is_arithmetic<std::decay_t<IT>>::value>>
		DataArray(const IT &first, size_t s) :parent(s)
		{
			std::copy(first, first+s, begin());
		}

		//! \brief См. \ref pg_CopyContructorOperatorEq
		DataArray &operator = (const self &original);

		//! \brief См. \ref pg_MoveOperations
		DataArray &operator = (self &&original) { owner_move(original); return *this; }

		//! \brief См. \ref pg_CopyContructorOperatorEq
		template<class T2> DataArray &operator = (const DataArray<T2> &original);

	public:
		using parent::uses_external_data; // делаем public
		using parent::size;
		using parent::step;
		using parent::at;

		using parent::empty;

		value_type *data();
		const value_type *data() const;

		using parent::realloc;
		//! \brief Переаллокировать, не сохраняя данные
		void	realloc(size_t s, const value_type &v){ parent::realloc(s); fill(v); }
		//! \brief Изменение размера с сохранением данных в общей части старого и нового массива.
		//! При увеличении размера элементы массива, начиная со старого size(), не инициализированы.
		void	resize(size_t newSize);

		//! \brief Заполнение массива заданным значением
		void	fill(const T &value);



		//
		//	Копирование данных
		//

		//! \brief Создает точную копию оригинала, для чего заново выделяет память
		template<class T2>
		inline void MakeCopy(const DataArray<T2> &original);

		/*!
			\brief Создает образ оригинала с поэлементным преобразованием данных,
			заново выделяет память

			Пример:
			~~~~
			DataArray<complexF64>	array(size);
			DataArray<double>	g(size);
			g.CopyData(array, real_functor());// действительная часть
			g.MakeCopy(array, cabs_functor());// модуль комплексного числа
			~~~~
		*/
		template<class T2, class F>
		inline void MakeCopy(const DataArray<T2> &original, const F& f);

		//! \brief Выделает память и копирует в нее диапазон итераторов
		template<class IT>
		inline void MakeCopy(IT &&first, IT &&last)
		{
			realloc(last - first);
			std::copy(first, last, begin());
		}

		//! \brief Выделает память и копирует заданное количество элементов по итератору
		template<class IT>
		inline void MakeCopy(const IT &&first, size_t s)
		{
			realloc(s);
			std::copy(first, first+s, begin());
		}



		//! \brief Копирует данные из оригинала в прежде выделенную память.
		//! При нехватке места данные подрезаются, при избытке экстраполяция заданным алгоритмом
		template<class T2>
		inline void CopyData(const DataArray<T2> &original, extrapolation::method ex = extrapolation::by_zero);

		//! \brief Копирует с поэлементным преобразованием данные из оригинала
		//! в прежде выделенную память.
		//! При нехватке места данные подрезаются, при избытке экстраполяция заданным алгоритмом
		template<class T2, class F>
		inline void CopyData(const DataArray<T2> &original, const F& f, extrapolation::method ex = extrapolation::by_zero);

		/*!
			\brief Копирует данные, лежащие по адресу указателя с заданным шагом, в выделенную память

			Замена ранее существовавшей функции ImportData, которая перед копированием пыталась еще
			и выделить памятью.
			Это, кажется, было избыточно.
		*/
		template<class T2>
		void	CopyData(const T2 *new_data, ptrdiff_t in_step = 1);

		/*!
			\brief Копирует с поэлементным преобразованием данные,
			лежащие по адресу указателя с заданным шагом, в выделенную память.

			См. CopyData без преобразования данных.
		*/
		template<class T2, class F>
		void	CopyData(const T2 *new_data, ptrdiff_t in_step, const F &f);

		//! \brief Переместить данные из original (без копирования). Контейнер original становится пустым
		void MoveData(self &original) { owner_move(original); }
		//! \brief Переместить данные из original (без копирования). Контейнер original становится пустым
		void MoveData(self &&original) { owner_move(original); }



		//! \brief Получить ссылку на подмножество элементов существующего контейнера
		//! в некотором диапазоне, возможен шаг больший единицы.
		//! Предполагается, что для Array реализована move-семантика
		template <class Array = self>
		Array GetDataFragment(size_t from, size_t to, ptrdiff_t in_step = 1);

		//! \brief Получить ссылку на подмножество элементов существующего контейнера
		//! в некотором диапазоне, возможен шаг больший единицы.
		//! Предполагается, что для Array реализована move-семантика
		template <class Array = invariable>
		Array GetDataFragment(size_t from, size_t to, ptrdiff_t in_step = 1) const;



		/*!
			\name Access: Извлечение компонент данных (для комплексных, RGB и т.п.)

			См. \ref pg_ComponentSelector.

			@{
		*/

		//! \brief Получить ссылку на компоненту данных контейнера.
		//! См. \ref pg_ComponentSelector
		template<class T2, class ComponentSelector>
		void GetDataComponent(DataArray<T2> &component, ComponentSelector component_selector);

		//! \brief Получить ссылку на компоненту данных контейнера (данные константные).
		//! См. \ref pg_ComponentSelector
		template<class T2, class ComponentSelector>
		void GetDataComponent(DataArray<T2> &component,
				ComponentSelector component_selector) const;

	private:
		//! \brief Вспомогательный тип для задания параметра шаблона по умолчанию
		struct default_type_argument;

		//! \brief Вспомогательный шаблон для определения типа многомерного массива с компонентой
		//! по умолчанию
		template <class Array, class ValueType>
		struct default_array
		{
			using type = Array;
		};
		//! \brief Вспомогательный шаблон для определения типа многомерного массива с компонентой
		//! по умолчанию (специализация для типа по умолчанию)
		template <class ValueType>
		struct default_array<default_type_argument, ValueType>
		{
			using type = DataArray<std::remove_reference_t<ValueType>>;
		};
		//! \brief Вспомогательный шаблон для определения типа многомерного массива с компонентой
		//! по умолчанию
		template <class Array, class ValueType>
		using default_array_t = typename default_array<Array, ValueType>::type;

	public:
		//! \brief Получить ссылку на компоненту данных контейнера.
		//! Предполагается, что для Array реализована move-семантика.
		//! См. \ref pg_ComponentSelector
		template <class Array = default_type_argument, class ComponentSelector>
		auto GetDataComponent(ComponentSelector component_selector) ->
				default_array_t<Array,
						decltype(*std::declval<ComponentSelector>()(std::declval<value_type*>()))>;

		//! \brief Получить ссылку на компоненту данных контейнера.
		//! Array должен иметь константный тип данных.
		//! Предполагается, что для Array реализована move-семантика.
		//! См. \ref pg_ComponentSelector
		template<class Array = default_type_argument, class ComponentSelector>
		auto GetDataComponent(ComponentSelector component_selector) const ->
				default_array_t<Array,
						decltype(*std::declval<ComponentSelector>()(std::declval<value_type_invariable*>()))>;

		//! @}

		//
		//	Циклические сдвиги
		//

		/*!
			\brief Циклическое перемещение элементов на roll_distance отсчетов вправо
			(отрицательный сдвиг означает сдвиг влево)
		*/
		void	roll(ptrdiff_t roll_distance);

		/*!
			\brief Циклическое перемещение элементов на половину длины массива

			\param forward Определяет обратимость преобразования при
			нечетной длине массива. В остальных случаях на результат не влияет.
		*/
		void	roll_half(bool forward);


		//	Методы обращения с итераторами. Повторяют вызовы родительского класса.
		//	Родительские методы сделаны protected, поскольку
		//	бесконтрольное прямое их использование может оказаться опасным
		//	для подмножеств многомерных массивов.

		typedef typename parent::iterator iterator;
		typedef typename parent::const_iterator const_iterator;
		typedef typename parent::reverse_iterator reverse_iterator;
		typedef typename parent::const_reverse_iterator const_reverse_iterator;

		inline iterator begin(){ return parent::begin(); }
		inline iterator end(){ return parent::end(); }
		inline const_iterator begin() const{ return parent::begin(); }
		inline const_iterator end() const{ return parent::end(); }
		inline const_iterator cbegin() const{ return parent::cbegin(); }
		inline const_iterator cend() const{ return parent::cend(); }


		inline reverse_iterator rbegin(){ return parent::rbegin(); }
		inline reverse_iterator rend(){ return parent::rend(); }
		inline const_reverse_iterator rbegin() const{ return parent::rbegin(); }
		inline const_reverse_iterator rend() const{ return parent::rend(); }
		inline const_reverse_iterator crbegin() const{ return parent::crbegin(); }
		inline const_reverse_iterator crend() const{ return parent::crend(); }
};



//--------------------------------------------------------------
//
//	Number traits definitions. See comment in number_traits.h.
//
//--------------------------------------------------------------


template<class T>
inline const number_complexity_e complexity_e(const DataArray<T> &) { return number_complexity_e::array; }

template<class T>
inline const number_complexity::array *complexity_t(const DataArray<T> &) { return nullptr; }

template<class T>
inline size_t	n_components(const DataArray<T> &arr) { return arr.size(); }

template<class T>
inline T	&component(DataArray<T> &arr, size_t n)
{
	return arr.at(n);
}

template<class T>
inline const T	&component(const DataArray<T> &arr, size_t n)
{
	return arr.at(n);
}

template<class T0, class T1, class T2>
inline void scalar_product_action(T0& result, const DataArray<T1> &x, const DataArray<T2> &y)
{
	typename DataArray<T1>::const_iterator it1 = x.begin(), ie=x.end();
	typename DataArray<T2>::const_iterator it2 = y.begin();

	for(; it1 < ie; ++it1, ++it2)
	{
		scalar_product_action(result, *it1, *it2);
	}
}


template<class T>
inline double	norma(const DataArray<T> &x)
{
	typename DataArray<T>::const_iterator it = x.begin();
	double	result = 0;
	for(size_t i = 0; i < x.size(); ++i, ++it)
	{
		result += quadratic_norma(*it);
	}
	return sqrt(result);
}

template<class T>
inline double	fast_norma(const DataArray<T> &x)
{
	typename DataArray<T>::const_iterator it = x.begin();
	double	result = 0;
	for(size_t i = 0; i < x.size(); ++i, ++it)
	{
		result += fast_norma(*it);
	}
	return result;
}

template<class T>
inline double	quadratic_norma(const DataArray<T> &x)
{
	typename DataArray<T>::const_iterator it = x.begin();
	double	result = 0;
	for(size_t i = 0; i < x.size(); ++i, ++it)
	{
		result += quadratic_norma(*it);
	}
	return (result);
}


template<class T>
DataArray<T>	zero_value(const DataArray<T> &datum)
{
	return	DataArray<T>(datum.size(), zero_value(datum[0]));
}

template<class T>
void	make_zero(DataArray<T> &datum)
{
	make_zero(datum[0]);
	datum.fill(datum[0]);
}



check_if_number_traits_defined(DataArray<int>)

check_container_1d(DataArray, int)



//--------------------------------------------------------------
//
//	Сортировка элементов
//
//--------------------------------------------------------------

template<class T>
void	reorder_ascent(DataArray<T> &x);
template<class T>
void	reorder_descent(DataArray<T> &x);
template<class T, class Compare>
void	reorder(DataArray<T> &x, Compare comp);

//--------------------------------------------------------------
//
//	Копирование данных, не член класса.
//	Дублирует аналогичные функции-члены класса, возможно, что-то одно нужно убрать.
//
//--------------------------------------------------------------

template <class T1, class T2>
void	MakeCopy(DataArray<T1> &destination, const DataArray<T2> &source);
template <class T1, class T2, class F>
void	MakeCopy(DataArray<T1> &destination, const DataArray<T2> &original, const F &function);
template <class T1, class T2>
void	CopyData(DataArray<T1> &destination, const DataArray<T2> &source);
template <class T1, class T2, class F>
void	CopyData(DataArray<T1> &destination, const DataArray<T2> &original, const F &function);

//--------------------------------------------------------------

// template<class T>
// void	swap(DataArray<T> &arr1, DataArray<T> &arr2);

//--------------------------------------------------------------

/*!
	\brief Применение функтора `R f(T)` ко всем элементам массива

	Для каждого элемента массива выполняется `data[i] = f(data[i])`.

	В качестве function принимает классы-функторы (см. пространство имен Functors),
	лямбда-выражения и указатели на обычные функции (например, fabs).

	При передаче указателя на обычную функцию для перегруженных и шаблонных функций
	требуется в явном виде выбрать нужную функцию. При передаче класса-функтора
	с шаблонным или перегруженным operator() выбор нужной реализации operator()
	производится автоматически по типу принимаемого им аргумента.

	Для часто используемых функций функторы реализованы в файле Functors.h,
	см. также namespace Functors.

	\note
	Ранее использовалась с прототипами обычных функций, например
	`ApplyFunction(array, fabs)`.
	В своё время этот подход оказался неудачным и с точки зрения производительности,
	и в плане совместимости. Поэтому функция принимает на вход
	произвольный функтор.
*/
template <class T, class F>
void ApplyFunction(DataArray<T> &array, const F &function);

/*!
	\brief Применение функтора `void f(T &)` ко всем элементам массива

	Для каждого элемента массива выполняется `f(data[i])`.

	Может использоваться для модификации элементов.
*/
template <class T, class F>
void ApplyAction(DataArray<T> &array, const F &function);

//--------------------------------------------------------------

XRAD_END

#include "DataArray.hh"

//--------------------------------------------------------------
#endif // __data_array_h
