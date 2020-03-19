#ifdef __algebra_element_h_inside
#error Error: Recursive inclusion of AlgebraElement.h detected.
#endif
#ifndef __algebra_element_h
#define __algebra_element_h
#define __algebra_element_h_inside

#include "FieldElement.h"
#include <XRADBasic/Sources/Core/Functors.h>

//--------------------------------------------------------------

#define XRAD__template_1 class CONTAINER_T, class CHILD_T, class VT, class ST, class ALGORITHMS_T, class FIELD_TAG
#define XRAD__template_NF class CONTAINER_T, class CHILD_T, class VT, class ST, class ALGORITHMS_T
#define XRAD__template_2 class CONTAINER_T2, class CHILD_T_2, class VT2, class ST2, class ALGORITHMS_T2, class FIELD_TAG2

#define XRAD__template_1_args CONTAINER_T,CHILD_T,VT,ST,ALGORITHMS_T,FIELD_TAG
#define XRAD__template_NF_args CONTAINER_T,CHILD_T,VT,ST,ALGORITHMS_T
#define XRAD__template_2_args CONTAINER_T2,CHILD_T_2,VT2,ST2,ALGORITHMS_T2, FIELD_TAG2

XRAD_BEGIN
namespace	AlgebraicStructures
{

//--------------------------------------------------------------
/*!
	\brief Элемент алгебры, универсальная часть

	Класс должен соответствовать элементу некоторой алгебры
	(кольца, поля или тела: точнее не определяется)
	над полем скаляров ST; то есть, заданы операция сложения, умножения
	на скаляр и умножений между элементами алгебры.

	Этот класс используется только в качестве базового,
	поэтому конструкторы у него являются protected.
	Использовать непосредственно следует класс AlgebraElement.

	См. \ref pg_MoveOperations.
*/
template<XRAD__template_1>
class GenericAlgebraElement : public FieldElement<XRAD__template_1_args>
{
	private:
		typedef	CHILD_T child_type;
		typedef GenericAlgebraElement<XRAD__template_1_args> self;
		PARENT(FieldElement<XRAD__template_1_args>);

		child_type &child_ref(){return static_cast<child_type&>(*this);}
		const child_type &child_ref() const {return static_cast<const child_type&>(*this);}

	public:
		typedef typename CONTAINER_T::value_type value_type;
		typedef ST scalar_type;
		using algorithms_type = typename parent::algorithms_type;

	public:
		// Использование родительских конструкторов и operator=
		using parent::parent;
		GenericAlgebraElement() = default;
		GenericAlgebraElement(const parent &p): parent(p) {}
		GenericAlgebraElement(parent &&p): parent(std::move(p)) {}
		using parent::operator=;

	public:
		template<XRAD__template_2>
		child_type	&operator *= (const GenericAlgebraElement<XRAD__template_2_args> &original){return algorithms_type::AA_Op_Assign(child_ref(), original, Functors::multiply_assign());}
		template<XRAD__template_2>
		child_type	&operator /= (const GenericAlgebraElement<XRAD__template_2_args> &original){return algorithms_type::AA_Op_Assign(child_ref(), original, Functors::divide_assign());}
		template<XRAD__template_2>
		child_type	operator * (const GenericAlgebraElement<XRAD__template_2_args> &original) const {return algorithms_type::AA_Op_New(child_ref(), original, Functors::assign_multiply());}
		template<XRAD__template_2>
		child_type	operator / (const GenericAlgebraElement<XRAD__template_2_args> &original) const {return algorithms_type::AA_Op_New(child_ref(), original, Functors::assign_divide());}


		// тернарные действия: все массивы заранее имеют одинаковый размер,
		// действие вида a.multiply(b,c) означает a=b*c;
		// действие вида a.add_multiply(b,c) означает a+=b*c;
		// при этом никаких новых объектов не создается
		//TODO здесь и в FieldElement сделать более сложный шаблон, чтобы аргументы следующих функций могли быть разных типов
		template<XRAD__template_2>
		child_type	&multiply(const GenericAlgebraElement<XRAD__template_2_args> &f1, const GenericAlgebraElement<XRAD__template_2_args> &f2)
		{ return algorithms_type::AAA_Op_Assign(child_ref(), f1, f2, Functors::assign_multiply()); }
		template<XRAD__template_2>
		child_type	&divide(const GenericAlgebraElement<XRAD__template_2_args> &f1, const GenericAlgebraElement<XRAD__template_2_args> &f2)
		{ return algorithms_type::AAA_Op_Assign(child_ref(), f1, f2, Functors::assign_divide()); }

