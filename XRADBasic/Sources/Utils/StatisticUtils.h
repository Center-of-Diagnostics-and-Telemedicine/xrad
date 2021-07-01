/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//	file StatisticUtils.h
//	Created by ACS on 28.05.01
//--------------------------------------------------------------
#ifndef XRAD__File_statistic_utils_h
#define XRAD__File_statistic_utils_h

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


//!	\brief Гауссова случайная величина через преобразование Бокса-Мюллера
//!	\param sigma Среднеквадратичное отклонение получаемой величины
//!	\param mean Математическое ожидание получаемой величины
double RandomGaussianStandard(); // центрированная, СКО=1
double RandomGaussian(double sigma); // центрированная, СКО = sigma
double RandomGaussian(double mean, double sigma);	// мат. ожидание = mean, СКО = sigma


//!	\brief Случайная величина с распределением Лапласа (биэкспоненциальным).
//!	\param sigma Среднеквадратичное отклонение получаемой величины
inline double	RandomLaplacian(double sigma = 1)
{
	const	double factor = sigma/sqrt(2.);
	double	x = RandomUniformF64(0, 2);
	return x<1 ? log(x)*factor : -log(2.-x)*factor;
}

//!	\brief Функция плотности и функция распределения Гаусса
//!	\param sigma Среднеквадратичное отклонение
//!	\param average Математическое ожидание

double	gaussian_pdf(double x, double average, double sigma);
double	gaussian_cdf(double x, double average, double sigma);

//!	\brief Функция плотности распределения Пуассона
//!	\param lambda Параметр распределения
double	poisson_pdf(double x, double lambda);

//!	\brief Функция плотности и функция распределения Рэлея
//!	\param sigma Среднеквадратичное отклонение
double	rayleigh_pdf(double x, double sigma);
double	rayleigh_cdf(double x, double sigma);

//!	\brief Функция плотности и функция распределения Райса
//!	\param nu Смещение распределение
//!	\param sigma Шум распределения
double	rician_pdf(double x, double nu, double sigma);
double	rician_cdf(double x, double nu, double sigma);

double	log_rician_pdf(double x, double nu, double sigma);

XRAD_END

#endif // XRAD__File_StatisticUtils_h
