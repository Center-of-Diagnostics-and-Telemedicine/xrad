#ifndef __data_array_cc
#define __data_array_cc


#include "BasicArrayInteractions1D.h"
#include <XRADBasic/Sources/Core/Functors.h>

XRAD_BEGIN

//--------------------------------------------------------------

template<class T>
DataArray<T> &DataArray<T>::operator=(const self &original)
{
	return operator=<T>(original);
}


template<class T>
template<class T2>
DataArray<T> &DataArray<T>::operator=(const DataArray<T2> &original)
{
	// присваивание выполняется только между массивами одинаковых размеров.
	// единственное исключение -- если первый массив совсем пустой
	if(!size())
	{
		if (!original.size())
			return *this;
		realloc(original.size());
	}
	else if(size()!=original.size())
	{
		string problem_description = ssprintf("%s::operator=(%s original) -- array size mismatch: original size (%zu), current size (%zu)",
					EnsureType<const char*>(typeid(self).name()),
					EnsureType<const char*>(typeid(original).name()),
					EnsureType<size_t>(original.size()),
					EnsureType<size_t>(size()));
		ForceDebugBreak();
		throw(invalid_argument(problem_description));
	}
	Apply_AA_1D_F2(*this, original, Functors::assign());
	return (*this);
}

//--------------------------------------------------------------

template<class T>
auto DataArray<T>::data() -> value_type*
{
// доступ к данным как к указателю на сплошной массив памяти.
// возможно только тогда, когда эти данные реально существуют
// и когда шаг равен 1.
// возможен доступ не const, именно так.
	if(step() == 1 && size())
	{
		return &at(0);
	}
	else
	{
		string problem_description = typeid(self).name() +
			ssprintf("::data() -- invalid access, data step=%d, size is %d",
					step(), size());
		ForceDebugBreak();
		throw logic_error(problem_description);
	}
}

template<class T>
auto DataArray<T>::data() const -> const value_type *
{
// доступ к данным как к указателю на сплошной массив памяти.
// возможно только тогда, когда эти данные реально существуют
// и когда шаг равен 1.
	if(step() == 1 && size())
	{
		return &at(0);
	}
	else
	{
		string problem_description = typeid(self).name() +
			ssprintf("::data() -- invalid access, data step=%d, size is %d",
					step(), size());
		ForceDebugBreak();
		throw logic_error(problem_description);
	}
}

//--------------------------------------------------------------

template<class T>
void	DataArray<T>::resize(size_t in_size)
{
	if(!size())
	{
		realloc(in_size);
		return;
	}
	if(uses_external_data())
	{
		ForceDebugBreak();
		throw(logic_error("DataArray<T>::resize -- can't resize array which uses external data"));
	}
	if(in_size == size())
		return;	// ничего не делает, если размеры одинаковы
	//
	DataArray<T> new_data(in_size);
	if (!empty())
		new_data.CopyData(*this);
	MoveData(new_data);

//TODO Для наследников вызывается только родительский метод. Это может быть чревато ошибками.
	//	Предлагаю обдумать следующий вариант. В аргументы шаблона добавляется второй аргумент
	//	child_type (возможно, со значением по умолчанию =DataArray<T>), который будет вести в итоге к последнему наследнику.
	//	И его-то и использовать во всех подобных местах.
	//	Вышеприведенный код мог бы выглядеть следующим образом:
	//	child_type	new_data(in_size);
	//	auto	child_this = *reinterpret_cast<child_type*>(this);
	//	new_data.CopyData(child_this);
	//	child_this->MoveData(new_data);
}

//--------------------------------------------------------------

template<class T>
void DataArray<T>::fill(const T &value)
{
	Apply_A_1D_F1(*this, [&value](value_type &v) { v = value; });
}



//--------------------------------------------------------------



template<class T>
template<class T2>
void	DataArray<T>::MakeCopy(const DataArray<T2> &original)
{
	if(static_cast<const void*>(&original) == static_cast<const void*>(this))
	{
	// если предлагается скопировать себя в себя же, выходим сразу.
	// приведение к void* стало нужным для шаблонных вызовов с разными
	// типами массивов
		return;
	}
	realloc(original.size());
	Apply_AA_1D_F2(*this, original, Functors::assign());
}

template<class T>
template<class T2, class F>
void	DataArray<T>::MakeCopy(const DataArray<T2> &original, const F& function)
{
	realloc(original.size());
	Apply_AA_1D_F2(*this, original, function);
}



//--------------------------------------------------------------



