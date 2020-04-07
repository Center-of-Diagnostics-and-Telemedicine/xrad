#ifndef PixelNormalizersGUI_h__
#define PixelNormalizersGUI_h__

/********************************************************************
	created:	2015/03/12
	created:	12:3:2015   22:52
	author:		kns
*********************************************************************/

#include <PixelNormalizers.h>
#include <XRADBasic/Sources/Containers/DataArrayMD.h>
#include <XRADBasic/Sources/Utils/ImageUtils.h>

namespace XRAD_PixelNormalizers
{

// template<class T>
// 	double scalarize(const T&x){return double(x);}
//
// template<class T>
// 	double scalarize(const ComplexSample<T>&x){return cabs(x);}

enum color_rectifier_id_t
{
	color_rectifier_as_is,
	color_rectifier_fit_in_range_auto,
	color_rectifier_fit_in_range_manual,

	color_rectifier_lightness_as_is,
	color_rectifier_lightness_fit_in_range_auto,
	color_rectifier_lightness_fit_in_range_manual,
	color_rectifier_cancel,

	n_color_rectifiers
};


enum complex_rectifier_id_t
{
	complex_rectifier_linear_abs_auto,
	complex_rectifier_linear_abs_manual,

	complex_rectifier_real_auto,
	complex_rectifier_real_manual,

	complex_rectifier_imag_auto,
	complex_rectifier_imag_manual,

	complex_rectifier_logarithmic,
	complex_rectifier_cancel,

	n_complex_rectifiers
};

enum default_rectifier_id_t
{
	default_rectifier_linear_auto,
	default_rectifier_linear_manual,
	default_rectifier_linear_abs_auto,
	default_rectifier_linear_abs_manual,
	default_rectifier_logarithmic,
	default_rectifier_cancel,

	n_default_rectifiers
};

enum complex_data_extended_display_options
{
	display_real_part,
	display_imaginary_part,
	display_envelope,
	exit_complex_data_display,
	n_complex_data_extended_options
};

enum real_data_extended_display_options
{
	display_as_real_part,
	display_as_magnitude,
	display_as_radians_phase,
	display_as_degrees_phase,
	exit_real_data_display,
	n_real_data_extended_options
};

enum color_data_extended_display_options
{
	e_red_plate,
	e_green_plate,
	e_blue_plate,

	e_hue_plate,
	e_lightness_plate,
	e_saturation_plate,
	e_exit_color_data_display,

