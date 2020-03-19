#ifndef VectorFunction_h__
#define VectorFunction_h__

/********************************************************************
	created:	2016/09/29
	created:	29:9:2016   14:30
	author:		kns
*********************************************************************/
#include "DataArray.h"
#include <XRADBasic/Sources/Algebra/AlgebraicStructures1D.h>
#include <XRADBasic/Sources/Algebra/AlgebraicAlgorithmsDataArray.h>
#include "SpaceCoordinates.h"

XRAD_BEGIN

// вектор-функция в обычном понимании. Наследуется от FieldElement, а не от AlgebraElement,
// так как перемножение векторов, в отличие от скаляров, не задано.

template<class T, class ST, class FIELD_TAG, size_t N>
struct VectorFunctionType
{
	using type = point_N<T, ST, N, FIELD_TAG>;
};

template<class T, class ST, class FIELD_TAG>
struct VectorFunctionType<T, ST, FIELD_TAG, 2>
{
	using type = point_2<T, ST, FIELD_TAG>;
};

template<class T, class ST, class FIELD_TAG>
struct VectorFunctionType<T, ST, FIELD_TAG, 3>
{
	using type = point_3<T, ST, FIELD_TAG>;
};

template <class T, class ST, class FIELD_TAG, size_t N>
using VectorFunctionComponent = typename VectorFunctionType<T, ST, FIELD_TAG, N>::type;


template<class T, class ST, class FIELD_TAG, size_t N>
class	VectorFunction : public AlgebraicStructures::FieldElement<
		DataArray<VectorFunctionComponent<T,ST,FIELD_TAG,N>>,
		VectorFunction<T,ST,FIELD_TAG,N>,
		VectorFunctionComponent<T,ST,FIELD_TAG,N>,
		ST,
		AlgebraicStructures::AlgebraicAlgorithmsDataArray,
		FIELD_TAG>
{
public:
	PARENT(AlgebraicStructures::FieldElement<
			DataArray<VectorFunctionComponent<T,ST,FIELD_TAG,N>>,
			VectorFunction<T,ST,FIELD_TAG,N>,
			VectorFunctionComponent<T,ST,FIELD_TAG,N>,
			ST,
			AlgebraicStructures::AlgebraicAlgorithmsDataArray,
			FIELD_TAG>);

	typedef typename VectorFunction<T,ST,FIELD_TAG,N> self;
	typedef typename VectorFunction<const T,ST,FIELD_TAG,N> invariable;
	typedef ReferenceOwner<self> ref;
	typedef ReferenceOwner<invariable> ref_invariable;
		// про invariable см. подробный комментарий в DataOwner.h

	typedef VectorFunctionComponent<T,ST,FIELD_TAG,N> value_type;
	typedef ST scalar_type;

	VectorFunction(){}
	explicit VectorFunction(size_t s) : parent(s){}
	VectorFunction(size_t s, const value_type &default_value) : parent(s, default_value){}

	template<class T2, class ST2, class FIELD_TAG2>
	VectorFunction(const VectorFunction<T2, ST2, FIELD_TAG2, N> &original) : parent(original.size())
	{
		Apply_AA_1D_F2(*this, original, Functors::assign());
	}

	//
	//	inherited typedefs
	//

	typedef typename parent::iterator iterator;
	typedef typename parent::const_iterator const_iterator;
	typedef typename parent::reverse_iterator reverse_iterator;
	typedef typename parent::const_reverse_iterator const_reverse_iterator;

	//
	//	inherited methods
	//

	typedef typename parent::iterator iterator;
	typedef typename parent::const_iterator const_iterator;
	typedef typename parent::reverse_iterator reverse_iterator;
	typedef typename parent::const_reverse_iterator const_reverse_iterator;

	using parent::realloc;
	using parent::size;
	using parent::begin;
	using parent::end;
	using parent::rbegin;
	using parent::rend;
	using parent::at;
	using parent::uses_external_data;
};

template<class T, class ST, size_t N>
using ScalarVectorFunction = VectorFunction<T, ST, AlgebraicStructures::FieldTagScalar, N>;//TODO Вопрос, откуда FieldTagScalar в вектор-функции?

typedef	ScalarVectorFunction<int16_t, double, 2> VectorFunction2_I16;
typedef	ScalarVectorFunction<int32_t, double, 2> VectorFunction2_I32;
typedef	ScalarVectorFunction<float, double, 2> VectorFunction2_F32;
typedef	ScalarVectorFunction<double, double, 2> VectorFunction2_F64;

typedef	ScalarVectorFunction<int16_t, double, 3> VectorFunction3_I16;
typedef	ScalarVectorFunction<int32_t, double, 3> VectorFunction3_I32;
typedef	ScalarVectorFunction<float, double, 3> VectorFunction3_F32;
typedef	ScalarVectorFunction<double, double, 3> VectorFunction3_F64;


XRAD_END

#endif // VectorFunction_h__