template<class T>
template<class T2>
void	DataArray<T>::CopyData(const DataArray<T2> &original, extrapolation::method ex)
{
	if(static_cast<const void*>(&original) == static_cast<const void*>(this))
	{
	// если предлагается скопировать себя в себя же, выходим сразу.
	// приведение к void* стало нужным для шаблонных вызовов с разными
	// типами массивов
		return;
	}
	Apply_AA_1D_Different_F2(*this, original, Functors::assign(), ex);
}

template<class T>
template<class T2, class F>
void	DataArray<T>::CopyData(const DataArray<T2> &original, const F& function, extrapolation::method ex)
{
	Apply_AA_1D_Different_F2(*this, original, function, ex);
}

template<class T>
template<class T2>
void	DataArray<T>::CopyData(const T2 *new_data, ptrdiff_t in_step)
{
		step_iterator<const T2, iterator_checker_none<const T2, ptrdiff_t> > data_it(new_data, in_step, size(), 0);
		// в std::copy предупреждение C4244 (потеря точности) не по делу. поэтому цикл с явным указанием преобразования типов
		// std::copy(data_it, data_it+size(), begin());
		for(auto it = begin(); it<end(); ++it, ++data_it) *it = static_cast<value_type>(*data_it);
}

template<class T>
template<class T2, class F>
void	DataArray<T>::CopyData(const T2 *new_data, ptrdiff_t in_step, const F &function)
{
	step_iterator<const T2, iterator_checker_none<const T2, ptrdiff_t> > data_it(new_data, in_step, size(), 0);
	for(auto it = begin(); it<end(); ++it, ++data_it) *it = function(*data_it);
}



//--------------------------------------------------------------



template<class T>
template <class Array>
Array DataArray<T>::GetDataFragment(size_t from, size_t to, ptrdiff_t in_step)
{
	Array array;
	array.UseDataFragment(*this, from, to, in_step);
	return array;
}

template<class T>
template <class Array>
Array DataArray<T>::GetDataFragment(size_t from, size_t to, ptrdiff_t in_step) const
{
	Array array;
	array.UseDataFragment(*this, from, to, in_step);
	return array;
}



//--------------------------------------------------------------



template<class T>
template<class T2, class ComponentSelector>
void DataArray<T>::GetDataComponent(DataArray<T2> &component,
		ComponentSelector component_selector)
{
	using component_type = std::remove_reference_t<
			decltype(*component_selector(std::declval<value_type*>()))>;
	static_assert(sizeof(typename DataArray<T2>::value_type) == sizeof(component_type),
			"DataArray<T>::GetDataComponent: Incompatible data types.");
	// Проверка на то, что шаг данных у компоненты кратен размеру компоненты.
	// Это ограничение текущей реализации DataOwner.
	static_assert(sizeof(value_type) % sizeof(component_type) == 0,
			"DataArray<T>::GetDataComponent: Data step must be a multiple of data size for DataOwner.");
#ifdef XRAD_DEBUG
	{
		// Проверка на то, что компонента находится внутри исходных данных.
		auto *data_origin = &at(0);
		auto *component_origin = (void*)component_selector(data_origin);
		if (component_origin < (void*)data_origin ||
				component_origin >= (void*)(data_origin + 1))
		{
			throw runtime_error("DataArray<T>::GetDataComponent: the component selector gives "
					"a component outside original data.");
		}
	}
#endif
	component.UseData(component_selector(&at(0)), size(),
			step() * ptrdiff_t(sizeof(value_type) / sizeof(component_type)));
}

template<class T>
template<class T2, class ComponentSelector>
void DataArray<T>::GetDataComponent(DataArray<T2> &component,
		ComponentSelector component_selector) const
{
	using component_type = std::remove_reference_t<
			decltype(*component_selector(std::declval<value_type*>()))>;
	static_assert(sizeof(typename DataArray<T2>::value_type) == sizeof(component_type),
			"DataArray<T>::GetDataComponent: Incompatible data types.");
	// Проверка на то, что шаг данных у компоненты кратен размеру компоненты.
	// Это ограничение текущей реализации DataOwner.
	static_assert(sizeof(value_type) % sizeof(component_type) == 0,
			"DataArray<T>::GetDataComponent: Data step must be a multiple of data size for DataOwner.");
#ifdef XRAD_DEBUG
	{
		// Проверка на то, что компонента находится внутри исходных данных.
		auto *data_origin = &at(0);
		auto *component_origin = (void*)component_selector(data_origin);
		if (component_origin < (void*)data_origin ||
				component_origin >= (void*)(data_origin + 1))
		{
			throw runtime_error("DataArray<T>::GetDataComponent: the component selector gives "
					"a component outside original data.");
		}
	}
#endif
	component.UseData(component_selector(&at(0)), size(),
			step() * ptrdiff_t(sizeof(value_type) / sizeof(component_type)));
}

