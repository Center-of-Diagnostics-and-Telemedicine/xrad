#ifndef __data_array_multidimensional_cc
#define __data_array_multidimensional_cc

#include "BasicArrayInteractionsMD.h"

XRAD_BEGIN

//TODO #error навести порядок с проверкой пустого массива -- во всех размерностях. особо сделать функцию проверки "пусто-непусто" для многомерных

//--------------------------------------------------------------
//
//	конструкторы
//
//--------------------------------------------------------------



template<class A2DT>
DataArrayMD<A2DT>::DataArrayMD()
{
}

template<class A2DT>
DataArrayMD<A2DT>::DataArrayMD(const index_vector& iv)
{
	realloc(iv);
}

template<class A2DT>
DataArrayMD<A2DT>::DataArrayMD(const index_vector& iv, const value_type& default_value)
{
	realloc(iv);
	fill(default_value);
}



template<class A2DT>
DataArrayMD<A2DT>::DataArrayMD(const self &original)
{
	this->MakeCopy(original);
}

template<class A2DT>
DataArrayMD<A2DT>::DataArrayMD(self &&original)
{
	MoveData(original);
}

template <class A2DT>
DataArrayMD<A2DT> &DataArrayMD<A2DT>::operator=(const self &original)
{
	return operator=<A2DT>(original);
}

template <class A2DT>
DataArrayMD<A2DT> &DataArrayMD<A2DT>::operator=(self &&original)
{
	MoveData(original);
	return *this;
}



template<class A2DT>
template<class A2DT2>
DataArrayMD<A2DT>::DataArrayMD(const DataArrayMD<A2DT2> &original)
{
	this->MakeCopy(original);
}

template <class A2DT>
template <class A2DT2>
DataArrayMD<A2DT> &DataArrayMD<A2DT>::operator=(const DataArrayMD<A2DT2> &original)
{
	// присваивание выполняется только между массивами одинаковых размеров.
	// единственное исключение -- если первый массив совсем пустой
	if(!n_dimensions())
	{
		if(!original.n_dimensions())
		{
			// если оригинал оказался пустым, сразу выходим
			return *this;
		}
		realloc(original.sizes());
	}
	else if(sizes() != original.sizes())
	{
		string problem_description = ssprintf("%s::operator=(%s original) -- array size mismatch: (original - current)",
				EnsureType<const char*>(typeid(self).name()),
				EnsureType<const char*>(typeid(original).name()));
		problem_description += ssprintf(" dim = %zu - %zu",
				EnsureType<size_t>(sizes().size()),
				EnsureType<size_t>(original.sizes().size()));
		if (sizes().size() == original.sizes().size())
		{
			for(size_t i = 0; i < sizes().size(); ++i)
			{
				problem_description += ssprintf(", [%zu] = %zu - %zu",
						EnsureType<size_t>(i),
						EnsureType<size_t>(sizes()[i]),
						EnsureType<size_t>(original.sizes()[i]));
			}
		}
		ForceDebugBreak();
		throw invalid_argument(problem_description);
	}
	Apply_AA_MD_F2(*this, original, Functors::assign());
	return *this;
}

//--------------------------------------------------------------

template<class A2DT>
bool DataArrayMD<A2DT>::empty() const
{
	// 	все остальные элементы проверяются при выделении и освобождении массива функцией CheckDataIntegrity();
	return parent::empty();
}

template<class A2DT>
bool DataArrayMD<A2DT>::ready() const
{
	// 	все остальные элементы проверяются при выделении и освобождении массива функцией CheckDataIntegrity();
	return parent::ready();
}



//--------------------------------------------------------------

template<class A2DT>
void	DataArrayMD<A2DT>::realloc(const index_vector& iv)
{
	size_t	volume = MDAAuxiliaries::index_vector_volume(iv);
	if(!volume)
	{
		// передача пустого индексного вектора должлна приводить к освобождению памяти
		parent::realloc(0);
		m_sizes.realloc(0);
		m_steps.realloc(0);
	}
	else
	{
		if(iv.size() == n_dimensions())
		{
			// если массив уже был выделен с теми же размерами,
			// освобождение и вновь выделение памяти не производим
			if(iv == sizes()) return;
		}
		size_t	total_size = 1;
		bool fault = false;

		if(volume < 0) fault = true;

		if(iv.size() < 2) fault = true;
		for(size_t i = 0; i < iv.size(); ++i)
		{
			if(iv[i] <= 0) fault = true;
			total_size *= iv[i];
		}

		if(fault)
		{
			string problem_description = typeid(self).name() +
				string("::realloc(index_vector), invalid index vector = ") +
				MDAAuxiliaries::index_string(iv);
			ForceDebugBreak();
			throw(invalid_argument(problem_description));
		}

		parent::realloc(total_size);
		GenerateSizesAndSteps(iv);
	}
	CheckDataIntegrity();
}

