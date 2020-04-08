//
//  ShowComplex.h
//  SimpleTest_QT_XC
//
//  Created by macintosh on 10.09.13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef __ShowComplex_h
#define __ShowComplex_h

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

#endif
