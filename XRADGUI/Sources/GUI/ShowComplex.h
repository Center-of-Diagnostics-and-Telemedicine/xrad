﻿/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//
//  Created on 10.09.13.
//

#ifndef XRAD__File_ShowComplex_h
#define XRAD__File_ShowComplex_h

#include <XRADBasic/Sources/SampleTypes/ComplexSample.h>
#include "XRADGUI.h"

XRAD_BEGIN

template<class CT, class ST>
void	ShowComplex(const string &title, const ComplexSample<CT,ST>&x)
{
	string	re_im = ssprintf("Complex number:\nParts (re,im) =\t(%g,\t%g)", double(real(x)), double(imag(x)));
	string	r_fi = ssprintf("\nPolar (r,fi) =\t(%g,\t%g*pi)", double(cabs(x)), double(arg(x))/pi());

	ShowString(title, re_im + r_fi);
}

XRAD_END

#endif // XRAD__File_ShowComplex_h