template<class A2DT>
void DataArrayMD<A2DT>::resize(const index_vector &new_sizes)
{
	if(new_sizes.size() == n_dimensions())
	{
		if(new_sizes==sizes()) return;
	}

	self buffer(new_sizes);
	if (!empty())
		buffer.CopyData(*this);
	MoveData(buffer);
}

template<class A2DT>
void	DataArrayMD<A2DT>::reorder_dimensions(const index_vector& iv)
{
	index_vector	new_sizes(n_dimensions()), new_steps(n_dimensions());
	bool	fault = false;
	if(iv.size() != n_dimensions())
		fault = true;
	else for(size_t i = 0; i < n_dimensions(); ++i)
	{
		size_t j = 0;
		while(iv[j] != slice_mask(i) && j < n_dimensions())
			++j;
		if(j >= n_dimensions())
			fault = true;
		else
		{
			new_sizes[i] = sizes(j);
			new_steps[i] = steps(j);
		}
	}

	if(fault)
	{
		string problem_description = (
			typeid(self).name() +
			string("::reOrderDimensions(), invalid index vector = ") +
			MDAAuxiliaries::index_compare_string(iv, sizes())
			);
		ForceDebugBreak();
		throw(invalid_argument(problem_description));
	}
	m_sizes.CopyData(new_sizes);
	m_steps.CopyData(new_steps);
	CheckDataIntegrity();
}

template<class A2DT>
void	DataArrayMD<A2DT>::fill(const typename DataArrayMD<A2DT>::value_type &x)
{
	Apply_A_MD_F1(*this, [&x](value_type &v) { v = x; });
}

//--------------------------------------------------------------

template<class A2DT>
template<class A2DT2>
void DataArrayMD<A2DT>::MakeCopy(const DataArrayMD<A2DT2> &original)
{
	if(static_cast<const void*>(&original) == static_cast<const void*>(this))
	{
		// если предлагается скопировать себя в себя же, выходим сразу.
		// приведение к void* стало нужным для шаблонных вызовов с разными
		// типами массивов
		return;
	}
	realloc(original.sizes());
	// если оригинал пустой, просто освобождает свою память (алгоритм сразу же выходит)
	Apply_AA_MD_F2(*this, original, Functors::assign());
}

template<class A2DT>
template<class A2DT2, class functor>
void DataArrayMD<A2DT>::MakeCopy(const DataArrayMD<A2DT2> &original, const functor &f)
{
	realloc(original.sizes());
	// если оригинал пустой, просто освобождает свою память
	Apply_AA_MD_F2(*this, original, f);
}

//--------------------------------------------------------------

template<class A2DT>
template<class A2DT2>
void DataArrayMD<A2DT>::CopyData(const DataArrayMD<A2DT2> &original)
{
	if(static_cast<const void*>(&original) == static_cast<const void*>(this))
	{
		// если предлагается скопировать себя в себя же, выходим сразу.
		// приведение к void* стало нужным для шаблонных вызовов с разными
		// типами массивов
		return;
	}
	if(original.empty()) fill(zero_value(value_type()));
	else Apply_AA_MD_Different_F2(*this, original, Functors::assign());
}

template<class A2DT>
template<class A2DT2, class functor>
void DataArrayMD<A2DT>::CopyData(const DataArrayMD<A2DT2> &original, const functor &f)
{
	if(original.empty())
	{
		typename functor::result_type zero_converted;
		f(zero_converted, zero_value(value_type()));
		fill(zero_converted);
	}
	else Apply_AA_MD_Different_F2(*this, original, f);
}

//--------------------------------------------------------------

template<class A2DT>
void	DataArrayMD<A2DT>::UseData(value_type *new_data, const index_vector& in_sizes, ptrdiff_t in_data_step)
{
	size_t	total_size = 1;
	bool	fault = (in_data_step <= 0);

	if(in_sizes.size() < 2) fault = true;
	for(size_t i = 0; i < in_sizes.size(); ++i)
	{
		if(in_sizes[i] <= 0) fault = true;
		total_size *= in_sizes[i];
	}

	if(fault)
	{
		string problem_description = ssprintf("%s::UseData(value_type *, const index_vector &sizes, ptrdiff_t step), invalid sizes = %s or invalid step = %ti.",
				EnsureType<const char*>(typeid(self).name()),
				EnsureType<const char*>(MDAAuxiliaries::index_string(in_sizes).c_str()),
				EnsureType<ptrdiff_t>(in_data_step));
		ForceDebugBreak();
		throw invalid_argument(problem_description);
	}

	parent::UseData(new_data, total_size, in_data_step);
	GenerateSizesAndSteps(in_sizes);
	CheckDataIntegrity();
}

