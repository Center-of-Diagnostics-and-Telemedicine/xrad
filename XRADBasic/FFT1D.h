/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef FFT1D_h__
#define FFT1D_h__

/*!
* \file FFT1D.h
* \date 2017/05/26 16:08
*
* \author kulberg
*
* \brief Преобразование Фурье от контейнеров DataArray<complex>
*
* TODO: long description
*
* \note
*/

#include "Sources/Containers/DataArray.h"
#include "Sources/Fourier/FourierBasic.h"

XRAD_BEGIN


template<class T>
void	FFT(DataArray<T> &f, ftDirection direction)
{
	if(f.step()==1)
	{
		FFTPrimitives::FFT_ptr(f.data(), f.size(), direction);
	}
	else
	{
		DataArray<T>	buffer(f);
		FFTPrimitives::FFT_ptr(buffer.data(), f.size(), direction);
		f.CopyData(buffer);
	}
}


template<class T>
void FFTf(DataArray<T> &f, ft_flags fftFlags)
{
	if(f.step()==1)
	{
		FFTPrimitives::FFTf_ptr(f.data(), f.size(), fftFlags);
	}
	else
	{
		DataArray<T> buffer(f);
		FFTPrimitives::FFTf_ptr(buffer.data(), buffer.size(), fftFlags);
		f.CopyData(buffer);
	}
}


template<class T>
void	FT(DataArray<T> &f, ftDirection direction)
{
	if(f.step()==1)
	{
		FFTPrimitives::FT_ptr(f.data(), f.size(), direction);
	}
	else
	{
		DataArray<T>	buffer(f);
		FFTPrimitives::FT_ptr(buffer.data(), buffer.size(), direction);
		f.CopyData(buffer);
	}
}


XRAD_END

#endif // FFT1D_h__
