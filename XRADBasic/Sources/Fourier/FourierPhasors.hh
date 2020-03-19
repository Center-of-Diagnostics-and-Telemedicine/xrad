// file FourierPhasors.hh
//--------------------------------------------------------------

#include <XRADBasic/Sources/SampleTypes/ComplexSample.h>

XRAD_BEGIN

namespace FFTPrimitives
{

//--------------------------------------------------------------

template<class T>
Phasors<T> ::Phasors(size_t base)
{
	Initialize(base, max_FFT_size(base));
}

//--------------------------------------------------------------

template<class T>
Phasors<T> ::Phasors(size_t base, size_t max_order)
{
	Initialize(base, max_order);
}

//--------------------------------------------------------------

template<class T>
void	Phasors<T> ::Initialize(size_t base, size_t max_order)
{
	fft_base = base;

	m_max_fft_length = size_t(pow(double(base), double(max_order)));

	phasors.realloc(m_max_fft_length);

	for(size_t i = 0; i < m_max_fft_length; i++)
	{
		double	argument = -two_pi()*i / m_max_fft_length;
		phasors[i] = polar(1., argument);
	}
}

//--------------------------------------------------------------

template<class T>
size_t	Phasors<T>::max_FFT_size(size_t FT_base)
{
	switch(FT_base)
	{
		case 2:
			return 16;//pow(2, 16) = 65536;
//			return 13;//pow(2, 13) = 8192; тест, влияет ли размер этого буфера на быстродействие. оказалось, нет.
			break;
		case 3:
			return 10;//pow(3, 10) = 59049;
			break;
		default:
			throw invalid_argument("max_FFT_size, unknown FFT base");
	}
}

//--------------------------------------------------------------

template<class T>
size_t Phasors<T> ::ceil_fft_length(size_t n) const
{
	// сейчас поддерживаются только длины 1, fft_base^n
	if(n <= 1)
		return 1;

	size_t result = 1;

	while(result < n && result < m_max_fft_length)
	{
		//result <<= 1;
		result *= fft_base;
	}
	return result;
}

//--------------------------------------------------------------

} // namespace FFTPrimitives

XRAD_END

//--------------------------------------------------------------