template<class A2DT>
void DataArrayMD<A2DT>::UseData(value_type *new_data, const index_vector& in_sizes, const offset_vector& in_steps)
{
	size_t	total_size = 1;
	bool	fault = false;

	if(in_sizes.size() < 2) fault = true;
	for(size_t i = 0; i < in_sizes.size(); ++i)
	{
		if(in_sizes[i] <= 0) fault = true;
		if(in_steps[i] <= 0) fault = true;

		total_size += (in_sizes[i] - 1) * in_steps[i];
	}

	if(fault)
	{
		string problem_description = ssprintf("%s::UseData(value_type *, const index_vector &sizes, const offset_vector &steps), invalid sizes = %s or invalid steps = %s.",
				EnsureType<const char*>(typeid(self).name()),
				EnsureType<const char*>(MDAAuxiliaries::index_string(in_sizes).c_str()),
				EnsureType<const char*>(MDAAuxiliaries::index_string(in_steps).c_str()));
		ForceDebugBreak();
		throw invalid_argument(problem_description);
	}

	parent::UseData(new_data, total_size, 1);
	m_sizes.MakeCopy(in_sizes);
	m_steps.MakeCopy(in_steps);

	CheckDataIntegrity();
}

template<class A2DT>
void	DataArrayMD<A2DT>::UseData(typename DataArrayMD<A2DT>::parent &new_data, const index_vector& in_sizes, const index_vector& in_steps)
{
	size_t	total_size = 1;
	bool	fault = false;

	if(in_sizes.size() < 2) fault = true;
	for(size_t i = 0; i < in_sizes.size(); ++i)
	{
		if(in_sizes[i] <= 0) fault = true;
		if(in_steps[i] <= 0) fault = true;

		total_size += (in_sizes[i] - 1) * in_steps[i];
	}

	if(fault)
	{
		string problem_description = ssprintf("%s::UseData(value_type *, const index_vector &sizes, const index_vector &steps), invalid sizes = %s or invalid steps = %s.",
				EnsureType<const char*>(typeid(self).name()),
				EnsureType<const char*>(MDAAuxiliaries::index_string(in_sizes).c_str()),
				EnsureType<const char*>(MDAAuxiliaries::index_string(in_steps).c_str()));
		ForceDebugBreak();
		throw invalid_argument(problem_description);
	}

	parent::UseData(new_data, total_size, 1);
	m_sizes.MakeCopy(in_sizes);
	m_steps.MakeCopy(in_steps);
	CheckDataIntegrity();
}



template<class A2DT>
template<class A2DT2>
void	DataArrayMD<A2DT>::UseData(DataArrayMD<A2DT2> &new_data)
{
	index_vector	subset_iv(new_data.n_dimensions());
	for(size_t i = 0; i < new_data.n_dimensions(); ++i)
	{
		subset_iv[i] = slice_mask(i);
	}
	new_data.GetSubset(*this, subset_iv);
	CheckDataIntegrity();
}

template<class A2DT>
template<class A2DT2>
void	DataArrayMD<A2DT>::UseData(const DataArrayMD<A2DT2> &new_data)
{
	// эта функция скомпилируется только для массивов invariable, что и требуется
	index_vector	subset_iv(new_data.n_dimensions());
	for(size_t i = 0; i < new_data.n_dimensions(); ++i)
	{
		subset_iv[i] = slice_mask(i);
	}
	new_data.GetSubset(*this, subset_iv);
	CheckDataIntegrity();
}



template<class A2DT>
template<class A2DT2>
void DataArrayMD<A2DT>::UseDataFragment(DataArrayMD<A2DT2> &original, const index_vector &p0, const index_vector &p1)
{
	static_assert(sizeof(typename DataArrayMD<A2DT2>::value_type) == sizeof(value_type),
			"DataArrayMD<A2DT>::UseDataFragment: Incompatible data types.");
	bool	fault = false;
	size_t n_dim = original.n_dimensions();
	if (p0.size() != n_dim || p1.size() != n_dim)
		fault = true;
	else for (size_t i = 0; i < n_dim; ++i)
	{
		if (p0[i] >= p1[i] || p1[i] > original.sizes(i))
		{
			fault = true;
			break;
		}
	}
	if(fault)
	{
		ForceDebugBreak();
		throw invalid_argument(ssprintf("%s::UseDataFragment(DataArrayMD) -- invalid arguments",
				EnsureType<const char*>(typeid(self).name())));
	}

	value_type	*data_ptr = &original.at(p0);
	UseData(data_ptr, p1 - p0, original.steps_raw());
}

