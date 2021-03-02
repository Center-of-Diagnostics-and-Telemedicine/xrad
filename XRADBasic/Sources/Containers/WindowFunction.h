/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//	file WindowFunction.h
//--------------------------------------------------------------
#ifndef XRAD__File_window_function_h
#define XRAD__File_window_function_h
/*!
	\file
	\brief Вычисление типовых оконных функций,
	а также применение их к данным в контейнере MathFunction.
	Во всех случаях используется функтор от
	двух целых: номер индекса и размер массива
*/
//--------------------------------------------------------------

#include "MathFunction.h"
#include <memory>

XRAD_BEGIN

//--------------------------------------------------------------

//! \note 2017-06-27 Была неточность: некоторые окна на концах отрезка обращались в 0
//! из-за неверной формулы перевода "отсчет->double".
struct	window_function
{

	//! \brief Вычисление веса окна от double на отрезке (0,1)
	virtual double	operator()(double x) const = 0;

	// Вычисление веса от дискретного аргумента для окна длины N.
	// Возможны три варианта, о выборе подумать.
	//virtual double	operator()(size_t n, size_t N) const { return operator()(double(n)/double(N-1)); }
	// Вариант с https://en.wikipedia.org/wiki/Window_function,
	// приводит к обнулению концов отрезка на многих окнах.

	//! Здесь нули подразумеваются за пределами отрезка, кажется, должно быть более правильно
	virtual double	operator()(size_t n, size_t N) const { return operator()(double(n+1)/double(N+1)); }

	// Промежуточный вариант между 1 и 2. Для приподнятого косинуса сохраняет разбиение единицы
	//virtual double	operator()(size_t n, size_t N) const { return operator()((double(n)+0.5)/double(N)); }

	virtual ~window_function(){}
};

//--------------------------------------------------------------

//! \brief Считается по формуле exp(-x*x/(2*w);
//! где x по всему массиву пробегает значения [-1;1]
class	gauss_window : public window_function
{
		const double width;
	public:
		gauss_window(double w=sqrt(0.5)) : width(w){}
		double	operator()(double) const;
		using window_function::operator();
};


// \brief Гибрид плоского и косинусного окна
class	tukey_window : public window_function
{
		const double flatness;
	public:
		tukey_window(double f= 0.5) : flatness(f){}
		double	operator()(double) const;
		using window_function::operator();
};



//--------------------------------------------------------------
//
//	High- and moderate-resolution windows
//
//--------------------------------------------------------------



struct	constant_window : public window_function
{
	double	operator()(double x) const{ return in_range(x, 0, 1) ? 1:0; }
	using window_function::operator();
};

/*!
	\brief Треугольное окно

	Can be seen as the convolution of two half-sized rectangular windows,
	giving it a main lobe width of twice the width of a regular rectangular window.
	The nearest lobe is -26 dB down from the main lobe.
*/
struct	triangular_window : public window_function
{
	double	operator()(double) const;
	using window_function::operator();
};

typedef	triangular_window bartlett_window;

/*!
	\brief Класс косинусных окон

	Объединяет в один класс cos2_window, hamming_window.
	Видимо, следует подобным образом
	соединить окна Наттола-Блэкмена-Харриса, идущие ниже.
*/
struct	raised_cosine_window : public window_function
{
	protected:
		const double	a0, a1;
		raised_cosine_window(double in_a0, double in_a1) :a0(in_a0), a1(in_a1){};
	public:
		double	operator()(double) const;
		using window_function::operator();
};

/*!
	\brief Окно cos^2 (Hann window)

	The ends of the cosine just touch zero, so the side-lobes roll off at about 18 dB per octave.
	The Hann and Hamming windows, both of which are in the family known as "raised cosine" windows,
	are respectively named after Julius von Hann and Richard Hamming.
	The term "Hanning window" is sometimes used to refer to the Hann window.
*/
struct	cos2_window : public raised_cosine_window
{
	cos2_window() : raised_cosine_window(0.5, 0.5){}
};

typedef	cos2_window hann_window;

