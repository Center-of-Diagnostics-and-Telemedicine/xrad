/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 2019/11/25
	\author kulberg
*/
#include <XRADBasic/Sources/Utils/BitmapContainer.h>

XRAD_BEGIN


class scale_and_trim_pixelvalue
{
	const double black, white;
	double	factor;

	double	scale(double x) const { return range((x - black)*factor, 0, 255); }

public:
	scale_and_trim_pixelvalue(double b, double w) : black(b), white(w) { factor = 255. / (white - black); }

	template<class T1, class T2>
	T1	&operator()(T1 &y, const T2 &x) const { return y = scale(x); }

	template<class T1, class T2>
	RGBColorSample<T1>	operator()(RGBColorSample<T1> &y, const RGBColorSample<T2> &x) const { return y = RGBColorSample<T1>(scale(x.red()), scale(x.green()), scale(x.blue())); }

	template<class T1, class T2, class ST>
	T1	operator()(T1 &y, const ComplexSample<T2, ST> &x) const { return y = T1(scale(cabs(x))); }
};

template<class T>
struct DependentBitmapContainer : public BitmapContainerIndexed
{};

template<class T>
struct DependentBitmapContainer<RGBColorSample<T>> : public BitmapContainerRGB
{};


void	SaveImageUtil(wstring filename, const BitmapContainerRGB &bitmap);
void	SaveImageUtil(wstring filename, const BitmapContainerIndexed &bitmap);

template<class ROW_T>
void	SaveRasterImage(const wstring &filename, DataArray2D<ROW_T> &image, double black, double white)
{
	DependentBitmapContainer<typename ROW_T::value_type> buffer;

	buffer.SetSizes(image.vsize(), image.hsize());

	for (size_t i = 0; i < image.vsize(); ++i)
	{
		buffer.row(i).CopyData(image.row(image.vsize() - 1 - i), scale_and_trim_pixelvalue(black, white));
	}

	SaveImageUtil(filename, buffer);
}


XRAD_END
