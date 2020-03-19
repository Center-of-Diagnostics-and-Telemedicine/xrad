#ifndef __data_array_2d_cc
#define __data_array_2d_cc

#include "BasicArrayInteractions2D.h"

XRAD_BEGIN

//--------------------------------------------------------------
//
//	Конструкторы и operator =
//
//--------------------------------------------------------------

template<class RT>
DataArray2D<RT>::DataArray2D()
{
	realloc(0, 0);
}


template<class RT>
DataArray2D<RT>::DataArray2D(size_t v, size_t h)
{
	realloc(v, h);
}


template<class RT>
DataArray2D<RT>::DataArray2D(size_t v, size_t h, const value_type &default_value)
{
	realloc(v, h, default_value);
}



template<class RT>
DataArray2D<RT>::DataArray2D(const DataArray2D<RT> &original)
{
	this->MakeCopy(original);
}

template<class RT>
DataArray2D<RT>::DataArray2D(DataArray2D<RT> &&original)
{
	MoveData(original);
}

template<class RT>
DataArray2D<RT> &DataArray2D<RT>::operator=(const DataArray2D<RT> &original)
{
	return operator=<RT>(original);
}

template<class RT>
DataArray2D<RT> &DataArray2D<RT>::operator=(DataArray2D<RT> &&original)
{
	MoveData(original);
	return *this;
}

template<class RT>
template<class RT2>
DataArray2D<RT>::DataArray2D(const DataArray2D<RT2> &original)
{
	this->MakeCopy(original);
}

template<class RT>
template<class RT2>
DataArray2D<RT> &DataArray2D<RT>::operator=(const DataArray2D<RT2> &original)
{
	// присваивание выполняется только между массивами одинаковых размеров.
	// единственное исключение -- если первый массив совсем пустой
	if(!vsize() || !hsize())
	{
		if (!original.vsize() || !original.hsize())
			return *this;
		realloc(original.vsize(), original.hsize());
	}
	else if(original.vsize() != vsize() || original.hsize() != hsize())
	{
		string problem_description = ssprintf("%s::operator=(%s original) -- array size mismatch: original sizes (%zu, %zu), current sizes (%zu, %zu)",
				EnsureType<const char*>(typeid(self).name()),
				EnsureType<const char*>(typeid(original).name()),
				EnsureType<size_t>(original.vsize()),
				EnsureType<size_t>(original.hsize()),
				EnsureType<size_t>(vsize()),
				EnsureType<size_t>(hsize()));
		ForceDebugBreak();
		throw(invalid_argument(problem_description));
	}
	Apply_AA_2D_F2(*this, original, Functors::assign());
	return *this;
}



//--------------------------------------------------------------



//! \todo Проверить состояние объекта при исключениях в данной функции.
//! Объект должен оказаться в допустимом состоянии (не должно быть "висячих" указателей).
template<class RT>
void	DataArray2D<RT>::realloc(size_t in_vs, size_t in_hs)
{
	// прежние данные удаляются в два приема:
	//	1.	при вызове parent::realloc (с проверкой, нельзя ли оставить
	//		без переаллокирования, если размеры одинаковы
	//	2.	в MapCells с такой же проверкой обрабатываются
	//		контейнеры строк и столбцов
	parent::realloc(in_vs*in_hs);

	if(!in_vs || !in_hs)
	{
		m_sizes[0] = 0;
		m_sizes[1] = 0;
		m_steps[0] = 0;// то же, что m_sizes[1]
		m_steps[1] = 0;
	}
	else
	{
		m_sizes[0] = in_vs;
		m_sizes[1] = in_hs;
		m_steps[0] = hsize();// то же, что m_sizes[1]
		m_steps[1] = 1;
	}

	origin_offset = 0;

	MapCells(); ///
}

//! \todo Подумать об использовании move-семантики при перемещении элементов в (*1529063555).
template<class RT>
void	DataArray2D<RT>::resize(size_t in_vs, size_t in_hs)
{
	if(parent::uses_external_data())
	{
		ForceDebugBreak();
		throw(logic_error(string(typeid(self).name()) + string("::resize -- Can't resize array with external data!")));
	}
	if(in_vs == vsize() && in_hs == hsize())
		return; // ничего не делает, если размеры одинаковы,
	//
	DataArray2D<RT> new_data(in_vs, in_hs);
	new_data.CopyData(*this); // (*1529063555)
	MoveData(new_data);
}

template<class RT>
void	DataArray2D<RT>::fill(const value_type &value)
{
	Apply_A_2D_F1(*this, [&value](value_type &v) { v = value; });
}



