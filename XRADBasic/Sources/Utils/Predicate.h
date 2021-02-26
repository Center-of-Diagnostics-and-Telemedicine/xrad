/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef Predicate_h__
#define Predicate_h__
/*!
	\file
	\date 2018/02/19 17:40
	\author kulberg

	\brief Объекты — логические предикаты, например, для организации сложных
	фильтров данных

	Основным классом для применения является predicate::checker, у которого есть bool operator().
	Внутри checker находится объект condition m_condition, у которого есть виртуальный метод
	bool check().
	В predicate::checker содержится параметр bool m_direct. Если m_direct=false, условие,
	заданное в m_condition, инвертируется.
	Алгоритм проверки задается в объектах-наследниках от predicate::condition,
	которые содержатся в predicate::checker.

	Существуют абстрактный унарный predicate::condition, от которого следует производить наследников
	с конкретными реализациями методов проверки.
	Также существует бинарный condition, который объединяет пару predicate::checker
	по одной из логических операций (&|^).

	Прямое использование в пользовательском коде классов checker и m_condition может быть неудобным
	из-за громоздкости выражений.
	Поэтому желательно под каждый случай создавать функции-генераторы предикатов,
	как это сделано в файле:

	\example value_predicate.h

	Пример использования:
	\code
	auto p1 = predicate::equals(2);
	auto p2 = predicate::greater(3);
	auto p3 = p1 | p2;
	p3(0);//false
	p3(2);//true
	p3(4);//true
	\endcode

	Предполагается создать на этой базе предикаты для фильтрации наборов dicom данных.
	Сложные фильтры могут понадобиться, чтобы набирать данные по совокупности условий,
	например (модальность=КТ, возраст >50, пол=F).
*/
//--------------------------------------------------------------

#include <XRADBasic/Sources/Core/cloning_ptr.h>

XRAD_BEGIN

namespace predicate
{

//--------------------------------------------------------------

template<class T> class checker;

//!	\brief Абстрактный класс, определяющий логическое высказывание на множестве T
//! \details Кажому элементу множества T сопоставляется значение true или false.
//! Результат вычисляется виртуальным методом check(), который должен быть переопределен
//!	в наследниках. Использовать непосредственно condition нельзя. Используются только
//!	указатели на его наследников, являющиеся членом определенного ниже класса checker.
template<class T>
class condition
{
public:
	friend class cloning_ptr<T>;
	friend class checker<T>;

	using value_type = T;
	using self = condition<T>;

	virtual	self	*clone() const = 0;
	virtual ~condition() = default;

private:
	virtual	bool	check(const value_type &) const = 0;

};


//!	\brief Основной класс-контейнер предикатов. Содержит указатель на объект-наследник
//!	condition. С помощью флага m_direct может инвертировать условие
template<class T>
class checker
{
private:
	cloning_ptr<condition<T>> m_condition;
	bool	m_direct;

public:
	using value_type = T;
	using self = checker<T>;

	//! \brief Проверка предиката выполняется через оператор ().
	//! т.о., их можно использовать для построения бинарных масок через функции вида CopyData() с функтором
	bool	operator()(const value_type &m) const
	{
		return m_direct ? m_condition->check(m) : !m_condition->check(m);
	}

	checker(const condition<T> &dfc, bool in_direct = true)
	{
		m_condition = dfc.clone();
		m_direct = in_direct;
	}

	self	operator!()
	{
		auto result = *this;
		result.m_direct  = !m_direct;
		return result;
	}

	void	inverse(){ m_direct = !m_direct; }

	static self true_predicate() { return checker(fixed_condition<true>()); }
	static self false_predicate() { return checker(fixed_condition<false>()); }

private:
	//! \brief Предикат с фиксированным результатом
	template <bool Result>
	class	fixed_condition : public condition<T>
	{
		bool check(const value_type &) const override { return Result; }
	public:
		condition<T> *clone() const override { return new fixed_condition(); }
	};
};

//--------------------------------------------------------------

enum class logic_operation_t
{
	// Слова and, or, xor являются ключевыми словами C++, добавляем префикс op_.
	op_and, op_or, op_xor
};

//--------------------------------------------------------------

/*!
	\brief Класс-наследник condition, объединяющий пару предикатов checker<T> через логическое выражение (and, or, xor)

	Особенность реализации:
	для логических функций and и or вызов второго предиката не производится, если значение первого
	определяет однозначно результат логической функции.
*/
template<class T, logic_operation_t L>
class binary_condition : public condition<T>
{
public:
	using value_type = typename condition<T>::value_type;

public:
	binary_condition(checker<T>	in_f1, checker<T>	in_f2) : f1(in_f1), f2(in_f2) {}

private:
	template <logic_operation_t LL>
	inline std::enable_if_t<LL==logic_operation_t::op_and, bool> do_check(const value_type &m) const
	{
		return f1(m) && f2(m);
	}

	template <logic_operation_t LL>
	inline std::enable_if_t<LL==logic_operation_t::op_or, bool> do_check(const value_type &m) const
	{
		return f1(m) || f2(m);
	}

	template <logic_operation_t LL>
	inline std::enable_if_t<LL==logic_operation_t::op_xor, bool> do_check(const value_type &m) const
	{
		return f1(m) != f2(m);
	}

	virtual	bool	check(const value_type &m) const override
	{
		return do_check<L>(m);
	}

	virtual	condition<T> *clone() const override
	{
		return new binary_condition<T, L>(f1, f2);
	}

private:
	checker<T>	f1, f2;
};

//! brief Операторы, позволяющие строить логические комбинации предикатов

template<class T>
checker<T>	operator &(const checker<T> &f1, const checker<T> &f2)
{
	return checker<T>(binary_condition<T, logic_operation_t::op_and>(f1, f2));
}

template<class T>
checker<T>	operator |(const checker<T> &f1, const checker<T> &f2)
{
	return checker<T>(binary_condition<T, logic_operation_t::op_or>(f1, f2));
}

template<class T>
checker<T>	operator ^(const checker<T> &f1, const checker<T> &f2)
{
	return checker<T>(binary_condition<T, logic_operation_t::op_xor>(f1, f2));
}

template<class T>
checker<T>	operator &=(checker<T> &f1, const checker<T> &f2)
{
	return f1 = (f1&f2);
}

template<class T>
checker<T>	operator |= (checker<T> &f1, const checker<T> &f2)
{
	return f1 = (f1|f2);
}

template<class T>
checker<T>	operator ^=(checker<T> &f1, const checker<T> &f2)
{
	return f1 = (f1^f2);
}


//--------------------------------------------------------------

}//namespace predicate

XRAD_END

//--------------------------------------------------------------
#endif // Predicate_h__
