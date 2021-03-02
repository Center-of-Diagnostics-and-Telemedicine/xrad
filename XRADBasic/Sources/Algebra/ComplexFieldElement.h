/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifdef ComplexFieldElement_h___inside
#error Error: Recursive inclusion of ComplexFieldElement.h detected.
#endif
#ifndef ComplexFieldElement_h__
#define ComplexFieldElement_h__
#define ComplexFieldElement_h___inside
/*!
	\addtogroup gr_Algebra
	@{

	\file
	\date 2016-04-07 11:58
	\author kns
	\brief Специфические действия для комплексных полей

	@}
*/
//--------------------------------------------------------------

#include "FieldElement.h"
#include <XRADBasic/Sources/SampleTypes/ComplexSample.h>

XRAD_BEGIN
namespace	AlgebraicStructures
{

//--------------------------------------------------------------

/*!
	\brief Тег для FieldElement, задающий комплексную реализацию

	Внутри структуры определяем тип tag_t = FieldTagComplex для того, чтобы реализации
	FieldTraits<T> могли быть, для краткости записи, унаследованы от этой структуры.
*/
struct FieldTagComplex
{
	using tag_t = FieldTagComplex;
};

/*!
	\brief Специализация для ComplexSample: задает комплексную раелизацию FieldElement
*/
template<class T1, class T2>
struct FieldTraits<ComplexSample<T1, T2> >: FieldTagComplex {};

/*!
	\brief Шаблонный класс-помощник для реализации FieldElementMath<..., FieldTagComplex>,
	определяющий тип complex_component_type

	Общий шаблон не используется, используются специализации.

	Специализации должны определять тип "complex_component_type", равный используемому
	типу компоненты комплексного числа: PT у ComplexSample<PT,ST>.

	Параметр class Conditional = void нужен для условной перегрузки шаблона
	(с помощью std::enable_if и аналогичных механизмов).
	Используются специализации с Conditional = void.
*/
template<class T, class Conditional = void>
struct ComplexFieldTraits;

/*!
	\brief Специализация для const T
*/
template<class T>
struct ComplexFieldTraits<const T>
{
	using complex_component_type = typename ComplexFieldTraits<T>::complex_component_type;
};

/*!
	\brief Специализация для задания ComplexFieldTraits для классов вместе с наследниками

	Работает аналогично (*1490625828) для FieldTraits.
*/
template<class T>
struct ComplexFieldTraits<T,
		typename std::enable_if<
				!std::is_same<
						decltype(GetComplexFieldElementTag(std::declval<typename std::remove_cv<T>::type*>())),
						void>::value,
				void>::type>
{
	using complex_component_type = decltype(GetComplexFieldElementTag(std::declval<typename std::remove_cv<T>::type*>()));
};

/*!
	\brief Специализация для ComplexSample
*/
template<class T1, class T2>
struct ComplexFieldTraits<ComplexSample<T1, T2>>
{
	using complex_component_type = T1;
};

#define XRAD__fce_template class CONTAINER_T, class CHILD_T, class VT, class ST, class ALG_T
#define XRAD__fce_template_args CONTAINER_T,CHILD_T,VT,ST,ALG_T
#define XRAD__template_2 class CONTAINER_T2, class CHILD_T_2, class VT2, class ST2, class ALG_T2
#define XRAD__template_2_args CONTAINER_T2,CHILD_T_2,VT2,ST2,ALG_T2

/*!
	\brief Специализация FieldElementMath комплексного вида (FieldTagComplex)

	См. \ref pg_MoveOperations.
*/
template<XRAD__fce_template>
class FieldElementMath<XRAD__fce_template_args, FieldTagComplex> : public GenericFieldElement<XRAD__fce_template_args>
{
	private:
		typedef	CHILD_T child_type;
		typedef FieldElementMath<XRAD__fce_template_args,FieldTagComplex> self;
		PARENT(GenericFieldElement<XRAD__fce_template_args>);

		child_type &child_ref(){return static_cast<child_type&>(*this);}
		const child_type &child_ref() const {return static_cast<const child_type&>(*this);}

	public:
		typedef VT value_type;
		typedef typename ComplexFieldTraits<VT>::complex_component_type complex_component_type;
		typedef ST scalar_type;
		typedef FieldTagComplex field_tag;
		using algorithms_type = typename parent::algorithms_type;

	public:
		// Использование родительских конструкторов и operator=
		using parent::parent;
		FieldElementMath() = default;
		FieldElementMath(const parent &p): parent(p) {}
		FieldElementMath(parent &&p): parent(std::move(p)) {}
		using parent::operator=;

	public:

		//	операции, специфические для комплекснозначных объектов
		//
		//	1. сложение с действительным скаляром (из экономии -- не прибавляется ноль к мнимой части)
		//	умножение на действительную константу и сложение с действительной функцией
		//	реализовано как действия в структуре GenericFieldElement, так как они вписываются в стандартную
		//	алгебраическую модель
		// требует уточнения: как и копирующий конструктор, надо объявить оператор= и шаблоном, и точно от своего типа. иначе вызывается копи-конструктор по умолчанию