template<class A2DT>
template<class A2DT2>
void DataArrayMD<A2DT>::UseDataFragment(const DataArrayMD<A2DT2> &original, const index_vector &p0, const index_vector &p1)
{
	static_assert(sizeof(typename DataArrayMD<A2DT2>::value_type) == sizeof(value_type),
			"DataArrayMD<A2DT>::UseDataFragment: Incompatible data types.");
	bool	fault = false;
	size_t n_dim = original.n_dimensions();
	if (p0.size() != n_dim || p1.size() != n_dim)
		fault = true;
	else for (size_t i = 0; i < n_dim; ++i)
	{
		if (p0[i] >= p1[i] || p1[i] > original.sizes(i))
		{
			fault = true;
			break;
		}
	}
	if(fault)
	{
		ForceDebugBreak();
		throw invalid_argument(ssprintf("%s::UseDataFragment(DataArrayMD) -- invalid arguments",
				EnsureType<const char*>(typeid(self).name())));
	}

	value_type	*data_ptr = &original.at(p0);
	UseData(data_ptr, p1 - p0, original.steps_raw());
}



template<class A2DT>
void DataArrayMD<A2DT>::MoveData(DataArrayMD &original)
{
	parent::owner_move(original);
	m_sizes.MoveData(original.m_sizes);
	m_steps.MoveData(original.m_steps);
}



template<class A2DT>
template <class ArrayMD>
ArrayMD DataArrayMD<A2DT>::GetDataFragment(const index_vector &p0, const index_vector &p1)
{
	ArrayMD array;
	array.UseDataFragment(*this, p0, p1);
	return array;
}

template<class A2DT>
template <class ArrayMD>
ArrayMD DataArrayMD<A2DT>::GetDataFragment(const index_vector &p0, const index_vector &p1) const
{
	ArrayMD array;
	array.UseDataFragment(*this, p0, p1);
	return array;
}



//--------------------------------------------------------------



template<class A2DT>
typename DataArrayMD<A2DT>::value_type	&DataArrayMD<A2DT>::at(const index_vector& index)
{
#if XRAD_CHECK_ARRAY_BOUNDARIES
	bool	fault = false;
	if(index.size() != n_dimensions()) fault = true;
	else for(size_t i = 0; i < n_dimensions(); ++i)
	{
		if(!in_range(index[i], 0u, sizes(i) - 1)) fault = true;
	}

	if(fault)
	{
		string problem_description =
			typeid(self).name() +
			string("::at(index_vector), invalid index vector = ") +
			MDAAuxiliaries::index_compare_string(index, sizes());
		ForceDebugBreak();
		throw(out_of_range(problem_description));
	}
#endif //XRAD_CHECK_ARRAY_BOUNDARIES

	return parent::at(ElementOffset(index));
}

template<class A2DT>
const typename DataArrayMD<A2DT>::value_type	&DataArrayMD<A2DT>::at(const index_vector& index) const
{
#if XRAD_CHECK_ARRAY_BOUNDARIES
	bool	fault = false;
	if(index.size() != n_dimensions()) fault = true;
	else for(size_t i = 0; i < n_dimensions(); ++i)
	{
		if(!in_range(index[i], 0u, sizes(i) - 1)) fault = true;
	}

	if(fault)
	{
		string problem_description = typeid(self).name() +
			string("::at(index_vector), invalid index vector = ") +
			MDAAuxiliaries::index_compare_string(index, sizes());
		ForceDebugBreak();
		throw(out_of_range(problem_description));
	}
#endif //XRAD_CHECK_ARRAY_BOUNDARIES

	return parent::at(ElementOffset(index));
}



//--------------------------------------------------------------



template<class A2DT>
void	DataArrayMD<A2DT>::GetRow(typename DataArrayMD<A2DT>::row_type &row, const index_vector& iv)
{
	size_t	row_index;
	GetRowIndex(row_index, iv);
	size_t	offset = ElementOffset(iv);

	row.UseData(&parent::at(offset), sizes(row_index), steps_raw(row_index));
}

template<class A2DT>
void	DataArrayMD<A2DT>::GetRow(typename DataArrayMD<A2DT>::row_type::invariable &row, const index_vector& iv) const
{
	size_t	row_index;
	size_t	offset = ElementOffset(iv);
	GetRowIndex(row_index, iv);

	row.UseData(&parent::at(offset), sizes(row_index), steps_raw(row_index));
}

template<class A2DT>
template <class Array>
Array DataArrayMD<A2DT>::GetRow(const index_vector &index)
{
	Array array;
	GetRow(array, index);
	return array;
}

