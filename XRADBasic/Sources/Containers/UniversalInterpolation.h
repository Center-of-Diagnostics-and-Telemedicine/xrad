#ifndef __universal_interpolation_h
#define __universal_interpolation_h
/*!
	\file
	Подробное описание принципов работы интерполяторов см. в файле UnversalInterpolation2D.hh.
*/

#include <XRADBasic/FIRFilterKernelTypes.h>

XRAD_BEGIN

//--------------------------------------------------------------

//! \brief Генератор произвольных интерполирующих фильтров (абстрактный класс)
template<class FILTER>
class InterpolationFilterGenerator
{
	protected:
		InterpolationFilterGenerator(int in_filter_order) : filter_order(in_filter_order){}
	public:
		const int filter_order;

		typedef FILTER	filter_type;

		virtual double	GenerateFilter(filter_type &filter, double x) const = 0;
		void	SetOffsetCorrection(double &) const;
};



//--------------------------------------------------------------
//
// Генераторы конкретных интерполирующих фильтров
//
//--------------------------------------------------------------



template<class FILTER>
class	BSplineFilterGenerator : public InterpolationFilterGenerator<FILTER>
{
		PARENT(InterpolationFilterGenerator<FILTER>);
	private:
		//! \brief Порядок сплайна: для "ближайшего соседа" 0, линейного 1, квадратичного 2 и т.д.
		const int spline_order;
	public:
		typedef FILTER filter_type;
		typedef BSplineFilterGenerator<FILTER> self;
		using parent::filter_order;
	public:
		BSplineFilterGenerator(int in_spline_order) : InterpolationFilterGenerator<FILTER>(in_spline_order + 1), spline_order(in_spline_order){}
		double	GenerateFilter(filter_type &filter, double x) const;
};

template<class FILTER>
class	ISplineFilterGenerator : public InterpolationFilterGenerator<FILTER>
{
		PARENT(InterpolationFilterGenerator<FILTER>);
	private:
		//! \brief Порядок сплайна: для "ближайшего соседа" 0, линейного 1, квадратичного 2 и т.д.
		const int spline_order;
	public:
		typedef FILTER filter_type;
		typedef ISplineFilterGenerator<FILTER> self;
		using parent::filter_order;
	public:
		ISplineFilterGenerator(int in_spline_order) : InterpolationFilterGenerator<FILTER>(in_spline_order + 1), spline_order(in_spline_order){}
		double	GenerateFilter(filter_type &filter, double x) const;
};

//--------------------------------------------------------------

template<class FILTER>
class	SincFilterGenerator : public InterpolationFilterGenerator<FILTER>
{
		PARENT(InterpolationFilterGenerator<FILTER>);
		//SincFilterGenerator(){throw runtime_error(typeid(self).name() + "::Default constructor not allowed");} //private, not allowed to call
		SincFilterGenerator(){throw runtime_error(string(typeid(self).name()) + string("::Default constructor not allowed"));} //private, not allowed to call
	public:
		typedef FILTER filter_type;
		typedef SincFilterGenerator<FILTER> self;
		using parent::filter_order;
	public:
		SincFilterGenerator(int in_filter_order);
		double	GenerateFilter(filter_type &filter, double x) const;
};



//--------------------------------------------------------------
//
//	Для производных -- уже не шаблоны, так как заложенный механизм
//	нахождения производных не терпит изменения несущей частоты.
//
//--------------------------------------------------------------

class	SincDerivativeFilterGenerator : public InterpolationFilterGenerator<FilterKernelReal>
{
		PARENT(InterpolationFilterGenerator<FilterKernelReal>);
		SincDerivativeFilterGenerator(); //private, not allowed to call
	public:
		typedef SincDerivativeFilterGenerator self;
		typedef FilterKernelReal filter_type;
		using parent::filter_order;
	public:
		SincDerivativeFilterGenerator(int in_filter_order);
		double	GenerateFilter(filter_type &filter, double x) const;
};

class	SincDerivativeComplexFilterGenerator : public InterpolationFilterGenerator<FilterKernelComplex>
{
		PARENT(InterpolationFilterGenerator<FilterKernelComplex>);
		SincDerivativeComplexFilterGenerator(); //private, not allowed to call
		const double	carrier;
	public:
		typedef SincDerivativeComplexFilterGenerator self;
		typedef FilterKernelComplex filter_type;
		using parent::filter_order;
	public:
		SincDerivativeComplexFilterGenerator(int in_filter_order, double in_carrier);
		double GenerateFilter(filter_type &filter, double x) const;
};



//--------------------------------------------------------------
/*!
	\brief Интерполятор для преобразования Гильберта. Не шаблон, так как
	фильтр может давать только комплексный результат
*/
class	HilbertFilterGenerator : public InterpolationFilterGenerator<FilterKernelReal>
{
		PARENT(InterpolationFilterGenerator<FilterKernelReal>);
		HilbertFilterGenerator() = delete; //private, not allowed to call
		const ComplexNumberPart complex_number_part;
	public:
		typedef HilbertFilterGenerator self;
		typedef FilterKernelReal filter_type;
		using parent::filter_order;
	public:
		HilbertFilterGenerator(int in_filter_order, ComplexNumberPart cp);
		double	GenerateFilter(filter_type &filter, double x) const;
};

//--------------------------------------------------------------

