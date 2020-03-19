#ifndef LinearVectorFixedDimensions_h__
#define LinearVectorFixedDimensions_h__
/*!
	\file
	\date 2015-01-26 17:34
	\author	kns
	\brief Вектор с фиксированным числом измерений
*/
//--------------------------------------------------------------

#include "FixedSizeArray.h"
#include <XRADBasic/Sources/Algebra/AlgebraicStructures1D.h>
#include <XRADBasic/Sources/Algebra/AlgebraicAlgorithmsFixedSizeArray.h>

XRAD_BEGIN

//--------------------------------------------------------------

#define XRAD__linear_vector_template class CHILD, class T, class ST, class FIELD_TAG, size_t N
#define XRAD__linear_vector_template_args CHILD, T, ST, FIELD_TAG, N

//! \brief Вектор с фиксированным числом измерений. Основное назначение -
//! двумерные и трехмерные вектора и диапазоны
template<XRAD__linear_vector_template>
class	LinearVectorFixedDimensions: public AlgebraicStructures::FieldElement<
		FixedSizeArray<T,N>,
		CHILD,
		T,
		ST,
		AlgebraicStructures::AlgebraicAlgorithmsFixedSizeArray,
		FIELD_TAG>
{
	public:
		PARENT(AlgebraicStructures::FieldElement<FixedSizeArray<T,N>, CHILD, T, ST,
				AlgebraicStructures::AlgebraicAlgorithmsFixedSizeArray,
				FIELD_TAG>);

		typedef LinearVectorFixedDimensions<XRAD__linear_vector_template_args> self;
		typedef LinearVectorFixedDimensions<CHILD, const T, ST, FIELD_TAG, N> invariable;
		typedef ReferenceOwner<self> ref;
		typedef ReferenceOwner<invariable> ref_invariable;
			// про invariable см. подробный комментарий в DataOwner.h

		typedef T value_type;
		typedef ST scalar_type;

	public:
		// Использование родительских конструкторов и operator=
		using parent::parent;
		LinearVectorFixedDimensions() = default;
		LinearVectorFixedDimensions(const parent &p): parent(p) {}
		LinearVectorFixedDimensions(parent &&p): parent(std::move(p)) {}
		using parent::operator=;
};

//--------------------------------------------------------------

XRAD_END

#include "LinearVectorFixedDimensions.hh"

//--------------------------------------------------------------
#endif // LinearVectorFixedDimensions_h__
