﻿/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_lab_color_sample_h
#define XRAD__File_lab_color_sample_h

#include "ColorSample.h"

XRAD_BEGIN

class	LABColorSample;

class	XYZColorSample
{
	public:

	double	X, Y, Z;
	XYZColorSample(){};
	XYZColorSample(const ColorSampleF64 &);
	XYZColorSample(const double &v){(*this) = ColorSampleF64(v);}
	XYZColorSample(const LABColorSample &);

	operator ColorSampleF64() const;
};


class	LABColorSample
{
//	CIE-L*ab
public:
	double L, a, b;

	LABColorSample(){}
	LABColorSample(const ColorSampleF64 &);
	LABColorSample(const double &v){L=v;a=b=0;}
	LABColorSample(const XYZColorSample &);

	operator ColorSampleF64() const;
	LABColorSample &operator=(const ColorSampleF64 &);
};

inline LABColorSample zero_value(const LABColorSample &){return LABColorSample (0);}
inline XYZColorSample zero_value(const XYZColorSample &){return XYZColorSample (0);}


XRAD_END

#endif //XRAD__File_lab_color_sample_h
