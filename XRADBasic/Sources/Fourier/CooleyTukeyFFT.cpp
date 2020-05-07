// file CooleyTukeyFFT.cpp
//--------------------------------------------------------------
#include "pre.h"

#include "CooleyTukeyFFT.h"
#include "WinogradShortFFT.h"
#include <omp.h>

XRAD_BEGIN

//--------------------------------------------------------------

namespace CooleyTukeyFFT
{

XRAD_USING

//--------------------------------------------------------------
//

template<class T, class T2>
Transformer<T, T2> ::Transformer(const Phasors<phasor_sample_t> *in_h) : phasors(in_h)
{
	recursion_level = 0;
}


//--------------------------------------------------------------
//

template<class T, class T2>
void	Transformer<T, T2>::intermediate_array_reorder(fft_sample_t *array_ptr, size_t sub_size_1, size_t sub_size_2)
{
	auto	*array_it = array_ptr;
	auto	*buffer_it = reorder_buffer.data();
	auto	*buffer_end = reorder_buffer.data() + sub_size_2*sub_size_1;

	for(; buffer_it < buffer_end; /*buffer_it += sub_size_1,*/ ++array_it)
	{
		auto	*buffer_row_end = buffer_it + sub_size_1;
		auto	*a0 = array_it;

		for(; buffer_it < buffer_row_end; ++buffer_it, a0 += sub_size_2)
		{
			*buffer_it = *a0;
		}
	}
	std::copy(reorder_buffer.data(), reorder_buffer.data() + sub_size_1*sub_size_2, array_ptr);
}

//--------------------------------------------------------------

template<class T, class T2>
void	Transformer<T, T2> ::partial_fourier_transform(
	fft_sample_t *array_ptr,
	size_t sub_size_1,
	size_t sub_size_2,
	bool transposed,
	ftDirection direction)
{
	size_t	vsize, hsize;

	ptrdiff_t vstep;
	ptrdiff_t hstep;

	if(transposed)
	{
		vsize = sub_size_2;
		hsize = sub_size_1;

		vstep = 1;
		hstep = sub_size_2;
	}
	else
	{
		vsize = sub_size_1;
		hsize = sub_size_2;

		vstep = sub_size_2;
		hstep = 1;
	}


	fft_sample_t *a0(array_ptr);
	fft_sample_t *a1(array_ptr + vsize*vstep);
	switch(hsize)
	{
		case 2:
			for(; a0 < a1; a0 += vstep) WinogradFFT::fft_2(a0, hstep, direction);
			break;
		case 3:
			for(; a0 < a1; a0 += vstep) WinogradFFT::fft_3(a0, hstep, direction);
			break;
		case 4:
			for(; a0 < a1; a0 += vstep) WinogradFFT::fft_4(a0, hstep, direction);
			break;
		case 8:
			for(; a0 < a1; a0 += vstep) WinogradFFT::fft_8(a0, hstep, direction);
			break;
		default:
		{
			XRAD_ASSERT_THROW(hstep == 1);
			for(; a0 < a1; a0 += vstep) FFT(a0, hsize, direction);
		}
		break;
	}
}

//--------------------------------------------------------------

template<class T, class T2>
inline void	Transformer<T, T2> ::normalize_fft_result(fft_sample_t *array_ptr, size_t size)
{
	if(recursion_level) return;

	double	factor = sqrt(1./size);
	auto	a0(array_ptr), a1(array_ptr + size);
	for(; a0 < a1; ++a0) *a0 *= factor;
}


template<class T, class T2>
void	Transformer<T, T2> ::FFT(fft_sample_t *array_ptr, size_t size, ftDirection direction)
{
	// частные случае предопределенных
	// коротких преобразований
	switch(size)
	{
		case 2:
			WinogradFFT::fft_2(array_ptr, direction);
			normalize_fft_result(array_ptr, size);
			return;
		case 3:
			WinogradFFT::fft_3(array_ptr, direction);
			normalize_fft_result(array_ptr, size);
			return;
		case 4:
			WinogradFFT::fft_4(array_ptr, direction);
			normalize_fft_result(array_ptr, size);
			return;
		case 8:
			WinogradFFT::fft_8(array_ptr, direction);
			normalize_fft_result(array_ptr, size);
			return;
	}

	if (size > phasors->max_fft_length())
	{
		throw runtime_error(ssprintf(
				"FourierTransformer::FFT(). Array is too long (%zu), maximum allowed size is %zu.",
				EnsureType<size_t>(size),
				EnsureType<size_t>(phasors->max_fft_length())));
	}

	if (!phasors->is_allowed_fft_length(size))
	{
		throw runtime_error(ssprintf("FourierTransformer::FFT(). Invalid FFT length (%zu).",
				EnsureType<size_t>(size)));
	}

	if(!recursion_level)
	{
		if(reorder_buffer.size() < phasors->max_fft_length()) reorder_buffer.realloc(phasors->max_fft_length());
	}

	// выбор вариантов расщепления
	//
	// sub_size_1 -- длина одного из коротких алгоритмов винограда,
	// выбирается по возможности большей по двум причинам:
	// 1. алгоритм винограда во всех случаях быстрее рекурсивного по 2
	// 2. уменьшается глубина рекурсии
	//
	// sub_size_2 -- длина остаточного преобразования

	size_t	sub_size_1 = phasors->next_fourier_division(size);
	size_t	sub_size_2 = size/sub_size_1;

	// первичное кратокое преобразование (разреженное)

	partial_fourier_transform(
		array_ptr,
		sub_size_1, sub_size_2, true,
		direction);

	// фазовращение

	for(size_t j = 1; j < sub_size_1; j++)
	{
		size_t	phasor_step = (phasors->max_fft_length()*j) / size;

		auto *arr0 = array_ptr + j*sub_size_2;
		auto *arr1 = arr0 + sub_size_2;
		auto *p0 = phasors->phasors.data();

		switch(direction)
		{
			case ftForward:
				for(; arr0 < arr1; p0 += phasor_step, ++arr0)
				{
					*arr0 *= *p0;
				}
				break;
			case ftReverse:
				for(; arr0 < arr1; p0 += phasor_step, ++arr0)
				{
					*arr0 %= *p0;
				}
				break;
			default: throw runtime_error("Unknown FFT direction!");
		}
	}

	// вторичное кратокое преобразование (сплошное)

	++recursion_level;

	partial_fourier_transform(array_ptr,
			sub_size_1, sub_size_2, false,
			direction);

	--recursion_level;

	intermediate_array_reorder(array_ptr, sub_size_1, sub_size_2);
	normalize_fft_result(array_ptr, size);
}

//--------------------------------------------------------------

template<class T, class T2>
TransformerSet<T, T2>::TransformerSet(const Phasors<T2>	*in_phasors) : phasors(in_phasors)
{
	size_t	n = omp_get_num_procs();
	for(size_t i = 0; i < n; ++i)
	{
		transformers.push_back(make_unique<transformer_t>(phasors));
	}
}


template<class T, class T2>
void	TransformerSet<T, T2>::FFT(T *data, size_t size, ftDirection direction)
{
	size_t	n(0);
	while(!transformers[n]->try_lock())
	{
		(++n)%=transformers.size();
	}
	transformers[n]->FFT(data, size, direction);
	transformers[n]->unlock();
}

//--------------------------------------------------------------

//	преобразование по степеням 3 также возможно, для этого нужно использовать следующий объект:
//Phasors<phasor_value_type> fft_phasors(3);
Phasors<phasor_value_type> fft_phasors(2);

TransformerSet<complexF32, phasor_value_type>	FTCT_F32(&fft_phasors);
TransformerSet<complexF64, phasor_value_type>	FTCT_F64(&fft_phasors);

//--------------------------------------------------------------

template class Transformer<complexF32, phasor_value_type>;
template class Transformer<complexF64, phasor_value_type>;

template class TransformerSet<complexF32, phasor_value_type>;
template class TransformerSet<complexF64, phasor_value_type>;

//--------------------------------------------------------------

} // namespace CooleyTukeyFFT

//--------------------------------------------------------------

XRAD_END
