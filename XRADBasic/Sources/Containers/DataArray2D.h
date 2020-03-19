#ifndef __data_array_2d_h
#define __data_array_2d_h
//--------------------------------------------------------------

#include "DataArray.h"

XRAD_BEGIN

//--------------------------------------------------------------

/*!
	\brief Двумерный массив

	В задании двумерных массивов только один аргумент шаблона:

	~~~~
	template <class row_type>
	~~~~

	Информация о value_type однозначно содержится в классе row_type.

	Единицей измерения для шага служит шаг базового массива (parent::step()).
	Например, в функции real(ComplexFunction2D) шаг строки равен 1; при этом
	шаг базового массива равен 2.

	Копирование данных:
	- MakeCopy -- выделяет память под новые размеры и копирует туда данные оригинала.
	- CopyData -- копирует данные (или их часть) в существующие массивы.
	- MoveData -- перемещает данные из оригинала без копирования, оригинал становится пустым.
*/
template <class RT>
class DataArray2D : private DataOwner<typename RT::value_type>
{
	private:
		PARENT(DataOwner<typename RT::value_type>);
		using parent::size;
	public:
		typedef DataArray2D<RT>	self;
		typedef DataArray2D<typename RT::invariable> invariable;
		typedef DataArray2D<typename RT::variable> variable;
		// про invariable см. подробный комментарий в DataOwner.h

		typedef RT row_type;
		typedef typename RT::invariable row_type_invariable;
		typedef typename RT::variable row_type_variable;

		typedef typename parent::value_type value_type;
		typedef	typename parent::value_type_variable value_type_variable;
		typedef	typename parent::value_type_invariable value_type_invariable;

		typedef	typename RT::iterator row_iterator;
		typedef typename RT::const_iterator const_row_iterator;
		typedef	typename RT::reverse_iterator reverse_row_iterator;
		typedef typename RT::const_reverse_iterator const_reverse_row_iterator;

		// Все "сплошные" итераторы по ячейкам в массивах высокой размерности исключены.
		// Итераторы двумерного массива содержат ссылку на строку.
		typedef	typename DataArray<RT>::iterator iterator;
		typedef typename DataArray<RT>::const_iterator const_iterator;
		typedef	typename DataArray<RT>::reverse_iterator reverse_iterator;
		typedef typename DataArray<RT>::const_reverse_iterator const_reverse_iterator;

		typedef ReferenceOwner<self> ref;
		typedef ReferenceOwner<invariable> ref_invariable;
		typedef ReferenceOwner<row_type> row_ref;
		typedef ReferenceOwner<row_type_invariable> row_ref_invariable;

		//! \name Конструкторы и operator=
		//! @{
	public:
		DataArray2D();
		DataArray2D(size_t vs, size_t hs);
		DataArray2D(size_t vs, size_t hs, const value_type &default_value);

		//! \brief См. \ref pg_CopyContructorOperatorEq
		DataArray2D(const self &original);
		//! \brief См. \ref pg_MoveOperations
		DataArray2D(self &&original);
		//! \brief См. \ref pg_CopyContructorOperatorEq
		DataArray2D &operator=(const self &m);
		//! \brief См. \ref pg_MoveOperations
		DataArray2D &operator=(self &&original);

		//! \brief См. \ref pg_CopyContructorOperatorEq
		template<class RT2>
		DataArray2D(const DataArray2D<RT2> &original);
		//! \brief См. \ref pg_CopyContructorOperatorEq
		template<class RT2>
		DataArray2D &operator=(const DataArray2D<RT2> &m);
		//! @}

		//! \name Инициализация
		//! @{
	public:
		//! \brief Изменение размера, для типов без конструктора контейнер содержит произвольные значения
		void	realloc(size_t vs, size_t hs);
		//! \brief Изменение размера с заполнением контейнера заданным значением
		void	realloc(size_t vs, size_t hs, const value_type &default_value)
		{ realloc(vs, hs); fill(default_value); }
		//! \brief Изменение размера с сохранением данных в общей части и обнулением новых ячеек при расширении
		void	resize(size_t vs, size_t hs);

		void	fill(const value_type &value);

