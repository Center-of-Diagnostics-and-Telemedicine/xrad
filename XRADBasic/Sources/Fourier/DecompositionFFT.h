// file DecompositionFFT.h
//--------------------------------------------------------------
#ifndef XRAD__DecompositionFFT_h
#define XRAD__DecompositionFFT_h
//--------------------------------------------------------------

#include "FourierDefs.h"
#include <XRADBasic/Sources/SampleTypes/ComplexSample.h>
#include <XRADBasic/Sources/Containers/DataArray.h>

XRAD_BEGIN

namespace DecompositionFFT
{

//--------------------------------------------------------------

//! \brief Подготовить таблицу обращения индексов для длины преобразования data_size
DataArray<size_t> MakeRevIndexTable(size_t data_size);

//--------------------------------------------------------------

//! \brief Рассчитать множители для FFT длины data_size
//!
//! Внимание: Размер массива множителей вдвое меньше data_size.
template <class ComplexFunction_t>
ComplexFunction_t ComputePhasors(size_t data_size)
{
	ComplexFunction_t phasors(data_size/2);
	for (size_t i = 0; i < data_size/2; ++i)
	{
		phasors[i] = polar(1., -2.*pi()*(double)i/(double)data_size);
	}
	return phasors;
}

//--------------------------------------------------------------

/*!
	\brief FFT, data_size >= 2

	\param phasors Множители, рассчитанные функцией ComputePhasors().
	\param buffer Буфер размера data_size для промежуточных данных.
	\param rev_index_table Таблица обращения индексов, рассчитанная функцией MakeRevIndexTable().

	Доступны реализации для complex_t = complexF32, complexF64.
*/
template <class complex_t>
void DecompositionFFT(complex_t *data, size_t data_size, ftDirection direction,
		const complex_t *phasors,
		complex_t *buffer,
		const size_t *rev_index_table);

//--------------------------------------------------------------

/*!
	\brief Получить ближайшую длину FFT, большую или равную заданной

	Сейчас длина FFT может быть только степенью 2.

	\param power_of_2 [out] Степень 2, дающая result: result = 2^n.
	\return
		- Допустимая длина FFT, если она укладывается в size_t.
		- Исключение, если при вычислении происходит переполнение size_t.
*/
size_t ceil_fft_length(size_t length, size_t *power_of_2 = nullptr);

/*!
	\brief Инициализировать модуль FFT для максимально допустимой длины length

	Длина length нормализуется до ближайшей подходящей при помощи ceil_fft_length.
*/
void InitializeFFT(size_t length);

//! \brief FFT, size >= 2
void FFT(complexF32 *array, size_t size, ftDirection direction);

//! \brief FFT, size >= 2
void FFT(complexF64 *array, size_t size, ftDirection direction);

//--------------------------------------------------------------

} // namespace DecompositionFFT

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__DecompositionFFT_h