		template<XRAD__template_2>
		child_type	&add_multiply(const GenericAlgebraElement<XRAD__template_2_args> &f1, const GenericAlgebraElement<XRAD__template_2_args> &f2)
		{ return algorithms_type::AAA_Op_Assign(child_ref(), f1, f2, Functors::plus_assign_multiply()); }
		template<XRAD__template_2>
		child_type	&add_divide(const GenericAlgebraElement<XRAD__template_2_args> &f1, const GenericAlgebraElement<XRAD__template_2_args> &f2)
		{ return algorithms_type::AAA_Op_Assign(child_ref(), f1, f2, Functors::plus_assign_divide()); }
		template<XRAD__template_2>
		child_type	&subtract_multiply(const GenericAlgebraElement<XRAD__template_2_args> &f1, const GenericAlgebraElement<XRAD__template_2_args> &f2)
		{ return algorithms_type::AAA_Op_Assign(child_ref(), f1, f2, Functors::minus_assign_multiply()); }
		template<XRAD__template_2>
		child_type	&subtract_divide(const GenericAlgebraElement<XRAD__template_2_args> &f1, const GenericAlgebraElement<XRAD__template_2_args> &f2)
		{ return algorithms_type::AAA_Op_Assign(child_ref(), f1, f2, Functors::minus_assign_divide()); }

		// умножение другого массива на свой value_type
		// например, у
		template<XRAD__template_2>
		child_type	&multiply(const GenericAlgebraElement<XRAD__template_2_args> &f1, const value_type &x)
		{ return algorithms_type::AAS_Op_Assign(child_ref(), f1, x, Functors::assign_multiply()); }
		template<XRAD__template_2>
		child_type	&divide(const GenericAlgebraElement<XRAD__template_2_args> &f1, const value_type &x)
		{ return algorithms_type::AAS_Op_Assign(child_ref(), f1, x, Functors::assign_divide()); }

		template<XRAD__template_2>
		child_type	&add_multiply(const GenericAlgebraElement<XRAD__template_2_args> &f1, const value_type &x)
		{ return algorithms_type::AAS_Op_Assign(child_ref(), f1, x, Functors::plus_assign_multiply()); }
		template<XRAD__template_2>
		child_type	&add_divide(const GenericAlgebraElement<XRAD__template_2_args> &f1, const value_type &x)
		{ return algorithms_type::AAS_Op_Assign(child_ref(), f1, x, Functors::plus_assign_divide()); }
		template<XRAD__template_2>
		child_type	&subtract_multiply(const GenericAlgebraElement<XRAD__template_2_args> &f1, const value_type &x)
		{ return algorithms_type::AAS_Op_Assign(child_ref(), f1, x, Functors::minus_assign_multiply()); }
		template<XRAD__template_2>
		child_type	&subtract_divide(const GenericAlgebraElement<XRAD__template_2_args> &f1, const value_type &x)
		{ return algorithms_type::AAS_Op_Assign(child_ref(), f1, x, Functors::minus_assign_divide()); }

		//
		//	Делаем доступными переопределенные здесь родительские методы
		//
		using parent::operator *=;
		using parent::operator /=;

		using parent::operator *;
		using parent::operator /;

		using parent::multiply;
		using parent::divide;
		using parent::add_multiply;
		using parent::add_divide;
		using parent::subtract_multiply;
		using parent::subtract_divide;
};


//--------------------------------------------------------------
/*!
	\brief Общий шаблон AlgebraElement — элемента алгебры

	Сам общий шаблон не используется, используются только его специализации по параметру шаблона FIELD_TAG.
*/
template<XRAD__template_1>
class AlgebraElement;

//--------------------------------------------------------------
/*!
	\brief Реализация AlgebraElement для "скалярных" типов данных (специализация для FieldTagScalar)

	Служит основой для всех действительных арифметических классов.

	См. \ref pg_MoveOperations.
*/
template<XRAD__template_NF>
class AlgebraElement<XRAD__template_NF_args, FieldTagScalar> : public GenericAlgebraElement<XRAD__template_NF_args, FieldTagScalar>
{
	// все, что есть в этом классе, унаследовано напрямую от предка.
	private:
		PARENT(GenericAlgebraElement<XRAD__template_NF_args, FieldTagScalar>);
		typedef AlgebraElement<XRAD__template_NF_args, FieldTagScalar> self;

	public:
		// Использование родительских конструкторов и operator=
		using parent::parent;
		AlgebraElement() = default;
		AlgebraElement(const parent &p): parent(p) {}
		AlgebraElement(parent &&p): parent(std::move(p)) {}
		using parent::operator=;
};

//--------------------------------------------------------------

} // namespace	AlgebraicStructures

//--------------------------------------------------------------

#define XRAD__define_action(action_name)\
template<XRAD__template_1, XRAD__template_2> \
AlgebraicStructures::GenericAlgebraElement<XRAD__template_1_args> &\
		action_name(\
				AlgebraicStructures::GenericAlgebraElement<XRAD__template_1_args> &x1,\
				const AlgebraicStructures::GenericAlgebraElement<XRAD__template_2_args> &x2,\
				const AlgebraicStructures::GenericAlgebraElement<XRAD__template_2_args> &x3)\
	{\
	return x1.action_name(x2, x3);\
	}

XRAD__define_action(multiply)
XRAD__define_action(divide)
XRAD__define_action(add_multiply)
XRAD__define_action(add_divide)
XRAD__define_action(subtract_multiply)
XRAD__define_action(subtract_divide)

#undef XRAD__define_action

//--------------------------------------------------------------

XRAD_END

#undef XRAD__template_1
#undef XRAD__template_2

#undef XRAD__template_1_args
#undef XRAD__template_2_args

//--------------------------------------------------------------
#undef __algebra_element_h_inside
#endif  //__algebra_element_h