		template<class RT2>
		void	MakeCopy(const DataArray2D<RT2> &original);
		template<class RT2, class F>
		void	MakeCopy(const DataArray2D<RT2> &original, const F& f);
		template<class RT2>
		void	CopyData(const DataArray2D<RT2> &original, extrapolation::method ex = extrapolation::by_zero);
		template<class RT2, class F>
		void	CopyData(const DataArray2D<RT2> &original, const F& f, extrapolation::method ex = extrapolation::by_zero);

		/*!
			\brief Копирует данные, лежащие по адресу указателя с заданным шагом, в выделенную память

			Замена ранее существовавшей функции ImportData, которая перед копированием пыталась еще
			и выделить памятью.
			Это, кажется, было избыточно.
		*/
		template<class T2>
		void	CopyData(const T2 *new_data, ptrdiff_t in_data_step = 1);

		template<class T2, class F>
		void	CopyData(const T2 *new_data, ptrdiff_t in_data_step, const F &f);

		//! \brief Структура контейнера используется для доступа к непрерывному
		//! массиву данных по адресу new_data (v столбцов, h строк)
		void	UseData(value_type *new_data, size_t v, size_t h);

		//! \brief Структура контейнера используется для доступа к разрывному
		//! массиву данных по заданному адресу. Указатель new_data указывает на
		//! элемент at(0,0) будущего массива. Любой шаг может быть:
		//! более единицы (разреженный массив по соответствующей координате),
		//! отрицательным (обратный отсчет по соответствующей координате).
		void	UseData(value_type *new_data, size_t v, size_t h, ptrdiff_t st_v, ptrdiff_t st_h);

		/*!
			\brief Структура контейнера используется для доступа к двумерному
			массиву данных по адресу new_data (v столбцов, h строк)

			Строки new_data должны лежать в памяти с одинаковым шагом.
		*/
		void	UseData(value_type **new_data, size_t v, size_t h);

		//! \brief Создать ссылку на элементы существующего контейнера, полностью повторяя его свойства
		void	UseData(DataArray2D &new_data);

		//! \brief Создать ссылку на элементы существующего контейнера, полностью повторяя его свойства
		//!
		//! Может использоваться для создания const ссылок.
		template <class RT2>
		void UseData(DataArray2D<RT2> &new_data);

		//! \brief Создать ссылку на элементы существующего контейнера, полностью повторяя его свойства
		//!
		//! Используется для создания const ссылок, тип данных текущего класса должен быть const.
		template<class RT2>
		void UseData(const DataArray2D<RT2> &new_data);

		template <class RT2>
		void	UseDataFragment(DataArray2D<RT2> &new_data, size_t v0, size_t h0, size_t v1, size_t h1);

		template <class RT2>
		void	UseDataFragment(const DataArray2D<RT2> &new_data, size_t v0, size_t h0, size_t v1, size_t h1);

		//! \brief Переместить данные из original (без копирования). Контейнер original становится пустым
		void MoveData(DataArray2D &original);

		//! \brief Переместить данные из original (без копирования). Контейнер original становится пустым
		void MoveData(DataArray2D &&original);

		//! \brief Получить ссылку на подмножество элементов существующего контейнера
		//! в некотором диапазоне.
		//! Предполагается, что для Array2D реализована move-семантика
		template <class Array2D = self>
		Array2D GetDataFragment(size_t v0, size_t h0, size_t v1, size_t h1);

		//! \brief Получить ссылку на подмножество элементов существующего контейнера
		//! в некотором диапазоне.
		//! Предполагается, что для Array2D реализована move-семантика
		template <class Array2D = invariable>
		Array2D GetDataFragment(size_t v0, size_t h0, size_t v1, size_t h1) const;
		//! @}

		/*!
			\name Access: Извлечение компонент данных (для комплексных, RGB и т.п.)

			См. \ref pg_ComponentSelector.

			@{
		*/
		//! \brief Получить ссылку на компоненту данных контейнера.
		//! См. \ref pg_ComponentSelector
		template<class RT2, class ComponentSelector>
		void GetDataComponent(DataArray2D<RT2> &component, ComponentSelector component_selector);

