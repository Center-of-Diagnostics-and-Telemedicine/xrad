// file Fourier.h
// Created by KNS, modified by ACS
// FFT routines, machine-independent interface
//--------------------------------------------------------------
#ifndef __Fourier_h
#define __Fourier_h
//--------------------------------------------------------------

#include "FourierDefs.h"
#include <XRADBasic/Sources/SampleTypes/ComplexSample.h>

//--------------------------------------------------------------
/*
	под прямым преобразованием фурье понимается разложение по базису exp(+iwt).
	таким образом, прямое преобразование фурье от функции:
	{
		for(int i=0; i<size; ++i)
			func[i] = polar(1., 0.5*PI*i);
	}
	даст 1 в области положительных частот:
	{
		FFT_ptr(func, size, ftForward);
		if(!func[size/4])
			Error("Something is wrong in FFT!");
	}

	нормировка на sqrt(size):
	{
		printf("func[size/4].re=%f, must be equal to %f\n", func[size/4].re, sqrt(size));
	}
	При такой нормировке 4-кратное применение прямого (обратного) FFT даст исходные данные
	с точностью до погрешностей вычислений.
*/
//--------------------------------------------------------------

XRAD_BEGIN

//--------------------------------------------------------------

//! \brief Ближайшая длина FFT, >= n
//!
//! Кидает исключение, если подобрать такую длину невозможно (слишком большая длина).
size_t ceil_fft_length(size_t n);

//--------------------------------------------------------------

/*!
	\brief Инициализировать структуры данных FFT для длин <= maximum_allowed_fft_length

	Эту функцию следует вызывать только в том случае, если понадобится
	максимальная длина преобразования, превышающая 64k отсчетов.
*/
void InitFourierTransform(size_t maximum_allowed_fft_length);



//--------------------------------------------------------------



namespace FFTPrimitives
{

//--------------------------------------------------------------
//
// быстрое фурье-преобразование длины 2^n
//
void FFT_ptr(complexF32 *array, size_t size, ftDirection direction);
void FFT_ptr(complexF64 *array, size_t size, ftDirection direction);

//
// медленное фурье-преобразование произвольной длины.
// вызывать только для однократного нечастого использования, например,
// при анализе спектра единичной функции
//
void FT_ptr(complexF32 *array, size_t size, ftDirection direction);
void FT_ptr(complexF64 *array, size_t size, ftDirection direction);

//--------------------------------------------------------------

void FFTf_ptr(complexF32 *array, size_t size, ft_flags flags);
void FFTf_ptr(complexF64 *array, size_t size, ft_flags flags);

//--------------------------------------------------------------

}//namespace FFTPrimitives

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------

#endif
