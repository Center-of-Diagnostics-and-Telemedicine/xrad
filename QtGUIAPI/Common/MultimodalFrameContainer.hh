#include <XRADBasic/Sources/Containers/DataArrayAnalyze2D.h>

namespace XRAD_GUI
{
using namespace xrad;



inline void XRAD_GUI::MultimodalFrameContainer::GenerateBitmap(FrameBitmapContainer &bitmap, double min_value, double max_value, double gamma, bool transpose)
	{
	brightness_correction_functor<uint8_t> functor(min_value, max_value, gamma);

	switch(ContainerMode())
		{
		case rgba_sample_ui8:
			bitmap.SetFrameData(rgba_ui8, transpose, functor);
			break;

		case indexed_color_sample_ui8:
		case gray_sample_ui8:
			bitmap.SetFrameData(gray_ui8, transpose, functor);
			break;

		case gray_sample_i16:
			bitmap.SetFrameData(gray_i16, transpose, functor);
			break;

		case gray_sample_ui16:
			bitmap.SetFrameData(gray_ui16, transpose, functor);
			break;

		case gray_sample_i32:
			bitmap.SetFrameData(gray_i32, transpose, functor);
			break;

		case gray_sample_ui32:
			bitmap.SetFrameData(gray_ui32, transpose, functor);
			break;

		case gray_sample_f32:
			bitmap.SetFrameData(gray_f32, transpose, functor);
			break;

		case gray_sample_f64:
			bitmap.SetFrameData(gray_f64, transpose, functor);
			break;

		case rgb_sample_f32:
			bitmap.SetFrameData(rgb_f32, transpose, functor);
			break;

		case complex_sample_f32:
			bitmap.SetFrameData(complex_f32, transpose, functor);
			break;

		case complex_sample_f64:
			bitmap.SetFrameData(complex_f64, transpose, functor);
			break;

		case complex_sample_i32:
			bitmap.SetFrameData(complex_i32, transpose, functor);
			break;

		case complex_sample_i16:
			bitmap.SetFrameData(complex_i16, transpose, functor);
			break;
		}
	}


inline display_sample_type MultimodalFrameContainer::ContainerMode()
	{
	int	counter(0);

	if(rgba_ui8.ready()) ++counter;
	if(gray_ui8.ready()) ++counter;
	if(gray_ui16.ready()) ++counter;
	if(gray_i16.ready()) ++counter;
	if(gray_ui32.ready()) ++counter;
	if(gray_i32.ready()) ++counter;
	if(gray_f32.ready()) ++counter;
	if(gray_f64.ready()) ++counter;
	if(rgb_f32.ready()) ++counter;
	if(complex_f32.ready()) ++counter;
	if(complex_f64.ready()) ++counter;
	if(complex_i32.ready()) ++counter;
	if(complex_i16.ready()) ++counter;

	if(counter != 1)
		{
		ForceDebugBreak();
		throw invalid_argument("MultimodalFrameContainer::ContainerMode, invalide container mode");
		}

	if(rgba_ui8.ready())
		{
		return rgba_sample_ui8;
		}
	if(gray_ui8.ready())
		{
		return gray_sample_ui8;
		}
	if(gray_ui16.ready())
		{
		return gray_sample_ui16;
		}
	if(gray_i16.ready())
		{
		return gray_sample_i16;
		}
	if(gray_ui32.ready())
		{
		return gray_sample_ui32;
		}
	if(gray_i32.ready())
		{
		return gray_sample_i32;
		}
	if(gray_f32.ready())
		{
		return gray_sample_f32;
		}
	if(gray_f64.ready())
		{
		return gray_sample_f64;
		}
	if(rgb_f32.ready())
		{
		return rgb_sample_f32;
		}
	if(complex_f32.ready())
		{
		return complex_sample_f32;
		}
	if(complex_f64.ready())
		{
		return complex_sample_f64;
		}
	if(complex_i32.ready())
		{
		return complex_sample_i32;
		}
	if(complex_i16.ready())
		{
		return complex_sample_i16;
		}


	// попадание сюда логикой исключается, но во избежание предупреждений генерируем исключение
	ForceDebugBreak();
	throw invalid_argument("MultimodalFrameContainer::ContainerMode, invalide container mode");
	}

inline void MultimodalFrameContainer::ImportFrame( const void *in_data, int n_rows, int n_columns, display_sample_type sample_type )
	{
	if(sample_type != gray_sample_ui8 && sample_type != indexed_color_sample_ui8) gray_ui8.realloc(0,0);
	if(sample_type != rgba_sample_ui8) rgba_ui8.realloc(0,0);
	if(sample_type != gray_sample_i16) gray_i16.realloc(0,0);
	if(sample_type != gray_sample_ui16) gray_ui16.realloc(0,0);
	if(sample_type != gray_sample_i32) gray_i32.realloc(0,0);
	if(sample_type != gray_sample_ui32) gray_ui32.realloc(0,0);
	if(sample_type != gray_sample_f32) gray_f32.realloc(0,0);
	if(sample_type != gray_sample_f64) gray_f64.realloc(0,0);
	if(sample_type != rgb_sample_f32) rgb_f32.realloc(0,0);
	if(sample_type != complex_sample_f32) complex_f32.realloc(0,0);
	if(sample_type != complex_sample_f64) complex_f64.realloc(0,0);
	if(sample_type != complex_sample_i32) complex_i32.realloc(0,0);
	if(sample_type != complex_sample_i16) complex_i16.realloc(0,0);

	if(sample_type == gray_sample_ui8 || sample_type == indexed_color_sample_ui8)
		{
		gray_ui8.realloc(n_rows, n_columns);
		gray_ui8.CopyData(static_cast<const uint8_t*>(in_data));
		}
	if(sample_type == rgba_sample_ui8)
		{
		rgba_ui8.realloc(n_rows, n_columns);
		rgba_ui8.CopyData(static_cast<const ColorPixel*>(in_data));
		}
	if(sample_type == gray_sample_i16)
		{
		gray_i16.realloc(n_rows, n_columns);
		gray_i16.CopyData(static_cast<const int16_t*>(in_data));
		}
	if(sample_type == gray_sample_ui16)
		{
		gray_ui16.realloc(n_rows, n_columns);
		gray_ui16.CopyData(static_cast<const uint16_t*>(in_data));
		}
	if(sample_type == gray_sample_i32)
		{
		gray_i32.realloc(n_rows, n_columns);
		gray_i32.CopyData(static_cast<const int32_t*>(in_data));
		}
	if(sample_type == gray_sample_ui32)
		{
		gray_ui32.realloc(n_rows, n_columns);
		gray_ui32.CopyData(static_cast<const uint32_t*>(in_data));
		}
	if(sample_type == gray_sample_f32)
		{
		gray_f32.realloc(n_rows, n_columns);
		gray_f32.CopyData(static_cast<const float*>(in_data));
		}
	if(sample_type == gray_sample_f64)
		{
		gray_f64.realloc(n_rows, n_columns);
		gray_f64.CopyData(static_cast<const double*>(in_data));
		}
	if(sample_type == rgb_sample_f32)
		{
		rgb_f32.realloc(n_rows, n_columns);
		rgb_f32.CopyData(static_cast<const ColorSampleF32*>(in_data));
		}
	if(sample_type == complex_sample_f32)
		{
		complex_f32.realloc(n_rows, n_columns);
		complex_f32.CopyData(static_cast<const complexF32*>(in_data));
		}
	if(sample_type == complex_sample_f64)
		{
		complex_f64.realloc(n_rows, n_columns);
		complex_f64.CopyData(static_cast<const complexF64*>(in_data));
		}
	if(sample_type == complex_sample_i32)
		{
		complex_i32.realloc(n_rows, n_columns);
		complex_i32.CopyData(static_cast<const complexI32*>(in_data));
		}
	if(sample_type == complex_sample_i16)
		{
		complex_i16.realloc(n_rows, n_columns);
		complex_i16.CopyData(static_cast<const complexI16*>(in_data));
		}
	}

inline int display_value_order(double a){return a>1e3 ? log10(a) : log10(a)-1;}

inline string ComplexValueLegend(complexF64 v)
	{
	double	a = cabs(v);
	if(a && (a > 1e3 || a<1e-1))
		{
		int order = display_value_order(a);
		v /= pow(10., order);
		return ssprintf("[re=%.2f, im=%.2f, magn = %.2f]e%d, arg = %.2f", double(v.re), double(v.im), cabs(v), order, arg(v));
		}
	else
		{
		return ssprintf("re=%.2f, im=%.2f, magn = %.2f, arg = %.2f", double(v.re), double(v.im), cabs(v), arg(v));
		}
	}

template<class RGB_T>
inline string RGBValueLegend(RGBColorSample<RGB_T> v)
	{
	//TODO если сюда попадают отрицательные значения компонент rgb, возможны ошибки. разбираться
	double	a = fabs(lightness(v));
	if(a && (a > 1e3 || a<1e-1))
		{
		int order = display_value_order(a);
		v /= pow(10., order);
		return ssprintf("[r=%.2f, g=%.2f, b = %.2f, l=%.2f]e%d", double(v.red()), double(v.green()), double(v.blue()), double(lightness(v)), order);
		}
	else
		{
		return ssprintf("r=%.2f, g=%.2f, b = %.2f, l=%.2f", double(v.red()), double(v.green()), double(v.blue()), double(lightness(v)));
		}
	}


inline QString MultimodalFrameContainer::GetValueLegend( int row_no, int col_no )
	{
	string	result;
	switch(ContainerMode())
		{
		case rgba_sample_ui8:
			//result = ssprintf("r=%d, g=%d, b=%d", rgba_ui8.at(row_no, col_no).red(), rgba_ui8.at(row_no, col_no).green(), rgba_ui8.at(row_no, col_no).blue());
			result = RGBValueLegend(rgba_ui8.at(row_no, col_no));
			break;

		case indexed_color_sample_ui8:
		case gray_sample_ui8:
			result = ssprintf("%d", gray_ui8.at(row_no, col_no));
			break;

		case gray_sample_i16:
			result = ssprintf("%d", gray_i16.at(row_no, col_no));
			break;

		case gray_sample_ui16:
			result = ssprintf("%d", gray_ui16.at(row_no, col_no));
			break;

		case gray_sample_i32:
			result = ssprintf("%d", gray_i32.at(row_no, col_no));
			break;

		case gray_sample_ui32:
			result = ssprintf("%d", gray_ui32.at(row_no, col_no));
			break;

		case gray_sample_f32:
			result = ssprintf("%g", gray_f32.at(row_no, col_no));
			break;

		case gray_sample_f64:
			result = ssprintf("%g", gray_f64.at(row_no, col_no));
			break;

		case rgb_sample_f32:
// 			result = ssprintf("r=%g, g=%g, b=%g", rgb_f32.at(row_no, col_no).red(), rgb_f32.at(row_no, col_no).green(), rgb_f32.at(row_no, col_no).blue());
			result = RGBValueLegend(rgb_f32.at(row_no, col_no));
			break;

		case complex_sample_f32:
// 			result = ssprintf("re=%g, im=%g, magn = %g, arg = %g", complex_f32.at(row_no, col_no).re, complex_f32.at(row_no, col_no).im, cabs(complex_f32.at(row_no, col_no)), arg(complex_f32.at(row_no, col_no)));
			result = ComplexValueLegend(complex_f32.at(row_no, col_no));
			break;

		case complex_sample_f64:
// 			result = ssprintf("re=%g, im=%g, magn = %g, arg = %g", complex_f64.at(row_no, col_no).re, complex_f64.at(row_no, col_no).im, cabs(complex_f64.at(row_no, col_no)), arg(complex_f64.at(row_no, col_no)));
			result = ComplexValueLegend(complex_f64.at(row_no, col_no));
			break;

		case complex_sample_i32:
// 			result = ssprintf("re=%g, im=%g, magn = %g, arg = %g", double(complex_i32.at(row_no, col_no).re), double(complex_i32.at(row_no, col_no).im), cabs(complex_i32.at(row_no, col_no)), arg(complex_i32.at(row_no, col_no)));
			result = ComplexValueLegend(complex_i32.at(row_no, col_no));
			break;

		case complex_sample_i16:
//			result = ssprintf("re=%d, im=%d, magn = %g, arg = %g", double(complex_i16.at(row_no, col_no).re), double(complex_i16.at(row_no, col_no).im), cabs(complex_i16.at(row_no, col_no)), arg(complex_i16.at(row_no, col_no)));
			result = ComplexValueLegend(complex_i16.at(row_no, col_no));
			break;
		}
	return QString::fromStdString(result);
	}


inline double MultimodalFrameContainer::MaxComponentValue()
	{
	switch(ContainerMode())
		{
		case rgba_sample_ui8:
			return xrad::MaxComponentValue(rgba_ui8);
			break;

		case indexed_color_sample_ui8:
		case gray_sample_ui8:
			return xrad::MaxComponentValue(gray_ui8);
			break;

		case gray_sample_i16:
			return xrad::MaxComponentValue(gray_i16);
			break;

		case gray_sample_ui16:
			return xrad::MaxComponentValue(gray_ui16);
			break;

		case gray_sample_i32:
			return xrad::MaxComponentValue(gray_i32);
			break;

		case gray_sample_ui32:
			return xrad::MaxComponentValue(gray_ui32);
			break;

		case gray_sample_f32:
			return xrad::MaxComponentValue(gray_f32);
			break;

		case gray_sample_f64:
			return xrad::MaxComponentValue(gray_f64);
			break;

		case rgb_sample_f32:
			return xrad::MaxComponentValue(rgb_f32);
			break;

		case complex_sample_f32:
			return xrad::MaxComponentValue(complex_f32);
			break;

		case complex_sample_f64:
			return xrad::MaxComponentValue(complex_f64);
			break;

		case complex_sample_i32:
			return xrad::MaxComponentValue(complex_i32);
			break;

		case complex_sample_i16:
			return xrad::MaxComponentValue(complex_i16);
			break;
		default:
			throw invalid_argument("MultimodalFrameContainer::MaxComponentValue, invalid container mode");
		}
	}


inline double MultimodalFrameContainer::MinComponentValue()
	{
	switch(ContainerMode())
		{
		case rgba_sample_ui8:
			return xrad::MinComponentValue(rgba_ui8);
			break;

		case indexed_color_sample_ui8:
		case gray_sample_ui8:
			return xrad::MinComponentValue(gray_ui8);
			break;

		case gray_sample_i16:
			return xrad::MinComponentValue(gray_i16);
			break;

		case gray_sample_ui16:
			return xrad::MinComponentValue(gray_ui16);
			break;

		case gray_sample_i32:
			return xrad::MinComponentValue(gray_i32);
			break;

		case gray_sample_ui32:
			return xrad::MinComponentValue(gray_ui32);
			break;

		case gray_sample_f32:
			return xrad::MinComponentValue(gray_f32);
			break;

		case gray_sample_f64:
			return xrad::MinComponentValue(gray_f64);
			break;

		case rgb_sample_f32:
			return xrad::MinComponentValue(rgb_f32);
			break;

		case complex_sample_f32:
			return xrad::MinComponentValue(complex_f32);
			break;

		case complex_sample_f64:
			return xrad::MinComponentValue(complex_f64);
			break;

		case complex_sample_i32:
			return xrad::MinComponentValue(complex_i32);
			break;

		case complex_sample_i16:
			return xrad::MinComponentValue(complex_i16);
			break;
		default:
			throw invalid_argument("MultimodalFrameContainer::MinComponentValue, invalid container mode");
		}
	}



} //namespace XRAD_GUI
