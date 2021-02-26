/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifdef XRAD__File_field_element_h_inside
#error Error: Recursive inclusion of FieldElement.h detected.
#endif
#ifndef XRAD__File_field_element_h
#define XRAD__File_field_element_h
#define XRAD__File_field_element_h_inside
/*!
	\addtogroup gr_Algebra
	@{

	\file
	\brief Базовая реализация полей

	@}
*/

#include <type_traits>
#include "FieldTraits.h"
#include <XRADBasic/Sources/Core/Functors.h>

XRAD_BEGIN

//--------------------------------------------------------------

#define XRAD__template_1 class CONTAINER_T, class CHILD_T, class VT, class ST, class ALG_T
#define XRAD__template_2 class CONTAINER_T2, class CHILD_T_2, class VT2, class ST2, class ALG_T2

#define XRAD__template_1_args CONTAINER_T,CHILD_T,VT,ST,ALG_T
#define XRAD__template_2_args CONTAINER_T2,CHILD_T_2,VT2,ST2,ALG_T2

/*!
	\addtogroup gr_Algebra
	@{

	\brief Пространство имен для механизма алгебр
*/
namespace	AlgebraicStructures
{

//--------------------------------------------------------------
/*!
	\brief Элемент поля, универсальная часть

	Класс должен соответствовать элементу произвольного поля (линейного векторного
	пространства) над полем скаляров ST; то есть, заданы операция сложения и умножения
	на скаляр.

	Строгого соответствия с алгебраической структурой здесь добиться не удается
	по следующим причинам:

	1. В качестве скалярного типа здесь допускается использование множеств,
	не образующих поле (а, например, только кольцо -- например, целых чисел).
	В этом случае операция деления становится некорректной. однако для приближенных
	вычислений и это можно считать допустимым.

	2. Допускается применение операторов к объектам, являющихся, строго говоря,
	различными алгебраическими структурами:например, сложение комплексного и действительного
	векторов. В этом случае действительный объект рассматривается как формально принадлежащий
	и к множеству комплексных. определяющим является тип первого аргумента:
	таким образом, к комплексному объекту можно прибавить действительный; обратное невозможно.

	До апреля 2013 г. этот класс относился только к одномерным
	объектам (т.е., простым векторам). Потом сделан универсальным
	для произвольного количества размерностей (механизм вызова алгоритмов через
	класс AlgebraicAlgorithms).

	Составить более подробный комментарий по поводу следующего:
	собственно вектором является наследник типа child_type.
	Все операторы должны возвращать именно этот тип, что и делается
	посредством static_cast.

	См. \ref pg_MoveOperations.
*/
template<XRAD__template_1>
class GenericFieldElement : public CONTAINER_T
{
	private:
		typedef	CHILD_T child_type;

		child_type &child_ref(){ return static_cast<child_type&>(*this); }
		const child_type &child_ref() const { return static_cast<const child_type&>(*this); }

	public:
		PARENT(CONTAINER_T);
		typedef GenericFieldElement<XRAD__template_1_args> self;
		typedef typename CONTAINER_T::value_type value_type;
		typedef ST scalar_type;
		//typedef typename CHILD_T::scalar_type scalar_type;
		typedef ALG_T algorithms_type;
		using value_type_variable = typename CONTAINER_T::value_type_variable;

	public:
		// Использование родительских конструкторов и operator=
		using parent::parent;
		GenericFieldElement() = default;
		GenericFieldElement(const parent &p): parent(p) {}
		GenericFieldElement(parent &&p): parent(std::move(p)) {}
		using parent::operator=;

	public:
		template<XRAD__template_2>
		child_type	&operator += (const GenericFieldElement<XRAD__template_2_args> &f2){ return algorithms_type::AA_Op_Assign(child_ref(), f2, Functors::plus_assign()); }

		template<XRAD__template_2>
		child_type	&operator -= (const GenericFieldElement<XRAD__template_2_args> &f2) { return algorithms_type::AA_Op_Assign(child_ref(), f2, Functors::minus_assign()); }

		template<XRAD__template_2>
		child_type	operator + (const GenericFieldElement<XRAD__template_2_args> &f2) const { return algorithms_type::AA_Op_New(child_ref(), f2, Functors::assign_plus()); }

		template<XRAD__template_2>
		child_type	operator - (const GenericFieldElement<XRAD__template_2_args> &f2) const { return algorithms_type::AA_Op_New(child_ref(), f2, Functors::assign_minus()); }


