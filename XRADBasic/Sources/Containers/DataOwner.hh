//	file DataOwner.hh
//--------------------------------------------------------------
// File "pre.h" should not be included

#ifndef XRAD__File_dataowner_cc
#define XRAD__File_dataowner_cc

#include <XRADBasic/Sources/Core/String.h>
#include <XRADBasic/Sources/Core/BasicUtils.h>
#include <type_traits>

XRAD_BEGIN


//--------------------------------------------------------------
//
//	работа с памятью
//
//	оказалось, что Code Composer страшно медленно обрабатывает конструкцию с qdo_create etc.
//	(инициализация по умолчанию новых массивов). с new[] таких проблем не возникает
//	в этом случае от идеи с изначально проинициализированными массивами
//	пришлось отказаться, но это действительно излишество, которое могло
//	быть терпимо, пока не создавало проблем
//


template<class VT>
void DataOwner<VT>::allocate(size_t s)
{
	// все инициализируется нулями для корректной обработки ошибок в последующем
	m_size = 0;
	m_step = 0;
	m_data = NULL;
	m_ownData = false;

	if(s>0)
	{
		// аллокируем через new[], память инициализируется только
		// при наличии конструктора (для простых типов вроде int, double
		// остается неинициализированной)
		// удаляем через delete[]

		try
		{
			m_size = s;
			m_step = 1;
			m_ownData = true;
			// TODO: Для const-типов не долюно быть операций allocate вообще.
			// Контейнеры с такими типами могут использоваться только как ссылки на внешние данные
			// или быть пустыми.
			m_data = new std::remove_const_t<VT>[m_size];
		}
		catch(bad_alloc &)
		{
			// нехватка памяти
			ForceDebugBreak();

			m_size = 0;
			m_step = 0;
			m_ownData = false;
			m_data = NULL;

			throw;
			// альтернативный вариант обработки: генерировать исключение с развернутой информацией в what(). к сожалению,
			// передать текст внутри bad_alloc не получается, поэтому нужно передавать какой-то более развитый тип.
			// throw(invalid_argument(typeid(self).name() + ssprintf("::allocate(int s = %d), out of memory", s)));
		}
		catch(...)
		{
			// проверка других исключений
			ForceDebugBreak();

			m_size = 0;
			m_step = 0;
			m_ownData = false;
			// вначале m_data была проинициализирована нулем, так что
			// удаление ненулевого массива здесь корректно
			DestroyArray(m_data);

			throw;
		}
	}
}


template<class VT>
void DataOwner<VT>::dispose()
{
	if(m_ownData)
	{
		delete[] (m_step >= 0 || !m_size? m_data: m_data + m_step * ptrdiff_t(m_size - 1));
		m_data = nullptr;
		m_ownData = false;
	}
	else
	{
		m_data = nullptr;
	}
	m_size = 0;
	m_step = 0;
}



//--------------------------------------------------------------
//
//	использование внешних данных
//	UseData - обрабатывает внешний указатель
//
//--------------------------------------------------------------


template<class VT>
inline void	DataOwner<VT>::UseData(VT *new_data, size_t in_size, ptrdiff_t in_step)
{
	if(m_ownData)
		dispose();
	//	здесь и ниже:
	//	4 функции присваивания выполнялись впустую.
	//	при частом пользовании возможна потеря темпа
	m_data = new_data;
	m_size = in_size;
	m_step = in_step;
	m_ownData = false;
}

template<class VT>
inline void	DataOwner<VT>::UseData(DataOwner<VT> &new_data)
{
	if(m_ownData)
		dispose();
	m_data = new_data.m_data;
	m_size = new_data.m_size;
	m_step = new_data.m_step;
	m_ownData = false;
}

template<class VT>
template <class VT2>
void DataOwner<VT>::UseData(DataOwner<VT2> &new_data)
{
	new_data.GetData(*this);
}