//! \brief Комплексный интерполятор с выбором центральной частоты
template<class CFILTER_GENERATOR>
class	ComplexInterpolatorGenerator : public CFILTER_GENERATOR
{
		PARENT(CFILTER_GENERATOR);
	private:
		const double	carrier;
		//! \brief Функция SetFilterCarrier обязательно private. Единственное место, откуда можно ее вызывать,
		//! это функция GenerateFilter
		void	SetFilterCarrier(typename CFILTER_GENERATOR::filter_type &filter, double x) const;
	public:
		typedef typename CFILTER_GENERATOR::filter_type filter_type;
		typedef CFILTER_GENERATOR basic_generator_type;
		using parent::filter_order;
	public:
		ComplexInterpolatorGenerator(const CFILTER_GENERATOR& g, double in_carrier): CFILTER_GENERATOR(g), carrier(in_carrier){} //sinc

		//double	GenerateFilter(typename CFILTER_GENERATOR::filter_type &filter, double x) const
		double	GenerateFilter(filter_type &filter, double x) const
		{
			double	normalizer = CFILTER_GENERATOR::GenerateFilter(filter, x);
			//	Нормализатор находим до внесения осцилляции; это позволяет правильно оценить
			//	изменения энергетики, вносимые фильтром, и сделать поправку на них.
			SetFilterCarrier(filter, x);
			return normalizer;
		}
};

//--------------------------------------------------------------

class	BSplineComplexFilterGenerator : public ComplexInterpolatorGenerator<BSplineFilterGenerator<FilterKernelComplex> >
{
		PARENT(ComplexInterpolatorGenerator<BSplineFilterGenerator<FilterKernelComplex> >);
	public:
		typedef parent::filter_type filter_type;
		typedef parent::basic_generator_type basic_generator_type;
	public:
		BSplineComplexFilterGenerator(int in_filter_order, double in_carrier): parent(basic_generator_type(in_filter_order), in_carrier){}
};

class	ISplineComplexFilterGenerator : public ComplexInterpolatorGenerator<ISplineFilterGenerator<FilterKernelComplex> >
{
		PARENT(ComplexInterpolatorGenerator<ISplineFilterGenerator<FilterKernelComplex> >);
	public:
		typedef parent::filter_type filter_type;
		typedef parent::basic_generator_type basic_generator_type;
	public:
		ISplineComplexFilterGenerator(int in_filter_order, double in_carrier) : parent(basic_generator_type(in_filter_order), in_carrier){}
};

//--------------------------------------------------------------

class	SincComplexFilterGenerator : public ComplexInterpolatorGenerator<SincFilterGenerator<FilterKernelComplex> >
{
		PARENT(ComplexInterpolatorGenerator<SincFilterGenerator<FilterKernelComplex> >);
	public:
		typedef parent::filter_type filter_type;
		typedef parent::basic_generator_type basic_generator_type;
	public:
		SincComplexFilterGenerator(int in_filter_order, double in_carrier): parent(basic_generator_type(in_filter_order), in_carrier){}
};



//--------------------------------------------------------------

//! \brief Собственно интерполятор
template<class FILTER>
class UniversalInterpolator
{
		int	n_filters;

		/*!
			Для нечетных порядков фильтра 0.5; для четных 0.
			Исправляет сдвиг, возникающий при нечетных порядках интерполирующего фильтра.
			Задается при инициализации функцией класса InterpolationFilterGenerator.
		*/
		double	x_offset;

		DataArray<FILTER> InterpolationFilters;
	public:
		typedef UniversalInterpolator<FILTER> self;
		typedef	FILTER filter_type;
		typedef typename FILTER::value_type result_type;

		UniversalInterpolator(){n_filters = 0;}

		//	void	InitFilters(int in_n_divisions, const InterpolationFilterGenerator<FILTER> *);
		void	InitFilters(int in_n_divisions, const InterpolationFilterGenerator<FILTER> &);

		const FILTER *GetNeededFilter(double x) const;
		void	ApplyOffsetCorrection(double &x) const {x += x_offset;}

		//friend void	Display(const self &);
};


//--------------------------------------------------------------

//! \brief Конктретные фильтры для непосредственного пользования
struct	interpolators
{
	//! \name Действительные интерполяторы
	//! @{
	static	UniversalInterpolator<FilterKernelReal> nearest_neighbour;
	static	UniversalInterpolator<FilterKernelReal> linear;
	static	UniversalInterpolator<FilterKernelReal> quadratic;
	static	UniversalInterpolator<FilterKernelReal> cubic;
	static	UniversalInterpolator<FilterKernelReal> icubic;

	static	UniversalInterpolator<FilterKernelReal> sinc;
	//! @}

	//! \name Действительный дифференцирующий фильтр
	//! @{
	static	UniversalInterpolator<FilterKernelReal> sinc_derivative;
	//! @}

	//! \name Прямая фаза, carrier = 0.5
	//! @{
	static	UniversalInterpolator<FilterKernelComplex> complex_icubic;
	static	UniversalInterpolator<FilterKernelComplex> complex_sinc;
	//! @}

	//! \name Обратная фаза, carrier = -0.5
	//! @{
	static	UniversalInterpolator<FilterKernelComplex> complex_icubicT;
	static	UniversalInterpolator<FilterKernelComplex> complex_sincT;
	//! @}

	//! \name Комплексный осциллирующий фильтр
	//! @{
	static	UniversalInterpolator<FilterKernelComplex> complex_sinc_derivative;
	static	UniversalInterpolator<FilterKernelComplex> complex_sinc_derivativeT;
	//! @}

	//! \name Преобразование Гильберта
	//! @{
	static	UniversalInterpolator<FilterKernelReal> hilbert_re;
	static	UniversalInterpolator<FilterKernelReal> hilbert_im;

	static	UniversalInterpolator<FilterKernelReal> hilbert_re8;
	static	UniversalInterpolator<FilterKernelReal> hilbert_im8;
	//! @}

	interpolators();
};

//--------------------------------------------------------------

XRAD_END

#include "UniversalInterpolation.hh"

//--------------------------------------------------------------
#endif //__universal_interpolation_h
