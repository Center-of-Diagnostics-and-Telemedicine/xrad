#ifndef XRAD__File_boolean_function_h
#define XRAD__File_boolean_function_h

#include "DataArray.h"
#include <XRADBasic/Sources/Algebra/AlgebraicStructures1D.h>

XRAD_BEGIN

//--------------------------------------------------------------

//! \brief Одномерная логическая булева функция
template<class T>
class	BooleanFunctionLogical: public BooleanAlgebraLogical1D<BooleanFunctionLogical<T>, T>
{
		PARENT(BooleanAlgebraLogical1D<BooleanFunctionLogical<T>, T>);

	public:
		typedef BooleanFunctionLogical<T> self;
		typedef BooleanFunctionLogical<T> child_type;

		child_type &child_ref(){return (*this);}
		const child_type &child_ref() const {return (*this);}

		typedef typename parent::value_type value_type;

	public:
		// Использование родительских конструкторов и operator=
		using parent::parent;
		BooleanFunctionLogical() = default;
		BooleanFunctionLogical(const parent &p): parent(p) {}
		BooleanFunctionLogical(parent &&p): parent(std::move(p)) {}
		using parent::operator=;
};

//--------------------------------------------------------------

//! \brief Одномерная побитовая булева функция
template<class T>
class	BooleanFunctionBitwise: public BooleanAlgebraBitwise1D<BooleanFunctionBitwise<T>, T>
{
		PARENT(BooleanAlgebraBitwise1D<BooleanFunctionBitwise<T>, T>);

	public:
		typedef BooleanFunctionBitwise<T> self;
		typedef BooleanFunctionBitwise<T> child_type;

		child_type &child_ref(){return (*this);}
		const child_type &child_ref() const {return (*this);}

		typedef typename parent::value_type value_type;

	public:
		// Использование родительских конструкторов и operator=
		using parent::parent;
		BooleanFunctionBitwise() = default;
		BooleanFunctionBitwise(const parent &p): parent(p) {}
		BooleanFunctionBitwise(parent &&p): parent(std::move(p)) {}
		using parent::operator=;
};

//--------------------------------------------------------------

XRAD_END

#endif //XRAD__File_boolean_function_h