template<class A2DT>
template <class Array>
Array DataArrayMD<A2DT>::GetRow(const index_vector &index) const
{
	Array array;
	GetRow(array, index);
	return array;
}

//--------------------------------------------------------------

template<class A2DT>
void	DataArrayMD<A2DT>::GetSlice(typename DataArrayMD<A2DT>::slice_type &slice, const index_vector& iv)
{
	size_t	slice_index_0, slice_index_1;
	GetSliceIndices(slice_index_0, slice_index_1, iv);
	slice.UseData(&parent::at(ElementOffset(iv)),
		sizes(slice_index_0), sizes(slice_index_1),
		steps_raw(slice_index_0), steps_raw(slice_index_1));
}


template<class A2DT>
void	DataArrayMD<A2DT>::GetSlice(typename DataArrayMD<A2DT>::slice_type::invariable &slice, const index_vector& iv) const
{
	size_t	slice_index_0, slice_index_1;
	GetSliceIndices(slice_index_0, slice_index_1, iv);
	slice.UseData(&parent::at(ElementOffset(iv)),
		sizes(slice_index_0), sizes(slice_index_1),
		steps_raw(slice_index_0), steps_raw(slice_index_1));
}

template<class A2DT>
template <class Array2D>
Array2D DataArrayMD<A2DT>::GetSlice(const index_vector &index)
{
	Array2D array;
	GetSlice(array, index);
	return array;
}

template<class A2DT>
template <class Array2D>
Array2D DataArrayMD<A2DT>::GetSlice(const index_vector &index) const
{
	Array2D array;
	GetSlice(array, index);
	return array;
}

//--------------------------------------------------------------

template<class A2DT>
template<class A2DT2>
void	DataArrayMD<A2DT>::GetSubset(DataArrayMD<A2DT2> &subset, const index_vector& iv)
{
	static_assert(sizeof(typename DataArrayMD<A2DT2>::value_type) == sizeof(value_type),
			"DataArrayMD<A2DT>::GetSubset: Incompatible data types.");
	index_vector	subset_indices;
	index_vector	subset_sizes, subset_steps;
	GetSubsetIndices(subset_indices, iv);
	// все проверки границ диапазонов и т.п. должны быть в этом вызове

	subset_sizes.realloc(subset_indices.size());
	subset_steps.realloc(subset_indices.size());

	for(size_t i = 0; i < subset_indices.size(); ++i)
	{
		subset_sizes[i] = sizes(subset_indices[i]);
		subset_steps[i] = steps_raw(subset_indices[i]);
	}
	subset.UseData(&parent::at(ElementOffset(iv)), subset_sizes, subset_steps);
}

template<class A2DT>
template<class A2DT2>
void	DataArrayMD<A2DT>::GetSubset(DataArrayMD<A2DT2> &subset, const index_vector& iv) const
{
	static_assert(sizeof(typename DataArrayMD<A2DT2>::value_type) == sizeof(value_type),
			"DataArrayMD<A2DT>::GetSubset: Incompatible data types.");
	index_vector	subset_indices;
	index_vector	subset_sizes, subset_steps;
	GetSubsetIndices(subset_indices, iv);
	// все проверки границ диапазонов и т.п. должны быть в этом вызове

//	subset.m_dimensions_number = subset_indices.size();
	subset_sizes.realloc(subset_indices.size());
	subset_steps.realloc(subset_indices.size());

	for(size_t i = 0; i < subset_indices.size(); ++i)
	{
		subset_sizes[i] = sizes(subset_indices[i]);
		subset_steps[i] = steps_raw(subset_indices[i]);
	}
	subset.UseData(&parent::at(ElementOffset(iv)), subset_sizes, subset_steps);
}

template<class A2DT>
template <class ArrayMD>
ArrayMD DataArrayMD<A2DT>::GetSubset(const index_vector &index)
{
	ArrayMD array;
	GetSubset(array, index);
	return array;
}

template<class A2DT>
template <class ArrayMD>
ArrayMD DataArrayMD<A2DT>::GetSubset(const index_vector &index) const
{
	ArrayMD array;
	GetSubset(array, index);
	return array;
}



//--------------------------------------------------------------



