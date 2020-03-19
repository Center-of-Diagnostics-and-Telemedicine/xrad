// file CooleyTukeyFFT.h
//--------------------------------------------------------------
#ifndef XRAD__CooleyTukeyFFT_h
#define XRAD__CooleyTukeyFFT_h
/*!
	\file
	\brief Алгоритм БПФ Кули-Тьюки (Cooley-Tukey FFT algorithm)
*/
//--------------------------------------------------------------

#include "FourierPhasors.h"
#include "FourierDefs.h"
#include <XRADBasic/Sources/Containers/DataArray.h>
#include <XRADBasic/Sources/SampleTypes/ComplexSample.h>
#include <memory>
#include <vector>
#include <mutex>

XRAD_BEGIN

namespace CooleyTukeyFFT
{

//--------------------------------------------------------------

template <class T>
using Phasors = FFTPrimitives::Phasors<T>;

//--------------------------------------------------------------

//! \brief Fourier transform Cooley-Tukey class
template<class T, class T2>
class	Transformer : public std::mutex
{
	typedef T fft_sample_t;
	typedef T2 phasor_sample_t;

private:

	size_t	recursion_level;

	// phasors
	const Phasors<phasor_sample_t> *phasors;
	DataArray<fft_sample_t> reorder_buffer;

	void	intermediate_array_reorder(fft_sample_t *array_ptr, size_t sub_size_1, size_t sub_size_2);
	void	partial_fourier_transform(fft_sample_t *array_ptr, size_t vsize, size_t hsize, bool transposed, ftDirection direction);
	inline void	normalize_fft_result(fft_sample_t *array_ptr, size_t size);

public:

	Transformer(const Phasors<phasor_sample_t> *in_h);
	Transformer(const Transformer &) = delete;
	Transformer &operator=(const Transformer &) = delete;

	//! \brief FFT, size >= 2
	void	FFT(fft_sample_t *array_ptr, size_t size, ftDirection direction);
};

//--------------------------------------------------------------

template<class T, class T2>
class TransformerSet
{
	typedef Transformer<T, T2> transformer_t;
	std::vector<unique_ptr<transformer_t>> transformers;
	const Phasors<T2>	*phasors;
public:
	TransformerSet(const Phasors<T2>	*in_phasors);
	TransformerSet(const TransformerSet &) = delete;
	TransformerSet &operator=(const TransformerSet &) = delete;

	//! \brief FFT, size >= 2
	void	FFT(T *data, size_t size, ftDirection direction);
};

//--------------------------------------------------------------

typedef	complexF64 phasor_value_type;

extern Phasors<phasor_value_type> fft_phasors;

extern TransformerSet<complexF32, phasor_value_type>	FTCT_F32;
extern TransformerSet<complexF64, phasor_value_type>	FTCT_F64;

//--------------------------------------------------------------

}// namespace CooleyTukeyFFT

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__CooleyTukeyFFT_h
