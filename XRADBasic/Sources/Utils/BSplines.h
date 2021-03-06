﻿/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_BSplines_h
#define XRAD__File_BSplines_h

//	created:	2014/11/18
//	created:	18.11.2014   13:08
//	author:		kns

#include <XRADBasic/Core.h>

XRAD_BEGIN

double	b_spline(int n, double x);
double	b_spline_integral(int n, double x);

XRAD_END


#endif //XRAD__File_BSplines_h