template<class A2DT>
template<class A2DT2, class ComponentSelector>
void DataArrayMD<A2DT>::GetDataComponent(DataArrayMD<A2DT2> &component,
		ComponentSelector component_selector)
{
	using component_type = std::remove_reference_t<
			decltype(*component_selector(std::declval<value_type*>()))>;
	static_assert(sizeof(typename DataArrayMD<A2DT2>::value_type) == sizeof(component_type),
			"DataArrayMD<A2DT>::GetDataComponent: Incompatible data types.");
	// Проверка на то, что шаг данных у компоненты кратен размеру компоненты.
	// Это ограничение текущей реализации DataOwner.
	static_assert(sizeof(value_type) % sizeof(component_type) == 0,
			"DataArrayMD<A2DT>::GetDataComponent: Data step must be a multiple of data size for "
			"DataOwner.");
#ifdef XRAD_DEBUG
	{
		// Проверка на то, что компонента находится внутри исходных данных.
		auto *data_origin = &parent::at(0);
		auto *component_origin = (void*)component_selector(data_origin);
		if (component_origin < (void*)data_origin ||
				component_origin >= (void*)(data_origin + 1))
		{
			throw runtime_error("DataArrayMD<A2DT>::GetDataComponent: the component selector gives "
					"a component outside original data.");
		}
	}
#endif
	index_vector o(n_dimensions(), 0);
	offset_vector component_steps = m_steps * (sizeof(value_type) / sizeof(component_type));
	component.UseData(component_selector(&parent::at(ElementOffset(o))), sizes(), component_steps);
}

template<class A2DT>
template<class A2DT2, class ComponentSelector>
void DataArrayMD<A2DT>::GetDataComponent(DataArrayMD<A2DT2> &component,
		ComponentSelector component_selector) const
{
	using component_type = std::remove_reference_t<
			decltype(*component_selector(std::declval<value_type*>()))>;
	static_assert(sizeof(typename DataArrayMD<A2DT2>::value_type) == sizeof(component_type),
			"DataArrayMD<A2DT>::GetDataComponent: Incompatible data types.");
	// Проверка на то, что шаг данных у компоненты кратен размеру компоненты.
	// Это ограничение текущей реализации DataOwner.
	static_assert(sizeof(value_type) % sizeof(component_type) == 0,
			"DataArrayMD<A2DT>::GetDataComponent: Data step must be a multiple of data size for "
			"DataOwner.");
#ifdef XRAD_DEBUG
	{
		// Проверка на то, что компонента находится внутри исходных данных.
		auto *data_origin = &parent::at(0);
		auto *component_origin = (void*)component_selector(data_origin);
		if (component_origin < (void*)data_origin ||
				component_origin >= (void*)(data_origin + 1))
		{
			throw runtime_error("DataArrayMD<A2DT>::GetDataComponent: the component selector gives "
					"a component outside original data.");
		}
	}
#endif
	index_vector o(n_dimensions(), 0);
	offset_vector component_steps = m_steps * (sizeof(value_type) / sizeof(component_type));
	component.UseData(component_selector(&parent::at(ElementOffset(o))), sizes(), component_steps);
}

template<class A2DT>
template<class ArrayMD, class ComponentSelector>
auto DataArrayMD<A2DT>::GetDataComponent(ComponentSelector component_selector) ->
		default_array_t<ArrayMD,
				decltype(*std::declval<ComponentSelector>()(std::declval<value_type*>()))>
{
	default_array_t<ArrayMD,
			decltype(*std::declval<ComponentSelector>()(std::declval<value_type*>()))> array;
	GetDataComponent(array, component_selector);
	return array;
}

template<class A2DT>
template<class ArrayMD, class ComponentSelector>
auto DataArrayMD<A2DT>::GetDataComponent(ComponentSelector component_selector) const ->
		default_array_t<ArrayMD,
				decltype(*std::declval<ComponentSelector>()(std::declval<value_type_invariable*>()))>
{
	default_array_t<ArrayMD,
			decltype(*std::declval<ComponentSelector>()(std::declval<value_type_invariable*>()))> array;
	GetDataComponent(array, component_selector);
	return array;
}



//--------------------------------------------------------------



template<class A2DT>
typename DataArrayMD<A2DT>::slice_ref DataArrayMD<A2DT>::slice(const index_vector& iv)
{
	slice_ref	result;
	GetSlice(result, iv);
	return result;
}

template<class A2DT>
typename DataArrayMD<A2DT>::slice_ref_invariable DataArrayMD<A2DT>::slice(const index_vector& iv) const
{
	slice_ref_invariable	result;
	GetSlice(result, iv);
	return result;
}

template<class A2DT>
typename DataArrayMD<A2DT>::row_ref DataArrayMD<A2DT>::row(const index_vector& iv)
{
	row_ref	result;
	GetRow(result, iv);
	return result;
}

template<class A2DT>
typename DataArrayMD<A2DT>::row_ref_invariable DataArrayMD<A2DT>::row(const index_vector& iv) const
{
	row_ref_invariable	result;
	GetRow(result, iv);
	return result;
}


template<class A2DT>
typename DataArrayMD<A2DT>::ref DataArrayMD<A2DT>::subset(const index_vector& iv)
{
	row_ref	result;
	GetSlice(result, iv);
	return result;
}

