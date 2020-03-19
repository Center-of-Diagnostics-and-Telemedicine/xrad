#include "SolveLinearSystem.h"

XRAD_BEGIN

namespace least_squares_classes
{


template<class ARR_T>
void	LSDetector<ARR_T>::PrepareLinearSystem(size_t order, const abstract_LS_basis_function &f)
	{
	// составление системы линейных уравнений "в лоб",
	// без какой-либо оптимизации.
	// набор аппроксимирующих функций произвольный
	matrix.realloc(order, order+1, 0);
	size_t	s = samples.size();

	for(size_t i = 0; i < s; ++i)
		{
		double	x = grid(i);
		double	y = sample(i);
		double	w = weight(i);
		for(size_t l = 0; l < order; ++l)
			{
			for(size_t k = 0; k < order; ++k)
				{
				matrix.at(k,l) += w*f(x,l)*f(x, k);
				}
			matrix.at(l,order) += w*f(x, l)*y;//правая часть
			}
		}
	}


template<class ARR_T>
void	LSDetector<ARR_T>::PrepareLinearSystem(size_t order, const x_power_function&)
	{
	// составление системы линейных уравнений для аппроксимации
	// функциями вида f_n(x) = x^n. представляет собой оптимизированную
	// под конкретный случай версию предыдущей функции
	matrix.realloc(order, order+1, 0);
	size_t	s = samples.size();

	for(size_t i = 0; i < s; ++i)
		{
		ARR_T	increment(order+1);
		double	x = grid(i);
		double	y = sample(i);
		double	w = weight(i);

		increment[0] = w;
		for(size_t k = 1; k < order; ++k) increment[k] = increment[k-1]*x;
		increment[order] = w*y; // к правой части

		for(size_t l = 0; l < order; ++l)
			{
			matrix.row(l) += increment;
			increment*=x;
			}
		}
	}


template<class ARR_T>
template<class function_t, XRAD__LinearVector_template>
void	LSDetector<ARR_T>::DetectLSUniversal(LinearVector<XRAD__LinearVector_template_args> &coefficients, const function_t &f)
	{
	PrepareLinearSystem(coefficients.size(), f);
// 	SolveLinearSystem(matrix, coefficients);
//	предыдущий вызов создает внутри себя буфер для сохранения исходной матрицы. нам это ни к чему
	SolveLinearSystemNS::SolveLinearSystemDestructive(coefficients, matrix);
	}

template<class ARR_T>
void	LSDetector<ARR_T>::DetectLSLinear(double &a0, double &a1)
	{
	const size_t s = samples.size();
	double	x0(0), x1(0), x2(0);
	double	x_fi(0), fi(0);
//	ARR_T::const_iterator samples_it = samples.begin();

	for(size_t	i = 0; i < s; ++i)
		{
		double	x = grid(i);
		double	y = sample(i);
		double	w = weight(i);

		x0 += w;
		x1 += w*x;
		x2 += w*x*x;
		fi += w*y;
		x_fi += w*x*y;
		}

	a1 = (x0*x_fi - x1*fi)/(x0*x2 - x1*x1);
	a0 = (fi - a1*x1)/x0;
	}


template<class ARR_T>
void	LSDetector<ARR_T>::DetectLSSquare(double &a0, double &a1, double &a2)
	{
	const	size_t	s = samples.size();
	double	x0(0), x1(0), x2(0), x3(0), x4(0);
	double	x2_fi(0), x_fi(0), fi(0);
//	ARR_T::const_iterator samples_it = samples.begin();

	for(size_t	i = 0; i < s; ++i)
		{
		/*
		x1 += i;
		x2 += i*i;
		x3 += i*i*i;
		x4 += i*i*i*i;

		fi += samples[i];
		k_fi += i*samples[i];
		x2_fi += i*i*samples[i];
		*/
		double y = sample(i);
		double w = weight(i);
		double x = grid(i);
		double	pow_x = w;

		// power 0
		x0 += pow_x;
		fi += pow_x*y;

		//power 1
		pow_x*=x;
		x1 += pow_x;
		x_fi += pow_x*y;

		//power 2
		pow_x*=x;
		x2 += pow_x;
		x2_fi += pow_x*y;

		//power 3
		pow_x*=x;
		x3 += pow_x;

		//power 4
		pow_x*=x;
		x4 += pow_x;

		double	L4 = x0*x4 - x2*x2;
		double	L3 = x0*x3 - x2*x1;
		double	L2 = x0*x2 - x1*x1;

		double	M1 = x0*x_fi - x1*fi;
		double	M2 = x0*x2_fi - x2*fi;

		a2 = (M1*L3 - M2*L2)/(L3*L3 - L4*L2);
		a1 = (M2*L3 - M1*L4)/(L3*L3 - L4*L2);
		a0 = (fi - a2*x2 - a1*x1)/x0;
		}
	}

template<class detector_t, XRAD__LinearVector_template>
void	DetectLSPolynomTemplate(detector_t &detector, LinearVector<XRAD__LinearVector_template_args> &coefficients)
	{
	switch(coefficients.size())
		{
		case 1:
			detector.DetectLSConst(coefficients[0]);
			break;

		case 2:
			detector.DetectLSLinear(coefficients[0], coefficients[1]);
			break;

		case 3:
			detector.DetectLSSquare(coefficients[0], coefficients[1], coefficients[2]);
			break;

		default:
			detector.DetectLSUniversal(coefficients, x_power_function());
			break;
		}
	}

template<class detector_t, class ARR_T>
void	DetectLSUniversalTemplate(detector_t &detector, const abstract_LS_basis_function&f, ARR_T &coefficients)
	{
	detector.DetectLSUniversal(coefficients, f);
	}


} // namespace least_squares_classes

XRAD_END
