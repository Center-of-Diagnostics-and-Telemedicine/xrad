// file FixedSizeArrayAnalyze.h
//--------------------------------------------------------------
#ifndef XRAD__File_FixedSizeArrayAnalyze_h
#define XRAD__File_FixedSizeArrayAnalyze_h
//--------------------------------------------------------------

#include "FixedSizeArray.h"
#include "BasicArrayInteractions1D.h"
#include "ArrayAnalyzeFunctors.h"

XRAD_BEGIN

//--------------------------------------------------------------

template <class FindFunctor, class T, size_t N>
T	FindValue(const FixedSizeArray<T, N> &array, size_t *p = NULL)
{
	static_assert(N > 0, "FindValue<>: array size is 0.");
	//эти функции не следует делать через transformed (возможна потеря быстродействия)
	typedef typename remove_cv<T>::type result_type;
	size_t position = 0;
	result_type value(array[0]);
	Apply_IA_1D_F2(array, FindFunctor(&position, &value), 1, array.size());

	if(p)
		*p = position;
	return value;
}

template <class T, size_t N>
T	MaxValue(const FixedSizeArray<T, N> &array, size_t *p = NULL)
{
	typedef typename remove_cv<T>::type result_type;
	return FindValue<Functors::find_iv<Functors::find_predicate_max, result_type>>(array, p);
}

template <class T, size_t N>
T	MinValue(const FixedSizeArray<T, N> &array, size_t *p = NULL)
{
	typedef typename remove_cv<T>::type result_type;
	return FindValue<Functors::find_iv<Functors::find_predicate_min, result_type>>(array, p);
}

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__File_FixedSizeArrayAnalyze_h