		// сложение и вычитание комплексного массива с действительным числом.
		// аналогичные действия для комплексного слагаемого унаследованы от GenericFieldElement
		//
		child_type	&operator += (const complex_component_type &x) {return algorithms_type::AS_Op_Assign(child_ref(), x, Functors::plus_assign());}
		child_type	&operator -= (const complex_component_type &x) {return algorithms_type::AS_Op_Assign(child_ref(), x, Functors::minus_assign());}
		child_type	operator + (const complex_component_type &x) const {return algorithms_type::AS_Op_New(child_ref(), x, Functors::assign_plus());}
		child_type	operator - (const complex_component_type &x) const {return algorithms_type::AS_Op_New(child_ref(), x, Functors::assign_minus());}

		//	2. умножение и деление на комплексную величину,
		//	в том числе с другими аргументами комплексного шаблона,
		//	в т.ч. сопряженное

		template<class T2, class ST2>
		child_type	&operator *= (const ComplexSample<T2,ST2> &x){return algorithms_type::AS_Op_Assign(child_ref(), x, Functors::multiply_assign());}
		template<class T2, class ST2>
		child_type	&operator /= (const ComplexSample<T2,ST2> &x){return algorithms_type::AS_Op_Assign(child_ref(), x, Functors::divide_assign());}
		template<class T2, class ST2>
		child_type	&operator %= (const ComplexSample<T2,ST2> &x){return algorithms_type::AS_Op_Assign(child_ref(), x, Functors::percent_assign());}
		template<class T2, class ST2>
		child_type	operator * (const ComplexSample<T2,ST2> &x) const{return algorithms_type::AS_Op_New(child_ref(), x, Functors::assign_multiply());}
		template<class T2, class ST2>
		child_type	operator / (const ComplexSample<T2,ST2> &x) const{return algorithms_type::AS_Op_New(child_ref(), x, Functors::assign_divide());}
		template<class T2, class ST2>
		child_type operator % (const ComplexSample<T2,ST2> &x) const{return algorithms_type::AS_Op_New(child_ref(), x, Functors::assign_percent());}

		// 3. унарный префиксный оператор комплексного сопряжения, применение по аналогии с унарным "-"

		child_type	operator ~ () const {return algorithms_type::A_Op_New(child_ref(), Functors::assign_bitwise_not());}
		child_type	&conjugate() { return algorithms_type::A_Op_Assign(child_ref(), Functors::bitwise_not_inplace()); }

		// 4. тернарные действия. см. комментарий в FieldElement
		//	важно: во всех последующих объявлениях аргумент-функция необязательно комплексный
		// (это определяется видом аргументов шаблона: "template_2, template_2_args" вместо "template_2C, template_2C_args"

		// тернаное сложение и вычитание массива с действительным числом
		template<XRAD__template_2>
		child_type	&add(const GenericFieldElement<XRAD__template_2_args> &f1, const complex_component_type &x)
		{ return algorithms_type::AAS_Op_Assign(child_ref(), f1, x, Functors::assign_plus()); }
		template<XRAD__template_2>
		child_type	&subtract(const GenericFieldElement<XRAD__template_2_args> &f1, const complex_component_type &x)
		{ return algorithms_type::AAS_Op_Assign(child_ref(), f1, x, Functors::assign_minus()); }

		// Для функтора assign_percent, чтобы это была комплексная операция, оба аргумента должны быть комплексными числами.
		template<XRAD__template_2, class T3, class ST3>
		child_type	&multiply_conj(const FieldElementMath<XRAD__template_2_args, FieldTagComplex> &f1, const ComplexSample<T3, ST3> &x)
		{ return algorithms_type::AAS_Op_Assign(child_ref(), f1, x, Functors::assign_percent()); }

		//	Делаем доступными переопределенные здесь родительские методы

		using parent::operator +=;
		using parent::operator -=;
		using parent::operator *=;
		using parent::operator /=;

		using parent::operator +;
		using parent::operator -;
		using parent::operator *;
		using parent::operator /;

		using parent::add;
		using parent::subtract;
};

#undef XRAD__fce_template_args
#undef XRAD__template_2
#undef XRAD__template_2_args

/*!
	\brief Вспомогательная функция для реализации специализаций ComplexFieldTraits
	для наследников FieldElementMath<CONTAINER_T, CHILD_T, VT, ST, FieldTagComplex>.
	Задает тип компоненты, равный типу компоненты VT

	Используется в конструкциях типа ComplexFunction<ComplexFunction<double, double>, double>.
*/
template<class CONTAINER_T, class CHILD_T, class VT, class ST, class ALG_T>
typename ComplexFieldTraits<VT>::complex_component_type
	GetComplexFieldElementTag(FieldElementMath<CONTAINER_T, CHILD_T, VT, ST, ALG_T, FieldTagComplex>*);

//--------------------------------------------------------------

}//namespace	AlgebraicStructures
XRAD_END

#undef ComplexFieldElement_h___inside
#endif // ComplexFieldElement_h__