template<class A2DT>
typename DataArrayMD<A2DT>::ref_invariable DataArrayMD<A2DT>::subset(const index_vector& iv) const
{
	row_ref_invariable	result;
	GetSlice(result, iv);
	return result;
}



//--------------------------------------------------------------



template<class A2DT>
size_t	DataArrayMD<A2DT>::ElementOffset(const index_vector& index) const
{
	size_t	result = 0;
	const_steps_vector_iterator	st = m_steps.begin();
	const_index_vector_iterator	ind = index.begin();

	for(size_t i = 0; i < n_dimensions(); ++i, ++st, ++ind)
	{
		//		if(*ind >= 0) result += (*st)* (*ind);
		if(!is_slice_mask(*ind)) result += (*st)* (*ind);
		// это последнее условие показывает смещение на начало
		// строки, по координате, выделяемой маркером slice_mask(*).
		// таких координат может быть несколько, в этом случае смещение
		// на ближайший к началу координат угол выделяемого подмножества
	}
	return result;
}

//--------------------------------------------------------------

template<class A2DT>
void	DataArrayMD<A2DT>::GetRowIndex(size_t &row_index, const index_vector& iv) const
{
	row_index = 0;
	size_t	slice_dimension = 0;

	for(size_t i = 0; i < iv.size(); ++i)
	{
		size_t	index = iv[i];
		if(is_slice_mask(index))
		{
			switch(dimension_no(index))
			{
				case 0:
					++slice_dimension;
					row_index = i;
					break;
				default:
				{
					string problem_description = typeid(self).name() +
						string("::GetRowIndex(), invalid index vector = ") +
						MDAAuxiliaries::index_compare_string(iv, sizes());
					ForceDebugBreak();
					throw(invalid_argument(problem_description));
				}
			}
		}
	}

#if XRAD_CHECK_ARRAY_BOUNDARIES
	{
		bool	fault = false;
		if(iv.size() != n_dimensions() || slice_dimension != 1) fault = true;
		else for(size_t i = 0; i < n_dimensions(); ++i)
		{
			if(!in_range(iv[i], 0u, sizes(i) - 1) && iv[i] != slice_mask(0)) fault = true;
		}

		if(fault)
		{
			string problem_description = typeid(self).name() +
				string("::GetRowIndex(), invalid index vector = ") +
				MDAAuxiliaries::index_compare_string(iv, sizes());
			ForceDebugBreak();
			throw(invalid_argument(problem_description));
		}
	}
#endif //XRAD_CHECK_ARRAY_BOUNDARIES
}

//--------------------------------------------------------------

template<class A2DT>
void	DataArrayMD<A2DT>::GetSliceIndices(size_t &slice_index_0, size_t &slice_index_1, const index_vector &iv) const
{
	size_t	slice_dimension = 0;
	slice_index_0 = slice_index_1 = 0;
	//	bool	found_0(false), found_1(false);
	int	fault = 0x03;//00000011;
	size_t	m1 = 0xFFFFFFFF - 1;//11111110
	size_t	m2 = 0xFFFFFFFF - 2;//b11111101

	for(size_t i = 0; i < iv.size(); ++i)
	{
		size_t	index = iv[i];
		if(is_slice_mask(index))
		{
			switch(dimension_no(index))
			{
				case 0:
					++slice_dimension;
					slice_index_0 = i;
					//					found_0 = true;
					fault &= m1;
					break;

				case 1:
					++slice_dimension;
					slice_index_1 = i;
					//					found_1 = true;
					fault &= m2;
					break;

				default:
					//					fault = true;
					break;
			}
		}
	}
	//	if(!(found_0 && found_1)) fault = true;

#if XRAD_CHECK_ARRAY_BOUNDARIES
	if(!fault)
	{
		if(iv.size() != n_dimensions() || slice_dimension != 2) fault = true;
		else for(size_t i = 0; i < n_dimensions(); ++i)
		{
			if(!in_range(iv[i], 0u, sizes(i) - 1) && !is_slice_mask(iv[i])) fault |= 1 << (i + 2);
		}

	}
#endif //XRAD_CHECK_ARRAY_BOUNDARIES

	if(fault)
	{
		string	problem_description =
			typeid(self).name() +
			string("::GetSliceIndices(), invalid index vector = ") +
			MDAAuxiliaries::index_compare_string(iv, sizes()) +
			ssprintf("\nFault code = %X", fault);
		ForceDebugBreak();
		throw(invalid_argument(problem_description));
	}
}