		//! \brief Получить ссылку на компоненту данных контейнера (данные константные).
		//! См. \ref pg_ComponentSelector
		template<class RT2, class ComponentSelector>
		void GetDataComponent(DataArray2D<RT2> &component,
				ComponentSelector component_selector) const;

	private:
		//! \brief Вспомогательный тип для задания параметра шаблона по умолчанию
		struct default_type_argument;

		//! \brief Вспомогательный шаблон для определения типа многомерного массива с компонентой
		//! по умолчанию
		template <class Array2D, class ValueType>
		struct default_array
		{
			using type = Array2D;
		};
		//! \brief Вспомогательный шаблон для определения типа многомерного массива с компонентой
		//! по умолчанию (специализация для типа по умолчанию)
		template <class ValueType>
		struct default_array<default_type_argument, ValueType>
		{
			using type = DataArray2D<DataArray<std::remove_reference_t<ValueType>>>;
		};
		//! \brief Вспомогательный шаблон для определения типа многомерного массива с компонентой
		//! по умолчанию
		template <class Array2D, class ValueType>
		using default_array_t = typename default_array<Array2D, ValueType>::type;

	public:
		//! \brief Получить ссылку на компоненту данных контейнера.
		//! Предполагается, что для Array2D реализована move-семантика.
		//! См. \ref pg_ComponentSelector
		template <class Array2D = default_type_argument, class ComponentSelector>
		auto GetDataComponent(ComponentSelector component_selector) ->
				default_array_t<Array2D,
						decltype(*std::declval<ComponentSelector>()(std::declval<value_type*>()))>;

		//! \brief Получить ссылку на компоненту данных контейнера.
		//! Array2D должен иметь константный тип данных.
		//! Предполагается, что для Array2D реализована move-семантика.
		//! См. \ref pg_ComponentSelector
		template<class Array2D = default_type_argument, class ComponentSelector>
		auto GetDataComponent(ComponentSelector component_selector) const ->
				default_array_t<Array2D,
						decltype(*std::declval<ComponentSelector>()(std::declval<value_type_invariable*>()))>;

		//! @}

		//! \name Информация
		//! @{
	public:
		bool empty() const;
		bool ready() const;

		bool	valid_indices(size_t v, size_t h) const { return (in_range(v, 0u, vsize()-1) && in_range(h, 0u, hsize()-1)); }

		static constexpr size_t n_dimensions() { return 2; }

		//! \brief Количество элементов в массиве
		inline size_t element_count() const { return m_sizes[0]*m_sizes[1]; }
		using parent::element_size;

		inline size_t vsize() const { return m_sizes[0]; }
		inline size_t hsize() const { return m_sizes[1]; }
		inline size_t column_size() const { return m_sizes[0]; }
		inline size_t row_size() const { return m_sizes[1]; }

		inline ptrdiff_t steps(size_t n) const { return m_steps[n]; }
		inline size_t sizes(size_t n) const { return m_sizes[n]; }

		//! \brief Расстояние между элементами строки, измеренное в шагах базового массива parent::step(). См. \ref column_step()
		inline ptrdiff_t vstep() const { return m_steps[0]; }
		//! \brief Расстояние между элементами столбца, измеренное в шагах базового массива parent::step(). См. \ref raw_step()
		inline ptrdiff_t hstep() const { return m_steps[1]; }
		//! \brief Синоним \ref vstep()
		inline ptrdiff_t column_step() const { return m_steps[0]; }
		//! \brief Синоним \ref hstep()
		inline ptrdiff_t row_step() const { return m_steps[1]; }

		ptrdiff_t vstep_raw() const { return m_steps[0]*parent::step(); }
		ptrdiff_t hstep_raw() const { return m_steps[1]*parent::step(); }
		//! @}

		//! \name Public data access
		//! @{
	public:
		row_type &row(size_t v);
		const row_type &row(size_t v) const;
		row_type &col(size_t h);
		const row_type &col(size_t h) const;

// 		row_type &operator[](size_t v){ return row(v); }
// 		const row_type &operator[](size_t v) const{ return row(v); }

