// file RealFunction.h
//--------------------------------------------------------------
#ifndef XRAD__File_RealFunction_h
#define XRAD__File_RealFunction_h
//--------------------------------------------------------------

#include "MathFunction.h"
#include <XRADBasic/Sources/SampleTypes/HomomorphSamples.h>

XRAD_BEGIN

//--------------------------------------------------------------

#define XRAD__RealFunction_template class T, class ST
#define XRAD__RealFunction_template_args T, ST
/*!
	\brief Действительная одномерная дискретная функция

	В дополнение к методам базового класса MathFunction
	реализует функции интерполяции и дифференцирования, специфические
	для действительных функций.
*/
template<XRAD__RealFunction_template>
class	RealFunction : public MathFunction<T,ST,AlgebraicStructures::FieldTagScalar>
{
	private:
		PARENT(MathFunction<T,ST,AlgebraicStructures::FieldTagScalar>);

	public:
		typedef RealFunction self;
		typedef RealFunction<const T,ST> invariable;
		typedef ReferenceOwner<self> ref;
		typedef ReferenceOwner<invariable> ref_invariable;
			// про invariable см. подробный комментарий в DataOwner.h

		typedef typename parent::value_type value_type;
		typedef typename parent::value_type_invariable value_type_invariable;
		typedef typename parent::value_type_variable value_type_variable;

	public:
		// Использование родительских конструкторов и operator=
		using parent::parent;
		RealFunction() = default;
		RealFunction(const parent &p): parent(p) {}
		RealFunction(parent &&p): parent(std::move(p)) {}
		using parent::operator=;

	public:
		//! \name Интерполяция и дифференцирование
		//! @{
		using parent::in;
		floating64_type<value_type>	in(double x) const;
		floating64_type<value_type> d_dx(double x) const;
		//! @}
};

//--------------------------------------------------------------

XRAD_END

#include "RealFunction.hh"

//--------------------------------------------------------------
#endif // XRAD__File_RealFunction_h