		child_type	&operator *= (const scalar_type &x) { return algorithms_type::AS_Op_Assign(child_ref(), x, Functors::multiply_assign()); }
		child_type	&operator /= (const scalar_type &x) { return algorithms_type::AS_Op_Assign(child_ref(), x, Functors::divide_assign()); }

		child_type	operator * (const scalar_type &x) const { return algorithms_type::AS_Op_New(child_ref(), x, Functors::assign_multiply()); }
		child_type	operator / (const scalar_type &x) const { return algorithms_type::AS_Op_New(child_ref(), x, Functors::assign_divide()); }

		// следующие действия (сложение с числом типа "компонент вектора") строго в алгебре не заданы.
		// речь идет о действии с вектором, все компоненты которого равны x (иначе -- с константной функцией)
		child_type	&operator += (const value_type &x) { return algorithms_type::AS_Op_Assign(child_ref(), x, Functors::plus_assign()); }
		child_type	&operator -= (const value_type &x) { return algorithms_type::AS_Op_Assign(child_ref(), x, Functors::minus_assign()); }
		child_type	operator + (const value_type &x) const { return algorithms_type::AS_Op_New(child_ref(), x, Functors::assign_plus()); }
		child_type	operator - (const value_type &x) const { return algorithms_type::AS_Op_New(child_ref(), x, Functors::assign_minus()); }
		// инкремент префиксный, поэлементное сложение или вычитание с единицей
		child_type	&operator ++ () { return algorithms_type::A_Op_Assign(child_ref(), Functors::increment()); }
		child_type	&operator -- () { return algorithms_type::A_Op_Assign(child_ref(), Functors::decrement()); }
		// инкремент постфиксный (неэффективен по сравнению с префиксным)
		child_type	operator ++ (int) { child_type result(child_ref()); this->operator++(); return result; }
		child_type	operator -- (int) { child_type result(child_ref()); this->operator--(); return result; }

		// тернарные действия: все массивы заранее имеют одинаковый размер,
		// действие вида a.add(b,c) означает a=b+c;
		// действие вида a.add_multiply(b,c) означает a+=b*c;
		// при этом никаких новых объектов не создается
		template<XRAD__template_2>
		child_type	&add(const GenericFieldElement<XRAD__template_2_args> &f1, const GenericFieldElement<XRAD__template_2_args> &f2)
		{
			return algorithms_type::AAA_Op_Assign(child_ref(), f1, f2, Functors::assign_plus());
		}
		template<XRAD__template_2>
		child_type	&subtract(const GenericFieldElement<XRAD__template_2_args> &f1, const GenericFieldElement<XRAD__template_2_args> &f2)
		{
			return algorithms_type::AAA_Op_Assign(child_ref(), f1, f2, Functors::assign_minus());
		}
		template<XRAD__template_2>
		child_type	&multiply(const GenericFieldElement<XRAD__template_2_args> &f1, const scalar_type &x)
		{
			return algorithms_type::AAS_Op_Assign(child_ref(), f1, x, Functors::assign_multiply());
		}
		template<XRAD__template_2>
		child_type	&divide(const GenericFieldElement<XRAD__template_2_args> &f1, const scalar_type &x)
		{
			return algorithms_type::AAS_Op_Assign(child_ref(), f1, x, Functors::assign_divide());
		}
		template<XRAD__template_2>
		child_type	&add_multiply(const GenericFieldElement<XRAD__template_2_args> &f1, const scalar_type &x)
		{
			return algorithms_type::AAS_Op_Assign(child_ref(), f1, x, Functors::plus_assign_multiply());
		}
		template<XRAD__template_2>
		child_type	&add_divide(const GenericFieldElement<XRAD__template_2_args> &f1, const scalar_type &x)
		{
			return algorithms_type::AAS_Op_Assign(child_ref(), f1, x, Functors::plus_assign_divide());
		}
		template<XRAD__template_2>
		child_type	&subtract_multiply(const GenericFieldElement<XRAD__template_2_args> &f1, const scalar_type &x)
		{
			return algorithms_type::AAS_Op_Assign(child_ref(), f1, x, Functors::minus_assign_multiply());
		}
		template<XRAD__template_2>
		child_type	&subtract_divide(const GenericFieldElement<XRAD__template_2_args> &f1, const scalar_type &x)
		{
			return algorithms_type::AAS_Op_Assign(child_ref(), f1, x, Functors::minus_assign_divide());
		}