//--------------------------------------------------------------
//
//	Копирование данных
//
//--------------------------------------------------------------



template<class RT>
template<class RT2>
void	DataArray2D<RT>::MakeCopy(const DataArray2D<RT2> &original)
{
	if(static_cast<const void*>(&original) == static_cast<const void*>(this))
	{
		// если предлагается скопировать себя в себя же, выходим сразу.
		// приведение к void* нужно для шаблонных вызовов с разными
		// типами массивов
		return;
	}
	realloc(original.vsize(), original.hsize());
	Apply_AA_2D_F2(*this, original, Functors::assign());
}

template<class RT>
template<class RT2, class F>
void	DataArray2D<RT>::MakeCopy(const DataArray2D<RT2> &original, const F &f)
{
	realloc(original.vsize(), original.hsize());
	Apply_AA_2D_F2(*this, original, f);
}

template<class RT>
template<class RT2>
void	DataArray2D<RT>::CopyData(const DataArray2D<RT2> &original, extrapolation::method ex)
{
	if(static_cast<const void*>(&original) == static_cast<const void*>(this))
	{
		// если предлагается скопировать себя в себя же, выходим сразу.
		// приведение к void* нужно для шаблонных вызовов с разными
		// типами массивов
		return;
	}
	if(!vsize() || !hsize()) return;

	Apply_AA_2D_Different_F2(*this, original, Functors::assign(), ex);
}

template<class RT>
template<class RT2, class F>
void	DataArray2D<RT>::CopyData(const DataArray2D<RT2> &original, const F &f, extrapolation::method ex)
{
	//TODO в двумерных алгоритмах сделать проверку на "пусто"
	Apply_AA_2D_Different_F2(*this, original, f, ex);
}


template<class RT>
template<class T2>
void	DataArray2D<RT>::CopyData(const T2 *new_data, ptrdiff_t in_data_step)
{
	step_iterator<const T2, iterator_checker_none<const T2, ptrdiff_t> > data_it(new_data, in_data_step, size(), 0);
	for(size_t i = 0; i < vsize(); ++i)
	{
		//в std::copy предупреждение C4244 (потеря точности) не по делу. поэтому цикл с явным указанием преобразования типов
		// copy( data_it, data_it+hsize(), row(i).begin());
		auto &current_row = row(i);
		auto it = current_row.begin(), ie = current_row.end();
		for(;it<ie; ++it, ++data_it)
		{
			*it = static_cast<value_type>(*data_it);
		}
	}
}

template<class RT>
template<class T2, class F>
void	DataArray2D<RT>::CopyData(const T2 *new_data, ptrdiff_t in_data_step, const F &function)
{
	step_iterator<const T2, iterator_checker_none<const T2, ptrdiff_t> > data_it(new_data, in_data_step, size(), 0);
	for(size_t i = 0; i < vsize(); ++i)
	{
		auto &current_row = row(i);
		auto it = current_row.begin(), ie = current_row.end();
		for(;it<ie; ++it, ++data_it)
		{
			*it = function(*data_it);
		}
	}
}



//--------------------------------------------------------------



template<class RT>
void DataArray2D<RT>::UseData(value_type *new_data, size_t in_vs, size_t in_hs)
{
	if(!in_vs && !in_hs)
	{
		realloc(0, 0);
		return;
	}

	// этим же высвобождаются ранее использованные данные, если таковые были
	parent::UseData(new_data, in_vs*in_hs);

	m_sizes[0] = in_vs;
	m_sizes[1] = in_hs;
	m_steps[0] = hsize();// то же, что m_sizes[1]
	m_steps[1] = 1;

	origin_offset = 0;

	MapCells();
}

//--------------------------------------------------------------