template<class VT>
template <class VT2>
void DataOwner<VT>::UseData(const DataOwner<VT2> &new_data)
{
	new_data.GetData(*this);
}

template<class VT>
inline void	DataOwner<VT>::UseData(DataOwner<VT> &new_data, size_t in_size, ptrdiff_t in_step)
{
	// в этой функции шаг измеряется в элементах исходного массива, а не в ячейках памяти
	if(m_ownData)
		dispose();

	if(!in_range(in_size*in_step, 0, new_data.m_size))
	{
		ForceDebugBreak();
	}


	m_data = new_data.m_data;
	m_size = in_size;
	m_step = new_data.m_step * in_step;
	m_ownData = false;
}

template<class VT>
void	DataOwner<VT>::UseData(typename DataOwner<VT>::iterator new_data_start, typename DataOwner<VT>::iterator new_data_end)
{
	ptrdiff_t	new_step = (&*new_data_end - &*new_data_start)/(new_data_end-new_data_start);
	UseData(&*new_data_start, new_data_end-new_data_start, new_step);
}



template<class VT>
template<class VT2>
void	DataOwner<VT>::UseDataFragment(DataOwner<VT2> &new_data, size_t from, size_t to, ptrdiff_t in_step)
{
	static_assert(sizeof(typename DataOwner<VT2>::value_type) == sizeof(value_type),
			"DataOwner<VT>::UseDataFragment: Incompatible data types.");
	if(from >= to || to > new_data.size() || in_step <= 0)
	{
		string problem_description = ssprintf("%s::UseDataFragment() -- invalid params. Data range: (0, %zu), requested range: (%zu, %zu), step: %ti",
				EnsureType<const char*>(typeid(self).name()),
				EnsureType<size_t>(new_data.size()),
				EnsureType<size_t>(from),
				EnsureType<size_t>(to),
				EnsureType<ptrdiff_t>(in_step));
		ForceDebugBreak();
		throw invalid_argument(problem_description);
	}
	if(m_ownData)
	{
		dispose();
	}
	UseData(&new_data.at(from), (to-from)/in_step, in_step*new_data.step());
}

template<class VT>
template<class VT2>
void	DataOwner<VT>::UseDataFragment(const DataOwner<VT2> &new_data, size_t from, size_t to, ptrdiff_t in_step)
{
	static_assert(sizeof(typename DataOwner<VT2>::value_type) == sizeof(value_type),
			"DataOwner<VT>::UseDataFragment: Incompatible data types.");
	if(from >= to || to > new_data.size() || in_step <= 0)
	{
		string problem_description = ssprintf("%s::UseDataFragment() -- invalid params. Data range: (0, %zu), requested range: (%zu, %zu), step: %ti",
				EnsureType<const char*>(typeid(self).name()),
				EnsureType<size_t>(new_data.size()),
				EnsureType<size_t>(from),
				EnsureType<size_t>(to),
				EnsureType<ptrdiff_t>(in_step));
		ForceDebugBreak();
		throw invalid_argument(problem_description);
	}
	if(m_ownData)
	{
		dispose();
	}
	UseData(&new_data.at(from), (to-from)/in_step, in_step*new_data.step());
}



//--------------------------------------------------------------



template<class VT>
template <class VT2>
void DataOwner<VT>::GetData(DataOwner<VT2> &container)
{
	static_assert(sizeof(typename DataOwner<VT2>::value_type) == sizeof(value_type),
			"DataOwner<VT>::GetData: Incompatible data types.");
	if ((void*)&container == (void*)this)
		throw invalid_argument("Error: DataOwner::GetData(DataOwner &container): &container == this.");
	container.UseData(m_data, m_size, m_step);
}

template<class VT>
template <class VT2>
void DataOwner<VT>::GetData(DataOwner<VT2> &container) const
{
	static_assert(sizeof(typename DataOwner<VT2>::value_type) == sizeof(value_type),
			"DataOwner<VT>::GetData: Incompatible data types.");
	if ((void*)&container == (void*)this)
		throw invalid_argument("Error: DataOwner::GetData(DataOwner &container): &container == this.");
	const value_type *const_data = m_data;
	container.UseData(const_data, m_size, m_step);
}