	n_color_data_extended_options
};


// inline double	DefaultDynamicRange(){return 60;}
inline double	MinDisplayTreshold(){ return 0.002; }
inline double	MaxDisplayTreshold(){ return 0.998; }
// TODO величины порогов продумать еще

#define	case_select(x) case x: return x

inline complex_rectifier_id_t GetComplexRectifierId()
{
	size_t	rectifier_type = Decide("Choose complex-to-grayscale transform", //n_complex_rectifiers,
	{"Linear, fit absolute values to range (auto)",
	"Linear, fit absolute values to range (manual)",

	"Real part, fit values to range (auto)",
	"Real part, fit values to range (manual)",

	"Imag part, fit values to range (auto)",
	"Imag part, fit values to range (manual)",
	"Logarithmic",

	"Cancel"}
	);
//TODO: Decide при первом вызове дает здесь 0 вместо последнего значения, если нажать esc. видимо, дело в самой функции. разобраться
	switch(rectifier_type)
	{
		case_select(complex_rectifier_linear_abs_auto);
		case_select(complex_rectifier_linear_abs_manual);

		case_select(complex_rectifier_real_auto);
		case_select(complex_rectifier_real_manual);

		case_select(complex_rectifier_imag_auto);
		case_select(complex_rectifier_imag_manual);

		case_select(complex_rectifier_logarithmic);

		case complex_rectifier_cancel:
			throw canceled_operation("complex rectifier selection canceled");

		default:
			ForceDebugBreak();
			FatalError(ssprintf("GetComplexRectifierId(), invalid option = %d", rectifier_type));
	}
}

inline color_rectifier_id_t GetColorRectifierId()
{
	size_t	rectifier_type = Decide("Choose color transform", //n_color_rectifiers,
	{
	"Display color as is",
	"Fit color values to range (auto)",
	"Fit color values to range (manual)",

	"Display lightness as is",
	"Fit lightness values to range (auto)",
	"Fit lightness values to range (manual)",
	"Cancel"
	});

	switch(rectifier_type)
	{
		case_select(color_rectifier_as_is);
		case_select(color_rectifier_fit_in_range_auto);
		case_select(color_rectifier_fit_in_range_manual);

		case_select(color_rectifier_lightness_as_is);
		case_select(color_rectifier_lightness_fit_in_range_auto);
		case_select(color_rectifier_lightness_fit_in_range_manual);

		case color_rectifier_cancel:
			throw canceled_operation("color rectifier selection canceled");

		default:
			ForceDebugBreak();
			FatalError(ssprintf("GetColorRectifierId(), invalid option = %d", rectifier_type));
	}
}


inline default_rectifier_id_t GetDefaultRectifierId()
{
	size_t	rectifier_type = Decide("Choose value-to-grayscale transform", //n_default_rectifiers,
	{
	"Linear, fit signed values to range (auto)",
	"Linear, fit signed values to range (manual)",
	"Linear, fit absolute values to range (auto)",
	"Linear, fit absolute values to range (manual)",
	"Logarithmic",
	"Cancel"
	});

	switch(rectifier_type)
	{
		case_select(default_rectifier_linear_auto);
		case_select(default_rectifier_linear_manual);
		case_select(default_rectifier_linear_abs_auto);
		case_select(default_rectifier_linear_abs_manual);
		case_select(default_rectifier_logarithmic);
		case default_rectifier_cancel:
			throw canceled_operation("default rectifier selection canceled");

		default:
			ForceDebugBreak();
			FatalError(ssprintf("GetDefaultRectifierId(), invalid option = %d", rectifier_type));
	}
}

inline complex_data_extended_display_options GetExtendedComplexDataDisplayOption(const wstring &title)
{
	size_t	option = Decide(convert_to_string(title, e_encode_literals), //n_complex_data_extended_options,
	{
		"Real part separately",
		"Imaginary part separately",
		"Envelope",
		"Exit display"
	});

	switch(option)
	{
		case_select(display_real_part);
		case_select(display_imaginary_part);
		case_select(display_envelope);
		case_select(exit_complex_data_display);
		default:
			ForceDebugBreak();
			FatalError(ssprintf("GetComplexDataDisplayOption(), invalid option = %d", option));
	}
}

inline real_data_extended_display_options GetExtendedRealDataDisplayOption(const wstring &title)
{
	size_t	option = Decide(convert_to_string(title, e_encode_literals), //n_real_data_extended_options,
	{
	"Display as a real part of complex signal",
	"Display as a magnitude of complex signal",
	"Display as a radians phase of complex signal",
	"Display as a degrees phase of complex signal",
	"Exit display"
	});

	switch(option)
	{
		case_select(display_as_real_part);
		case_select(display_as_magnitude);
		case_select(display_as_radians_phase);
		case_select(display_as_degrees_phase);
		case_select(exit_real_data_display);
		default:
			ForceDebugBreak();
			FatalError(ssprintf("GetRealDataDisplayOption(), invalid option = %d", option));
	}
}

inline color_data_extended_display_options GetExtendedColorDataDisplayOption(const wstring &title)
{
	size_t	option = Decide(convert_to_string(title, e_encode_literals), //n_color_data_extended_options,
	{
	"Red plate",
	"Green plate",
	"Blue plate",

	"Hue plate",
	"Lightness plate",
	"Saturation plate",

	"Exit display"
	});



	switch(option)
	{
		case_select(e_red_plate);
		case_select(e_green_plate);
		case_select(e_blue_plate);

		case_select(e_hue_plate);
		case_select(e_lightness_plate);
		case_select(e_saturation_plate);
		case_select(e_exit_color_data_display);

		default:
			ForceDebugBreak();
			FatalError(ssprintf("GetColorDataDisplayOption(), invalid option = %d", option));
	}
}


#undef case_select


inline void	GetDisplayRange(range1_F64 &result_range, double &gamma, const range1_F64& recommended_range, const range1_F64& absolute_range)
{
	result_range.x1() = GetFloating("Blackpoint value", recommended_range.x1(), absolute_range.p1(), absolute_range.p2(), out_of_range_allowed);
	result_range.x2() = GetFloating("Whitepoint value", recommended_range.x2(), result_range.x1(), absolute_range.p2(), out_of_range_allowed);
	gamma = GetFloating("Gamma value", gamma, 0., numeric_limits<double>::infinity());
}

template<class T, class PT>
assign_linear_abs_normalized<T, PT> GetComplexRectifierLinearAbs(complex_rectifier_id_t rectifier_type, const range1_F64& recommended_range, const range1_F64& absolute_range)
{
	if(rectifier_type != complex_rectifier_linear_abs_auto && rectifier_type != complex_rectifier_linear_abs_manual)
	{
		ForceDebugBreak();
		throw invalid_argument(ssprintf("GetComplexRectifierLinearAbs, invalid argument value = %d", int(rectifier_type)));
	}

	range1_F64 result_range = recommended_range;
	double	gamma = 1;

	if(rectifier_type == complex_rectifier_linear_abs_manual)
	{
		GetDisplayRange(result_range, gamma, recommended_range, absolute_range);
// 		result_range.x1() = GetFloating("Blackpoint value", result_range.x1(), absolute_range.p1(), absolute_range.p2(), out_of_range_allowed);
// 		result_range.x2() = GetFloating("Whitepoint value", result_range.x2(), result_range.x1(), absolute_range.p2(), out_of_range_allowed);
// 		gamma = GetFloating("Gamma value", gamma, 0, numeric_limits<double>::infinity()());
	}
	return	assign_linear_abs_normalized<T, PT>(result_range, gamma);
}

template<class T, class PT>
assign_linear_real_normalized<T, PT> GetComplexRectifierReal(complex_rectifier_id_t rectifier_type, const range1_F64& recommended_range, const range1_F64& absolute_range)
{
	if(rectifier_type != complex_rectifier_real_auto && rectifier_type != complex_rectifier_real_manual)
	{
		ForceDebugBreak();
		throw invalid_argument(ssprintf("GetComplexRectifierReal, invalid argument value = %d", int(rectifier_type)));
	}

	range1_F64 result_range = recommended_range;
	double	gamma = 1;

	if(rectifier_type == complex_rectifier_real_manual)
	{
		GetDisplayRange(result_range, gamma, recommended_range, absolute_range);
// 		result_range.x1() = GetFloating("Blackpoint value", result_range.x1(), absolute_range.p1(), absolute_range.p2(), out_of_range_allowed);
// 		result_range.x2() = GetFloating("Whitepoint value", result_range.x2(), result_range.x1(), absolute_range.p2(), out_of_range_allowed);
// 		gamma = GetFloating("Gamma value", gamma, 0, numeric_limits<double>::infinity()());
	}
	return	assign_linear_real_normalized<T, PT>(result_range, gamma);
}

template<class T, class PT>
assign_linear_imag_normalized<T, PT> GetComplexRectifierImag(complex_rectifier_id_t rectifier_type, const range1_F64& recommended_range, const range1_F64& absolute_range)
{
	if(rectifier_type != complex_rectifier_imag_auto && rectifier_type != complex_rectifier_imag_manual)
	{
		ForceDebugBreak();
		throw invalid_argument(ssprintf("GetComplexRectifierImag, invalid argument value = %d", int(rectifier_type)));
	}

	range1_F64 result_range = recommended_range;
	double	gamma = 1;

	if(rectifier_type == complex_rectifier_imag_manual)
	{
		GetDisplayRange(result_range, gamma, recommended_range, absolute_range);
// 		result_range.x1() = GetFloating("Blackpoint value", result_range.x1(), absolute_range.p1(), absolute_range.p2(), out_of_range_allowed);
// 		result_range.x2() = GetFloating("Whitepoint value", result_range.x2(), result_range.x1(), absolute_range.p2(), out_of_range_allowed);
// 		gamma = GetFloating("Gamma value", gamma, 0, numeric_limits<double>::infinity()());
	}
	return	assign_linear_imag_normalized<T, PT>(result_range, gamma);
}

inline	void	AdjustDynamicalRange(range1_F64 &recommended_range, const range1_F64 absolute_range)
{
// на входе динамический диапазон представляет собой логарифм от исходных значений.
// для запроса он смещается таким образом, чтобы 0 ДД соответствовал максимуму исходной
// картины. положительные числа определяют удаление от нуля вниз
	recommended_range.x1() = absolute_range.x2() - GetFloating("Dynamical range max",
															   absolute_range.x2() - recommended_range.x1(),
															   0,
															   absolute_range.delta(), out_of_range_allowed);

	recommended_range.x2() = absolute_range.x2() - GetFloating("Dynamical range min",
															   absolute_range.x2() - recommended_range.x2(),
															   0,
															   absolute_range.x2() - recommended_range.x1(), out_of_range_allowed);
}


template<class T, class PT>
assign_log_normalized<T, PT> GetComplexRectifierLogarithmic(complex_rectifier_id_t rectifier_type, const range1_F64& recommended_range, const range1_F64& absolute_range)
{
	if(rectifier_type != complex_rectifier_logarithmic)
	{
		ForceDebugBreak();
		throw invalid_argument(ssprintf("GetComplexRectifierLogarithmice, invalid argument value = %d", int(rectifier_type)));
	}
	range1_F64	adjusted_range(recommended_range);
	double gamma = 1;
	AdjustDynamicalRange(adjusted_range, absolute_range);

	return assign_log_normalized<T, PT>(adjusted_range, gamma);
}

template<class T, class PT>
assign_linear_normalized<T, PT> GetDefaultRectifierLinear(default_rectifier_id_t rectifier_type, const range1_F64& recommended_range, const range1_F64& absolute_range)
{
	if(rectifier_type != default_rectifier_linear_auto && rectifier_type != default_rectifier_linear_manual)
	{
		ForceDebugBreak();
		throw invalid_argument(ssprintf("GetDefaultRectifierLinear, invalid argument value = %d", int(rectifier_type)));
	}

	range1_F64 result_range = recommended_range;
	double	gamma = 1;

	if(rectifier_type == default_rectifier_linear_manual)
	{
		GetDisplayRange(result_range, gamma, recommended_range, absolute_range);
// 		result_range.x1() = GetFloating("Blackpoint value", result_range.x1(), absolute_range.p1(), absolute_range.p2(), out_of_range_allowed);
// 		result_range.x2() = GetFloating("Whitepoint value", result_range.x2(), result_range.x1(), absolute_range.p2(), out_of_range_allowed);
// 		gamma = GetFloating("Gamma value", gamma, 0, numeric_limits<double>::infinity()());
	}
	return assign_linear_normalized<T, PT>(result_range, gamma);
}

template<class T, class PT>
assign_linear_abs_normalized<T, PT> GetDefaultRectifierLinearAbs(default_rectifier_id_t rectifier_type, const range1_F64& recommended_range, const range1_F64& absolute_range)
{
	if(rectifier_type != default_rectifier_linear_abs_auto && rectifier_type != default_rectifier_linear_abs_manual)
	{
		ForceDebugBreak();
		throw invalid_argument(ssprintf("GetDefaultRectifierLinearAbs, invalid argument value = %d", int(rectifier_type)));
	}

	range1_F64 result_range = recommended_range;
	double	gamma = 1;

	if(rectifier_type == default_rectifier_linear_abs_manual)
	{
		GetDisplayRange(result_range, gamma, recommended_range, absolute_range);
// 		result_range.x1() = GetFloating("Blackpoint value", result_range.x1(), absolute_range.p1(), result_range.x2(), out_of_range_allowed);
// 		result_range.x2() = GetFloating("Whitepoint value", result_range.x2(), result_range.x1(), absolute_range.p2(), out_of_range_allowed);
// 		gamma = GetFloating("Gamma value", gamma, 0, numeric_limits<double>::infinity()());
	}
	return assign_linear_abs_normalized<T, PT>(result_range, gamma);
}



template<class T, class PT>
assign_log_normalized<T, PT> GetDefaultRectifierLog(default_rectifier_id_t rectifier_type, const range1_F64& recommended_range, const range1_F64& absolute_range)
{
	if(rectifier_type != default_rectifier_logarithmic)
	{
		ForceDebugBreak();
		throw invalid_argument(ssprintf("GetDefaultRectifierLog, invalid argument value = %d", int(rectifier_type)));
	}

	range1_F64	adjusted_range(recommended_range);
	double	gamma = 1;
	AdjustDynamicalRange(adjusted_range, absolute_range);

	return assign_log_normalized<T, PT>(adjusted_range, gamma);
}




template<class T, class PT>
assign_color_normalized<T, PT> GetColorRectifierRange(color_rectifier_id_t rectifier_type, const range1_F64& recommended_range, const range1_F64& absolute_range)
{
	if(rectifier_type != color_rectifier_fit_in_range_auto && rectifier_type != color_rectifier_fit_in_range_manual)
	{
		ForceDebugBreak();
		throw invalid_argument(ssprintf("GetColorRectifierRange, invalid argument value = %d", int(rectifier_type)));
	}

	range1_F64 result_range = recommended_range;
	double	gamma = 1;

	if(rectifier_type == color_rectifier_fit_in_range_manual)
	{
		GetDisplayRange(result_range, gamma, recommended_range, absolute_range);
// 		result_range.x1() = GetFloating("Blackpoint value", result_range.x1(), absolute_range.p1(), absolute_range.p2(), out_of_range_allowed);
// 		result_range.x2() = GetFloating("Whitepoint value", result_range.x2(), result_range.x1(), absolute_range.p2(), out_of_range_allowed);
// 		gamma = GetFloating("Gamma value", gamma, 0, numeric_limits<double>::infinity()());
	}
	return	assign_color_normalized<T, PT>(result_range, gamma);
}


template<class T, class PT>
assign_color_lightness_normalized<T, PT> GetColorLightnessRectifierRange(color_rectifier_id_t rectifier_type, const range1_F64& recommended_range, const range1_F64& absolute_range)
{
	if(rectifier_type != color_rectifier_lightness_fit_in_range_auto && rectifier_type != color_rectifier_lightness_fit_in_range_manual)
	{
		ForceDebugBreak();
		throw invalid_argument(ssprintf("GetColorLightnessRectifierRange, invalid argument value = %d", int(rectifier_type)));
	}

	range1_F64 result_range = recommended_range;
	double	gamma = 1;

	if(rectifier_type == color_rectifier_lightness_fit_in_range_manual)
	{
		GetDisplayRange(result_range, gamma, recommended_range, absolute_range);
// 		result_range.x1() = GetFloating("Blackpoint value", result_range.x1(), absolute_range.p1(), absolute_range.p2(), out_of_range_allowed);
// 		result_range.x2() = GetFloating("Whitepoint value", result_range.x2(), result_range.x1(), absolute_range.p2(), out_of_range_allowed);
// 		gamma = GetFloating("Gamma value", gamma, 0, numeric_limits<double>::infinity()());
	}
	return	assign_color_lightness_normalized<T, PT>(result_range, gamma);
}



template<class A2T, class F>
void	ComputeDisplayRanges(const DataArray2D<A2T> &image, range1_F64 &recommended_range, range1_F64 &absolute_range, const F& functor)
{
	using namespace XRAD_PixelNormalizers;

	absolute_range.x1() = MinValueTransformed(image, functor);
	absolute_range.x2() = MaxValueTransformed(image, functor);
	RealFunctionF64	histogram(10000);
	try
	{
		ComputeHistogramTransformed(image, histogram, absolute_range, functor);
		recommended_range = ComputeQuantilesRange(histogram, absolute_range, range1_F64(MinDisplayTreshold(), MaxDisplayTreshold()));
	}
	catch(...)
	{
		recommended_range = absolute_range;
	}
}

template<class A2T, class F>
void	ComputeDisplayRanges(const DataArrayMD<A2T> &image_md, range1_F64 &recommended_range, range1_F64 &absolute_range, const F& functor)
{
	using namespace XRAD_PixelNormalizers;

	absolute_range.x1() = MinValueTransformed(image_md, functor);
	absolute_range.x2() = MaxValueTransformed(image_md, functor);
	RealFunctionF64	histogram(10000);
	try
	{
		ComputeHistogramTransformed(image_md, histogram, absolute_range, functor);
		recommended_range = ComputeQuantilesRange(histogram, absolute_range, range1_F64(MinDisplayTreshold(), MaxDisplayTreshold()));
	}
	catch(...)
	{
		recommended_range = absolute_range;
	}
}


}
//namespace XRAD_PixelNormalizers

#endif // PixelNormalizersGUI_h__