template<class RT>
void DataArray2D<RT>::UseData(
	value_type *new_data,
	size_t in_vs, size_t in_hs,
	ptrdiff_t in_vstep, ptrdiff_t in_hstep)
{
	if(!in_vs && !in_hs)
	{
		realloc(0, 0);
		return;
	}

	ptrdiff_t st = min(abs(in_vstep), abs(in_hstep));

	if(!st || in_vstep%st || in_hstep%st)
	{
		// Нулевой шаг запрещаем. отрицательный разрешаем (с 11.04.14 -- кнс).
		// Проверка, чтобы больший шаг нацело делился на меньший, иначе ошибка.
		ForceDebugBreak();
		throw invalid_argument(ssprintf("%s::UseData: invalid step combination: vstep = %ti, hstep = %ti.",
				EnsureType<const char*>(typeid(self).name()),
				EnsureType<ptrdiff_t>(in_vstep),
				EnsureType<ptrdiff_t>(in_hstep)));
	}

	ptrdiff_t norm_vstep = in_vstep/st;
	ptrdiff_t norm_hstep = in_hstep/st;
	m_sizes[0] = in_vs;
	m_sizes[1] = in_hs;
	m_steps[0] = norm_vstep;
	m_steps[1] = norm_hstep;

	size_t	max_possible_size = max(in_vs*abs(norm_vstep), in_hs*abs(norm_hstep));
	// Для непрерывных массивов это то же самое, что vs*hs.
	// Для массивов, у которых vstep > hsize или hstep > vsize указывает
	// на размах данных исходного массива, чтобы исключить выход за допустимые границы данных,
	// но не совпадает с количеством элементов в двумерном массиве.

	ptrdiff_t	offset_v = norm_vstep>0 ? 0 : -norm_vstep*(in_vs-1);
	ptrdiff_t	offset_h = norm_hstep>0 ? 0 : -norm_hstep*(in_hs-1);
	origin_offset = offset_v + offset_h;

	parent::UseData(new_data - origin_offset*st, max_possible_size, st);
		// метод DataOwner, отвечающего за индексирование базового массива new_data

	MapCells();
}

//--------------------------------------------------------------

//! \todo Исправить спецификатор формата `%d` в ssprintf на соответствующий типу данных (size_t — `%zi`, ptrdiff_t — `%ti`).
//! Здесь и везде.
template<class RT>
void DataArray2D<RT>::UseData(value_type **new_data, size_t in_vsize, size_t in_hsize)
{
	if(!in_vsize && !in_hsize)
	{
		realloc(0, 0);
		return;
	}

// 	bool	fault = false;
	if(!new_data)
	{
		ForceDebugBreak();
		throw(invalid_argument(string(typeid(self).name()) + ssprintf("::UseData(value_type **): invalid arguments combination. data=%d, vsize = %d, hsize = %d",
				size_t(new_data), in_vsize, in_hsize)));
	}

	ptrdiff_t	hst = 1;
	ptrdiff_t	vst = new_data[1]-new_data[0];
	for(size_t i = 1; i < in_vsize-1; ++i)
	{
		ptrdiff_t	vst2 = new_data[i+1] - new_data[i];
		if(!vst || vst != vst2 || abs(vst)<in_hsize)
		{
			ForceDebugBreak();
			throw(invalid_argument(string(typeid(self).name()) + ssprintf("::UseData(value_type **): invalid vsteps combination. vstep1 = %d, vstep2 = %d",
					vst, vst2)));
		}
	}

	UseData(*new_data, in_vsize, in_hsize, vst, hst);
}

//--------------------------------------------------------------

template<class RT>
void DataArray2D<RT>::UseData(DataArray2D &new_data)
{
	value_type	*new_data_ptr = &(new_data.at(0, 0));

	size_t	vs = new_data.vsize();
	size_t	hs = new_data.hsize();
	ptrdiff_t	vst = new_data.vstep_raw();
	ptrdiff_t	hst = new_data.hstep_raw();

	UseData(new_data_ptr, vs, hs, vst, hst);
}

//--------------------------------------------------------------

template<class RT>
template <class RT2>
void DataArray2D<RT>::UseData(DataArray2D<RT2> &new_data)
{
	static_assert(sizeof(typename DataArray2D<RT2>::value_type) == sizeof(value_type),
			"DataArray2D<RT>::UseData: Incompatible data types.");
	UseData(&new_data.at(0, 0), new_data.vsize(), new_data.hsize(),
			new_data.vstep_raw(), new_data.hstep_raw());
}

//--------------------------------------------------------------

template<class RT>
template<class RT2>
void DataArray2D<RT>::UseData(const DataArray2D<RT2> &new_data)
{
	static_assert(sizeof(typename DataArray2D<RT2>::value_type) == sizeof(value_type),
			"DataArray2D<RT>::UseData: Incompatible data types.");
	UseData(&new_data.at(0, 0), new_data.vsize(), new_data.hsize(),
			new_data.vstep_raw(), new_data.hstep_raw());
}

//--------------------------------------------------------------

