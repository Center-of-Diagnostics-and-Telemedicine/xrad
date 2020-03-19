#include "pre.h"
#include "LABColorSample.h"

XRAD_BEGIN

//--------------------------------------------------------------
//
//	XYZ <-> RGB
//
//--------------------------------------------------------------

XYZColorSample :: operator ColorSampleF64() const
{
	double	var_X = X / 100;        //Where X = 0 Ц  95.047
	double	var_Y = Y / 100;        //Where Y = 0 Ц 100.000
	double	var_Z = Z / 100;        //Where Z = 0 Ц 108.883

	double	var_R = var_X *  3.2406 + var_Y * -1.5372 + var_Z * -0.4986;
	double	var_G = var_X * -0.9689 + var_Y *  1.8758 + var_Z *  0.0415;
	double	var_B = var_X *  0.0557 + var_Y * -0.2040 + var_Z *  1.0570;

	if(var_R > 0.0031308) var_R = 1.055 * pow(var_R, (1/2.4)) - 0.055;
	else                     var_R = 12.92 * var_R;
	if(var_G > 0.0031308) var_G = 1.055 * pow(var_G, (1/2.4)) - 0.055;
	else                     var_G = 12.92 * var_G;
	if(var_B > 0.0031308) var_B = 1.055 * pow(var_B, (1/2.4)) - 0.055;
	else                     var_B = 12.92 * var_B;

	return ColorSampleF64(range(var_R*255, 0, 255), range(var_G*255, 0, 255), range(var_B*255, 0, 255));
}


XYZColorSample::XYZColorSample(const ColorSampleF64 &s)
{
	double	var_R = (s.red() / 255);        //Where R = 0 Ц 255
	double	var_G = (s.green() / 255);        //Where G = 0 Ц 255
	double	var_B = (s.blue() / 255);        //Where B = 0 Ц 255

	if(var_R > 0.04045) var_R = pow((var_R + 0.055) / 1.055, 2.4);
	else                   var_R = var_R / 12.92;
	if(var_G > 0.04045) var_G = pow((var_G + 0.055) / 1.055, 2.4);
	else                   var_G = var_G / 12.92;
	if(var_B > 0.04045) var_B = pow((var_B + 0.055) / 1.055, 2.4);
	else                   var_B = var_B / 12.92;

	var_R *= 100;
	var_G *= 100;
	var_B *= 100;

	//Observer. = 2Ў, Illuminant = D65
	X = var_R * 0.4124 + var_G * 0.3576 + var_B * 0.1805;
	Y = var_R * 0.2126 + var_G * 0.7152 + var_B * 0.0722;
	Z = var_R * 0.0193 + var_G * 0.1192 + var_B * 0.9505;
}


//--------------------------------------------------------------
//
//	XYZ <-> LAB
//
//--------------------------------------------------------------

#ifdef CIE_LAB

#define	NO_WHITEPOINT_CORRECTION 0
namespace	color_conversion_constants
{

#if NO_WHITEPOINT_CORRECTION
const	double ref_X = 100.000;
const	double ref_Y = 100.000;
const	double ref_Z = 100.000;
#else
const	double ref_X =  95.047;//  Observer= 2Ў, Illuminant= D65
const	double ref_Y = 100.000;//
const	double ref_Z = 108.883;//
#endif
}
using	namespace	color_conversion_constants;

#endif //CIE_LAB


LABColorSample::LABColorSample(const XYZColorSample &s)
{
#ifdef CIE_LAB
	using	namespace	color_conversion_constants;
	double	var_X = s.X / ref_X;
	double	var_Y = s.Y / ref_Y;
	double	var_Z = s.Z / ref_Z;

	if(var_X > 0.008856) var_X = pow(var_X, 1./3.);
	else                    var_X = (7.787 * var_X) + (16./116.);
	if(var_Y > 0.008856) var_Y = pow(var_Y, 1./3.);
	else                    var_Y = (7.787 * var_Y) + (16./116.);
	if(var_Z > 0.008856) var_Z = pow(var_Z, 1./3.);
	else                    var_Z = (7.787 * var_Z) + (16./116.);

	L = (116 * var_Y) - 16;
	a = 500 * (var_X - var_Y);
	b = 200 * (var_Y - var_Z);
#else	//Hunter Lab
	L = 10 * sqrt(s.Y);
	a = 17.5 * (((1.02*s.X) - s.Y) / sqrt(s.Y));
	b = 7 * ((s.Y - (0.847 * s.Z)) / sqrt(s.Y));

#endif //CIE_LAB
}



XYZColorSample::XYZColorSample(const LABColorSample &s){
#ifdef CIE_LAB
	double	var_Y = (s.L + 16) / 116;
	double	var_X = s.a / 500 + var_Y;
	double	var_Z = var_Y - s.b / 200;

	if(pow(var_Y, 3) > 0.008856) var_Y = pow(var_Y, 3);
	else                      var_Y = (var_Y - 16./116.) / 7.787;
	if(pow(var_X, 3) > 0.008856) var_X = pow(var_X, 3);
	else                      var_X = (var_X - 16./116.) / 7.787;
	if(pow(var_Z, 3) > 0.008856) var_Z = pow(var_Z, 3);
	else                      var_Z = (var_Z - 16./116.) / 7.787;

	X = ref_X * var_X;
	Y = ref_Y * var_Y;
	Z = ref_Z * var_Z;
#else	//Hunter lab

	double	var_Y = s.L / 10;
	double	var_X = (s.a / 17.5) * (s.L / 10);
	double	var_Z = (s.b / 7) * (s.L / 10);

	Y = square(var_Y);
	X = (var_X + Y) / 1.02;
	Z = -(var_Z - Y) / 0.847;


#endif //CIE_LAB
}

//--------------------------------------------------------------
//
//	rgb <-> LAB
//
//--------------------------------------------------------------


LABColorSample :: operator ColorSampleF64() const{
	XYZColorSample	s(*this);
	return s.operator ColorSampleF64();
}

LABColorSample::LABColorSample(const ColorSampleF64 &s)
{
	*this = LABColorSample(XYZColorSample(s));
}

LABColorSample &LABColorSample :: operator=(const ColorSampleF64 &s)
{
	return *this = LABColorSample(XYZColorSample(s));
}



XRAD_END
