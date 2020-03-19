#ifndef __math_constants_h
#define __math_constants_h

#include "Config.h"
#include "BasicMacros.h"
#include <cmath>

XRAD_BEGIN



//--------------------------------------------------------------
//
//	базовые математические константы
//
//--------------------------------------------------------------



inline double pi()
{
	return 3.1415926535897932384626433832795028841971;
}

inline double two_pi()// 2*pi, слишком часто приходится выписывать
{
	return 2.*pi();
}

inline double sqrt_two_pi()// sqrt(2*pi), слишком часто приходится выписывать
{
	return sqrt(2.*pi());
}

inline double pi_2()
{
	return pi()/2;
}

inline double sqrt_pi()
{
	return sqrt(pi());
}

inline double sqrt_2()
{
	return 1.41421356237309504880168872420969808;
}

inline double sqrt_3()
{
	return 1.73205080756887729352744634150587237;
}

inline double ln_2()
{
	return 0.693147180559945;
}

inline double ln_10()
{
	return 2.30258509299405;
}

inline double euler_e()
{
	return 2.7182818284590452353602874713526624977572;
}

inline double	euler_gamma()
{
	return 0.577215664901532860606512090082402431042159335;
}

inline double degrees_per_radian(){return 180./pi();}
inline double radians_per_degree(){return pi()/180.;}



//--------------------------------------------------------------
//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
#endif //__math_constants_h