template<class RT>
template <class RT2>
void DataArray2D<RT>::UseDataFragment(DataArray2D<RT2> &new_data, size_t v0, size_t h0, size_t v1, size_t h1)
{
	static_assert(sizeof(typename DataArray2D<RT2>::value_type) == sizeof(value_type),
			"DataArray2D<RT>::UseDataFragment: Incompatible data types.");
	bool	fault = false;
	if(v0 >= v1 || v1 > new_data.vsize())
		fault = true;
	else if(h0 >= h1 || h1 > new_data.hsize())
		fault = true;
	if(fault)
	{
		ForceDebugBreak();
		throw invalid_argument(ssprintf(
				"%s::UseDataFragment(DataArray2D) -- invalid ranges. "
				"vertical segment=(%zu,%zu), allowed range is (%zu,%zu), "
				"horizontal segment=(%zu,%zu) allowed range is (%zu,%zu)",
				EnsureType<const char*>(typeid(self).name()),
				EnsureType<size_t>(v0), EnsureType<size_t>(v1), size_t(0), EnsureType<size_t>(new_data.vsize()),
				EnsureType<size_t>(h0), EnsureType<size_t>(h1), size_t(0), EnsureType<size_t>(new_data.hsize())));
	}

	auto new_data_ptr = &new_data.at(v0, h0);
	size_t	vs = v1 - v0;
	size_t	hs = h1 - h0;
	ptrdiff_t	vst = new_data.vstep_raw();
	ptrdiff_t	hst = new_data.hstep_raw();
	UseData(new_data_ptr, vs, hs, vst, hst);
}

template<class RT>
template <class RT2>
void DataArray2D<RT>::UseDataFragment(const DataArray2D<RT2> &new_data, size_t v0, size_t h0, size_t v1, size_t h1)
{
	static_assert(sizeof(typename DataArray2D<RT2>::value_type) == sizeof(value_type),
			"DataArray2D<RT>::UseDataFragment: Incompatible data types.");
	bool	fault = false;
	if(v0 >= v1 || v1 > new_data.vsize())
		fault = true;
	else if(h0 >= h1 || h1 > new_data.hsize())
		fault = true;
	if(fault)
	{
		ForceDebugBreak();
		throw invalid_argument(ssprintf(
				"%s::UseDataFragment(DataArray2D) -- invalid ranges. "
				"vertical segment=(%zu,%zu), allowed range is (%zu,%zu), "
				"horizontal segment=(%zu,%zu) allowed range is (%zu,%zu)",
				EnsureType<const char*>(typeid(self).name()),
				EnsureType<size_t>(v0), EnsureType<size_t>(v1), size_t(0), EnsureType<size_t>(new_data.vsize()),
				EnsureType<size_t>(h0), EnsureType<size_t>(h1), size_t(0), EnsureType<size_t>(new_data.hsize())));
	}

	auto new_data_ptr = &new_data.at(v0, h0);
	size_t	vs = v1 - v0;
	size_t	hs = h1 - h0;
	ptrdiff_t	vst = new_data.vstep_raw();
	ptrdiff_t	hst = new_data.hstep_raw();
	UseData(new_data_ptr, vs, hs, vst, hst);
}

//--------------------------------------------------------------

template<class RT>
void DataArray2D<RT>::MoveData(DataArray2D &original)
{
	parent::owner_move(original);
	m_rows.MoveData(original.m_rows);
	m_columns.MoveData(original.m_columns);
	for (size_t i=0; i<m_dimensions_number; ++i)
	{
		m_sizes[i] = original.m_sizes[i];
		m_steps[i] = original.m_steps[i];
		original.m_sizes[i] = 0;
		original.m_steps[i] = 0;
	}
	origin_offset = original.origin_offset;
	original.origin_offset = 0;
}

template<class RT>
void DataArray2D<RT>::MoveData(DataArray2D &&original)
{
	MoveData(original);
}

//--------------------------------------------------------------

template<class RT>
template <class Array2D>
Array2D DataArray2D<RT>::GetDataFragment(size_t v0, size_t h0, size_t v1, size_t h1)
{
	Array2D array;
	array.UseDataFragment(*this, v0, h0, v1, h1);
	return array;
}

template<class RT>
template <class Array2D>
Array2D DataArray2D<RT>::GetDataFragment(size_t v0, size_t h0, size_t v1, size_t h1) const
{
	Array2D array;
	array.UseDataFragment(*this, v0, h0, v1, h1);
	return array;
}



//--------------------------------------------------------------



