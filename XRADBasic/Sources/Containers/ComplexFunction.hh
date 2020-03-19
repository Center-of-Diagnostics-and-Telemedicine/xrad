// TODO: Разорвать эту зависимость от посторонних типов данных.
#include "UniversalInterpolation.h"

XRAD_BEGIN

//--------------------------------------------------------------
//
//	фильтрация с учетом несущей
//

#if 0
template<class T, class ST>
void	ComplexFunction<T, ST>::FilterGaussCarrier(double dispersion, double carrier, double value_at_edge, extrapolation::method ex)
{
	#error Переписать без использования статических переменных (для потоковой безопасности)!
	// 1. Вынести фильтр в параметры функции.
	// 2. Разобраться с параметром tau().

	// функция переписана, но не отлаживалась
	static	FilterKernelComplex	filter;
	static	double	last_dispersion = 0;
	static	double	last_value_at_edge = 1;
	static	double	last_carrier = 0;

	if(dispersion != last_dispersion || value_at_edge != last_value_at_edge || carrier != last_carrier)
	{
		// при последовательном вызове фильтрации с одинаковыми параметрами
		// фильтр сохраняется. повторная инициализация только при изменении
		// параметров
		InitFIRFilterGaussian(filter, dispersion, value_at_edge);
		for(size_t i = 0; i < filter.size(); ++i)
		{
			double	x = i - filter.size()/2;
			filter[i] *= polar(1., tau()*carrier*x);
		}
		last_dispersion = dispersion;
		last_value_at_edge = value_at_edge;
		last_carrier = carrier;
	}

	filter.SetExtrapolationMethod(ex);
	Filter(filter);
}
#endif



//--------------------------------------------------------------
//
//	интерполяция с учетом характера осцилляции
//
template<class T, class ST>
auto ComplexFunction<T, ST>::in(double x, complex_function_oscillation osc) const -> value_type
{
	switch(osc)
	{
		case no_oscillation:
			return	inherited::in(x, &interpolators::sinc);
			break;
		case positive_oscillation:
			return	inherited::in(x, &interpolators::complex_sinc);
			break;
		case negative_oscillation:
			return	inherited::in(x, &interpolators::complex_sincT);
			break;
		default:
			throw invalid_argument(ssprintf("ComplexFunction<T,ST>::in -- invalid oscillation index = %d", osc));
	}
}

template<class T, class ST>
auto ComplexFunction<T, ST>::d_dx(double x, complex_function_oscillation osc) const -> value_type
{
	switch(osc)
	{
		case no_oscillation:
			return	inherited::in(x, &interpolators::sinc_derivative);
			break;
		case positive_oscillation:
			return	inherited::in(x, &interpolators::complex_sinc_derivative);
			break;
		case negative_oscillation:
			return	inherited::in(x, &interpolators::complex_sinc_derivativeT);
			break;
		default:
			throw invalid_argument(ssprintf("ComplexFunction<T,ST>::d_dx -- invalid oscillation index = %d", osc));
	}
}



//--------------------------------------------------------------

XRAD_END
