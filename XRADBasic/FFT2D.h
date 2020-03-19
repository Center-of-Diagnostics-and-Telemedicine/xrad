#ifndef FFT2D_h__
#define FFT2D_h__

/*!
* \file FFT2D.h
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

#include "Sources/Containers/DataArray2D.h"
#include "FFT1D.h"


XRAD_BEGIN

template<class RT>
void FFTf(DataArray2D<RT> &f, ft_flags rows_flags, ft_flags columns_flags, omp_usage_t omp = e_dont_use_omp)
{
	if(omp==e_use_omp)
	{
		if(rows_flags)
		{
			ThreadErrorCollector ec("FFT 2D (rows)");
			#pragma omp parallel for schedule (guided)
			for(ptrdiff_t i=0; i < ptrdiff_t(f.vsize()); ++i)
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
					FFTf(f.row(i), rows_flags);
				}
				catch (...)
				{
					ec.CatchException();
				}
			}
			ec.ThrowIfErrors();
		}
		if(columns_flags)
		{
			ThreadErrorCollector ec("FFT 2D (columns)");
			#pragma omp parallel for schedule (guided)
			for(ptrdiff_t i=0; i < ptrdiff_t(f.hsize()); ++i)
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
					FFTf(f.col(i), columns_flags);
				}
				catch (...)
				{
					ec.CatchException();
				}
			}
			ec.ThrowIfErrors();
		}
	}
	else
	{
		if(rows_flags)
		{
			for(size_t i=0; i<f.vsize(); ++i)
			{
				FFTf(f.row(i), rows_flags);
			}
		}
		if(columns_flags)
		{
			for(size_t i=0; i<f.hsize(); ++i)
			{
				FFTf(f.col(i), columns_flags);
			}
		}
	}
}

template<class RT>
void FFT(DataArray2D<RT> &f, ftDirection direction, omp_usage_t omp = e_dont_use_omp)
{
	if(omp==e_use_omp)
	{
		ThreadErrorCollector ec("FFT 2D (columns)");
		#pragma omp parallel for schedule (guided)
		for(ptrdiff_t i=0; i<ptrdiff_t(f.hsize()); ++i)
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
				FFT(f.col(i), direction);
			}
			catch (...)
			{
				ec.CatchException();
			}
		}
		ec.ThrowIfErrors();

		ec.Reset("FFT 2D (rows)");
		#pragma omp parallel for schedule (guided)
		for(ptrdiff_t i=0; i<ptrdiff_t(f.vsize()); ++i)
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
				FFT(f.row(i), direction);
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
		for(size_t i=0; i<f.hsize(); ++i)
		{
			FFT(f.col(i), direction);
		}

		for(size_t i=0; i<f.vsize(); ++i)
		{
			FFT(f.row(i), direction);
		}
	}

}

template<class RT>
void FT(DataArray2D<RT> &f, ftDirection direction, omp_usage_t omp = e_dont_use_omp)
{
	if(omp==e_use_omp)
	{
		ThreadErrorCollector ec("FT 2D (columns)");
		#pragma omp parallel for schedule (guided)
		for(ptrdiff_t i=0; i<ptrdiff_t(f.hsize()); ++i)
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
				FT(f.col(i), direction);
			}
			catch (...)
			{
				ec.CatchException();
			}
		}
		ec.ThrowIfErrors();

		ec.Reset("FT 2D (rows)");
		#pragma omp parallel for schedule (guided)
		for(ptrdiff_t i=0; i<ptrdiff_t(f.vsize()); ++i)
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
				FT(f.row(i), direction);
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
		for(size_t i=0; i<f.hsize(); ++i)
		{
			FT(f.col(i), direction);
		}

		for(size_t i=0; i<f.vsize(); ++i)
		{
			FT(f.row(i), direction);
		}
	}
}


XRAD_END

#endif // FFT2D_h__