template<class RT>
template<class RT2, class ComponentSelector>
void DataArray2D<RT>::GetDataComponent(DataArray2D<RT2> &component,
		ComponentSelector component_selector)
{
	using component_type = std::remove_reference_t<
			decltype(*component_selector(std::declval<value_type*>()))>;
	static_assert(sizeof(typename DataArray2D<RT2>::value_type) == sizeof(component_type),
			"DataArray2D<RT>::GetDataComponent: Incompatible data types.");
	// Проверка на то, что шаг данных у компоненты кратен размеру компоненты.
	// Это ограничение текущей реализации DataOwner.
	static_assert(sizeof(value_type) % sizeof(component_type) == 0,
			"DataArray2D<RT>::GetDataComponent: Data step must be a multiple of data size for DataOwner.");
#ifdef _DEBUG
	{
		// Проверка на то, что компонента находится внутри исходных данных.
		auto *data_origin = &parent::at(0);
		auto *component_origin = (void*)component_selector(data_origin);
		if (component_origin < (void*)data_origin ||
				component_origin >= (void*)(data_origin + 1))
		{
			throw runtime_error("DataArray2D<RT>::GetDataComponent: the component selector gives "
					"a component outside original data.");
		}
	}
#endif
	component.UseData(component_selector(&at(0, 0)), vsize(), hsize(),
			vstep() * ptrdiff_t(sizeof(value_type) / sizeof(component_type)),
			hstep() * ptrdiff_t(sizeof(value_type) / sizeof(component_type)));
}

template<class RT>
template<class RT2, class ComponentSelector>
void DataArray2D<RT>::GetDataComponent(DataArray2D<RT2> &component,
		ComponentSelector component_selector) const
{
	using component_type = std::remove_reference_t<
			decltype(*component_selector(std::declval<value_type*>()))>;
	static_assert(sizeof(typename DataArray2D<RT2>::value_type) == sizeof(component_type),
			"DataArray2D<RT>::GetDataComponent: Incompatible data types.");
	// Проверка на то, что шаг данных у компоненты кратен размеру компоненты.
	// Это ограничение текущей реализации DataOwner.
	static_assert(sizeof(value_type) % sizeof(component_type) == 0,
			"DataArray2D<RT>::GetDataComponent: Data step must be a multiple of data size for DataOwner.");
#ifdef _DEBUG
	{
		// Проверка на то, что компонента находится внутри исходных данных.
		auto *data_origin = &parent::at(0);
		auto *component_origin = (void*)component_selector(data_origin);
		if (component_origin < (void*)data_origin ||
				component_origin >= (void*)(data_origin + 1))
		{
			throw runtime_error("DataArray2D<RT>::GetDataComponent: the component selector gives "
					"a component outside original data.");
		}
	}
#endif
	component.UseData(component_selector(&at(0, 0)), vsize(), hsize(),
			vstep() * ptrdiff_t(sizeof(value_type) / sizeof(component_type)),
			hstep() * ptrdiff_t(sizeof(value_type) / sizeof(component_type)));
}

template<class RT>
template<class Array, class ComponentSelector>
auto DataArray2D<RT>::GetDataComponent(ComponentSelector component_selector) ->
		default_array_t<Array,
				decltype(*std::declval<ComponentSelector>()(std::declval<value_type*>()))>
{
	default_array_t<Array,
			decltype(*std::declval<ComponentSelector>()(std::declval<value_type*>()))> array;
	GetDataComponent(array, component_selector);
	return array;
}

template<class RT>
template<class Array, class ComponentSelector>
auto DataArray2D<RT>::GetDataComponent(ComponentSelector component_selector) const ->
		default_array_t<Array,
				decltype(*std::declval<ComponentSelector>()(std::declval<value_type_invariable*>()))>
{
	default_array_t<Array,
			decltype(*std::declval<ComponentSelector>()(std::declval<value_type_invariable*>()))> array;
	GetDataComponent(array, component_selector);
	return array;
}



//--------------------------------------------------------------



template<class RT>
bool DataArray2D<RT>::empty() const
{
#ifdef _DEBUG
	if(parent::empty() && (m_sizes[0] || m_sizes[1] || m_steps[0] || m_steps[1] || !m_rows.empty() || !m_columns.empty()))
	{
		ForceDebugBreak();
		// проверка на случай неполного удаления элементов контейнера.
		// относится только к отладке самого контейнера
		throw logic_error(string(string(typeid(self).name())) + "::empty(), invalid array content");
	}
#endif //_DEBUG
	return parent::empty();
}

template<class RT>
bool DataArray2D<RT>::ready() const
{
#ifdef _DEBUG
	if(parent::ready() && (!m_sizes[0] || !m_sizes[1] || !m_steps[0] || !m_steps[1] || m_rows.empty() || m_columns.empty()))
	{
		ForceDebugBreak();
		// проверка на случай неполной инициализации элементов контейнера.
		// относится только к отладке самого контейнера
		throw logic_error(string(string(typeid(self).name())) + "::ready(), invalid array content");
	}
#endif //_DEBUG
	return parent::ready()&&vsize()&&hsize();
}



