#ifndef FFTMD_h__
#define FFTMD_h__

/*!
* \file FFTMD.h
* \date 2017/05/26 16:07
*
* \author kulberg
*
* \brief
*
* TODO: long description
*
* \note
*/

#include "Sources/Containers/DataArrayMD.h"
#include "FFT2D.h"
#include <omp.h>

XRAD_BEGIN




//	быстрое преобразование Фурье, только три измерения.
//TODO развить на много измерений, использовать наработки по roll().
//TODO сделать версию FFTf (см. двумерную реализацию)
template<class A2DT>
void	FFT_3D(DataArrayMD<A2DT> &f, ftDirection dir, omp_usage_t omp = e_use_omp)
{
	size_t	n_threads = omp_get_max_threads();
	size_t	n_slices = f.sizes(0);
	size_t	n_stages = (n_slices + n_threads - 1) / n_threads;

	for(size_t i = 0; i < n_stages; ++i)
	{
		size_t piece_size = min(n_threads, n_slices-i*n_threads);
		if(omp == e_use_omp)
		{
			ThreadErrorCollector ec("FFT 3D (slices)");
			#pragma omp parallel for schedule (guided)
			for(ptrdiff_t j = 0; j < ptrdiff_t(piece_size); ++j)
			{
				if (ec.HasErrors())
				{
#ifdef XRAD_COMPILER_MSC
					break;
#else
					continue;
#endif
				}
				ThreadSetup ts; (void)ts;
				try
				{
					typename DataArrayMD<A2DT>::slice_type	slice;
					size_t	slice_no = i*n_threads + j;
					f.GetSlice(slice, {slice_no, slice_mask(0), slice_mask(1)});
					FFT(slice, dir, e_dont_use_omp);
				}
				catch (...)
				{
					ec.CatchException();
				}
			}
			ec.ThrowIfErrors();
		}
		else
		{
			for(ptrdiff_t j = 0; j < ptrdiff_t(piece_size); ++j)
			{
				typename DataArrayMD<A2DT>::slice_type	slice;
				size_t	slice_no = i*n_threads + j;
				f.GetSlice(slice, {slice_no, slice_mask(0), slice_mask(1)});
				FFT(slice, dir, e_dont_use_omp);
			}
		}
	}
	for(size_t i = 0; i < f.sizes(1); ++i)
	{
		if(omp == e_use_omp)
		{
			ThreadErrorCollector ec("FFT 3D (rows)");
			#pragma omp parallel for schedule (guided)
			for(ptrdiff_t j = 0; j < ptrdiff_t(f.sizes(2)); ++j)
			{
				if (ec.HasErrors())
				{
#ifdef XRAD_COMPILER_MSC
					break;
#else
					continue;
#endif
				}
				ThreadSetup ts; (void)ts;
				try
				{
					typename DataArrayMD<A2DT>::row_type	row;
					f.GetRow(row, {slice_mask(0), i, size_t(j)});
					FFT(row, dir);
				}
				catch (...)
				{
					ec.CatchException();
				}
			}
			ec.ThrowIfErrors();
		}
		else
		{
			for(ptrdiff_t j = 0; j < ptrdiff_t(f.sizes(2)); ++j)
			{
				typename DataArrayMD<A2DT>::row_type	row;
				f.GetRow(row, {slice_mask(0), i, size_t(j)});
				FFT(row, dir);
			}
		}
	}
}


XRAD_END

#endif // FFTMD_h__
