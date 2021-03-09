/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file Iterators.hh
// Created by ACS on 17.02.02
//--------------------------------------------------------------

#ifndef XRAD__File_iterators_h
	#error This file must be included from <Iterators.h> only!
#endif

#include <XRADBasic/Sources/Core/String.h>

XRAD_BEGIN

//--------------------------------------------------------------
//
//	iterator_range_checker functions
//
//--------------------------------------------------------------



/*
template<class T, class DT>
inline void iterator_range_checker<T,DT>::
*/

template<class T, class DT>
string iterator_range_checker<T,DT>::iterator_problem_description() const
{
	return ssprintf("m_data = %p; m_position = %ti; min_pos = %ti; max_pos = %ti.",
		data(),
		EnsureType<ptrdiff_t>(position()), EnsureType<ptrdiff_t>(min_pos), EnsureType<ptrdiff_t>(max_pos));
}

template<class T, class DT>
string iterator_range_checker<T,DT>::value_problem_description() const
{
	return ssprintf("m_data = %p, index = %ti, size = %ti: '%s'",
		data(), EnsureType<ptrdiff_t>(position()), EnsureType<ptrdiff_t>(max_pos),
		not_a_number_description(*kernel_ptr->current_datum()).c_str()
		);
}

template<class T, class DT>
inline void iterator_range_checker<T,DT>::initialize_range_checker(iterator_kernel<T,DT> *in_it, value_type *data,
		difference_type min_p, difference_type max_p)
{
	kernel_ptr = in_it;
	m_data = data;
	min_pos = min_p;
	max_pos = max_p;
}

template<class T, class DT>
template <class T2, class DT2>
inline void	iterator_range_checker<T,DT>::initialize_range_checker(iterator_kernel<T,DT> *in_it, const iterator_range_checker<T2, DT2> &other)
{
	kernel_ptr = in_it;
	m_data = other.m_data;
	min_pos = other.min_pos;
	max_pos = other.max_pos;
}

//--------------------------------------------------------------

template<class T, class DT>
inline void iterator_range_checker<T,DT>::check_range() const
{
	if(!kernel_ptr)
	{
		if(min_pos||max_pos)
		{
			string	problem_description = ssprintf("iterator_range_checker<%s,%s>::check_range\n"
					"Null pointer, non-zero data limits (%ti,%ti)",
					typeid(T).name(),
					typeid(DT).name(),
					EnsureType<ptrdiff_t>(min_pos),
					EnsureType<ptrdiff_t>(max_pos));
			ForceDebugBreak();
			throw(invalid_argument(problem_description));
		}
		return;
	}
	if(!data())
	{
		// range_checker связан с неинициализированным итератором -- это допустимо.
		//  но если в нем оказалась ненулевая позиция, то ошибка
		if(position())
		{
			string	problem_description = ssprintf("iterator_range_checker<%s,%s>::check_range\n%s",
					typeid(T).name(),
					typeid(DT).name(),
					EnsureType<const char*>(iterator_problem_description().c_str()));
			ForceDebugBreak();
			throw(out_of_range(problem_description));
		}
		return;
	}

	// TODO: Значения указателей в итераторе end() не соответствуют стандарту при step > 1 и при step < 0
	// (значения находятся вне допустимого диапазона array_first .. array_last+1).
	// Возможно, проблема трудноразрешима.
	if(kernel_ptr->step() >= 0)
	{
		if(position() < min_pos || position() > max_pos)
		{
			string	problem_description = ssprintf("iterator_range_checker<%s,%s>::check_range\n%s",
					typeid(T).name(),
					typeid(DT).name(),
					EnsureType<const char*>(iterator_problem_description().c_str()));
			ForceDebugBreak();
			throw(out_of_range(problem_description));
		}
	}
	else
	{
		if(position() > min_pos || position() < max_pos)
		{
			string	problem_description = ssprintf("iterator_range_checker<%s,%s>::check_range\n%s",
					typeid(T).name(),
					typeid(DT).name(),
					EnsureType<const char*>(iterator_problem_description().c_str()));
			ForceDebugBreak();
			throw(out_of_range(problem_description));
		}
	}
}

//--------------------------------------------------------------

