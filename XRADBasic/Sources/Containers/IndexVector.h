/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_index_vector_h
#define XRAD__File_index_vector_h
/*!
	\file
	\brief Векторы индексов, смещений для обращения к многомерному массиву
*/
//--------------------------------------------------------------

#include "LinearVector.h"

XRAD_BEGIN

//--------------------------------------------------------------

//! \brief Вектор индексов для обращения к многомерному массиву
typedef	LinearVector<size_t, size_t, AlgebraicStructures::FieldTagScalar> index_vector;

//! \brief Вектор смещений для обращения к многомерному массиву
typedef	LinearVector<ptrdiff_t, ptrdiff_t, AlgebraicStructures::FieldTagScalar> offset_vector;

//
// inline index_vector	quick_iv(size_t i)
// 	{
// 	index_vector	result(1);
// 	result[0] = i;
//
// 	return result;
// 	}
//
// inline index_vector	quick_iv(size_t i, size_t j)
// 	{
// 	index_vector	result(2);
// 	result[0] = i;
// 	result[1] = j;
//
// 	return result;
// 	}
//
// inline index_vector	quick_iv(size_t i, size_t j, size_t k)
// 	{
// 	index_vector	result(3);
// 	result[0] = i;
// 	result[1] = j;
// 	result[2] = k;
//
// 	return result;
// 	}
//
// inline index_vector	quick_iv(size_t i, size_t j, size_t k, size_t l)
// 	{
// 	index_vector	result(4);
// 	result[0] = i;
// 	result[1] = j;
// 	result[2] = k;
// 	result[3] = l;
//
// 	return result;
// 	}

// Специальные значения для подмножеств многомерного массива

namespace internal_slices
{
#if XRAD_SIZE_T_BITS == 64
	inline	size_t highest_bit_mask(){return 0x8000000000000000u;}
	inline	size_t slice_xor_mask(){return 0xFFFFFFFFFFFFFFFFu;}
#elif XRAD_SIZE_T_BITS == 32
	inline	size_t highest_bit_mask(){return 0x80000000u;}
	inline	size_t slice_xor_mask(){return 0xFFFFFFFFu;}
#else
	#error "Unknown size_t width"
#endif
} // namespace internal_slices

inline bool	is_slice_mask(size_t i){return (i&internal_slices::highest_bit_mask())==0u ? false:true;}
inline size_t	slice_mask(size_t dimension_no){return (dimension_no ^ internal_slices::slice_xor_mask()); }
inline size_t	dimension_no(size_t slice_mask){return (slice_mask ^ internal_slices::slice_xor_mask()); }

//--------------------------------------------------------------

XRAD_END

#endif //XRAD__File_index_vector_h
