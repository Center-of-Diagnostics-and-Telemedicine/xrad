/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_least_squares_h
#define XRAD__File_least_squares_h

#include <XRADBasic/MathFunctionTypes.h>
#include <XRADBasic/LinearVectorTypes.h>
#include <XRADBasic/MathMatrixTypes.h>


XRAD_BEGIN

//--------------------------------------------------------------
//
//	класс нужен только для распознавания оптимизированной
//	процедуры, реально ничего в нем не вычисляется
//
struct	x_power_function
	{
	private:
		double operator()(double x, size_t n) const {return pow(x,int(n));}
	};

//--------------------------------------------------------------
//
//	задание абстрактной функции для аппроксимации
//
struct	abstract_LS_basis_function
	{
	virtual double operator()(double x, size_t n) const = 0;
	};

//--------------------------------------------------------------
//
//	аппроксимация на фиксированном интервале
//
struct	fixed_range_LS_basis_function : public abstract_LS_basis_function
	{
	double min_arg;
	double max_arg;

	fixed_range_LS_basis_function(double in_min_arg, double in_max_arg) : max_arg(in_max_arg), min_arg(in_min_arg){}
	};

struct	cosine_LS_function : public fixed_range_LS_basis_function
	{
//	тоже какая-то недоделанная
	cosine_LS_function(double in_min_arg, double in_max_arg) : fixed_range_LS_basis_function(in_min_arg, in_max_arg){}
	virtual double operator()(double x, size_t n) const {return cos(n*(x-min_arg)/(max_arg-min_arg));}
//	virtual double operator()(double x, size_t n) const {return cos(n*x);}
	};

struct	chebyshev_LS_polynom : public fixed_range_LS_basis_function
	{
	// полином чебышева без весовой функции
	public:
		chebyshev_LS_polynom(double in_min_arg=0, double in_max_arg=0) : fixed_range_LS_basis_function(in_min_arg, in_max_arg){}
//		chebyshev_LS_polynom(double in_min_arg, double in_max_arg) : max_arg(in_max_arg), min_arg(in_min_arg){}
		virtual double operator()(double x, size_t n) const {return cos(n*acos(-1 + 2*(x-min_arg)/(max_arg-min_arg)));}
	};

//--------------------------------------------------------------
//
//	набросок, непонятно чего
//
struct	shifted_gaussian_LS : public abstract_LS_basis_function
	{
	const double x0;
	const double dx;
	const double w;
	public:
		shifted_gaussian_LS(double in_x0, double in_dx, double in_w) : x0(in_x0), dx(in_dx), w(in_w){}
		virtual double operator()(double x, size_t n) const {return gauss(x - (x0 + n*dx), w);}
	};

//--------------------------------------------------------------
//
//	полиномы, которые делают без оптимизации в общем порядке
//	аппроксимацию по x^n. этот класс нужен более для отладки
//	и сравнения.
//
struct	direct_LS_polynom : public abstract_LS_basis_function
	{
	virtual double operator()(double x, size_t n) const {return pow(x,int(n));}
	};

//--------------------------------------------------------------
//
//	аппроксимация полиномом вида sum(a_k*x^k)
//
void	DetectLSPolynomUniformGrid(const RealFunctionF64 &samples, RealVectorF64 &coefficients);
void	DetectLSPolynomNonUniformGrid(const RealFunctionF64 &samples, const RealFunctionF64 &grid, RealVectorF64 &coefficients);
void	DetectLSPolynomWeighted(const RealFunctionF64 &samples, const RealFunctionF64 &grid, const RealFunctionF64 &weights, RealVectorF64 &coefficients);

//--------------------------------------------------------------
//
//	аппроксимация произвольно заданным набором функций
//	вида f_n(x). функция задана классом, наследуемым
//	от abstract_LS_basis_function. примеры см. выше
//
void	DetectLSUniversalUniformGrid(const RealFunctionF64 &samples, const abstract_LS_basis_function&f, RealVectorF64 &coefficients);
void	DetectLSUniversalNonUniformGrid(const RealFunctionF64 &samples, const abstract_LS_basis_function&f, const RealFunctionF64 &grid, RealVectorF64 &coefficients);
void	DetectLSUniversalWeighted(const RealFunctionF64 &samples, const abstract_LS_basis_function&f, const RealFunctionF64 &grid, const RealFunctionF64 &weights, RealVectorF64 &coefficients);


XRAD_END

#endif //XRAD__File_least_squares_h