template<class T, class DT>
inline void iterator_range_checker<T,DT>::check_data() const
{
	if(!kernel_ptr)
	{
		string	problem_description = ssprintf("iterator_range_checker<%s,%s>::check_data\nNull iterator pointer",
				typeid(T).name(),
				typeid(DT).name());
		ForceDebugBreak();
		throw(invalid_argument(problem_description));
	}
	if(!data())
	{
		string	problem_description = ssprintf("iterator_range_checker<%s,%s>::check_data\n%s",
				typeid(T).name(),
				typeid(DT).name(),
				EnsureType<const char*>(iterator_problem_description().c_str()));
		ForceDebugBreak();
		throw(out_of_range(problem_description));
	}
	if(kernel_ptr->step() >= 0)
	{
		if(position() < min_pos || position() >= max_pos)
		{
			string	problem_description = ssprintf("iterator_range_checker<%s,%s>::check_data\n%s",
					typeid(T).name(),
					typeid(DT).name(),
					EnsureType<const char*>(iterator_problem_description().c_str()));
			ForceDebugBreak();
			throw(out_of_range(problem_description));
		}
	}
	else
	{
		if(position() <= max_pos || position() > min_pos)
		{
			string	problem_description = ssprintf("iterator_range_checker<%s,%s>::check_data\n%s",
					typeid(T).name(),
					typeid(DT).name(),
					EnsureType<const char*>(iterator_problem_description().c_str()));
			ForceDebugBreak();
			throw(out_of_range(problem_description));
		}
	}
}

template<class T, class DT>
inline void iterator_range_and_value_checker<T,DT>::check_data() const
{
	// проверяет возможность обращения к массиву:
	parent::check_data();
#if XRAD_CHECK_NAN_FLOAT
	if(!is_number(*kernel_ptr->current_datum()))
	{
		string	problem_description = ssprintf("iterator_range_checker<%s,%s>::check_data\n%s",
				typeid(T).name(),
				typeid(DT).name(),
				EnsureType<const char*>(value_problem_description().c_str()));
		ForceDebugBreak();
		throw(invalid_argument(problem_description));
	}
#endif // XRAD_CHECK_NAN_FLOAT
}

template<class T, class DT>
inline void iterator_range_checker<T,DT>::check_pair(const self &second) const
{
	// проверка диапазонов с условием false, т.к. вызов данной функции
	// предусмотрен только в операторах сравнения
	check_range();
	second.check_range();
	if(!kernel_ptr || !second.kernel_ptr)
	{
		string	problem_description = ssprintf("iterator_range_checker<%s,%s>::check_pair\nNull iterator pointer",
				typeid(T).name(),
				typeid(DT).name());
		ForceDebugBreak();
		throw(invalid_argument(problem_description));
	}

	if(data() != second.data() || min_pos != second.min_pos || max_pos != second.max_pos)
	{
		string	problem_description = ssprintf("iterator_range_checker<%s,%s>::check_pair\n%s\n%s",
				typeid(T).name(),
				typeid(DT).name(),
				EnsureType<const char*>(iterator_problem_description().c_str()),
				EnsureType<const char*>(second.iterator_problem_description().c_str()));
		ForceDebugBreak();
		throw(invalid_argument(problem_description));
	}
}



//--------------------------------------------------------------
//
//	constructors
//
//--------------------------------------------------------------



template<class T, class CH>
inline step_iterator<T, CH>::step_iterator()
{
	kernel.initialize_iterator(NULL,0,0);
	checker.initialize_range_checker(NULL, NULL, 0, 0);
	checker.check_range();
}

//--------------------------------------------------------------

template<class T, class CH>
inline step_iterator<T, CH>::step_iterator( value_type *array, difference_type i_step, difference_type i_count, difference_type i_current)
{
	kernel.initialize_iterator(array, i_step, i_current);
	checker.initialize_range_checker(&kernel, array, 0, i_count*i_step);
	checker.check_range();
}



//--------------------------------------------------------------
//
//	compare operators
//
//--------------------------------------------------------------



template<class T, class CH>
inline typename step_iterator<T, CH>::difference_type step_iterator<T, CH>::operator-( const self &second) const
{
	checker.check_pair(second.checker);
		// сравнение итераторов с различными базовыми указателями
		// будет давать исключение invalid_argument
	return kernel-second.kernel;
}

//--------------------------------------------------------------