//--------------------------------------------------------------

template<class VT>
void	DataOwner<VT>::realloc(size_t newSize)
{
	if(newSize == m_size && !uses_external_data())
	{
		// после 03.05.2011 созданные массивы не будут инициализироваться
		// чем-либо кроме new[] при выделении памяти. потеря темпа очень ощутима
		// в некоторых случаях
		return;
	}

	if(m_ownData)
		dispose();
	allocate(newSize);
}

//--------------------------------------------------------------

template<class VT>
void DataOwner<VT>::reverse()
{
	if (empty())
		return;
	m_data += ptrdiff_t(m_size - 1) * m_step;
	m_step = -m_step;
}




//--------------------------------------------------------------
//
//	методы проверки границ массива и выявления плохих чисел
//

//	в общем случае только проверка границ
template<class VT>
inline	void DataOwner<VT>::check_data_and_index(size_t index) const
{
	check_array_boundaries(index);
}

//	для типов с плавающей запятой добавляется проверка NAN при включенной опции XRAD_CHECK_NAN_FLOAT
template<>
inline	void DataOwner<float>::check_data_and_index(size_t index) const
{
	check_array_boundaries(index);
	check_data_nan(index);
}

template<>
inline	void DataOwner<double>::check_data_and_index(size_t index) const
{
	check_array_boundaries(index);
	check_data_nan(index);
}




template<class VT>
#if !XRAD_CHECK_ARRAY_BOUNDARIES
void DataOwner<VT>::check_array_boundaries(size_t ) const{}
#else
void DataOwner<VT>::check_array_boundaries(size_t index) const
{
	if( !m_data)
	{
		string problem_description = typeid(self).name() + ssprintf("::at(size_t): m_data==NULL, index = %d, size = %d", index, size());
		ForceDebugBreak();
		throw(invalid_argument(problem_description));
	}
	if(index < 0 || index >= size())
	{
		string problem_description = typeid(self).name() + ssprintf("::at(size_t): m_data==%X, index = %d, size = %d", reinterpret_cast<const size_t>(m_data), index, size());
		ForceDebugBreak();
		throw(out_of_range(problem_description));
	}
}
#endif


template<class VT>
#if !XRAD_CHECK_NAN_FLOAT
void DataOwner<VT>::check_data_nan(size_t ) const{}
#else
void DataOwner<VT>::check_data_nan(size_t index) const
{
	if(!is_number( *(m_data+index*step())))
	{
		ForceDebugBreak();
		throw(invalid_argument(typeid(self).name() + ssprintf(
			"::at(size_t): m_data==%X, index = %d, size = %d: value='%s'", (unsigned size_t)m_data, index, size(),
			not_a_number_description(*(m_data+index*step())).c_str()
			)));
	}
}
#endif // XRAD_CHECK_NAN_FLOAT



//--------------------------------------------------------------
//
//	методы доступа
//
//--------------------------------------------------------------



template<class VT>
inline const	VT &DataOwner<VT>::at(size_t i) const
{
	check_data_and_index(i);
	return *(m_data+i*step());
}


template<class VT>
inline VT &DataOwner<VT>::at(size_t i)
{
	check_data_and_index(i);
	return *(m_data+i*step());
}


//--------------------------------------------------------------


namespace
{
	//! \brief Назначение этого вызова: см. комментарий к internal_ContainerChecksControl::InitFlowControl()
	//!
	//! Объявляется в неименованном namespace, чтобы не было конфликтов при линковке
	int	FlowControlStatus = internal_ContainerChecksControl::InitFlowControl();
}

//--------------------------------------------------------------
XRAD_END

#endif //XRAD__File_dataowner_cc