//--------------------------------------------------------------
//
//	строки и столбцы
//
//--------------------------------------------------------------

template<class RT>
RT &DataArray2D<RT>::row(size_t v)
{
	return m_rows.at(v);
}

template<class RT>
const	RT &DataArray2D<RT>::row(size_t v) const{
	return m_rows.at(v);
}

template<class RT>
RT &DataArray2D<RT>::col(size_t h)
{
	return m_columns.at(h);
}

template<class RT>
const	RT &DataArray2D<RT>::col(size_t h) const
{
	return m_columns.at(h);
}

template<class RT>
void	DataArray2D<RT>::calculate_oblique_scan_params(ptrdiff_t &v0, ptrdiff_t &h0, size_t &scan_size, ptrdiff_t &scan_step, ptrdiff_t origin_v, ptrdiff_t origin_h, ptrdiff_t dv, ptrdiff_t dh) const
{
	ptrdiff_t	ev = vsize() - origin_v - 1;
	ptrdiff_t	eh = hsize() - origin_h - 1;
	ptrdiff_t	steps_to_start = min((dv>0 ? origin_v/dv : -ev/dv), (dh>0 ? origin_h/dh : -eh/dh));
	ptrdiff_t	steps_to_end = min((dv<0 ? -origin_v/dv : ev/dv), (dh<0 ? -origin_h/dh : eh/dh)) + 1;

	v0 = origin_v - dv*steps_to_start;
	h0 = origin_h - dh*steps_to_start;
	scan_size = steps_to_end + steps_to_start;

	// поскольку целевая точка может находиться и за пределами массива, срез может совсем не иметь
	// пересечений с данными. проверка на этот случай
	if(
		in_range(v0, 0, vsize()-1)
		&& in_range(h0, 0, hsize()-1)
		&& in_range(scan_size, 0, max(vsize(), hsize()))
		)
	{
		// подходящие данные нашлись, осталось только вычислить шаг
		scan_step = dv*vstep_raw() + dh*hstep_raw();
	}
	else
	{
		// нет подходящих данных, устанавливаем нулевой размер.
		// ошибки нет, срез будет пустым контейнером.
		v0=h0=0;
		scan_size=0;
		scan_step = 1;
	}
}

template<class RT>
void	DataArray2D<RT>::oblique_scan(typename DataArray2D<RT>::row_type &scan, ptrdiff_t origin_v, ptrdiff_t origin_h, ptrdiff_t dv, ptrdiff_t dh)
{
	ptrdiff_t	v0;
	ptrdiff_t	h0;
	size_t scan_size;
	ptrdiff_t scan_step;

	calculate_oblique_scan_params(v0, h0, scan_size, scan_step, origin_v, origin_h, dv, dh);
	scan.UseData(&at(v0, h0), scan_size, scan_step);
}

template<class RT>
void	DataArray2D<RT>::oblique_scan(typename DataArray2D<RT>::row_type_invariable &scan, ptrdiff_t origin_v, ptrdiff_t origin_h, ptrdiff_t dv, ptrdiff_t dh) const
{
	ptrdiff_t	v0;
	ptrdiff_t	h0;
	size_t scan_size;
	ptrdiff_t scan_step;

	calculate_oblique_scan_params(v0, h0, scan_size, scan_step, origin_v, origin_h, dv, dh);
	scan.UseData(&at(v0, h0), scan_size, scan_step);
}



//--------------------------------------------------------------
//
//	доступ
//
//--------------------------------------------------------------

template<class RT>
inline
typename DataArray2D<RT>::value_type &DataArray2D<RT>::at(size_t i, size_t j)
{
#if XRAD_CHECK_ARRAY_BOUNDARIES
	if(!valid_indices(i, j))
	{
		ForceDebugBreak();
		throw(out_of_range(string(typeid(self).name()) + ssprintf("::at(size_t,size_t), indices = (%d,%d) out of range, array dimensions = (%d,%d)", i, j, vsize(), hsize())));
	}
#endif //XRAD_CHECK_ARRAY_BOUNDARIES
	return parent::at(ElementOffset(i, j));
}

template<class RT>
inline
const typename DataArray2D<RT>::value_type &DataArray2D<RT>::at(size_t i, size_t j) const
{
#if XRAD_CHECK_ARRAY_BOUNDARIES
	if(!valid_indices(i, j))
	{
		ForceDebugBreak();
		throw(out_of_range(string(typeid(self).name()) + ssprintf("::at(size_t,size_t), indices = (%d,%d) out of range, array dimensions = (%d,%d)", i, j, vsize(), hsize())));
	}
#endif //XRAD_CHECK_ARRAY_BOUNDARIES
	return parent::at(ElementOffset(i, j));
}


