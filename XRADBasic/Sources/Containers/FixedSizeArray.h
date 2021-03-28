/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef FixedSizeArray_h__
#define FixedSizeArray_h__
//--------------------------------------------------------------
/*!
	\file
	\date 2015-01-26 11:40
	\author kns
	\brief Контейнер фиксированного размера (размер задается на этапе компиляции)
*/
//--------------------------------------------------------------

#include "ReferenceOwner.h"
#include "Iterators.h"
#include "BasicArrayInteractions1D.h"
#include <XRADBasic/Sources/Core/Functors.h>


XRAD_BEGIN

//! \brief Контейнер фиксированного размера для массива вида vaule_type[N] (размер задается на этапе компиляции)
//!
//! \todo Проверить соответствие фиксированного массива с DataArray. Объединить общее в базовый класс.
template <class VT, size_t N>
class FixedSizeArray
{
	public:
		typedef VT value_type;
		typedef const VT value_type_invariable;
		typedef	typename remove_const<VT>::type value_type_variable;

		typedef FixedSizeArray<VT,N> self;
		typedef FixedSizeArray<const VT,N> invariable;

		//! \brief Константа с длиной массива для обращения снаружи
		static const size_t fixed_size = N;

		//! \name Internal data
		//! @{
	private:
		value_type m_data[N];

		enum
		{
			n_components = N
		};

		enum
		{
			m_step = 1,
			m_size = N,
			m_ownData = true
		};
		//! @}

		//! \name Constructors and destructor
		//! @{
	public:
		FixedSizeArray() = default;
		FixedSizeArray(const VT& default_value) { std::fill(begin(), end(), default_value); }

		template<class T2>
		FixedSizeArray(const FixedSizeArray<T2,N> &original) { MakeCopy(original); }
		//! @}

	protected:
		inline bool uses_external_data() const { return false;}

		//! \name Info
		//! @{

		//! \brief Количество элементов в массиве
		inline size_t element_count() const { return m_size;}
	public:
		inline size_t size() const { return m_size;}
		inline size_t step() const {return m_step;}
		inline size_t element_size() const { return sizeof(value_type); }

		// Для совместимости с многомерными алгоритмами
		inline size_t sizes(size_t) const {return m_size;}
		inline size_t steps(size_t) const {return m_step;}

		inline bool empty() const {return !N;}
		inline bool ready() const {return bool(N);}
		//! @}

	public:
		/*!
			\name Методы доступа к содержимому контейнера

			Конечному пользователю доступны только at() и operator[], а после и ().
			Оператор () задать после того, как во всех зависимых программах вызов интерполятора,
			ранее выполнявшийся через operator() (float), будет заменен на функцию in(size_t).
			Функции at_safe и т.п. уничтожаю совсем.

			@{
		*/

		const value_type &at(size_t i) const;
		inline	value_type &at(size_t i);

		const value_type &operator[](size_t i) const { return at(i);}
		value_type &operator[](size_t i) { return at(i);}
		//! @}

		void	fill(const value_type &x){std::fill(begin(), end(), x);};

	protected:
		// Проверка границ массива
		inline	void check_data_and_index(size_t index) const;
		inline	void check_array_boundaries(size_t index) const;



		//! \name Iterators
		//! @{
	public:

#if XRAD_CHECK_ITERATOR_BOUNDARIES

		typedef  step_iterator<VT, iterator_range_checker<VT,ptrdiff_t> > iterator;
		typedef  step_iterator<const VT, iterator_range_checker<const VT,ptrdiff_t> > const_iterator;

		iterator make_iterator(size_t i_current){return iterator(m_data, m_step, m_size, i_current);}
		const_iterator make_const_iterator(size_t i_current)const{return const_iterator(m_data, m_step, m_size, i_current);}
#else

		typedef  VT* iterator;
		typedef  const VT* const_iterator;

		iterator make_iterator(size_t i_current){return m_data + i_current;}
		const_iterator make_const_iterator(size_t i_current)const{return m_data + i_current;}

#endif //XRAD_CHECK_ITERATOR_BOUNDARIES

		inline iterator begin(){return make_iterator(0);}
		inline iterator end(){return make_iterator(m_size);}

		inline const_iterator begin() const{return make_const_iterator(0);}
		inline const_iterator end() const{return make_const_iterator(m_size);}

		inline const_iterator cbegin() const {return make_const_iterator(0);}
		inline const_iterator cend() const {return make_const_iterator(m_size);}

		// Обратные
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
		typedef std::reverse_iterator<iterator>	reverse_iterator;

		inline reverse_iterator rbegin(){return reverse_iterator(end());}
		inline reverse_iterator rend(){return reverse_iterator(begin());}

		inline const_reverse_iterator rbegin() const{return const_reverse_iterator(end());}
		inline const_reverse_iterator rend() const{return const_reverse_iterator(begin());}

		inline const_reverse_iterator crbegin() const{return const_reverse_iterator(cend());}
		inline const_reverse_iterator crend() const{return const_reverse_iterator(cbegin());}
		//! @} <!-- Iterators -->

		template<class T2>
		inline void MakeCopy(const FixedSizeArray<T2,N> &original){Apply_AA_1D_F2(*this, original, Functors::assign());}
		template<class T2>
		inline void CopyData(const FixedSizeArray<T2,N> &original){Apply_AA_1D_F2(*this, original, Functors::assign());}
};

//--------------------------------------------------------------

//! \brief Функция, являющаяся признаком того, что все наследники FixedSizeArray
//! являются массивами фиксированного размера, а также дающая значение этого размера.
//! См. \ref is_fixed_size_a_1d
template <class VT, size_t N>
constexpr integral_constant<size_t, N> FixedSize_A_1D(FixedSizeArray<VT, N> *);

//--------------------------------------------------------------

XRAD_END

#include "FixedSizeArray.hh"

//--------------------------------------------------------------
#endif // FixedSizeArray_h__
