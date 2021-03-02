/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_special_function_h
#define XRAD__File_special_function_h

#include <XRADBasic/Sources/SampleTypes/ComplexSample.h>

XRAD_BEGIN

//--------------------------------------------------------------

//! \brief Special functions
namespace	SpecialFunctions
{

// Три функции, взятые с сайта http://tdoc.ru
double log_gamma_function(double x);
double gamma_function(double x);
double beta_function(double x,double y);

// Несколько самодельных функций на разные случаи

complexF64 fresnel_integral(double);

//! \brief Функции Бесселя Jn от действительного аргумента
//! для произвольных действительных положительных порядков
double	Jn(double x, double nu);

//! \brief Производная функции Бесселя Jn
double	J1prim1(double x);

//! \brief Модифицированные функции Бесселя In от действительного аргумента
//! для произвольных действительных положительных порядков
double	In(double x, double nu);

//! \brief In(x)/exp(x).
//! При больших аргументах просто вычислять и делить обе функции нельзя из-за переполнения в обеих
double	In_exp(double x, double nu);

//! \brief Логарифм мод. функции Бесселя. При больших значениях аргумента лучше считать его,
//! чем брать log(In()). При малых (<nu*nu/2) все равно: внутри вызовется In и log
double	log_In(double x, double nu);

//! \brief In(x)/x^(n), эта функция прежде всего предназначена для анализа предельных значений при малых x
double	In_xn(double x, double nu);

//! \brief Отношение In/Im. При больших значениях аргумента эту функцию не следует
//! вычислять напрямую через In(x), т.к. экспонента приводит к переполнению
double	In_Im(double x, double nu, double mu);

//! \brief Q-функция Маркума (функция распределения Райса)
double	Qm(double a, double b, int M);
//! \brief Q-функция Маркума (функция распределения Райса)
double	Q1(double a, double b);

double erf( double t);

// Полиномы Чебышева 1-го рода
double	ChebyshevPolynom(double x, double order);
double	WeightedChebyshevPolynom(double x, double order);
complexF64	ChebyshevPolynomC(double x, double order);
complexF64	WeightedChebyshevPolynomC(double x, double order);

//! \brief Вес для ортогонализации
double	ChebyshevPolynomWeight(double x);

} // namespace	SpecialFunctions

using namespace SpecialFunctions;

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
#endif //XRAD__File_special_function_h