//--------------------------------------------------------------
//
//	Получение указателя на данные
//
//--------------------------------------------------------------

template<class RT>
typename DataArray2D<RT>::value_type *DataArray2D<RT>::data()
{
	// доступ к данным как к указателю на сплошной массив памяти.
	// возможно только тогда, когда эти данные реально существуют,
	// когда шаг по строке равен 1, а шаг по столбцу равен размеру строки
	// возможен доступ не const, именно так.
	if(row_step() == 1 && column_step()==ptrdiff_t(hsize()) && vsize() && hsize())
	{
		return &at(0, 0);
	}
	else
	{
		string problem_description = typeid(self).name() +
			ssprintf("::data() -- invalid access, steps=(%d,%d), sizes is (%d,%d)",
					row_step(), column_step(), vsize(), hsize());
		ForceDebugBreak();
		throw logic_error(problem_description);
	}
}

template<class RT>
const	typename DataArray2D<RT>::value_type *DataArray2D<RT>::data() const
{
	if(row_step() == 1 && column_step()==ptrdiff_t(hsize()) && vsize() && hsize())
	{
		return &at(0, 0);
	}
	else
	{
		string problem_description = typeid(self).name() +
			ssprintf("::data() -- invalid access, steps=(%d,%d), sizes is (%d,%d)",
					row_step(), column_step(), vsize(), hsize());
		ForceDebugBreak();
		throw logic_error(problem_description);
	}
}



//--------------------------------------------------------------
//
//	транспонирование
//
//--------------------------------------------------------------



template<class RT>
void	DataArray2D<RT>::transpose()
{
	using std::swap;
	swap(m_sizes[0], m_sizes[1]);
	swap(m_steps[0], m_steps[1]);
	swap(m_rows, m_columns);

	// внимание!!!
	// для объектов m_rows, m_columns здесь не следует вызывать просто swap:
	// есть вероятность, что вызовется метод из std,
	// внутри него вызывается копи-конструктор, теряется связь
	// с реальными данными, создается копия. только ранее определенная через container_swap!!!
//	m_rows.container_swap(m_columns);
}

template<class RT>
void DataArray2D<RT>::hflip()
{
	if (empty())
		return;
	origin_offset += m_steps[1] * ptrdiff_t(m_sizes[1] - 1);
#ifdef _DEBUG
	if (origin_offset < 0)
	{
		ForceDebugBreak();
		throw logic_error(ssprintf("DataArray2D::hflip(): invalid value computed for origin_offset: %ti.",
				EnsureType<ptrdiff_t>(origin_offset)));
	}
#endif
	m_steps[1] = -m_steps[1];
	m_columns.reverse();
	ApplyAction(m_rows, [](auto &row) { row.reverse(); });
}

template<class RT>
void DataArray2D<RT>::vflip()
{
	if (empty())
		return;
	origin_offset += m_steps[0] * ptrdiff_t(m_sizes[0] - 1);
#ifdef _DEBUG
	if (origin_offset < 0)
	{
		ForceDebugBreak();
		throw logic_error(ssprintf("DataArray2D::vflip(): invalid value computed for origin_offset: %ti.",
				EnsureType<ptrdiff_t>(origin_offset)));
	}
#endif
	m_steps[0] = -m_steps[0];
	m_rows.reverse();
	ApplyAction(m_columns, [](auto &col) { col.reverse(); });
}



//--------------------------------------------------------------
//
//	копирование сегмента одного массива в другой
//
//--------------------------------------------------------------

