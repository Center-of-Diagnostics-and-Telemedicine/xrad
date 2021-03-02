/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_real_interpolation_2d
#define XRAD__File_real_interpolation_2d
//--------------------------------------------------------------

#include <XRADBasic/FIRFilterKernelTypes2D.h>
#include <XRADBasic/Sources/Core/FlowControl.h>

XRAD_BEGIN

//--------------------------------------------------------------

//! \brief Генератор произвольных интерполирующих фильтров (абстрактный класс)
template<class FILTER>
class	InterpolationFilterGenerator2D
{
	protected:
		const int	filter_order_v, filter_order_h;
		InterpolationFilterGenerator2D(int fo) :filter_order_v(fo), filter_order_h(fo){}
		InterpolationFilterGenerator2D(int fov, int foh) :filter_order_v(fov), filter_order_h(foh){}
	public:
		typedef FILTER filter_type;
	public:
		//! \return Функция должна возвращать нормировочное значение фильтра такое, чтобы при делении на него
		//! мы получали нормальный фильтр.
		virtual	double	GenerateFilter(filter_type &filter, double v, double h) const = 0;
		void	SetOffsetCorrection(double &offset_correction_v, double &offset_correction_h) const;
};

//--------------------------------------------------------------

template<class FILTER>
class	BSplineFilterGenerator2D : public InterpolationFilterGenerator2D<FILTER>
{
		PARENT(InterpolationFilterGenerator2D<FILTER>);
	public:
		const int spline_order;
		typedef FILTER filter_type;
		using parent::filter_order_v;
		using parent::filter_order_h;
	public:
		BSplineFilterGenerator2D(int so) : spline_order(so), InterpolationFilterGenerator2D<FILTER>(so+1){}
		double	GenerateFilter(filter_type &filter, double v, double h) const;
};

template<class FILTER>
class	ISplineFilterGenerator2D : public InterpolationFilterGenerator2D<FILTER>
{
		PARENT(InterpolationFilterGenerator2D<FILTER>);
	public:
		const int spline_order;
		typedef FILTER filter_type;
		using parent::filter_order_v;
		using parent::filter_order_h;
	public:
		ISplineFilterGenerator2D(int so) : spline_order(so), InterpolationFilterGenerator2D<FILTER>(so+1){}
		double	GenerateFilter(filter_type &filter, double v, double h) const;
};

//--------------------------------------------------------------

template<class FILTER>
class	SincFilterGenerator2D : public InterpolationFilterGenerator2D<FILTER>
{
		PARENT(InterpolationFilterGenerator2D<FILTER>);
		SincFilterGenerator2D() = delete; // not allowed
	public:
		typedef FILTER filter_type;
		using parent::filter_order_v;
		using parent::filter_order_h;

	public:
		SincFilterGenerator2D(int fov, int foh);
		double	GenerateFilter(filter_type &filter, double v, double h) const;
};

//--------------------------------------------------------------

template<class FILTER>
class	BesselFilterGenerator : public InterpolationFilterGenerator2D<FILTER>
{
		PARENT(InterpolationFilterGenerator2D<FILTER>);
		BesselFilterGenerator() = delete; // not allowed
		const double	bessel_radius;

	public:
		typedef FILTER filter_type;
		using parent::filter_order_v;
		using parent::filter_order_h;

	public:
		BesselFilterGenerator(int fo, double br);
		double	GenerateFilter(filter_type &filter, double v, double h) const;
};


//--------------------------------------------------------------

/*!
	\brief Интерполятор для производных -- уже не шаблон, так как заложенный механизм
	нахождения производных не терпит изменения несущей частоты
*/
class	BesselDerivativeFilterGenerator : public InterpolationFilterGenerator2D<FIRFilter2DReal>
{
		PARENT(InterpolationFilterGenerator2D<FIRFilter2DReal>);
		BesselDerivativeFilterGenerator();
		const double	bessel_radius;
		const double	derivative_direction;

		double	BesselDerivative(double r, double x) const;

	public:
		typedef FIRFilter2DReal filter_type;
		using parent::filter_order_v;
		using parent::filter_order_h;

	public:
		BesselDerivativeFilterGenerator(int fo, double br, double dd);
		double	GenerateFilter(filter_type &filter, double v, double h) const;
};


//--------------------------------------------------------------