		/*!
			\brief Проводит наклонный срез через точку origin_v, origin_h с шагами dv, dh

			Точка не обязательно находится внутри массива, в этом случае продолжение линии должно проходить через нее.
			Если линия прерывистая (шаг больше 1), исходная точка должна принадлежать линии, а не попадать в разрыв.
		*/
	private:
		void	calculate_oblique_scan_params(ptrdiff_t &v0, ptrdiff_t &h0, size_t &size, ptrdiff_t &step, ptrdiff_t origin_v, ptrdiff_t origin_h, ptrdiff_t dv, ptrdiff_t dh) const;

	public:
		void	oblique_scan(row_type &scan, ptrdiff_t origin_v, ptrdiff_t origin_h, ptrdiff_t dv, ptrdiff_t dh);
		void	oblique_scan(row_type_invariable &scan, ptrdiff_t origin_v, ptrdiff_t origin_h, ptrdiff_t dv, ptrdiff_t dh) const;

	public:
		inline value_type &at(size_t i, size_t j);
		inline const value_type &at(size_t i, size_t j) const;

		//! \brief Получение указателя на данные, возможно только для сплошного массива
		value_type *data();
		//! \brief Получение указателя на данные, возможно только для сплошного массива
		const value_type *data() const;

		inline iterator begin(){ return m_rows.begin(); }
		inline iterator end(){ return m_rows.end(); }
		inline const_iterator begin() const{ return m_rows.begin(); }
		inline const_iterator end() const{ return m_rows.end(); }
		inline const_iterator cbegin() const{ return m_rows.cbegin(); }
		inline const_iterator cend() const{ return m_rows.cend(); }

		inline reverse_iterator rbegin(){ return m_rows.rbegin(); }
		inline reverse_iterator rend(){ return m_rows.rend(); }
		inline const_reverse_iterator rbegin() const{ return m_rows.rbegin(); }
		inline const_reverse_iterator rend() const{ return m_rows.rend(); }
		inline const_reverse_iterator crbegin() const{ return m_rows.crbegin(); }
		inline const_reverse_iterator crend() const{ return m_rows.crend(); }
		//! @}

		//! \name Транспонирование
		//! @{
	public:
		void transpose();
		void vflip();
		void hflip();
		//! @}

		//! \name Copy routines
		//! @{
	public:
		//! \brief Скопировать из другого массива прямоугольный фрагмент
		//!
		//! top, left означают левый верхний угол, откуда следует копировать.
		//! Координаты угла могут быть и отрицательными.
		//! Размер фрагмента равен размеру текущего массива.
		template<class RT2>
		void	GetDataSegment(const DataArray2D<RT2> &original, ptrdiff_t top, ptrdiff_t left);

		//! \brief Скопировать в другой массив прямоугольный фрагмент. См. GetDataSegment
		template<class RT2>
		void	PutDataSegment(DataArray2D<RT2> &destination, ptrdiff_t top, ptrdiff_t left) const;
		//! @}

	//protected:
	//	const ptrdiff_t	&parent_step() const {return parent::step();}

	private:
		enum { m_dimensions_number = 2 };

		//! \brief Размер, только положительный
		size_t	m_sizes[m_dimensions_number];
		//! \brief Шаг, со знаком. Измеряется в элементах массива parent
		ptrdiff_t	m_steps[m_dimensions_number];

		//! \brief Расстояние между нулевым элементом родителя parent::at()
		//! и нулевым элементом двумерного массива at(0,0). Измеряется в элементах
		//! массива parent. Всегда должен быть больше или равен 0?
		ptrdiff_t	origin_offset;

		//! \brief Базовый механизм для row() и operator []
		DataArray<RT>	m_rows;
		//! \brief Базовый механизм для col()
		DataArray<RT>	m_columns;

	private:
		//! \brief Вызывается, когда основные данные уже зааллокированы
		//!		и нужно только разместить ячейки
		void	MapCells();
		//! \brief Смещение в единицах parent::step
		ptrdiff_t	ElementOffset(size_t v, size_t h) const
		{
			return origin_offset + (v*vstep() + h*hstep());
		}
};



//--------------------------------------------------------------
//
//	Копирование данных между разнородными двумерыми массивами
//
//--------------------------------------------------------------



template<class F1, class F2>
void	MakeCopy(DataArray2D<F1> &destination, const DataArray2D<F2> &original)
{
	destination.MakeCopy(original);
}

