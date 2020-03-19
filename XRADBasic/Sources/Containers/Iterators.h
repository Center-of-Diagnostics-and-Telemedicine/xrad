// file Iterators.h
// Created by ACS on 17.02.02
//--------------------------------------------------------------
#ifndef __iterators_h
#define __iterators_h
/*!
	\file
	\brief Итераторы с шагом
*/
//--------------------------------------------------------------

#include <XRADBasic/Core.h>
#include "ContainersBasic.h"
#include <string>

XRAD_BEGIN

//--------------------------------------------------------------


/*!
	\brief Вспомогательная структура для проверки границ у итератора

	Данные этой структуры используются как в самом
	итераторе, так и в структуре, отвечающей за
	проверку границ.

	\todo TODO: Значения указателей в итераторе end() не соответствуют стандарту при step > 1 и при step < 0
	(значения находятся вне допустимого диапазона array_first .. array_last+1).
	Возможно, проблема трудноразрешима.
*/
template<class T, class DT>
class iterator_kernel
{
	public:
		typedef	T value_type;
		typedef DT difference_type;
		typedef iterator_kernel<T,DT> self;

	private:

		/*!
			\brief Указатель на текущий элемент

			В силу правил работы с указателями в C/C++
			операция data_pointer+step для последнего элемента массива может быть не определена.
			CodeGuard в C++Builder это знает и ругается, поэтому долгое время хранился исходный
			указатель (допустимый!) и смещение. Но это означало потерю производительности.
			Пробуем все-таки хранить указатели.
		*/
		value_type *m_current_datum;
		difference_type m_step;

	public:
		iterator_kernel() = default;

		void	initialize_iterator(value_type *d, difference_type st, difference_type cp)
		{
			m_step = st;
			m_current_datum = d + cp*st;
		}

		template <class T2>
		iterator_kernel(const iterator_kernel<T2, DT> &original)
		{
			m_current_datum = original.current_datum();
			m_step = original.step();
		}

		template <class T2>
		iterator_kernel &operator = (const iterator_kernel<T2, DT> &original)
		{
			m_current_datum = original.current_datum();
			m_step = original.step();
			return *this;
		}

		value_type			*current_datum()		{return m_current_datum;}
		const value_type	*current_datum() const	{return m_current_datum;}

		difference_type step() const{return m_step;}

		self &operator++(){m_current_datum+=m_step; return *this;}
		self &operator--(){m_current_datum-=m_step; return *this;}
		self &operator+=(difference_type d){m_current_datum+=m_step*d; return *this;}
		self &operator-=(difference_type d){m_current_datum-=m_step*d; return *this;}

		//! \brief 2015_07_28 добавлено для обработки случаев, когда алгоритм работает на краю массива и нужна экстраполяция последним текущим значением
		void freeze(){m_step = 0;}

		difference_type operator-(const self &second) const
		{
			return (current_datum() - second.current_datum())/m_step;
				// делить на step нужно, так как position включает число итераций, помноженное на шаг
		}
};



//--------------------------------------------------------------
//
//	классы, отвечающие за проверку диапазона
//
//--------------------------------------------------------------



//! \brief Класс, осуществляющий фиктивную проверку итератора
template<class T, class DT>
class iterator_checker_none
{
		typedef	DT difference_type;
		typedef iterator_checker_none<T,DT> self;
		typedef	iterator_kernel<T,DT> basic_params;
		using value_type = add_const_t<T>;

	public:
		inline void	initialize_range_checker(iterator_kernel<T,DT> *, value_type *, difference_type , difference_type ) {}

		//! \note kernel имеет те же шаблонные типы, что и текущий класс, а checker может иметь другие типы.
		template <class T2, class DT2>
		inline void	initialize_range_checker(iterator_kernel<T,DT> *, const iterator_checker_none<T2, DT2>&) {}

		inline void	check_range() const {}
		inline void	check_data()const {}
		inline void	check_pair(const self &) const{}
};


//! \brief Класс, осуществляющий действительную проверку диапазонов для итераторов
template<class T, class DT>
class iterator_range_checker
{
		template <class T2, class DT2>
		friend class iterator_range_checker;

		typedef	iterator_kernel<T,DT> basic_params;
		typedef	DT difference_type;
		typedef	iterator_range_checker<T,DT> self;

		using value_type = add_const_t<T>;

	protected:
		iterator_kernel<T,DT> *kernel_ptr;
		value_type *m_data;
		difference_type	min_pos, max_pos;

		value_type *data() const { return m_data; }
		difference_type	position() const { return kernel_ptr->current_datum() - m_data; }
		string	iterator_problem_description() const;
		string	value_problem_description() const;