template<class T, class CH>
inline bool step_iterator<T, CH>::operator==( const step_iterator &second) const
{
	checker.check_pair(second.checker);
	return kernel.current_datum() == second.kernel.current_datum();
}

template<class T, class CH>
inline bool step_iterator<T, CH>::operator > ( const step_iterator &second) const
{
	checker.check_pair(second.checker);
	return kernel.current_datum() > second.kernel.current_datum();
	//return (*this-second) > 0;
}

template<class T, class CH>
inline bool step_iterator<T, CH>::operator < ( const step_iterator &second) const
{
	checker.check_pair(second.checker);
	return kernel.current_datum() < second.kernel.current_datum();
	//return (*this-second) < 0;
}

template<class T, class CH>
inline bool step_iterator<T, CH>::operator >= ( const step_iterator &second) const
{
	checker.check_pair(second.checker);
	return kernel.current_datum() >= second.kernel.current_datum();
	//return (*this-second) >= 0;
}

template<class T, class CH>
inline bool step_iterator<T, CH>::operator <= ( const step_iterator &second) const
{
	checker.check_pair(second.checker);
	return kernel.current_datum() <= second.kernel.current_datum();
	//return (*this-second) <= 0;
}

template<class T, class CH>
inline bool step_iterator<T, CH>::operator!=( const step_iterator &second) const
{
	checker.check_pair(second.checker);
	return kernel.current_datum() != second.kernel.current_datum();
	//return (*this-second) != 0;
}



//--------------------------------------------------------------
//
//	arithmetics
//
//--------------------------------------------------------------



template<class T, class CH>
inline step_iterator<T, CH> &step_iterator<T, CH>::operator++()
{
	++kernel;
	checker.check_range();
	return *this;
}

template<class T, class CH>
inline step_iterator<T, CH> &step_iterator<T, CH>::operator--()
{
	--kernel;
	checker.check_range();
	return *this;
}

//--------------------------------------------------------------

template<class T, class CH>
inline step_iterator<T, CH> step_iterator<T, CH>::operator++(int)
{
	self tmp = *this;
	operator++();
	return tmp;
}

template<class T, class CH>
inline step_iterator<T, CH> step_iterator<T, CH>::operator--(int)
{
	self tmp = *this;
	operator--();
	return tmp;
}

//--------------------------------------------------------------

template<class T, class CH>
inline step_iterator<T, CH> step_iterator<T, CH>::operator+(difference_type d) const
{
	self tmp = *this;
	return tmp+=d;
}

template<class T, class CH>
inline step_iterator<T, CH> step_iterator<T, CH>::operator-(difference_type d) const
{
	self tmp = *this;
	return tmp-=d;
}

//--------------------------------------------------------------

template<class T, class CH>
inline step_iterator<T, CH> &step_iterator<T, CH>::operator+=( difference_type d)
{
	kernel += d;
	checker.check_range();
	return *this;
}

template<class T, class CH>
inline step_iterator<T, CH> &step_iterator<T, CH>::operator-=( difference_type d)
{
	kernel -= d;
	checker.check_range();
	return *this;
}



//--------------------------------------------------------------
//
//	access
//
//--------------------------------------------------------------



template<class T, class CH>
inline typename  step_iterator<T,CH>::reference step_iterator<T,CH>::operator[](typename step_iterator<T,CH>::difference_type i) const
{
	return *(*this + i);
}

template<class T, class CH>
inline typename  step_iterator<T,CH>::reference step_iterator<T,CH>::operator*() const
{
	checker.check_data();
	return *kernel.current_datum();
}

template<class T, class CH>
inline typename  step_iterator<T,CH>::pointer step_iterator<T,CH>::operator->() const
{
	checker.check_data();
	return kernel.current_datum();
}

template<class T, class CH>
inline typename  step_iterator<T,CH>::reference step_iterator<T,CH>::operator[](typename step_iterator<T,CH>::difference_type i)
{
	return *(*this + i);
}

template<class T, class CH>
inline typename  step_iterator<T,CH>::reference step_iterator<T,CH>::operator*()
{
	checker.check_data();
	return *kernel.current_datum();
}

template<class T, class CH>
inline typename  step_iterator<T,CH>::pointer step_iterator<T,CH>::operator->()
{
	checker.check_data();
	return kernel.current_datum();
}



XRAD_END
//--------------------------------------------------------------