template<class RT>
template<class RT2>
void	DataArray2D<RT>::GetDataSegment(const DataArray2D<RT2> &original, ptrdiff_t top, ptrdiff_t left)
{
	if(!original.ready())
	{
		fill(value_type(0));
		return;
	}
	// предусмотрено частичное перекрывание массивов:
	// копируемая область экстраполируется при выходе за границы.
	//
	// здесь и в следующей функции
	// индексы (i,j) относятся к текущему массиву
	// индексы (v,h) -- к аргументу original/destination
	//
	size_t	imin = max(ptrdiff_t(0), -top);// номер первой строки, в которую поступят данные
	size_t	vmin = top + imin;// номер первой копируемой строки оригинала, 0 или top (наоборот с imin)
	size_t	n_copied_rows = min(original.vsize()-vmin, vsize()-imin);
// 	size_t	vmax = vmin + n_copied_rows;
	size_t	imax = imin + n_copied_rows;
		// если верхний край лежит выше нулевой строки, строки 0...imin-1 подвергаются экстраполяции
		// количество копируемых строк соответственно уменьшается.
		// то же самое со столбцами (ниже)

	size_t	jmin = max(ptrdiff_t(0), -left);
	size_t	hmin = left + jmin;// номер первого копируемого столбца, 0 или left (наоборот с jmin)
	size_t	n_copied_columns = min(original.hsize()-hmin, hsize()-jmin);
	size_t	hmax = hmin + n_copied_columns;
	size_t	jmax = jmin + n_copied_columns;

	for(size_t i = 0; i < imin; ++i)
		row(i).fill(value_type(0));

	for(size_t i = imin, v = vmin; i < imax; ++i, ++v)
	{
		std::fill(row(i).begin(), row(i).begin()+jmin, value_type(0));
		std::copy(original.row(v).begin()+hmin, original.row(v).begin()+hmax, row(i).begin()+jmin);
		std::fill(row(i).begin()+jmax, row(i).end(), value_type(0));
	}

	for(size_t i = imax; i < vsize(); ++i)
		row(i).fill(value_type(0));
}

template<class RT>
template<class RT2>
void	DataArray2D<RT>::PutDataSegment(DataArray2D<RT2> &destination, ptrdiff_t top, ptrdiff_t left) const
{
	//	просто копирует данные на соответствующие позиции
	if(!destination.ready())
		return;

	// предусмотрено частичное перекрывание массивов:
	// копируется только та часть, которая попадает в разрешенный диапазон destination

	size_t	imin = max(ptrdiff_t(0), -top);// номер первой строки, в которую поступят данные
	size_t	vmin = top + imin;// номер первой копируемой строки оригинала, 0 или top (наоборот с imin)
	size_t	n_copied_rows = min(destination.vsize()-vmin, vsize()-imin);
//	size_t	vmax = vmin + n_copied_rows;
	size_t	imax = imin + n_copied_rows;

	size_t	jmin = max(ptrdiff_t(0), -left);
	size_t	hmin = left + jmin;// номер первого копируемого столбца, 0 или left (наоборот с jmin)
	size_t	n_copied_columns = min(destination.hsize()-hmin, hsize()-jmin);
//	size_t	hmax = hmin + n_copied_columns;
	size_t	jmax = jmin + n_copied_columns;


	for(size_t i = imin, v = vmin; i < imax; ++i, ++v)
	{
		std::copy(row(i).begin()+jmin, row(i).begin()+jmax, destination.row(v).begin()+hmin);
	}

}



//--------------------------------------------------------------



//! \todo Проверить состояние объекта при исключениях в данной функции.
//! Объект должен оказаться в допустимом состоянии (не должно быть "висячих" указателей).
template<class RT>
void	DataArray2D<RT>::MapCells()
{
	if(!vsize() || !hsize())
	{
		m_rows.realloc(0);
		m_columns.realloc(0);
	}
	else
	{
		if(vsize() != m_rows.size())
			m_rows.realloc(vsize());
		if(hsize() != m_columns.size())
			m_columns.realloc(hsize());

		for(size_t i = 0; i < vsize(); ++i)
			m_rows.at(i).UseData(&parent::at(ElementOffset(i, 0)), hsize(), hstep_raw());
		for(size_t j = 0; j < hsize(); ++j)
			m_columns.at(j).UseData(&parent::at(ElementOffset(0, j)), vsize(), vstep_raw());

		// доступ в последующем осуществляется двумя способами:
		//
		// 1. через m_rows[]/.at() и m_columns[]/.at(). каждый row/column содержит внутри себя указатели
		//	непосредственно на ячейки, содержащие обрабатываемые величины. доступ через
		//	одномерный массив parent не используется, поэтому вычисляется окончательный
		//	шаг, который здесь и умножается на parent_step().
		// 2. доступ через at(i,j), который вычисляет ElementOffset(v,h), который измеряется
		//	в элементах базового массива parent. поэтому шаги этого массива учитываются
		//	методом parent::at(ElementOffset(i,j)), а непосредственно нигде сами не
		//	фигурируют
	}
}



//--------------------------------------------------------------



template <class RT, class F>
void ApplyFunction(DataArray2D<RT> &array, const F &function)
{
	Apply_A_2D_RF1(array, function);
}

template <class RT, class F>
void ApplyAction(DataArray2D<RT> &array, const F &function)
{
	Apply_A_2D_F1(array, function);
}



//--------------------------------------------------------------

XRAD_END

#endif // __data_array_2d_cc
