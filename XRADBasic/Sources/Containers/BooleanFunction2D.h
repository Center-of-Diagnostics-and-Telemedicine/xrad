#ifndef __boolean_function_2d_h
#define __boolean_function_2d_h

#include "DataArray2D.h"
#include <XRADBasic/Sources/Algebra/AlgebraicStructures2D.h>
#include "BooleanFunction.h"

XRAD_BEGIN

//--------------------------------------------------------------

//! \brief Двумерная логическая булева функция
template<class T>
class	BooleanFunctionLogical2D: public BooleanAlgebraLogical2D<BooleanFunctionLogical2D<T>, BooleanFunctionLogical<T>, T>
{
		PARENT(BooleanAlgebraLogical2D<BooleanFunctionLogical2D, BooleanFunctionLogical<T>, T>);

	public:
		typedef BooleanFunctionLogical2D<T> self;
		typedef BooleanFunctionLogical2D<T> child_type;

		child_type &child_ref(){return (*this);}
		const child_type &child_ref() const {return (*this);}

		typedef typename parent::value_type value_type;

	public:
		// Использование родительских конструкторов и operator=
		using parent::parent;
		BooleanFunctionLogical2D() = default;
		BooleanFunctionLogical2D(const parent &p): parent(p) {}
		BooleanFunctionLogical2D(parent &&p): parent(std::move(p)) {}
		using parent::operator=;
};

//--------------------------------------------------------------

//! \brief Двумерная побитовая булева функция
template<class T>
class	BooleanFunctionBitwise2D: public BooleanAlgebraBitwise2D<BooleanFunctionBitwise2D<T>, BooleanFunctionBitwise<T>, T>
{
		PARENT(BooleanAlgebraBitwise2D<BooleanFunctionBitwise2D, BooleanFunctionBitwise<T>, T>);

	public:
		typedef BooleanFunctionBitwise2D<T> self;
		typedef BooleanFunctionBitwise2D<T> child_type;

		child_type &child_ref(){return (*this);}
		const child_type &child_ref() const {return (*this);}

		typedef typename parent::value_type value_type;

	public:
		// Использование родительских конструкторов и operator=
		using parent::parent;
		BooleanFunctionBitwise2D() = default;
		BooleanFunctionBitwise2D(const parent &p): parent(p) {}
		BooleanFunctionBitwise2D(parent &&p): parent(std::move(p)) {}
		using parent::operator=;
};

//--------------------------------------------------------------

XRAD_END

#endif //__boolean_function_2d_h