template<class T>
template<class Array, class ComponentSelector>
auto DataArray<T>::GetDataComponent(ComponentSelector component_selector) ->
		default_array_t<Array,
				decltype(*std::declval<ComponentSelector>()(std::declval<value_type*>()))>
{
	default_array_t<Array,
			decltype(*std::declval<ComponentSelector>()(std::declval<value_type*>()))> array;
	GetDataComponent(array, component_selector);
	return array;
}

template<class T>
template<class Array, class ComponentSelector>
auto DataArray<T>::GetDataComponent(ComponentSelector component_selector) const ->
		default_array_t<Array,
				decltype(*std::declval<ComponentSelector>()(std::declval<value_type_invariable*>()))>
{
	default_array_t<Array,
			decltype(*std::declval<ComponentSelector>()(std::declval<value_type_invariable*>()))> array;
	GetDataComponent(array, component_selector);
	return array;
}



//--------------------------------------------------------------



template<class T>
void	DataArray<T>::roll(ptrdiff_t roll_distance)
{
	while(roll_distance < 0) roll_distance += size();
	if(!(roll_distance %= size())) return;

	DataArray<T>	buffer(roll_distance);

	ptrdiff_t	i = 0;
	// для сравнения используем индекс, так как сравнение итераторов более накладно
	for(iterator buffer_it = buffer.begin(), it = end()-roll_distance; i<roll_distance; ++buffer_it, ++it, ++i)
	{
		*buffer_it = *it;
	}

	i=size()-roll_distance;
	for(iterator it1 = end()-roll_distance, it2 = end(); i>0;--i)
	{
		*(--it2) = *(--it1);
	}

	i=0;
	for(iterator buffer_it = buffer.begin(), it = begin(); i<roll_distance; ++it, ++buffer_it, ++i)
	{
		*it = *buffer_it;
	}
}

template<class T>
void	DataArray<T>::roll_half(bool forward)
{
	if(size() == 1) return;
	// корректно: для функции из одного отсчета roll_half ничего не меняет
	size_t	roll_distance = size()/2;

	// при нечетных size алгоритм без буфера работает неправильно
	if(size()%2)
	{
		if(forward) roll(roll_distance);
		else roll(roll_distance+1);
	}
	else
	{
		iterator it1 = begin(), it2 = it1+roll_distance;
		for(size_t i = 0; i < roll_distance; ++i, ++it1, ++it2)
		{
			std::swap(*it1, *it2);
		}
	}
}



//--------------------------------------------------------------



template<class T>
void	reorder_ascent(DataArray<T> &x)
{
	if(!x.size()) return;
	std::sort(x.begin(), x.end());
}

template<class T>
void	reorder_descent(DataArray<T> &x)
{
	if(!x.size()) return;
	std::sort(x.rbegin(), x.rend());
}

template<class T, class Compare>
void	reorder(DataArray<T> &x, Compare comp)
{
	if(!x.size()) return;
	std::sort(x.begin(), x.end(), comp);
}



//--------------------------------------------------------------



template <class T1, class T2>
void	MakeCopy(DataArray<T1> &destination, const DataArray<T2> &source)
{
	destination.MakeCopy(source);
}

template <class T1, class T2, class F>
void	MakeCopy(DataArray<T1> &destination, const DataArray<T2> &original, const F &function)
{
	destination.MakeCopy(original, function);
}

template <class T1, class T2>
void	CopyData(DataArray<T1> &destination, const DataArray<T2> &source)
{
	destination.CopyData(source);
}

template <class T1, class T2, class F>
void	CopyData(DataArray<T1> &destination, const DataArray<T2> &original, const F &function)
{
	destination.CopyData(original, function);
}



//--------------------------------------------------------------



template <class T, class F>
void ApplyFunction(DataArray<T> &array, const F &function)
{
	Apply_A_1D_RF1(array, function);
}

template <class T, class F>
void ApplyAction(DataArray<T> &array, const F &function)
{
	Apply_A_1D_F1(array, function);
}



//--------------------------------------------------------------

XRAD_END

#endif // __data_array_cc