/*!
	\brief Изотропный квази-сплайн фильтр, построенный на основе Фурье-преобразования
	функций Бесселя Jn(r)/(r^n). Импульсная характеристика представляет собой
	выражение (1-r*r)^(n-1)
*/
template<class FILTER>
class	QuasiSplineIsotropicFilterGenerator2D : public InterpolationFilterGenerator2D<FILTER>
{
		PARENT(InterpolationFilterGenerator2D<FILTER>);
		QuasiSplineIsotropicFilterGenerator2D() :bessel_order(0){ throw runtime_error("Default constructor not allowed"); }; //private, not allowed
		const double	bessel_order;
	public:
		typedef FILTER filter_type;
		using parent::filter_order_v;
		using parent::filter_order_h;

	public:
		QuasiSplineIsotropicFilterGenerator2D(double bo);
		double	GenerateFilter(filter_type &filter, double v, double h) const;
};


//--------------------------------------------------------------

//! \brief Комплексный интерполятор с выбором центральных частот по обеим координатам
template<class CFILTER_GENERATOR>
class	ComplexInterpolatorGenerator2D : public CFILTER_GENERATOR
{
		PARENT(CFILTER_GENERATOR);
	private:
		const double	carrier_v, carrier_h;
		//! \brief Эта функция обязательно private. Единственное место, откуда можно ее вызывать,
		//! это функция GenerateFilter
		void	SetFilterCarrier(typename CFILTER_GENERATOR::filter_type &filter, double v, double h) const;
	public:
		typedef typename CFILTER_GENERATOR::filter_type filter_type;
		using parent::filter_order_v;
		using parent::filter_order_h;

	public:
		ComplexInterpolatorGenerator2D(const CFILTER_GENERATOR& g, double cv, double ch) : CFILTER_GENERATOR(g), carrier_v(cv), carrier_h(ch){} //sinc

		double	GenerateFilter(typename CFILTER_GENERATOR::filter_type &filter, double v, double h) const
		{
			double	normalizer = CFILTER_GENERATOR::GenerateFilter(filter, v, h);
			//	Нормализатор находим до внесения осцилляции; это позволяет правильно оценить
			//	изменения энергетики, вносимые фильтром, и сделать поправку на них.
			SetFilterCarrier(filter, v, h);
			return normalizer;
		}
};

//--------------------------------------------------------------

class	SplineComplexFilterGenerator2D : public ComplexInterpolatorGenerator2D<BSplineFilterGenerator2D<FIRFilter2DComplex> >
{
		PARENT(ComplexInterpolatorGenerator2D<BSplineFilterGenerator2D<FIRFilter2DComplex> >);
	public:
		typedef parent::filter_type filter_type;
		using parent::filter_order_v;
		using parent::filter_order_h;

	public:
		SplineComplexFilterGenerator2D(int so, double cv, double ch) : parent(BSplineFilterGenerator2D<FIRFilter2DComplex>(so), cv, ch){}
};

//--------------------------------------------------------------

class	SincComplexFilterGenerator2D : public ComplexInterpolatorGenerator2D<SincFilterGenerator2D<FIRFilter2DComplex> >
{
		PARENT(ComplexInterpolatorGenerator2D<SincFilterGenerator2D<FIRFilter2DComplex> >);
	public:
		typedef parent::filter_type filter_type;
		using parent::filter_order_v;
		using parent::filter_order_h;

	public:
		SincComplexFilterGenerator2D(int fov, int foh, double cv, double ch) : parent(SincFilterGenerator2D<FIRFilter2DComplex>(fov, foh), cv, ch){}
};

//--------------------------------------------------------------

class	BesselComplexFilterGenerator : public ComplexInterpolatorGenerator2D<BesselFilterGenerator<FIRFilter2DComplex> >
{
		PARENT(ComplexInterpolatorGenerator2D<BesselFilterGenerator<FIRFilter2DComplex> >);
	public:
		typedef parent::filter_type filter_type;
		using parent::filter_order_v;
		using parent::filter_order_h;

	public:
		BesselComplexFilterGenerator(int fo, double br, double cv, double ch) : parent(BesselFilterGenerator<FIRFilter2DComplex>(fo, br), cv, ch){}
};

//--------------------------------------------------------------

//! \brief Класс, содержащий набор интерполирующих фильтров для конкретного случая
template<class FILTER>
class	UniversalInterpolator2D
{
		int	n_filters_v, n_filters_h;
		double	v_offset, h_offset;

