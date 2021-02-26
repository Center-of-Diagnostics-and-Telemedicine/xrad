/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#include "pre.h"
#include "HLSColorSample.h"
#include "LABColorSample.h"

XRAD_BEGIN

namespace{
const	double	sqrt32 = sqrt(3./2);
}



HLSColorSample::HLSColorSample(const ColorSampleF64 &s)
{
	FromRGB(s.red(), s.green(), s.blue());
}


HLSColorSample::HLSColorSample(const ColorPixel &s)
{
	FromRGB(s.red(), s.green(), s.blue());
}

HLSColorSample :: operator ColorSampleF64() const{
	float	r, g, b;
	ToRGB(r, g, b);
	return ColorSampleF64(r, g, b);
}




/*---------------------

angle	color

0	cyan
PI/3	blue
2*PI/3	magenta
PI	red
-2*PI/3	yellow
-PI/3	green

---------------------*/


#ifndef MUNSELL_HLS

void	HLSColorSample::ToRGB(float &r, float &g, float &b) const{
	const	double	p3 = pi()/3;

	float	h;
	if(H >= two_pi())
	{
		int	n = integral_part(H/two_pi());
		h = H - n*two_pi();
	}
	else if(H < 0)
	{
		int	n = integral_part(-H/two_pi());
		h = H + (n+1)*two_pi();
	}
	else h = H;

	float	min_color =  L*(1.-S);
//	float	max_color = L;
	float	delta_color = L*S;
//	float	dc2 = delta_color/2;

	int	h_segment = h/p3;
	float	dh = h/p3 - h_segment;
	float	increase = delta_color*dh;
	float	decrease = delta_color*(1-dh);

	switch(h_segment)
	{
		case 0:
		{
   // r is min
   // b is max;
   // g decreases
			r = 0;
			b = delta_color;
			g = decrease;
		}
		break;
		case 1:
		{
   // b is max
   // g is min
   // r increases
			g = 0;
			b = delta_color;
			r = increase;
		}
		break;
	// green is min
		case 2:
		{
   // r is max
   // g is min
   // b decreases
			g = 0;
			r = delta_color;
			b = decrease;
		}
		break;

		case 3:
		{
   // b is min
   // g increases
   // r is max
			b = 0;
			r = delta_color;
			g = increase;
		}
		break;

	// blue is min
		case 4:
		{
   // g is max
   // r decreases
   // b is min
			b = 0;
			g = delta_color;
			r = decrease;
		}
		break;
		case 5:
		{
   // g is max
   // b increases
   // r is min
			r = 0;
			g = delta_color;
			b = increase;
		}
		break;
		default: r=g=b=0;
	}

	r = range(r + min_color, 0, 255);
	g = range(g + min_color, 0, 255);
	b = range(b + min_color, 0, 255);
}

void	HLSColorSample::FromRGB(float r, float g, float b)
{
	const	double	p3 = pi()/3;
	L = max(max(r, g), b);
	if(L) S = (L - min(min(r, g), b))/L;
	else
	{
		S = 0;
		H = 0;
		return;
	}

	if(r < g && r < b){ // min color is red
		float g0 = g-r;
		float b0 = b-r;
		H = p3*(b0-g0)/max(g0, b0);
	}

	else if(g < b){ // min color is green
		float r0 = r-g;
		float b0 = b-g;
		H = 2*p3 + p3*(r0-b0)/max(r0, b0);
	}
	else{ // min color is blue
		float r0 = r-b;
		float g0 = g-b;
		H = 4*p3 + p3*(g0-r0)/max(r0, g0);
	}
}


#else
//Munsell model

void	HLSColorSample::FromRGB(float r, float g, float b)
{
	S = sqrt(r*r + g*g + b*b - r*g - r*b - g*b);
	L = (r + g + b)/3;
	H = asin(sqrt32*(g - r)/S);
}


void	HLSColorSample::ToRGB(float &r, float &g, float &b) const{
	double	r = range(L - S*cos(H)/3 - S*sin(H)/sqrt_3(), 0, 255);
	double	g = range(L + 2*S*cos(H)/3, 0, 255);
	double	b = range(L - S*cos(H)/3 + S*sin(H)/sqrt_3(), 0, 255);
}


#endif //MUNSELL_HLS



XRAD_END
