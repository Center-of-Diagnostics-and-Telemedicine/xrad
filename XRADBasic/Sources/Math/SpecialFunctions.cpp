/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#include "pre.h"
#include "SpecialFunctions.h"
#include <XRADBasic/MathFunctionTypes.h>
#include <XRADBasic/Sources/Containers/UniversalInterpolation.h>
#include <cmath>

//--------------------------------------------------------------
//
//	функции с сайта http://tdoc.ru
//
//
//	логарифм гамма-фукнции, сама гамма и бета для положительных аргументов двойной точности.
//
//	вычисление логарифма гамма-функции для x>0
//	double log_gamma_function(double x);
//
//	вычисление гамма-функции для x>0
//	double gamma_function(double x);
//
//	вычисление бета-функции для x>0, y>0
//	double beta_function(double x,double y);
//
//--------------------------------------------------------------

XRAD_BEGIN

namespace SpecialFunctions
{

static const double	max_argument_for_Jn_series = 35;
//TODO то же самое проделать с аргументом для обычных бесселей
static const double	max_argument_for_In_series(double nu)
{
	// этим условием исключаем возможность роста приращения в соответствующей формуле
	return max(17., square(nu)/2);
	//MaxDouble();//15;
}
//TODO 2014_09_22 пределы применимости формул функций бесселя нужно уточнить, также как и размер минимального приращения.
//

// оба порога выбраны исходя из максимальной точности double по отношению к 1.
// меньше нельзя, т.к. может зациклиться.
// больше можно, но возможна потеря точности
static const double	min_increment_for_bessel_series = 1e-15;

enum
{
	n_iterations_for_In_asymptotic  = 14
};


// таблица коэффициентов для вычисления гамма-функции
enum
{
	n_coefficients_for_gamma_f_approximation = 8
};

static const double cof[n_coefficients_for_gamma_f_approximation] =
{
	2.5066282746310005,
	1.0000000000190015,
	76.18009172947146,
	-86.50532032941677,
	24.01409824083091,
	-1.231739572450155,
	0.1208650973866179e-2,
	-0.5395239384953e-5,
};



//--------------------------------------------------------------



// логарифм гамма-фукнции
double log_gamma_function(double x)
{
	//	вычисление последовательностей
	double	ser=cof[1];
	double	y=x;
	const double *co=cof+2;

	for(int j=2; j<n_coefficients_for_gamma_f_approximation ; ++j, ++co)
	{
		++y;
		ser+=(*co)/y;
	}
	// 	и других частей функции
	y=x+5.5;
	y-=(x+0.5)*log(y);

	return(-y+log(cof[0]*ser/x));
}

//	сама гамма-функция
double gamma_function(double x)
{
	return(exp(log_gamma_function(x)));
}

//	бета-функция
double beta_function(double x,double y)
{
	return(exp(log_gamma_function(x)+log_gamma_function(y)-log_gamma_function(x+y)));
}



//--------------------------------------------------------------
//
//	интеграл френеля

complexF64	fresnel_integral(double x)
{
	static	int	table_size = 4096;
	static	ComplexFunctionF64 table(table_size);
	double	argument_sign = 1;

	if(x < 0) argument_sign = -1, x = -x;

	double	approximation_treshold = 5.0;
	double	table_step = approximation_treshold/table_size;
	double	table_x = x/table_step;

	if(table_x >= table_size-1)
	{
		// аппроксимация приближенной формулой для больших значений
		double	a = x*x*pi()/2.;
		double	x3 = x*x*x;
		double	x4 = x*x3;

		double	c = cos(a);
		double	s = sin(a);

		double	fi = (0.3183099 - 0.0968/x4)/x;
		double	se = (0.10132 - 0.154/x4)/x3;

		complexF64 result(0.5 + fi*s - se*c, 0.5 - fi*c - se*s);
		return	result*=argument_sign;
	}

	if(!table.size())
	{
		// численное интегрирование, результат помещается в таблицу
		table.realloc(table.size());

		table[0] = 0;
		for(int i = 1; i < table_size; i ++)
		{
			double	arg = table_step*i;
			table[i] = polar(table_step, arg*arg*pi()/2);
		}

		for(int i = 1; i < table_size; i ++)table[i] += table[i-1];
	}


	if(in_range(table_x, 1, table_size-2))
	{
		// в середине таблицы предпочтительна кубическая интерполяция
		return	table.in(table_x, &interpolators::cubic);
	}
	else
	{
		// а там, где недостает данных, довольствуемся линейной
		return	table.in(table_x, &interpolators::linear);
	}
}



//--------------------------------------------------------------
//
// функции бесселя первого рода
//
//--------------------------------------------------------------



double	Jn_asymptotic(double x, double nu)
{
	//	простая асимптотическая формула при больших порядках работает плохо,
	double	chi = x-(nu/2 + 0.25)*pi();
	double	f0 = sqrt(2/(pi()*x));

	return f0*cos(chi);
}

double	Jn_asymptotic_series_hankel(double x, double nu)
{
	//	асимптотическая формула ханкеля при больших аргументах
	const	double	chi = x-(nu/2 + 0.25)*pi();
	const	double	mu = 4*nu*nu;
	const	double	x8 = x*8;
	const	double	x82 = square(x8);

	double	pf = 1;
	double	qf = (mu-1)/x8;

	double	dp = pf;
	double	dq = qf;

	double	p = dp;
	double	q = dq;

	double	i = 0;
	while(fabs(dp) > min_increment_for_bessel_series && fabs(dq) > min_increment_for_bessel_series)
	{
		double	m = 2*i;
		double	m1 = (mu - square(m+1)) / (i+1);
		double	m3 = (mu - square(m+3)) / (i+2);
		double	m5 = (mu - square(m+5)) / (i+3);

		pf = - (m1 * m3) / x82;
		dp *= pf;

		qf = - (m3 * m5) / x82;
		dq *= qf;

		p += dp;
		q += dq;
		i+=2;
	}

	double	f0 = sqrt(2/(pi()*x));
	return f0*(p*cos(chi) - q*sin(chi));
}

double	Jn_taylor_series(double x, double nu)
{
	// разложение ф. бесселя в ряд тейлора. для аргументов менее 35 использовать ее
	double	k = 0;
	double	result = 0;
	double	t = x*x/4.;

	double	increment = pow(x/2.,nu)/gamma_function(nu+1);

	while(fabs(increment) > min_increment_for_bessel_series)
	{
		if(k)increment *= -t/(k*(k+nu));
		k++;
		result += increment;
	}

	return result;
}

/*
double	Jn_recurrent(double x, double nu)
{
	int	n = integral_part(nu);
	double	dn = fractional_part(nu);

	double	j0 = Jn_taylor_series(x, dn);
	double	j1 = Jn_taylor_series(x, dn+1);
	double	jnu = Jn_taylor_series(x, nu);

	for(int i = 2; i <= n; ++i)
	{
		double	j2 = 2*(n+dn)*j1/x - j0;
		j0 = j1;
		j1 = j2;
	}
	return jnu;
}
*/

double	Jn(double x, double nu)
{
	//	функции бесселя Jn от действительного аргумента
	//	для произвольных действительных порядков

	//if(nu < 0) return Jn_recurrent(x, nu);

	if(x > max_argument_for_Jn_series)
	//
	{
		return Jn_asymptotic_series_hankel(x,nu);
	}
	else
	{
		return Jn_taylor_series(x,nu);
	}
}



double	In_asymptotic_series_exp(double x, double nu)
{
	// асимптотический ряд при больших значениях аргумента,
	// аппроксимирующий In(x,nu) без умножения на exp(x)
	const	double	mu = 4*nu*nu;
	const	double	x8 = 8*x;

	double	result = 1;
	double	increment = 1;
	double	n = 0;
	double	n_x8 = x8;

	// цикл по фиксированному количеству итераций может давать критическую погрешность.
	// смотрим, как скажется на деле цикл с выходом по минимальному приращению.

	while(fabs(increment)>min_increment_for_bessel_series)
	{
		double	m = square(2*n + 1);
		//increment *= -(mu-m)/(x8*(n+1));
		increment *= -(mu-m)/n_x8;
		result += increment;
		++n;
		n_x8 += x8;
	}

	return result/sqrt(two_pi()*x);
}



double	In_xn(double x, double nu)
{
	// In(x)/x^(n), эта функция прежде всего предназначена для анализа предельных значений при малых x

	double	increment_log = -ln_2()*nu - log_gamma_function(nu+1);
	double increment = exp(increment_log); //increment = pow(x/2.,nu)/gamma_function(nu+1);

	if(!x) return increment;

//	double	t = square(x)/4.;
	double	t_inv = 4./square(x);
	double result = increment;
	double	k = t_inv;
	double	k_nu = 1+nu;

	while(fabs(increment) > min_increment_for_bessel_series)
	{
		increment /= (k*k_nu);
		result += increment;
		++k_nu;
		k+=t_inv;
	}
	return result;
}



double	In_taylor_series(double x, double nu)
{
	// iterative procedure by taylor series
	// очень большой порядок с очень большим аргументом вместе вызывают переполнение.
	// это законно, т.к. значение double ограничено. если надо анализировать большие
	// значениях, следует использовать функциии In_exp, log_In

	if(!x)
	{
		if(!nu) return 1;
		else return 0;
	}
	else
	{
		//TODO здесь некорректно отрабатывается отрицательные порядок и аргумент. аналогично в других местах
		double	increment_log = log(x/2.)*nu - log_gamma_function(nu+1);
		double increment = exp(increment_log); //increment = pow(x/2.,nu)/gamma_function(nu+1);
		double	t_inv = 4./square(x);
		double result = increment;

		double	k = t_inv;
		double	k_nu = 1+nu;

		while(fabs(increment) > min_increment_for_bessel_series)
		{
			increment /= (k*k_nu);
			result += increment;
			k+=t_inv;
			++k_nu;
		}
		return result;
	}
}



double	In_taylor_series_exp(double x, double nu)
{
	// iterative procedure by taylor series
	//TODO уточнять далее эту функцию

	if(!x)
	{
		if(!nu) return 1;
		else return 0;
	}
	else
	{
		double	increment_log = log(x/2.)*nu - log_gamma_function(nu+1) - x;
		//double	t = square(x)/4.;
		double	t_inv = 4./square(x);
		const	double	min_log = -log(max_double());

		double	k = t_inv;
		double	k_nu = 1+nu;

		if(increment_log > min_log)
		{
			double increment = 1.; // increment = pow(x/2.,nu)/gamma_function(nu+1);
			double result = increment;

			double	criterium = increment*min_increment_for_bessel_series;

			while(fabs(increment) > criterium)
			{
				increment /= (k*k_nu);
				result += increment;
				k+=t_inv;
				++k_nu;
			}
			return result*exp(increment_log);
		}
		else
		{
			//double	logt = -log(t_inv);
			double loginc = increment_log;
			double result = 0;
			double log_threshold = log(min_increment_for_bessel_series);

			//TODO следующие два цикла при больших nu (под 100) зацикливаются. подработать

			// поднимаем логарифм приращения, не подрастет ли?
			while(loginc < log_threshold)
			{
				double	logincinc = log(k*k_nu);
				if(logincinc < 0) return 0;
				loginc += logincinc;

				k+=t_inv;
				++k_nu;
			}

			while(loginc > log_threshold)
			{
				loginc += log(k*k_nu);
				result += exp(loginc);

				k+=t_inv;
				++k_nu;
			}
			return result;
		}
	}
}



double	In(double x, double nu)
{
	//	модифицированные функции бесселя In от действительного аргумента
	//	для произвольных действительных порядков

	if(fabs(x) > max_argument_for_In_series(nu))
	{
		return In_asymptotic_series_exp(x,nu) * exp(x);
	}
	else
	{
		return In_taylor_series(x,nu);
	}
}

double	In_exp(double x, double nu)
{
	//	модифицированные функции бесселя In от действительного аргумента
	//	для произвольных действительных порядков, деленые на exp(x)
	if(fabs(x) > max_argument_for_In_series(nu))
	{
		return In_asymptotic_series_exp(x,nu);
	}
	else
	{
		return In_taylor_series_exp(x,nu);
	}
}

double	log_In(double x, double nu)
{
	//	логарифм модифицированной функции бесселя In от действительного аргумента
	//	для произвольных действительных порядков
	return log(In_exp(x,nu)) + x;
}

//--------------------------------------------------------------
//
//	вычисление функции I_n/I_m. в частности, нужно для I~ = I1/I0, входящей в уравнение правдоподобия
//
double	In_Im(double x, double nu, double mu)
{
	return In_exp(x,nu)/In_exp(x,mu);
}

//--------------------------------------------------------------
//
//	вычисление Q-функции Маркума
//

#if 1

double	Qm_internal(double a, double b, int M)
{
	// корректно вычисляет функцию маркума Qm для случаев 0<=a<b
	double	result(0);
	//TODO сходимость ряда замедляется с ростом аргументов. подумать над возможностью оптимизации.
	// быть может, попробовать считать ряд через рекуррентные соотношения Абрамовиц-Стиган 9.6.26 стр. 197.
	// первый опыт приводил к потере точности, т.к. восходящий рекуррентный ряд расходится.
	if(a)
	{
		// сумма по формуле из справочника
		const double	ab = a*b;
		const double	a_b = a/b;

		int	k;
		double	a_b_k; //(a/b)^k
			// сразу умножать элементы приращений на гаусса нельзя: формально было бы верно, но слишком быстро сойдется не к тому

		if(M>1)
		{
			// первый конечный цикл от 1-M до M-1. на этих шагах выигрыш за счет повторного вычисления функции бесселя
			a_b_k = a_b;

			result = In_exp(ab, 0);//k=0
			for(k=1; k<=M-1; ++k)
			{
				result += (a_b_k + 1./a_b_k)*In_exp(ab, abs(k));
				a_b_k *= a_b;
			}
		}
		else
		{
			// пропуск элементов до 1-M
			a_b_k = pow(a_b, 1-M);
			k=1-M;
		}

		double increment = a_b_k*In_exp(ab, abs(k));
		do
		{
			result += increment;
			a_b_k *= a_b;
			++k;
			increment = a_b_k*In_exp(ab, abs(k));
		}while(increment > 1.e-15);// в условии именно инкремент, а не модуль его. любое отрицательное значение некорректно
	}
	else
	{
		// через предел в нуле
		const double	b2 = square(b);
		double	b2_k = 1;

		for(int k = 0; k<=M-1; ++k)// конечная сумма, при k<0 Ik(ab)/(ab)^-k тождественно обращается в ноль
		{
			result += b2_k*In_xn(0, abs(k));
			b2_k *= b2;
		}
	}

	if(!result)
	{
		// это ловушка для тех случаев, когда она зря заходит в цикл. проявляется только в debug-версии
		//ForceDebugBreak();
	}
	return result *= gauss(a - b, 1);
}

double	Qm(double a, double b, int M)
{
	//сначала наперед известные случаи (во избежание ненужных заходов в циклы)
	if(M<1 && !a)
	{
		return 0;
	}
	if(M>=1 && !b)
	{
		return 1;
	}

	if(a<b)
	{
		return Qm_internal(a, b, M);
	}
	else
	{
		return 1. - Qm_internal(b, a, 1-M);
	}
}



#else

double	Q1abQ1ba(double a, double b)
{
	// вспомогательная функция, Q1(a,b) + Q1(b,a), оказалась не нужна
	return  1. + In_exp(a*b,0) * gauss(a-b, 1);
}

// промежуточные варианты нахождения функции маркума, делались с целью отладки основной процедуры
// временно оставляется для справки, потом убрать

double	Q1_internal(double a, double b)
{
	// корректно вычисляет функцию маркума Q1 для случаев 0<a<b
	double	result(0);
	double	increment(1);
	// сразу умножать инкремент на гаусса нельзя, слишком быстро сойдется не к тому

	double	ab = a*b;
	double	a_b = a/b;
	double	a_b_k = 1; //(a/b)^k

	for(int k = 0; increment > 1.e-15; ++k)// в условии именно инкремент, а не модуль его. любое отрицательное значение некорректно
	{
		increment = a_b_k*In_exp(ab, k);
		result += increment;
		a_b_k *= a_b;
	}

	return result *= gauss(a - b, 1);
}

double	Q1(double a, double b)
{
	// тремя последующими условиями исключаются нулевые варианты !b внутри функции Q1_internal

	if(!a && !b) return 1;//!a и !b тоже дает 1
	if(!a) return gauss(b, 1);
	if(a>b)
	{
		return Q1abQ1ba(a,b) - Q1_internal(b,a);
		// при a>b сумма в вызываемой функции может разойтись
	}
	else
	{
		return Q1_internal(a,b);
	}
}

double	Qm(double a, double b, int M)
{
	//сначала наперед известные случаи (во избежание ненужных заходов в циклы)
	if(M<1 && !a)
	{
		return 0;
	}
	if(M>=1 && !b)
	{
		return 1;
	}
	// за основу берем бесконечный ряд из Q1. если M=1, его и возвращаем
	double	result = Q1(a,b);

	if(M == 1)
	{
		return result;
	}

	// затем прибавляем недостающие члены ряда
	double	ab = a*b;
	if(ab)
	{
		if(M>1)
		{
			double	b_a = b/a;
			double	power = b_a * gauss(a - b, 1);

			for(int i = 1; i <= M-1; ++i)
			{
				result += power*In_exp(ab, labs(i));
				power *= b_a;
			}
		}
		else //if(M<1)
		{
			double	a_b = a/b;
			double	power = gauss(a - b, 1);

			for(int i = 0; i < 1-M; ++i)
			{
				result -= power*In_exp(ab, labs(i));
				power *= a_b;
			}
		}
	}
	else if(M<1)//!b
	{
		double	power_acquisition_factor = square(a);//a/b;
		double	power = gauss(a - b, 1);

		for(int i = 0; i < 1-M; ++i)
		{
			result -= power*In_xn(0, labs(i));
			power *= power_acquisition_factor;
		}
	}
	else if(M>1)//!a
	{
		double	power_acquisition_factor = square(b);
		double	power = power_acquisition_factor*gauss(a - b, 1);

		for(int i = 1; i <= M-1; ++i)
		{
			result += power*In_xn(0, labs(i));
			power *= power_acquisition_factor;
		}
	}

	return result;
}

#endif



//--------------------------------------------------------------
//	первая производная от J1(x)

double	J1prim1(double x)
{
	//первая производная от J1
	double	v1, v2;

	v1 = Jn(x,0);
	if(x)
	{
		v2 = Jn(x,1)/x;
	}
	else
	{
		v2 = .5;
	}
	return v1-v2;
}



//--------------------------------------------------------------
//
//	erf(x):
//	в CodeWarrior эта функция реализована, я ее удалил было,
//	но BDS заставляет вернуть обратно
//
//--------------------------------------------------------------

//--------------------------------------------------------------

double ErfIntl( double t) // not useful
{
	/* 2.0 - 10.0 */
	double min = 0;
	double max = fabs(t);
	double step = 1e-4;
	double sum = 0;
	for( double x=min; x<max; x+=step)
	{
		if( x+step > max)
			step = max-x;
		double y1, y2;
		y1 = exp(-square(x));
		y2 = exp(-square(x+step));
		sum += 0.5*step*(y1+y2);
	}
	sum /= sqrt(pi());
	return 2.*(t>=0?sum:-sum);
}

//--------------------------------------------------------------

double ErfSum( double t)
{
	/* 0.0 - 3.0 (4.0) */
	double sum = 0;
	double tn = t;
	double w = 1;
	for( int n=0; fabs(w)>1e-10; ++n)
	{
		w = tn/(2*n+1);
		sum += w;
		tn *= -t*t/(n+1);
	}
	sum /= sqrt_pi();
	return 2.*sum;
}

//--------------------------------------------------------------

double ErfcSum( double t)
{
	// дополнительный интеграл вероятностей
	/* 10.0 - inf , = 1.0000000000 */
	double sum = 1.;
	double increment_factor = -1./(2*t*t);
	double increment = 1.;
	for( int n=1; fabs(increment)>1e-5; n+=2)
	{
		increment *= increment_factor*n;
		sum += increment;
	}
	return sum/(sqrt_pi()*t)*exp(-t*t);
}

//--------------------------------------------------------------

double erf( double t)
{
	if( t > 4.)
	{
		return 1.-ErfcSum(t);
	}
	else if( t < -4.)
	{
		return -1.-ErfcSum(t);
	}
	return ErfSum( t);
}



//--------------------------------------------------------------
//TODO вычисление полиномов чебышева, проверить заодно с интегралом френеля

double	ChebyshevPolynom(double x, double order)
{
	return cos(order*acos(x));
}

double	WeightedChebyshevPolynom(double x, double order)
{
	return	ChebyshevPolynom(x,order)*ChebyshevPolynomWeight(x);
}

complexF64	ChebyshevPolynomC(double x, double order)
{
	return polar(1., order*acos(x));
}

complexF64	WeightedChebyshevPolynomC(double x, double order)
{
	return polar(ChebyshevPolynomWeight(x), order*acos(x));
}

double	ChebyshevPolynomWeight(double x)
{
	// весовая функция для ортогонализации
	// полиномов чебышева
	return sqrt(1.-x*x);
}



} // namespace SpecialFunctions

XRAD_END
