/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//	file DataOwner.h
//--------------------------------------------------------------
#ifndef XRAD__File_data_owner_h
#define XRAD__File_data_owner_h
//--------------------------------------------------------------

#include "Iterators.h"

XRAD_BEGIN

//--------------------------------------------------------------
/*!
	\brief Массив, элементы которого могут следовать с разрывом между собой

	Является базовым для всех остальных контейнеров библиотеки.
	По ряду признаков существенно отличается от аналогичных
	классов STL (vecor, valarray, slice etc.), поэтому наследование от них исключено.
	Однако итераторы, заданные в этом классе и наследниках, совместимы с алгоритмами STL для "random access iterators".

	\par Константность данных

	- DataOwner &lt;VT&gt; - модифицируемые данные
	- DataOwner &lt;const VT&gt; - немодифицируемые данные
	- const DataOwner &lt;VT&gt; - немодифицируемые данные

	\par Устарело: Копирование объектов и копирование данных

	Для самого DataOwner (так же рекомендуется делать и для его наследников) копирующий
	конструктор и оператор присваивания переаллокироуют контейнер под новый размер и копируют данные.

	\todo
	У класса DataOwner много повторяющегося с классам FixedSizeArray. необходимо эти повторы объединить через наследование.

	\todo
	Подумать об использовании отрицательного шага. просто допустить его в классе и в итераторах нельзя, т.к.
	это повлечет необходимость дополнительных проверок при обращениях к данным. Но если ввести еще один параметр
	шаблона, и эта проверка будет сделана на этапе компиляции?
*/
template <class VT> class DataOwner
{
	public:
		typedef VT value_type;
		typedef const VT value_type_invariable;
		typedef	typename remove_const<VT>::type value_type_variable;

	protected:
		typedef DataOwner<VT> self;

		/*!
			\brief Тип, во всем тождественный self, но с неизменяемыми данными

			Его следует определять аналогичным образом для всех контейнеров.
			Назначение: создание срезов const-объектов. Эти срезы имеют
			полностью тот же функционал, что и исходный класс, но с запрещением
			всего, что может изменить данные в массиве.
			В отличие от const объектов, эти массивы можно переаллокировать.
		*/
		typedef DataOwner<const VT> invariable;

		typedef DataOwner<VT> root_owner;
		// Если далеким наследникам понадобилось вдруг что-то узнать о базовой структуре данных:
		// например, метод MapPlates в ColorImage.cpp

		//! \name Internal data
		//! @{
	private:
		//! \brief Доступ к данным только через at
		value_type *m_data;

		//TODO: int->size_t, ptrdiff_t
		size_t	m_size;

		/*!
			\brief Шаг данных, в единицах sizeof(value_type) (не в байтах!)

			Параметр step, очевидно, может понадобиться только для
			внешних данных. Поэтому он и задается только через
			функцию "UseData", которая одна эти данные получает.
			Во всех остальных случаях он равен 1.
		*/
		ptrdiff_t	m_step;
		bool	m_ownData;
		//! @}

		//! \name Constructors and destructor
		//!
		//! Как конструкторы, так и деструктор помещаются в protected области, потому что
		//! эти объекты в чистом виде создавать нельзя, только через наследников.
		//!
		//! Копирующие и перемещающие конструктор и оператор = использовать запрещено.
		//! Поэтому они объявлены = delete.
		//! @{
	protected:
		DataOwner(): m_ownData(false), m_size(0), m_step(0), m_data(0) {}
		explicit DataOwner(size_t in_size): m_ownData(false), m_size(0), m_step(0), m_data(0) {allocate(in_size);}
		virtual ~DataOwner() {dispose();}

	private:
		DataOwner(const DataOwner &) = delete;
		DataOwner(DataOwner &&) = delete;
		DataOwner &operator=(const DataOwner &) = delete;
		DataOwner &operator=(DataOwner &&) = delete;
		template<class T2>
		DataOwner(const DataOwner<T2> &) = delete;
		template<class T2>
		DataOwner &operator=(const DataOwner<T2> &) = delete;
		//! @}

	protected:
		//! \brief Функция для реализации перемещающих операций в наследниках
		void owner_move(self &other) noexcept
		{
			if (this == &other)
				return;
			if (m_ownData)
				dispose();
			m_ownData = other.m_ownData;
			m_size = other.m_size;
			m_step = other.m_step;
			m_data = other.m_data;
			other.m_ownData = false;
			other.m_size = 0;
			other.m_step = 0;
			other.m_data = nullptr;
		}

	protected:
		inline bool uses_external_data() const { return !m_ownData && m_data;}

		//! \name Info
		//! @{
	public:
		const size_t &size() const { return m_size;}
		const ptrdiff_t &step() const {return m_step;}
		static constexpr size_t element_size() { return sizeof(value_type); }

		// Для совместимости с многомерными алгоритмами
		const ptrdiff_t &steps(size_t) const {return m_step;}
		const size_t &sizes(size_t) const {return m_size;}

		//! \brief Количество элементов в массиве
		inline size_t element_count() const { return m_size; }

		inline bool empty() const {return !size();}
		inline bool ready() const {return (m_data&&size()&&step());}
		//! @}

	public:
		/*!
			\name Методы доступа к содержимому контейнера

			Конечному пользователю доступны только at() и operator[], а после и ().
			оператор () задать (реализовать) после того, как во всех зависимых программах вызов интерполятора,
			ранее выполнявшийся через operator() (float), будет заменен на функцию in(int).
			Функции at_safe и т.п. уничтожаю совсем.

			@{
		*/

		inline	const value_type &at(size_t i) const;
		inline	value_type &at(size_t i);

		const value_type &operator[](size_t i) const { return at(i);}
		value_type &operator[](size_t i) { return at(i);}
		//! @}

	protected:
		// Проверка границ массива
		inline	void check_data_and_index(size_t index) const;
		inline	void check_array_boundaries(size_t index) const;
		inline	void check_data_nan(size_t index) const;



		/*!
			\name Iterators

			Итераторы данного класса не должны быть public,
			наследники могут использовать данные не сплошным массивом,
			а с разрывами (подмножества многомерных массивов).

			@{
		*/
	protected:

#if XRAD_CHECK_ITERATOR_BOUNDARIES

		typedef  step_iterator<VT, iterator_range_checker<VT,ptrdiff_t> > iterator;
		typedef  step_iterator<const VT, iterator_range_checker<const VT,ptrdiff_t> > const_iterator;

#else

		typedef  step_iterator<VT, iterator_checker_none<VT,ptrdiff_t> > iterator;
		typedef  step_iterator<const VT, iterator_checker_none<const VT,ptrdiff_t> > const_iterator;

#endif //XRAD_CHECK_ITERATOR_BOUNDARIES

		inline iterator make_iterator(size_t pos){return iterator(m_data, m_step, m_size, pos);}
		inline const_iterator make_const_iterator(size_t pos) const {return const_iterator( m_data, m_step, m_size, pos);}

	protected:
		inline iterator begin(){return make_iterator(0);}
		inline iterator end(){return make_iterator(m_size);}
		inline const_iterator begin() const{return make_const_iterator(0);}
		inline const_iterator end() const{return make_const_iterator(m_size);}
		inline const_iterator cbegin() const{return make_const_iterator(0);}
		inline const_iterator cend() const{return make_const_iterator(m_size);}

		// Обратные итераторы
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
		typedef std::reverse_iterator<iterator>	reverse_iterator;

		inline reverse_iterator rbegin(){return reverse_iterator(end());}
		inline reverse_iterator rend(){return reverse_iterator(begin());}
		inline const_reverse_iterator rbegin() const{return const_reverse_iterator(end());}
		inline const_reverse_iterator rend() const{return const_reverse_iterator(begin());}
		inline const_reverse_iterator crbegin() const{return const_reverse_iterator(cend());}
		inline const_reverse_iterator crend() const{return const_reverse_iterator(cbegin());}
		//! @} <!-- Iterators -->



	public:
		//! \brief Изменение размера с потерей данных,
		//! для типов без конструктора контейнер содержит произвольные значения
		void	realloc(size_t newSize);

		//! \brief Виртуально поменять порядок следования элементов на обратный
		//! (сами данные массива в памяти не меняются, знак шага меняется на обратный)
		void reverse();
	private:
		void	allocate(size_t in_size = 0);
		/*!
			Метод dispose() вызывается независимо от параметра ownData.
			Для внешних данных он должен просто обнулять указатель и размеры, не освобождая память.
			В наследниках и извне пользоваться "realloc()".
		*/
		void	dispose();
	public:

		//! \brief Контейнер ссылается на элементы существующего массива с некоторым шагом
		void UseData(VT *new_data, size_t in_size, ptrdiff_t in_step = 1);

		//! \brief Создать ссылку на элементы существующего контейнера, полностью повторяя его свойства
		void UseData(DataOwner &new_data);

		//! \brief Создать ссылку на элементы существующего контейнера, полностью повторяя его свойства
		//!
		//! Может использоваться для создания const ссылок.
		template <class VT2>
		void UseData(DataOwner<VT2> &new_data);

		//! \brief Создать ссылку на элементы существующего контейнера, полностью повторяя его свойства
		//!
		//! Используется для создания const ссылок, тип данных текущего класса должен быть const.
		template<class VT2>
		void UseData(const DataOwner<VT2> &new_data);

		//! \brief Контейнер ссылается на часть элементов существующего контейнера,
		//! возможен меньший размер и шаг больший единицы
		void UseData(DataOwner &new_data, size_t in_size, ptrdiff_t in_step);

		//! \brief Опытный образец: контейнер натягивается на диапазон итераторов
		//!
		//! \note NB! Неопределенное поведение,
		//! если итераторы не указывают на непрерывный участок памяти.
		void UseData(iterator new_data_start, iterator new_data_end);

		//! \brief Контейнер ссылается на часть элементов существующего контейнера
		//! в некотором диапазоне, возможен шаг больший единицы
		template <class VT2>
		void UseDataFragment(DataOwner<VT2> &new_data, size_t from, size_t to, ptrdiff_t in_step);

		//! \brief Контейнер ссылается на часть элементов существующего контейнера
		//! в некотором диапазоне, возможен шаг больший единицы; для const данных
		template <class VT2>
		void UseDataFragment(const DataOwner<VT2> &new_data, size_t from, size_t to, ptrdiff_t in_step);

	public:
		template <class VT2>
		void GetData(DataOwner<VT2> &container);
		template <class VT2>
		void GetData(DataOwner<VT2> &container) const;
};

//--------------------------------------------------------------

XRAD_END

#include "DataOwner.hh"

//--------------------------------------------------------------
#endif //XRAD__File_data_owner_h
