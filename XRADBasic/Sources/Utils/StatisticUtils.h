//	file StatisticUtils.h
//	Created by ACS on 28.05.01
//--------------------------------------------------------------
#ifndef __statistic_utils_h
#define __statistic_utils_h

#include <XRADBasic/MathFunctionTypes.h>
#include "DistributionContainer.h"

XRAD_BEGIN


//--------------------------------------------------------------
//
//	"орлянка"
//
inline double	coin_flip()
	{
	const int	bound = RAND_MAX/2;
	return rand() < bound ? -1. : 1.;
	}


//--------------------------------------------------------------
//
//	простейший генератор равномерно распределеных случайных
//	чисел в заданном диапазоне [min;max) (не включая max, требуется min<=max).
//
//	для плавающей запятой функция без аргументов возвращает в диапазоне [0,1)
double RandomUniformF64();
double RandomUniformF64(double min, double max);
//
//	генератор целых равномерно распределенных чисел в диапазоне
//	0...0xFFFFu и 0xFFFFFFFFu
//
uint16_t	RandomUniformUI16();
uint32_t	RandomUniformUI32();
//
int16_t	RandomUniformI16();
int32_t	RandomUniformI32();
//
//--------------------------------------------------------------


//--------------------------------------------------------------
//
// гауссова случайная величина через преобразование бокса-мюллера
//

double RandomGaussianStandard(); // центрированная, СКО =1
double RandomGaussian(double sigma); // центрированная, СКО = sigma
double RandomGaussian(double mean, double sigma);	// мат. ожидание = mean, СКО = sigma

//
//--------------------------------------------------------------




//--------------------------------------------------------------
//
//	прочее
//
//--------------------------------------------------------------


// функция плотности и функция распределения гаусса
double	gaussian_pdf(double x, double average, double sigma);
double	gaussian_cdf(double x, double average, double sigma);

// функция плотности пуассона
double	poisson_pdf(double x, double lambda);

// функция плотности и функция распределения релея
double	rayleigh_pdf(double x, double sigma);
double	rayleigh_cdf(double x, double sigma);

// функция плотности и функция распределения райса
double	rician_pdf(double x, double nu, double sigma);
double	rician_cdf(double x, double nu, double sigma);

double	log_rician_pdf(double x, double nu, double sigma);

XRAD_END

#endif // __StatisticUtils_h