template<class F1, class F2>
void	CopyData(DataArray2D<F1> &destination, const DataArray2D<F2> &original, extrapolation::method ex = extrapolation::by_zero)
{
	destination.CopyData(original, ex);
}

//	эти функции копируют из массива одного типа в массив другого типа
//	нужную функцию с заданным преобразованием
//	ComplexFunction2D f(size1, size1);
//	RealFunction2D	g(size2, size2);
//	...
//	CopyData(g, f, real_functor());
//	MakeCopy(g, f, cabs_functor());

template <class FT1, class FT2, class F>
void	CopyData(DataArray2D<FT1> &destination, const DataArray2D<FT2> &original, const F &func, extrapolation::method ex = extrapolation::by_zero)
{
	destination.CopyData(original, func, ex);
}

template <class FT1, class FT2, class F>
void	MakeCopy(DataArray2D<FT1> &destination, const DataArray2D<FT2> &original, F func)
{
	destination.MakeCopy(original, func);
}



//--------------------------------------------------------------

/*!
	\brief Применение функтора `R f(T)` ко всем элементам массива

	Для каждого элемента массива выполняется `data[i] = f(data[i])`.
*/
template <class RT, class F>
void ApplyFunction(DataArray2D<RT> &array, const F &function);

/*!
	\brief Применение функтора `void f(T &)` ко всем элементам массива

	Для каждого элемента массива выполняется `f(data[i])`.

	Может использоваться для модификации элементов.
*/
template <class RT, class F>
void ApplyAction(DataArray2D<RT> &array, const F &function);

//--------------------------------------------------------------
//
//	Number traits definitions. See comment in number_traits.h.
//
//--------------------------------------------------------------


template<class RT>
inline const number_complexity_e complexity_e(const DataArray2D<RT> &) { return number_complexity_e::array2D; }

template<class RT>
inline const number_complexity::array2D *complexity_t(const DataArray2D<RT> &) { return nullptr; }

template<class RT>
inline size_t	n_components(const DataArray2D<RT> &arr) { return arr.vsize()*arr.hsize(); }

template<class RT>
inline typename RT::value_type	&component(DataArray2D<RT> &arr, size_t n)
{
	return arr.at(n/arr.vsize(), n%arr.hsize());
}

template<class RT>
inline const typename RT::value_type	&component(const DataArray2D<RT> &arr, size_t n)
{
	return arr.at(n/arr.vsize(), n%arr.hsize());
}

template<class T0, class RT1, class RT2>
inline void scalar_product_action(T0& result, const DataArray2D<RT1> &x, const DataArray2D<RT2> &y)
{
	for(size_t i = 0; i < x.vsize(); ++i)
	{
		scalar_product_action(result, x.row(i), y.row(i));
	}
}


template<class RT>
inline double	norma(const DataArray2D<RT> &x)
{
	double	result = 0;
	for(size_t i = 0; i < x.vsize(); ++i)
	{
		result += quadratic_norma(x.row(i));
	}
	return sqrt(result);
}

template<class RT>
inline double	fast_norma(const DataArray2D<RT> &x)
{
	double	result = 0;
	for(size_t i = 0; i < x.vsize(); ++i)
	{
		result += fast_norma(x.row(i));
	}
	return result;
}

template<class RT>
inline double	quadratic_norma(const DataArray2D<RT> &x)
{
	double	result = 0;
	for(size_t i = 0; i < x.vsize(); ++i)
	{
		result += quadratic_norma(x.row(i));
	}
	return result;
}


template<class RT>
DataArray2D<RT>	zero_value(const DataArray2D<RT> &datum)
{
	return DataArray2D<RT>(datum.vsize(), datum.hsize(), zero_value(datum.at(0, 0)));
}

template<class RT>
void	make_zero(DataArray2D<RT> &datum)
{
	make_zero(datum.at(0, 0));
	datum.fill(datum.at(0, 0));
}

check_if_number_traits_defined(DataArray2D<DataArray<int> >)

check_container_2d(DataArray2D, DataArray<int>)

//--------------------------------------------------------------

XRAD_END

#include "DataArray2D.hh"

//--------------------------------------------------------------
#endif //__data_array_2d_h