template<class A2DT>
void	DataArrayMD<A2DT>::GetSubsetIndices(index_vector &subset_indices, const index_vector &iv) const
{
	bool	fault = false;
	size_t	subset_dimension = 0;

	if(iv.size() < 2 || iv.size() != n_dimensions())
	{
		// размерность многомерного массива не менее двух;
		// индексы подмножества должны соответствовать размерности массива
		fault = true;
	}

	if(!fault)
	{
		size_t	i(0), j(0);

		subset_indices.realloc(n_dimensions());
		// определяем размерность подмножества: ищем, есть ли в iv маркеры
		// 0, 1 и последующих координат
		while(i < n_dimensions() && j < n_dimensions())
		{
			j = 0;

			while(j < n_dimensions() && slice_mask(i) != iv[j]) ++j;
			if(j < n_dimensions())
			{
				++subset_dimension;
				subset_indices[i] = j;
				// маркер найден, увеличиваем размерность и отмечаем соотв. индекс.
				// однако возможна следующая ошибка, номер координаты может повторяться:
				for(size_t k = j + 1; k < n_dimensions(); ++k)
				{
					if(slice_mask(i) == iv[k]) fault = true, j = n_dimensions();
					//ошибка, выход из цикла
				}
			}
			++i;
		}
		subset_indices.resize(subset_dimension);

		if(!n_dimensions()) fault = true;
	}

	if(!fault)
	{
		for(size_t j = 0; j < n_dimensions(); ++j)
		{
			if(is_slice_mask(iv[j]))
			{
				// нет ли разрывов в номерах координат? например, запрошены
				// координаты среза 0, 1, 3. вышенаписанный алгоритм найдет
				// subset_dimension = 2, ошибку с "перескоком" находим здесь
				if(dimension_no(iv[j]) >= subset_dimension) fault = true;
			}
			else
			{
				// простая проверка диапазонов в номерах срезов
				if(!in_range(iv[j], 0u, sizes(j) - 1)) fault = true;
				// HERE
			}
		}

	}

	if(fault)
	{
		string	problem_description =
			typeid(self).name() +
			string("::GetSubsetIndices(subset_indices, index_vector), invalid index vector = ") +
			MDAAuxiliaries::index_compare_string(iv, sizes());
		ForceDebugBreak();
		throw(invalid_argument(problem_description));
	}
}

//--------------------------------------------------------------

template<class A2DT>
void	DataArrayMD<A2DT>::GenerateSizesAndSteps(const index_vector& iv)
{
	m_sizes.MakeCopy(iv);
	m_steps.realloc(iv.size(), 1);

	//ptrdiff_t, со знаком. условие выхода из цикла допускает отрицательные числа
	for(ptrdiff_t i = iv.size() - 2; i >= 0; --i)
	{
		m_steps[i] = steps(i+1) * sizes(i+1);
	}
}

template<class A2DT>
void DataArrayMD<A2DT>::CheckDataIntegrity() const
{
	//TODO CheckDataIntegrity() проверить тщательно
	bool	fault = false;
	if(n_dimensions() && parent::empty()) fault = true;
	if(!n_dimensions() && !parent::empty()) fault = true;

	if(!parent::empty()) for(size_t i = 0; i < n_dimensions(); ++i)
	{
		if(sizes(i) <= 0 || steps(i) <= 0) fault = true;
	}
	if(fault)
	{
		string problem_description = typeid(self).name() + string("::CheckDataIntegrity(), invalid dimensions,\nsizes=") + MDAAuxiliaries::index_string(sizes()) +
			string("\nsteps = ") + MDAAuxiliaries::index_string(steps());
		ForceDebugBreak();
		throw logic_error(problem_description);
	}
}



//--------------------------------------------------------------



template<class A2DT, class A2DT2>
void MakeCopy(DataArrayMD<A2DT> &destination, const DataArrayMD<A2DT2> &original)
{
	destination.MakeCopy(original);
}

template<class A2DT, class A2DT2>
void CopyData(DataArrayMD<A2DT> &destination, const DataArrayMD<A2DT2> &original)
	{
	Apply_AA_MD_Different_F2(destination, original, Functors::assign());
	}

template<class A2DT, class A2DT2, class functor>
void MakeCopy(DataArrayMD<A2DT> &destination, const DataArrayMD<A2DT2> &original, const functor &f)
{
	destination.MakeCopy(original, f);
}



//--------------------------------------------------------------



template <class A2DT, class F>
void ApplyFunction(DataArrayMD<A2DT> &array, const F &function)
{
	Apply_A_MD_RF1(array, function);
}

template <class A2DT, class F>
void ApplyAction(DataArrayMD<A2DT> &array, const F &function)
{
	Apply_A_MD_F1(array, function);
}



//--------------------------------------------------------------

XRAD_END

#endif //__data_array_multidimensional_cc
