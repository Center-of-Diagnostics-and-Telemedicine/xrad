/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file FourierPhasors.h
//--------------------------------------------------------------
#ifndef XRAD__FourierPhasors_h
#define XRAD__FourierPhasors_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>
#include <XRADBasic/Sources/Containers/DataArray.h>

XRAD_BEGIN

namespace FFTPrimitives
{

//--------------------------------------------------------------

template<class T>
class	Phasors
{
	typedef T phasor_sample_t;

	size_t	fft_base;// основание преобразования фурье
	size_t	m_max_fft_length;

public:

	DataArray<phasor_sample_t> phasors;

	Phasors(size_t base);
	Phasors(size_t base, size_t max_order);

	void	Initialize(size_t base, size_t max_order);
	size_t	max_FFT_size(size_t FT_base);
	size_t	ceil_fft_length(size_t) const;
	size_t	max_fft_length() const { return m_max_fft_length; };
	bool	is_allowed_fft_length(size_t size) const { return !(size % fft_base); }

	size_t	next_fourier_division(size_t size) const
	{

		if(!(size % 8))
		{
			return 8;
		}
		else if(!(size % 4))
		{
			return 4;
		}
		else if(!(size % fft_base))
		{
			return fft_base;
		}
		else
		{
			throw runtime_error(ssprintf("Phasors::next_fourier_division(). Invalid FFT length (%lu)!", size));
		}
	}
};

//--------------------------------------------------------------

} // namespace FFTPrimitives

XRAD_END

#include "FourierPhasors.hh"

//--------------------------------------------------------------
#endif // XRAD__FourierPhasors_h
