#ifndef __math_function_multidimensional_h
#define __math_function_multidimensional_h
//--------------------------------------------------------------

#include "DataArrayMD.h"
#include "MathFunction.h"
#include "MathFunction2D.h"
#include <XRADBasic/Sources/Algebra/AlgebraicStructuresMD.h>

XRAD_BEGIN

//--------------------------------------------------------------

template <class A2DT>
class	MathFunctionMD : public AlgebraMD<MathFunctionMD<A2DT>, A2DT, typename A2DT::value_type, typename A2DT::scalar_type, typename A2DT::field_tag>
{
		PARENT(AlgebraMD<MathFunctionMD, A2DT, typename A2DT::value_type, typename A2DT::scalar_type, typename A2DT::field_tag>);
	public:
		typedef MathFunctionMD<A2DT> self;
		typedef MathFunctionMD<typename A2DT::invariable> invariable;
		typedef ReferenceOwner<self> ref;
		typedef ReferenceOwner<invariable> ref_invariable;
			// про invariable см. подробный комментарий в DataOwner.h

		using typename parent::value_type;

		//! \name Конструкторы. См. \ref pg_CopyContructorOperatorEq, \ref pg_MoveOperations
		//! @{
	public:
		// Использование родительских конструкторов и operator=
		using parent::parent;
		MathFunctionMD() = default;
		MathFunctionMD(const parent &p): parent(p) {}
		MathFunctionMD(parent &&p): parent(std::move(p)) {}
		// Не форвардим родительский метод, т.к. он возвращает ссылку на родительский тип:
		// using parent::operator=;

		//! \brief См. \ref pg_CopyContructorOperatorEq
		//!
		//! Не форвардим родительский метод, т.к. он возвращает ссылку на родительский тип.
		template<class A2DT2> self	&operator = (const DataArrayMD<A2DT2> &original){ parent::operator=(original); return *this; }

		template<class A2DT2> self	&operator = (DataArrayMD<A2DT2> &&original){ parent::operator=(std::move(original)); return *this; }
		//! @}
};

//--------------------------------------------------------------

template<class A2DT>
MathFunctionMD<A2DT>	zero_value(const MathFunctionMD<A2DT> &datum)
{
	index_vector	access_v(datum.sizes());
	access_v.fill(0);
	return MathFunctionMD<A2DT>(datum.sizes(), zero_value(datum.at(access_v)));
}

template<class A2DT>
void	make_zero(MathFunctionMD<A2DT> &datum)
{
	index_vector	access_v(datum.sizes());
	access_v.fill(0);
	make_zero(datum.at(access_v));
	datum.fill(datum.at(access_v));
}

//--------------------------------------------------------------

check_container_md(MathFunctionMD, MathFunction2D<MathFunction<double,double,AlgebraicStructures::FieldTagScalar> >)

//--------------------------------------------------------------

XRAD_END

#include "MathFunctionMD.hh"

//--------------------------------------------------------------
#endif //__math_function_multidimensional_h