	public:
		/*!
			\brief Инициализировать проверяемый итератор

			\param min_p Минимальная позиция (аналог индекса 0).
			\param max_p Максимальная позиция + 1 (аналог индекса, равного size(): индекс элемента, следующего за последним).
		*/
		inline void	initialize_range_checker(iterator_kernel<T,DT> *in_it, value_type *data, difference_type min_p, difference_type max_p);

		//! \note kernel имеет те же шаблонные типы, что и текущий класс, а checker может иметь другие типы.
		template <class T2, class DT2>
		inline void	initialize_range_checker(iterator_kernel<T,DT> *in_it, const iterator_range_checker<T2, DT2> &other);

		/*!
			\brief Проверяет корректность текущей позиции итератора, но не предполагает возможности обращения к массиву
				(позиция end() является допустимой)

			1. Разрешается существование корректно заданного пустого массива (m_data = NULL).
			2. Разрешается ситуация, когда current_pos() == max_pos, что при разыменовании должно было бы
			привести к ошибке (это нужно для итераторов, обозначающих конец разрешенного диапазона
			и не предназначенных для разыменования).

			Эту проверку надо вызывать во всех операторах, кроме * и -> (там используется check_data).
		*/
		inline void	check_range() const;
		/*!
			\brief Проверяет возможность обращения к массиву в текущей позиции (позиция end() не является допустимой)

			Запрещается как m_data == NULL, так и *current_pos_p == max_pos.

			Обязательная проверка в операторах * и ->.
		*/
		inline void	check_data() const;
		inline void	check_pair(const self& second) const;
};


/*!
	\brief Класс, осуществляющий действительную проверку диапазонов для итераторов, а также
	контроль возникновения NAN
*/
template<class T, class DT>
class iterator_range_and_value_checker : public iterator_range_checker<T,DT>
{
	PARENT(iterator_range_checker<T,DT>);
	public:
		inline void	check_data() const;
};



//--------------------------------------------------------------



//! \brief Итератор
template<class T, class CH>
class step_iterator : public iterator_traits<T*>
{
	public:
		// наследование от std::iterator_traits<T*> не дает нужного эффекта под BCB 5.0
		// typedef из iterator_traits почему-то не "видятся" здесь.
		// это происходит и под Metrowerks CodeWarrior 8
		typedef step_iterator<T, CH> self;
		typedef T value_type;
		typedef ptrdiff_t difference_type;
		typedef T* pointer;
		typedef T& reference;

		XRAD_CheckedIterator;// Сообщает компилятору, что итератор контролирует свои границы во избежание предупреждения C4996 под MSVS.
		//
	public:
		template <class T2, class CH2>
		friend class step_iterator;

		inline	step_iterator();
		inline	step_iterator( value_type *array, difference_type i_step, difference_type i_count, difference_type i_current);

		inline step_iterator(const self &original)
		{
			// копи-конструктор и operator= оба должны быть переписаны явно,
			// так как при включенной проверке границ checker содержит указатель на проверяемый итератор.

			kernel=original.kernel;
			checker.initialize_range_checker(&kernel, original.checker);
			checker.check_range();
		}

		inline self &operator=(const self &original)
		{
			kernel=original.kernel;
			checker.initialize_range_checker(&kernel, original.checker);
			checker.check_range();
			return *this;
		}

		template <class T2, class CH2>
		inline step_iterator(const step_iterator<T2, CH2> &original):
				kernel(original.kernel)
		{
			checker.initialize_range_checker(&kernel, original.checker);
			checker.check_range();
		}

		template <class T2, class CH2>
		inline step_iterator &operator = (const step_iterator<T2, CH2> &original)
		{
			kernel=original.kernel;
			checker.initialize_range_checker(&kernel, original.checker);
			checker.check_range();
			return *this;
		}



	public:
		inline reference operator[](difference_type) const;
		inline reference operator*() const;
		inline pointer operator->() const;

		inline reference operator[](difference_type);
		inline reference operator*();
		inline pointer operator->();
		//
	public:
		inline bool operator ==(const self &second) const;
		inline bool operator !=(const self &second) const;
		inline bool operator > (const self &second) const;
		inline bool operator < (const self &second) const;
		inline bool operator >=(const self &second) const;
		inline bool operator <=(const self &second) const;
	public:
		inline self &operator++();
		inline self &operator--();
		inline self &operator+=( difference_type d);
		inline self &operator-=( difference_type d);
		inline difference_type operator-( const self &second) const;

		inline self operator--(int); // int, not difference_type
		inline self operator++(int);

		inline self operator+(difference_type) const;
		inline self operator-(difference_type) const;

		inline void freeze(){kernel.freeze();}
		//
	private:
		CH	checker;
		iterator_kernel<value_type, difference_type> kernel;
};

//--------------------------------------------------------------

XRAD_END

#include "Iterators.hh"

//--------------------------------------------------------------
#endif // __iterators_h