/*!
	\brief Окно Хэмминга (уточненные коэффициенты)

	The "raised cosine" with these particular coefficients
	was proposed by Richard W. Hamming. The window is optimized
	to minimize the maximum (nearest) side lobe, giving it a height
	of about one-fifth that of the Hann window,
	a raised cosine with simpler coefficients.

	Здесь используется уточненная формула. Классическая формула
	предполагает значения коэффициентов 0.54, 0.46.
*/
struct	hamming_window : public raised_cosine_window
{
	hamming_window() : raised_cosine_window(0.53836, 0.46164){}
};



//--------------------------------------------------------------
//
//	Low-resolution (high-dynamic-range) windows
//
//--------------------------------------------------------------



struct	nuttall_window : public window_function
{
//	double	operator()(size_t n, size_t N) const;
	virtual double	operator()(double x) const;
	using window_function::operator();
};

/*!
	\brief Окно Блэкмана-Харриса

	A generalization of the Hamming family, produced by adding more shifted sinc functions,
	meant to minimize side-lobe levels.
*/
struct	blackman_harris_window : public window_function
{
	double	operator()(double) const;
	using window_function::operator();
};

//! \brief All side lobes at level about 100 dB (almost non-decreasing)
struct	blackman_nuttall_window : public window_function
{
	double	operator()(double) const;
	using window_function::operator();
};

struct	flat_top_window : public window_function
{
	double	operator()(double) const;
	using window_function::operator();
};



//--------------------------------------------------------------
//
//	Вычисление окна для конкретного массива.
//
//	Возможно несимметричное окно, состоящее	из двух половинок стандартных окон.
//
//	Наиболее очевидный случай использования несимметричного окна - оконное БПФ:
//	на краю нужна константа, далее косинусное окно с разбиением единицы.
//--------------------------------------------------------------



template <XRAD__MathFunction_template> void ApplyWindowFunction(MathFunction<XRAD__MathFunction_template_args> &original, const window_function &win_left, const window_function &win_right, size_t s0 = 0, size_t s1 = 0);
template <XRAD__MathFunction_template> void ApplyWindowFunction(MathFunction<XRAD__MathFunction_template_args> &original, const window_function &win, size_t s0 = 0, size_t s1 = 0);

template <XRAD__MathFunction_template> void CreateWindowFunction(MathFunction<XRAD__MathFunction_template_args> &original, const window_function &win_left, const window_function &win_right, size_t s0 = 0, size_t s1 = 0);
template <XRAD__MathFunction_template> void CreateWindowFunction(MathFunction<XRAD__MathFunction_template_args> &original, const window_function &win, size_t s0 = 0, size_t s1 = 0);



//--------------------------------------------------------------
//
//	Задание окна через enum
//
//--------------------------------------------------------------



enum	window_function_e
{
	e_constant_window = 0,
	e_triangular_window,
	e_bartlett_window = e_triangular_window,
	e_cos2_window,
	e_hamming_window,
	e_nuttall_window,
	e_blackman_harris_window,
	e_blackman_nuttall_window,
	e_flat_top_window,

	n_window_functions
};

string	GetWindowFunctionName(window_function_e);
shared_ptr<window_function> GetWindowFunctionByEnum(window_function_e);

template <XRAD__MathFunction_template> void ApplyWindowFunction(MathFunction<XRAD__MathFunction_template_args> &original, window_function_e win_left, window_function_e win_right, size_t s0 = 0, size_t s1 = 0);
template <XRAD__MathFunction_template> void ApplyWindowFunction(MathFunction<XRAD__MathFunction_template_args> &original, window_function_e win, size_t s0 = 0, size_t s1 = 0);

template <XRAD__MathFunction_template> void CreateWindowFunction(MathFunction<XRAD__MathFunction_template_args> &original, window_function_e win_left, window_function_e win_right, size_t s0 = 0, size_t s1 = 0);
template <XRAD__MathFunction_template> void CreateWindowFunction(MathFunction<XRAD__MathFunction_template_args> &original, window_function_e win, size_t s0 = 0, size_t s1 = 0);



//--------------------------------------------------------------

XRAD_END

#include "WindowFunction.hh"

//--------------------------------------------------------------
#endif //XRAD__File_window_function_h