		// взвешенное сложение двух массивов, результат равен f1*a1 + f2*a2
		template<XRAD__template_2>
		child_type	&mix(const GenericFieldElement<XRAD__template_2_args> &f1, const GenericFieldElement<XRAD__template_2_args> &f2, typename CHILD_T_2::scalar_type a1, typename CHILD_T_2::scalar_type a2)
		{
			return algorithms_type::AAA_Op_Assign(child_ref(), f1, f2, Functors::create_assign_mix(a1, a2));
		}

		// сложение с числом (т.е. с вектором, все компоненты которого равны этому числу)
		template<XRAD__template_2>
		child_type	&add(const GenericFieldElement<XRAD__template_2_args> &f1, const value_type &x)
		{
			return algorithms_type::AAS_Op_Assign(child_ref(), f1, x, Functors::assign_plus());
		}
		template<XRAD__template_2>
		child_type	&subtract(const GenericFieldElement<XRAD__template_2_args> &f1, const value_type &x)
		{
			return algorithms_type::AAS_Op_Assign(child_ref(), f1, x, Functors::assign_minus());
		}

		// инверсия знака
		child_type	operator - () const { return algorithms_type::A_Op_New(child_ref(), Functors::assign_unary_minus()); }

		// скалярное произведение, результат пишется в первый аргумент, тип результата задается извне
		template<class RT, XRAD__template_2>
		void scalar_product(RT &result, const GenericFieldElement<XRAD__template_2_args> &f2) const
		{
			make_zero(result);
			algorithms_type::AA_Op_Assign(child_ref(), f2, Functors::scalar_product_acquire<RT>(&result));
		}

		// действия со скалярным результатом
		// скалярное произведение, результат возвращается, тип результата совпадает с value_type
		template<XRAD__template_2>
		[[deprecated("FieldElement::operator | is deprecated. Use sp() function instead")]]
		value_type	operator | (const GenericFieldElement<XRAD__template_2_args> &f2) const
		{
			return sp(f2);
		}

		template<XRAD__template_2>
		value_type	sp(const GenericFieldElement<XRAD__template_2_args> &f2) const
		{
			value_type_variable result(0);
			algorithms_type::AA_Op_Assign(child_ref(), f2, Functors::scalar_product_acquire<value_type_variable>(&result));
			return result;
		}

		template<XRAD__template_2>
		bool	operator == (const GenericFieldElement<XRAD__template_2_args> &f2) const
		{
			if(!algorithms_type::AA_EqSize(child_ref(), f2))
				return false;
			return !algorithms_type::AA_Any(child_ref(), f2, Functors::not_equal());
		}

		template<XRAD__template_2>
		bool	operator != (const GenericFieldElement<XRAD__template_2_args> &f2) const
		{
			if(!algorithms_type::AA_EqSize(child_ref(), f2))
				return true;
			return algorithms_type::AA_Any(child_ref(), f2, Functors::not_equal());
		}
};

template<XRAD__template_1, XRAD__template_2>
VT	sp(const GenericFieldElement<XRAD__template_1_args> &x, const GenericFieldElement<XRAD__template_2_args> &y)
{
	return x.sp(y);
}

//--------------------------------------------------------------
/*!
	\brief Объявление шаблона реализации FieldElement

	Использование FieldElementMath должно производиться через шаблон FieldElement.

	Параметры шаблона VT, ST передаются из FieldElement в FieldElementMath всегда без const-volatile.

	Обший шаблон FieldElementMath не используется, используются только его специализации.
	Специализации различаются по типу tag. В библиотеке задаются две специализации:
	скалярная (tag = FieldTagScalar) и комплексная (tag = FieldTagComplex).

	Предполагается, что класс FieldElementMath может выступать сам в качестве
	типа данных для FieldElementMath, причем с тем же самым tag:
	FieldElementMath<container, child, FieldElementMath<container_2, child_2, vt, st, tag>, st, tag>.

	Можно определять собственные специализации FieldElementMath по типу tag.
	Для этого необходимо:
	- определить новый класс тега (он будет идентифицировать новый вид FieldElement);
	- задать специализацию FieldTraits для нужного типа данных,
		которая будет возвращать новый класс тега;
	- реализовать специализацию FieldElementMath для нового класса тега.

	Параметр ALG_T задает алгоритмы работы с контейнером. См. \ref pg_FieldElement_Algorithm.
*/
template<class CONTAINER_T, class CHILD_T, class VT, class ST, class ALG_T, class tag>
class FieldElementMath;

/*!
	\brief Определение шаблонного FieldElement

	Вызывает реализацию FieldElementMath. При передаче параметров шаблона
	снимает признаки const-volatile.
*/
template<class CONTAINER_T, class CHILD_T, class VT, class ST, class ALGORITHMS_T, class FIELD_TAG>
using FieldElement = FieldElementMath<CONTAINER_T, CHILD_T,
	typename std::remove_cv<VT>::type,
	typename std::remove_cv<ST>::type,
	ALGORITHMS_T,
	FIELD_TAG>;

/*!
	\brief Вспомогательная функция для реализации специализаций FieldTraits
	для наследников FieldElementMath, см. (*1490625828). Задает тег, равный
	тегу FieldElementMath
*/
template<class CONTAINER_T, class CHILD_T, class VT, class ST, class ALG_T, class tag>
tag GetFieldElementTag(FieldElementMath<CONTAINER_T, CHILD_T, VT, ST, ALG_T, tag>*);

/*!
	\brief Специализация FieldElementMath скалярного вида (FieldTagScalar)

	См. \ref pg_MoveOperations.
*/
template<XRAD__template_1>
class FieldElementMath<XRAD__template_1_args, FieldTagScalar> : public GenericFieldElement<XRAD__template_1_args>
{
	// все, что есть в этом классе, унаследовано напрямую от предка.
	private:
		PARENT(GenericFieldElement<XRAD__template_1_args>);
		typedef FieldElementMath<XRAD__template_1_args, FieldTagScalar> self;