		DataArray2D<DataArray<FILTER> >	InterpolationFilters;
	public:
		typedef FILTER filter_type;

	public:
		//Initialization
		UniversalInterpolator2D(){ n_filters_v = n_filters_h = 0; }
		void	InitFilters(int in_n_divisions_v, int in_n_divisions_h, const InterpolationFilterGenerator2D<FILTER> &);
		void	SetExtrapolationMethod(extrapolation::method em);

		//Work
		const FILTER	*GetNeededFilter(double v, double h) const;
		void	ApplyOffsetCorrection(double &v, double &h) const{ v += v_offset; h += h_offset; };
};


//--------------------------------------------------------------

typedef UniversalInterpolator2D<FIRFilter2DReal> RealInterpolator2D;
typedef	UniversalInterpolator2D<FIRFilter2DComplex> ComplexInterpolator2D;

struct	interpolators2D
{
	//! \name Constants
	//! @{
	//static	const	int	default_filter_size;

	static	const	int	default_interpolator_division;
	static	const	int	default_complex_interpolator_division;
	static	const	int	default_derivative_division;

	//! \brief Параметр бесселевского фильтра, соответствующий
	//! наилучшей сохранности изображения при некоторой потере изотропии.
	//! Для наилучшей изотропности требуется 1/sqrt(2), см. besselRadius_ISOTROPIC
	static	const	double	besselRadiusMIN_LOST;
	//! \brief Параметр бесселевского фильтра, соответствующий наилучшей изотропности.
	//! Равен 1/sqrt(2)
	static	const	double	besselRadius_ISOTROPIC;
	//! @}

	//! \name Real filters (are applicable to complex or rgb data too)
	//! @{

	static	RealInterpolator2D nearest_neighbour;
	static	RealInterpolator2D bilinear;
	static	RealInterpolator2D biquadratic;
	static	RealInterpolator2D bicubic;
	static	RealInterpolator2D ibicubic;

	static	RealInterpolator2D sinc;
	static	RealInterpolator2D bessel1_isotropic;
	static	RealInterpolator2D bessel1_min_lost;

	static	RealInterpolator2D quasi_spline_2;
	static	RealInterpolator2D quasi_spline_5;

	static	RealInterpolator2D bessel_ddx;
	static	RealInterpolator2D bessel_ddy;
	//! @}

	/*!
		\name Complex filters with carrier.
		Самый частый вариант в ультразвуке: по первой координате
		осцилляции нет, по второй есть.
		@{
	*/

	//! \brief Прямая фаза: carrier = (0, 0.5)
	static	ComplexInterpolator2D	complex_biquadratic;
	//! \brief Прямая фаза: carrier = (0, 0.5)
	static	ComplexInterpolator2D	complex_sinc;
	//! \brief Прямая фаза: carrier = (0, 0.5)
	static	ComplexInterpolator2D	complex_bessel;

	//! \brief Обращенная фаза: carrier = (0, -0.5)
	static	ComplexInterpolator2D	complex_biquadraticT;
	//! \brief Обращенная фаза: carrier = (0, -0.5)
	static	ComplexInterpolator2D	complex_sincT;
	//! \brief Обращенная фаза: carrier = (0, -0.5)
	static	ComplexInterpolator2D	complex_besselT;

	//! \brief Данные с осцилляцией по двум направлениям: carrier = (0.5, 0,5).
	//! На практики таких данных у нас не было, на всякий случай закладываю
	static	ComplexInterpolator2D	complex_bessel_2osc;
	//! \brief Данные с осцилляцией по двум направлениям: carrier = (0.5, 0,5).
	//! На практики таких данных у нас не было, на всякий случай закладываю
	static	ComplexInterpolator2D	complex_bessel_2oscT;

	//! @}



	//! \brief Initialization routine
	static	void	Init(ProgressProxy progress);
};



//--------------------------------------------------------------

/*!
	\brief Процедура инициализации двумерных фильтров

	Если предполагается использовать стандартный набор фильтров из класса
	interpolators2D, следует вызвать в любом месте до первого использования.

	При необходимости (особенно во встраиваемом коде) следует инициализировать отдельные
	интерполяторы.
*/
void	Init2DInterpolators(ProgressProxy progress);

//--------------------------------------------------------------

XRAD_END

#include "UniversalInterpolation2D.hh"

//--------------------------------------------------------------
#endif //XRAD__File_real_interpolation_2d
