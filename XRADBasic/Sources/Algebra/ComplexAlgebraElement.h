#ifndef XRAD__File_complex_algebra_element_h
#define XRAD__File_complex_algebra_element_h
/*!
	\addtogroup gr_Algebra
	@{

	\file
	\brief Специфические действия для комплексной алгебры

	@}
*/
//--------------------------------------------------------------

#include "AlgebraElement.h"
#include "ComplexFieldElement.h"

XRAD_BEGIN
namespace	AlgebraicStructures
{

//--------------------------------------------------------------

// для комплексной алгебры
// третий аргумент шаблона CT -- не комплексный тип, а тип комплексной компоненты
// порождается тип ComplexSample<CT>
#define XRAD__template_1C class CONTAINER_T, class CHILD_T, class VT, class ST, class ALGORITHMS_T
#define XRAD__template_2C class CONTAINER_T2, class CHILD_T_2, class VT2, class ST2, class ALGORITHMS_T2

#define XRAD__template_1C_args CONTAINER_T,CHILD_T,VT,ST,ALGORITHMS_T,FieldTagComplex
#define XRAD__template_2C_args CONTAINER_T2,CHILD_T_2,VT2,ST2,ALGORITHMS_T2,FieldTagComplex

/*!
	\brief Реализация AlgebraElement для комплекснозначных типов данных (специализация для FieldTagComplex)

	См. \ref pg_MoveOperations.
*/
template<XRAD__template_1C>
class AlgebraElement<XRAD__template_1C_args> : public GenericAlgebraElement<XRAD__template_1C_args>
{
	private:
		typedef	CHILD_T child_type;
		typedef AlgebraElement<XRAD__template_1C_args> self;
		PARENT(GenericAlgebraElement<XRAD__template_1C_args>);

		child_type &child_ref(){return static_cast<child_type&>(*this);}
		const child_type &child_ref() const {return static_cast<const child_type&>(*this);}

	public:
		typedef VT value_type;
		typedef ComplexFieldTraits<ComplexSample<double, double>>::complex_component_type __debug_type;
		typedef typename ComplexFieldTraits<VT>::complex_component_type complex_component_type;
		typedef ST scalar_type;
		using algorithms_type = typename parent::algorithms_type;

	public:
		// Использование родительских конструкторов и operator=
		using parent::parent;
		AlgebraElement() = default;
		AlgebraElement(const parent &p): parent(p) {}
		AlgebraElement(parent &&p): parent(std::move(p)) {}
		using parent::operator=;



		//	сопряженное умножение на другую комплексную функцию

		template<XRAD__template_2C>
		child_type	&operator %= (const AlgebraElement<XRAD__template_2C_args> &x){return algorithms_type::AA_Op_Assign(child_ref(), x, Functors::percent_assign());}
		template<XRAD__template_2C>
		child_type	operator % (const AlgebraElement<XRAD__template_2C_args> &x) const {return algorithms_type::AA_Op_New(child_ref(), x, Functors::assign_percent());}

		// унарный префиксный оператор комплексного сопряжения, реализован в FieldElement<complex_args>
		// тернарное умножение, деление и сопряженное умножение на комплексный скаляр реализовано в FieldElement<complex_args>
		//
		// тернарное сопряженное перемножение
		// оба аргумента должны быть комплексными
		template<XRAD__template_2C>
		child_type	&multiply_conj(const AlgebraElement<XRAD__template_2C_args> &f1, const AlgebraElement<XRAD__template_2C_args> &f2)
		{ return algorithms_type::AAA_Op_Assign(child_ref(), f1, f2, Functors::assign_percent()); }


		//	Делаем доступными переопределенные здесь родительские методы
		using parent::operator %=;
		using parent::operator %;
		using parent::multiply_conj;
};

#undef XRAD__template_1C
#undef XRAD__template_2C

#undef XRAD__template_1C_args
#undef XRAD__template_2C_args

//--------------------------------------------------------------

}//namespace	AlgebraicStructures
XRAD_END

#endif //XRAD__File_complex_algebra_element_h