	public:
		typedef FieldTagScalar field_tag;

	public:
		// Использование родительских конструкторов и operator=
		using parent::parent;
		FieldElementMath() = default;
		FieldElementMath(const parent &p): parent(p) {}
		FieldElementMath(parent &&p): parent(std::move(p)) {}
		using parent::operator=;
};

//--------------------------------------------------------------

}//namespace	AlgebraicStructures



//--------------------------------------------------------------
//	тернарные действия не-члены класса. действия вида add(a,b,c) означают a=b+c
//	ради совместимости сначала определяются для произвольных элементарных типов,
//	далее для элементов векторной алгебры.

template<class T1, class T2, class T3>
T1	&multiply(T1 &x1, const T2 &x2, const T3 &x3)
{
	return x1 = T1(x2)*x3;
	//Важно: преобразование к T1 должно делаться до умножения, иначе возможна неоправданная потеря точности.
	//То же во всех остальных функциях ниже
}

template<class T1, class T2, class T3>
T1	&divide(T1 &x1, const T2 &x2, const T3 &x3)
{
	return x1 = T1(x2)/x3;
}

template<class T1, class T2, class T3>
T1	&add(T1 &x1, const T2 &x2, const T3 &x3)
{
	return x1 = T1(x2)+x3;
}

template<class T1, class T2, class T3>
T1	&subtract(T1 &x1, const T2 &x2, const T3 &x3)
{
	return x1 = T1(x2)-x3;
}

template<class T1, class T2, class T3>
T1	&add_multiply(T1 &x1, const T2 &x2, const T3 &x3)
{
	return x1 += T1(x2)*x3;
}

template<class T1, class T2, class T3>
T1	&add_divide(T1 &x1, const T2 &x2, const T3 &x3)
{
	return x1 += T1(x2)/x3;
}

template<class T1, class T2, class T3>
T1	&subtract_multiply(T1 &x1, const T2 &x2, const T3 &x3)
{
	return x1 -= T1(x2)*x3;
}

template<class T1, class T2, class T3>
T1	&subtract_divide(T1 &x1, const T2 &x2, const T3 &x3)
{
	return x1 -= T1(x2)/x3;
}



#define XRAD__define_action(action_name)\
template<XRAD__template_1, class ST2>\
AlgebraicStructures::GenericFieldElement<XRAD__template_1_args>&\
		action_name(\
				AlgebraicStructures::GenericFieldElement<XRAD__template_1_args> &x1,\
				const AlgebraicStructures::GenericFieldElement<XRAD__template_1_args> &x2,\
				const ST2 &x3)\
	{\
	return x1.action_name(x2, x3);\
	}

XRAD__define_action(add)
XRAD__define_action(subtract)
XRAD__define_action(multiply)
XRAD__define_action(divide)
XRAD__define_action(add_multiply)
XRAD__define_action(add_divide)
XRAD__define_action(subtract_multiply)
XRAD__define_action(subtract_divide)

#undef XRAD__define_action

//--------------------------------------------------------------
//! @} <!-- ^group gr_Algebra -->

#undef XRAD__template_1
#undef XRAD__template_2

#undef XRAD__template_1_args
#undef XRAD__template_2_args

//--------------------------------------------------------------

XRAD_END

#undef XRAD__File_field_element_h_inside
#endif //XRAD__File_field_element_h
