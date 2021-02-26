/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#include "pre.h"
#include "FourierBasic.h"

// Выбор алгоритма БПФ по умолчанию. Нужно определить один из следующих макросов.
//#define XRAD_FFT_CooleyTukey
#define XRAD_FFT_Decomposition

#ifdef XRAD_FFT_CooleyTukey
#include "CooleyTukeyFFT.h"
#elif defined (XRAD_FFT_Decomposition)
#include "DecompositionFFT.h"
#else
	#error Unknown FFT algorithm.
#endif

#include <XRADBasic/Sources/Containers/ComplexFunction.h>

XRAD_BEGIN

//--------------------------------------------------------------

size_t ceil_fft_length(size_t n)
{
#ifdef XRAD_FFT_CooleyTukey
	auto result = CooleyTukeyFFT::fft_phasors.ceil_fft_length(n);
	if (result < n)
	{
		throw length_error(ssprintf("FFT length is too large: %zu.", EnsureType<size_t>(n)));
	}
	return result;
#elif defined (XRAD_FFT_Decomposition)
	return DecompositionFFT::ceil_fft_length(n);
#else
	#error Unknown FFT algorithm.
#endif
}

//--------------------------------------------------------------

void InitFourierTransform(size_t maximum_allowed_fft_length)
{
#ifdef XRAD_FFT_CooleyTukey
	size_t	base = 2;
	size_t	order = logn(maximum_allowed_fft_length, base);
	if(pow(base, order) < maximum_allowed_fft_length)
		++order;
	CooleyTukeyFFT::fft_phasors.Initialize(base, order);
#elif defined (XRAD_FFT_Decomposition)
	DecompositionFFT::InitializeFFT(maximum_allowed_fft_length);
#else
	#error Unknown FFT algorithm.
#endif
}

//--------------------------------------------------------------

namespace FFTPrimitives
{

//--------------------------------------------------------------

template<class T, class ST>
void	FT_template(ComplexSample<T, ST> *ptr, size_t size, ftDirection direction)
{
	typedef	ComplexFunction<ComplexSample<T, ST>, ST> cf_type;
	cf_type	original;
	original.UseData(ptr, size);
	cf_type	phasors(size);
	cf_type	factors(size, complexF64(1. / sqrt(double(size))));
	cf_type	result(size);

	double	dir_factor = ((direction == ftForward) ? two_pi() : -two_pi()) / size;

	for(size_t i = 0; i < size; ++i)
	{
		phasors[i] = polar(1., dir_factor*double(i));
	}
	for(size_t i = 0; i < size; ++i)
	{
		// 		result.add_multiply(original, factors);
		result[i] = sp(original, factors);
		factors *= phasors;
	}
	original.CopyData(result);
}

//--------------------------------------------------------------

void FT_ptr(complexF32 *array, size_t size, ftDirection direction)
{
	FT_template(array, size, direction);
}

void FT_ptr(complexF64 *array, size_t size, ftDirection direction)
{
	FT_template(array, size, direction);
}

//--------------------------------------------------------------

void	FFT_ptr(complexF32 *array, size_t size, ftDirection direction)
{
	if (size == 1)
		return; // корректно: для функции из одного отсчета дпф ничего не меняет
#ifdef XRAD_FFT_CooleyTukey
	CooleyTukeyFFT::FTCT_F32.FFT(array, size, direction);
#elif defined (XRAD_FFT_Decomposition)
	DecompositionFFT::FFT(array, size, direction);
#else
	#error Unknown FFT algorithm.
#endif
}

void	FFT_ptr(complexF64 *array, size_t size, ftDirection direction)
{
	if (size == 1)
		return; // корректно: для функции из одного отсчета дпф ничего не меняет
#ifdef XRAD_FFT_CooleyTukey
	CooleyTukeyFFT::FTCT_F64.FFT(array, size, direction);
#elif defined (XRAD_FFT_Decomposition)
	DecompositionFFT::FFT(array, size, direction);
#else
	#error Unknown FFT algorithm.
#endif
}

//--------------------------------------------------------------

template<class T>
void	FFTf_template(T *array, size_t size, ft_flags flags)
{
	// главное назначение флагов вращения -- привести функцию к виду, удобному для отображения
	// (нулевой аргумент в центре диапазона)
	// или наоборот, к виду, удобному для обработки (начало координат в крайнем левом элементе.
	// TODO прокомментировать получше направления вращения. возможно, следует уйти от bool и сделать собственный typedef для функции roll_half
	bool	fault = false;

	if(!size || !array) fault = true;
	if((flags & fftFwd) && (flags & fftRev)) fault = true;

	if(fault)
	{
		string	problem_description = ssprintf("FFT_flag_template<%s>, size = %d, flags=%X", typeid(T).name(), size, int(flags));
		ForceDebugBreak();
		throw invalid_argument(problem_description);
	}


	if(flags & fftRollBefore)
	{
		DataArray<T> array_shell;
		array_shell.UseData(array, size);
		array_shell.roll_half(false);
	}

	switch(flags & fftDirectionMask)
	{
		case fftFwd:
			FFT_ptr(array, size, ftForward);
			break;
		case fftRev:
			FFT_ptr(array, size, ftReverse);
			break;
		case fftNone:
			break;
		default:
			fault = true;
	}

	if(flags & fftRollAfter)
	{
		DataArray<T> array_shell;
		array_shell.UseData(array, size);
		array_shell.roll_half(true);
	}
}

//--------------------------------------------------------------

void	FFTf_ptr(complexF32 *array, size_t size, ft_flags flags)
{
	FFTf_template(array, size, flags);
}

void	FFTf_ptr(complexF64 *array, size_t size, ft_flags flags)
{
	FFTf_template(array, size, flags);
}

//--------------------------------------------------------------

}//namespace FFTPrimitives

//--------------------------------------------------------------

XRAD_END
