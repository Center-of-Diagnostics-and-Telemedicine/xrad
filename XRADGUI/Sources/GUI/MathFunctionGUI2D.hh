#include "MathFunctionGUI.h"
#include "GraphSet.h"
#include "PixelNormalizersGUI.h"
#include <XRADBasic/SampleTypes.h>
#include <XRADBasic/Sources/Containers/DataArrayAnalyze2D.h>
#include <XRADBasic/Sources/Containers/ComplexArrayAnalyzeFunctors.h>
#include <XRADBasic/Sources/Containers/DataArrayHistogram2D.h>
#include <XRADBasic/Sources/ScanConverter/ScanConverter.h>

XRAD_BEGIN

//--------------------------------------------------------------

namespace MF2DInterfaceAuxiliaries
{
using namespace XRAD_PixelNormalizers;

//--------------------------------------------------------------
//
// Два вспомогательных функтора для анализа локальной статистики изображения.
// используются еще при отображении многомерного массива
//
// оба класса узкого назначения, не претендуют на использование где-либо кроме
// двух упомянутых мест.

// 1. "Покомпонентное перемножение". используется для вычисления стат. моментов
//	(раздельно вычисляется момент каждой компоненты изображения)
class componentwise_multiply_assign
{
public:
	template<class T1, class T2>
	void operator()(T1 &x, const T2 &y) const
	{
		size_t	m_n_components = n_components(T1());

		for(size_t i = 0; i < m_n_components; ++i) component(x, i) *= component(y, i);
	}
};

// 2. "Покомпонентное возведение в степень".
// вычисление корня n степени из n-го момента. учитывается, что четные моменты положительны,
// а корень нечетной степени отрицательного числа тоже определен.
// для названного узкого назначения работает корректно, в других местах не использовать.
template<class T1, class T2=T1>
class componentwise_signed_pow
{
	const size_t	m_n_components;
	const double	power;//9

public:
	componentwise_signed_pow(double p) : m_n_components(n_components(T1())), power(p){};

// 	T1 &operator()(T1 &x, const T2 &y) const
// 	{
// 		for(size_t i = 0; i < m_n_components; ++i) component(x, i) = sign(component(y, i)) * pow(fabs(component(y, i)), power);
// 		return x;
// 	}

	T1 &operator()(T1 &x) const
	{
		for(size_t i = 0; i < m_n_components; ++i) component(x, i) = sign(component(x, i)) * pow(fabs(component(x, i)), power);
		return x;
	}

};

//--------------------------------------------------------------
//
//	отображение растра
//
//--------------------------------------------------------------

template<class image_t>
struct RasterWithLegend : public image_t
{
// сформированный растр и легенды по осям
	axis_legend ylegend, xlegend;
	RasterWithLegend() : ylegend(0, 1, L"cm"), xlegend(0, 1, L"cm"){}
};

template<class image_t>
void PrepareImageDisplaySC(RasterWithLegend<image_t> &raster, const image_t &image, const ScanConverterOptions &sco, const value_legend &vlegend)
{
	static	bool	transpose = false;
	static	bool	draw_grid = false;
	static	bool	fixed_image_height = false;

	if(!GetCheckboxDecision("Display options", //3,
			{"Transpose data?", "Add grid?", "Fixed height=512pix"},
			{&transpose, &draw_grid, &fixed_image_height}
			)) return;


	typedef typename image_t::value_type sample_t;
	ScanConverter<image_t> SC(image.vsize(), image.hsize());
	typedef	typename ScanConverter<image_t>::converted_sample_type scsample_t;
	SC.CopyScanConverterOptions(sco);
	CopyData(SC, image);

	SC.SetBackground(scsample_t(vlegend.display_range.p1()));
	if(draw_grid)
	{
		physical_length	grid_step = cm(GetFloating("Grid step (cm)", 1, 0.01, infinity()));
		SC.SetGrid(draw_grid, scsample_t(vlegend.display_range.p2()), grid_step);
	}

	if(transpose) SC.transpose();
	if(fixed_image_height)
	{
		size_t	image_height = GetUnsigned("Image height", 512, 16, max_long());
		SC.InitScanConverter(image_height);
	}
	else
	{
		SC.InitScanConverter();
	}

	SC.BuildConvertedImage();
	physical_length	vmin, vmax, hmin, hmax;
	SC.GetRasterDimensions(vmin, vmax, hmin, hmax);

	raster.MakeCopy(SC.GetConvertedImage());
	raster.ylegend = axis_legend(vmin.cm(), (vmax-vmin).cm()/raster.vsize(), L"cm");
	raster.xlegend = axis_legend(hmin.cm(), (hmax-hmin).cm()/raster.hsize(), L"cm");
}



template<class image_t>
void PrepareRasterSelector(RasterWithLegend<image_t> &raster, const image_t &image,
		const ScanConverterOptions &sco,
		const axis_legend &ylegend,
		const axis_legend &xlegend,
		const value_legend &vlegend)
{
	bool	no_scan_converter = sco.depth_range().cm() ? false : true;

	if(no_scan_converter)
	{
		raster.MakeCopy(image);
		raster.ylegend = ylegend;
		raster.xlegend = xlegend;
	}
	else
	{
		PrepareImageDisplaySC(raster, image, sco, vlegend);
	}
}

template<class image_t>
void DisplayRasterSelector(const image_t &image,
		wstring title,
		const ScanConverterOptions &sco,
		const axis_legend &ylegend,
		const axis_legend &xlegend,
		const value_legend &vlegend)
{
	RasterWithLegend<image_t>	raster;
	PrepareRasterSelector(raster, image, sco, ylegend, xlegend, vlegend);
	DisplayImage(title, raster.data(),
			raster.vsize(), raster.hsize(),
			raster.ylegend, raster.xlegend,
			vlegend);
}

template<class DISPLAY_TAG>
struct SelectRectifierAndDisplayRaster;

template<>
struct SelectRectifierAndDisplayRaster<DisplayTagScalar>
{
	template<class A2DT>
	static void	Display(const A2DT &image,
			const wstring title,
			axis_legend ylegend,
			axis_legend xlegend,
			value_legend vlegend,
			const ScanConverterOptions &sco)
	{
		typedef typename A2DT::value_type value_type;
		vlegend = value_legend(0, 255, 1, "");
		using namespace XRAD_PixelNormalizers;
		default_rectifier_id_t	rectifier_id = GetDefaultRectifierId();


		switch(rectifier_id)
		{
			case default_rectifier_linear_auto:
			case default_rectifier_linear_manual:
			{
				range1_F64	recommended_range, absolute_range;
				ComputeDisplayRanges(image, recommended_range, absolute_range, Functors::identity());

				if(rectifier_id == default_rectifier_linear_manual)
				{
					GetDisplayRange(vlegend.display_range, vlegend.gamma, recommended_range, absolute_range);
				}
				else vlegend.display_range = recommended_range;

				RealFunction2D_F32	pixmap_data(image);
				DisplayRasterSelector(pixmap_data,
										title, sco,
										ylegend, xlegend, vlegend);
			}
			break;
			case default_rectifier_linear_abs_auto:
			case default_rectifier_linear_abs_manual:
			{
				RealFunction2D_F32 pixmap_data;
				range1_F64	recommended_range, absolute_range;
				ComputeDisplayRanges(image, recommended_range, absolute_range, Functors::absolute_value());
				if(rectifier_id == default_rectifier_linear_abs_manual)
				{
					GetDisplayRange(vlegend.display_range, vlegend.gamma, recommended_range, absolute_range);
				}
				else vlegend.display_range = recommended_range;
				pixmap_data.MakeCopy(image, Functors::assign_f1(Functors::absolute_value()));
				DisplayRasterSelector(pixmap_data,
										title, sco,
										ylegend, xlegend, vlegend);
			}
			break;
			case default_rectifier_logarithmic:
			{
				range1_F64	recommended_range, absolute_range;
				ComputeDisplayRanges(image, recommended_range, absolute_range, Functors::amplitude_to_decibel_value());
				vlegend.display_range = recommended_range;
				AdjustDynamicalRange(vlegend.display_range, absolute_range);
				vlegend.label = L"dB";

				RealFunction2D_F32 pixmap_data;
				pixmap_data.MakeCopy(image, Functors::assign_f1(Functors::amplitude_to_decibel_value()));
				DisplayRasterSelector(pixmap_data,
										title, sco,
										ylegend, xlegend, vlegend);
			}
			break;
		}
	}
};

template<>
struct SelectRectifierAndDisplayRaster<DisplayTagComplex>
{
	template<class A2DT>
	static void	Display(const A2DT &image,
											wstring title,
											axis_legend ylegend,
											axis_legend xlegend,
											value_legend vlegend,
											const ScanConverterOptions &sco)
	{
		ylegend;
		xlegend;
		sco;
		typedef typename A2DT::value_type value_type;

		vlegend = value_legend(0, 255, 1, "");

		using namespace XRAD_PixelNormalizers;
		complex_rectifier_id_t rectifier_id = GetComplexRectifierId();

		switch(rectifier_id)
		{
			case complex_rectifier_linear_abs_auto:
			case complex_rectifier_linear_abs_manual:
			{
				range1_F64	recommended_range, absolute_range;
				ComputeDisplayRanges(image, recommended_range, absolute_range, Functors::absolute_value());
				if(rectifier_id == complex_rectifier_linear_abs_manual)
					GetDisplayRange(vlegend.display_range, vlegend.gamma, recommended_range, absolute_range);
				else
					vlegend.display_range = recommended_range;
				RealFunction2D_F32 pixmap_data;
				MakeCopy(pixmap_data, image, Functors::assign_f1(Functors::absolute_value()));
				DisplayRasterSelector(pixmap_data,
										title, sco,
										ylegend, xlegend, vlegend);
			}
			break;

			case complex_rectifier_real_auto:
			case complex_rectifier_real_manual:
			{
				range1_F64	recommended_range, absolute_range;
				ComputeDisplayRanges(image, recommended_range, absolute_range, Functors::real_part());
				if(rectifier_id == complex_rectifier_real_manual)
					GetDisplayRange(vlegend.display_range, vlegend.gamma, recommended_range, absolute_range);
				else
					vlegend.display_range = recommended_range;
				RealFunction2D_F32 pixmap_data;
				MakeCopy(pixmap_data, image, Functors::assign_f1(Functors::real_part()));
				DisplayRasterSelector(pixmap_data,
										title, sco, ylegend, xlegend, vlegend);
			}
			break;

			case complex_rectifier_imag_auto:
			case complex_rectifier_imag_manual:
			{
				range1_F64	recommended_range, absolute_range;
				ComputeDisplayRanges(image, recommended_range, absolute_range, Functors::imag_part());
				if(rectifier_id == complex_rectifier_imag_manual)
					GetDisplayRange(vlegend.display_range, vlegend.gamma, recommended_range, absolute_range);
				else
					vlegend.display_range = recommended_range;
				RealFunction2D_F32 pixmap_data;
				MakeCopy(pixmap_data, image, Functors::assign_f1(Functors::imag_part()));
				DisplayRasterSelector(pixmap_data,
										title, sco,
										ylegend, xlegend, vlegend);
			}
			break;

			case complex_rectifier_logarithmic:
			{
				range1_F64	recommended_range, absolute_range;
				ComputeDisplayRanges(image, recommended_range, absolute_range, Functors::amplitude_to_decibel_value());
				vlegend.display_range = recommended_range;
				AdjustDynamicalRange(vlegend.display_range, absolute_range);
				vlegend.label = L"dB";

				RealFunction2D_F32 pixmap_data;
				pixmap_data.MakeCopy(image, Functors::assign_f1(Functors::amplitude_to_decibel_value()));

				DisplayRasterSelector(pixmap_data,
										title, sco,
										ylegend, xlegend, vlegend);
			}
			break;
		}
	}
};

template<>
struct SelectRectifierAndDisplayRaster<DisplayTagRGB>
{
	template<class A2DT>
	static void	Display(const A2DT &image,
											wstring title,
											axis_legend ylegend,
											axis_legend xlegend,
											value_legend vlegend,
											const ScanConverterOptions &sco)
	{
		typedef typename A2DT::value_type value_type;

		vlegend = value_legend(0, 255, 1, "");

		using namespace XRAD_PixelNormalizers;
		color_rectifier_id_t rectifier_id = GetColorRectifierId();

		switch(rectifier_id)
		{
			case color_rectifier_as_is:
			case color_rectifier_fit_in_range_auto:
			case color_rectifier_fit_in_range_manual:
			{
				if(rectifier_id == color_rectifier_as_is)
				{
					vlegend.display_range = range1_F64(0, 255);
					vlegend.gamma = 1;
				}
				else
				{
					range1_F64	recommended_range, absolute_range;
					range1_F64	recommended_range_r, absolute_range_r;
					range1_F64	recommended_range_g, absolute_range_g;
					range1_F64	recommended_range_b, absolute_range_b;
					ComputeDisplayRanges(image, recommended_range_r, absolute_range_r, Functors::red_functor());
					ComputeDisplayRanges(image, recommended_range_g, absolute_range_g, Functors::green_functor());
					ComputeDisplayRanges(image, recommended_range_b, absolute_range_b, Functors::blue_functor());

					recommended_range.x1() = vmin(recommended_range_r.x1(), recommended_range_g.x1(), recommended_range_b.x1());
					recommended_range.x2() = vmax(recommended_range_r.x2(), recommended_range_g.x2(), recommended_range_b.x2());
					absolute_range.x1() = vmin(absolute_range_r.x1(), absolute_range_g.x1(), absolute_range_b.x1());
					absolute_range.x2() = vmax(absolute_range_r.x2(), absolute_range_g.x2(), absolute_range_b.x2());

					if(rectifier_id == color_rectifier_fit_in_range_manual)
					{
						GetDisplayRange(vlegend.display_range, vlegend.gamma, recommended_range, absolute_range);
					}
					else
					{
						vlegend.display_range = recommended_range;
						vlegend.gamma = 1;
					}
				}

				ColorImageF32	pixmap_data(image);
				DisplayRasterSelector(pixmap_data,
										title, sco,
										ylegend, xlegend, vlegend);
			}
			break;

			case color_rectifier_lightness_as_is:
			{
				RealFunction2D_F32 pixmap_data;
				pixmap_data.MakeCopy(image, Functors::assign_f1(Functors::lightness_functor()));
				DisplayRasterSelector(pixmap_data,
										title, sco,
										ylegend, xlegend, vlegend);
			}
			break;

			case color_rectifier_lightness_fit_in_range_auto:
			case color_rectifier_lightness_fit_in_range_manual:
			{
				range1_F64	recommended_range, absolute_range;
				ComputeDisplayRanges(image, recommended_range, absolute_range, Functors::lightness_functor());
				GetDisplayRange(vlegend.display_range, vlegend.gamma, recommended_range, absolute_range);
				RealFunction2D_F32 pixmap_data;
				MakeCopy(pixmap_data, image, Functors::assign_f1(Functors::lightness_functor()));
				DisplayRasterSelector(pixmap_data,
										title, sco,
										ylegend, xlegend, vlegend);
				break;
			}
		}
	}
};




//--------------------------------------------------------------
template<class DISPLAY_TAG>
struct DisplayHistogram2D;

template<>
struct DisplayHistogram2D<DisplayTagScalar>
{
	template<class A2DT>
	static void Display(const A2DT &image, const wstring &title, size_t in_moments_sizes)
	{
		using namespace XRAD_PixelNormalizers;
		typedef typename A2DT::value_type value_type;

		size_t	histogram_type = Decide("Histogram type",
			{"Linear", "Log. absolute value"});
		size_t	data_count = image.vsize()*image.hsize();
		size_t	n = GetUnsigned("Histogram size", int(sqrt(double(data_count))), 3, data_count);
		RealFunctionF64	histogram(n);

		switch(histogram_type)
		{
			case 0:
			{
				range1_F64	absolute_range(MinValue(image), MaxValue(image));
				if(absolute_range.delta()>0)
				{

					ComputeHistogram(image, histogram, absolute_range);
					double	step = absolute_range.delta() / n;
					vector<double> moments(in_moments_sizes, 0);

					if(!CapsLock())
					{
	//					moments[0] = AverageValue(image);
						for (size_t i1 = 0; i1 < image.vsize(); ++i1)
						{
							for (size_t j1 = 0; j1 < image.hsize(); ++j1)
							{
								moments[0] += image.at(i1,j1);
							}
						}
						moments[0] /= (image.vsize()*image.hsize());
						for (size_t j = 1; j <= moments.size() - 1; ++j)
						{
							for (size_t k = 0; k < image.vsize(); ++k)
							{
								for (size_t l = 0; l < image.hsize(); ++l)
								{
									moments[j] += pow(double(image.at(k,l) - moments[0]), double(j + 1));
								}
							}
							moments[j] /= image.vsize()*image.hsize();
						}
					}

					else
					{
						for (size_t i = 0; i < histogram.size() - 1; ++i)
						{
							moments[0] += histogram[i] * (absolute_range.p1() + i*step + step / 2);
						}
						for (size_t j = 1; j <= moments.size() - 1; ++j)
						{
							for (size_t k = 0; k < histogram.size() - 1; ++k)
							{
								moments[j] += histogram[k] * pow(double(absolute_range.p1() + k*step + step / 2 - moments[0]), double(j + 1));
							}
						}
					}

					for (size_t j = 1; j <= moments.size() - 1; ++j)
					{
						moments[j] = pow(abs(moments[j]), (1./(j+1))) * sign(moments[j]);
					}
					string moments_title = " calculated moments:  ";
					for (size_t l = 0; l <= moments.size() - 1; ++l)
					{
						moments_title += ssprintf("m%d = %g; ", l+1, moments[l]);
					}
					DisplayMathFunction(histogram, absolute_range.p1() + step/2, step, "Histogram" + moments_title, "probability", "value");
				}
				else
				{
					ShowFloating("Function takes the only value:", absolute_range.x1());
				}
			}
			break;

			case 1:
			{
				range1_F64	recommended_range, absolute_range;
				ComputeDisplayRanges(image, recommended_range, absolute_range, Functors::amplitude_to_decibel_value());

				AdjustDynamicalRange(recommended_range, absolute_range);
				ComputeHistogramTransformed(image, histogram, recommended_range, Functors::amplitude_to_decibel_value());
				double	step = recommended_range.delta()/n;

				DisplayMathFunction(histogram, recommended_range.x1() + step/2,
									step, title + L"Histogram (log. abs)", L"probability", L"dB");
			}
			break;
		};

	}
};

template<>
struct DisplayHistogram2D<DisplayTagComplex>
{
	template<class A2DT>
	static void Display(const A2DT &image, const wstring &title, size_t /*in_moments_sizes*/)
	{
		using namespace XRAD_PixelNormalizers;
		typedef	typename A2DT::value_type complex_type;

		size_t	histogram_type = Decide("Histogram type", {"Real and imaginary separately", "Absolute value", "Log. absolute value"});
		size_t	data_count = image.vsize()*image.hsize();
		size_t	n = GetUnsigned("Histogram size", int(sqrt(double(data_count))), 3, data_count);


		switch(histogram_type)
		{
			case 0:
			{
				double	maxval = max(MaxValue(real(image)), MaxValue(imag(image)));
				double	minval = min(MinValue(real(image)), MinValue(imag(image)));

				RealFunction2D_F64	histogram(n_components(complex_type()), n);
				//TODO тоже привести диапазон в структурный вид
				ComputeComponentsHistogram(image, histogram, range1_F64(minval, maxval));
				double	step = (maxval-minval)/n;

				GraphSet	gs(title + L" Histogram", L"probability", L"value");
				gs.AddGraphUniform(histogram.row(0), minval+step/2, step, L"real part");
				gs.AddGraphUniform(histogram.row(1), minval+step/2, step, L"imag part");
				gs.Display();
			}
			break;

			case 1:
			{
				RealFunctionF64	histogram(n);
				range1_F64	absolute_range(cabs(MinValue(image)), cabs(MaxValue(image)));
				ComputeHistogramTransformed(image, histogram, absolute_range, Functors::absolute_value());

				double	step = absolute_range.delta()/n;

				DisplayMathFunction(histogram, absolute_range.x1() + step/2, step, title + L"Histogram (abs)", L"probability", L"abs. value");
			}
			break;

			case 2:
			{
				range1_F64	recommended_range, absolute_range;
				ComputeDisplayRanges(image, recommended_range, absolute_range, Functors::amplitude_to_decibel_value());

				AdjustDynamicalRange(recommended_range, absolute_range);
				RealFunctionF64	histogram(n);
				ComputeHistogramTransformed(image, histogram, recommended_range, Functors::amplitude_to_decibel_value());

				double	step = recommended_range.delta()/n;

				DisplayMathFunction(histogram, recommended_range.x1() + step/2,
									step, title + L"Histogram (log. abs)", L"probability", L"dB");
			}
			break;
		}
	}
};

template<>
struct DisplayHistogram2D<DisplayTagRGB>
{
	template<class A2DT>
	static void Display(const A2DT &image, const wstring &title, size_t /*in_moments_sizes*/)
	{
		using namespace XRAD_PixelNormalizers;
		typedef typename A2DT::value_type value_type;
		size_t	histogram_type = Decide("Histogram type", {"RGB components separately", "Lightness"});
		size_t	data_count = image.vsize()*image.hsize();
		size_t	n = GetUnsigned("Histogram size", int(sqrt(double(data_count))), 3, data_count);

		switch(histogram_type)
		{
			case 0:
			{
				double	maxval = MaxComponentValue(image);//max(MaxValue(real(image)), MaxValue(imag(image)));
				double	minval = MinComponentValue(image);//min(MinValue(real(image)), MinValue(imag(image)));

				RealFunction2D_F64	histogram(n_components(value_type()), n);
				//TODO тоже привести диапазон в структурный вид
				ComputeComponentsHistogram(image, histogram, range1_F64(minval, maxval));
				double	step = (maxval-minval)/n;

				GraphSet	gs(title + L" 'histogram'", L"probability", L"value");
				gs.AddGraphUniform(histogram.row(1), minval+step/2, step, L"green");
				gs.AddGraphUniform(histogram.row(2), minval+step/2, step, L"blue");
				gs.AddGraphUniform(histogram.row(0), minval+step/2, step, L"red");
				gs.Display();
			}
			break;

			case 1:
			{
				RealFunctionF64	histogram(n);
				range1_F64	absolute_range(lightness(MinValue(image)), lightness(MaxValue(image)));
				ComputeHistogramTransformed(image, histogram, absolute_range, Functors::lightness_functor());

				double	step = absolute_range.delta()/n;

				DisplayMathFunction(histogram, absolute_range.x1() + step/2, step, title + L"Histogram (abs)", L"probability", L"abs. value");
			}
			break;
		}
	}
};

template<class A2DT>
void GetFragment2D(const A2DT &image, range2_ST	&coordinatesXY, typename A2DT::variable &fragment)
{
	XRAD_ASSERT_THROW(coordinatesXY.delta().y() == fragment.vsize() && coordinatesXY.delta().x() == fragment.hsize());

	for (size_t i = 0; i < fragment.vsize(); ++i)
	{
		for (size_t j = 0; j < fragment.hsize(); ++j)
		{
			ptrdiff_t image_i = coordinatesXY.y1() + i;
			ptrdiff_t image_j = coordinatesXY.x1() + j;
			if (in_range(image_i, 0, image.vsize() - 1) && in_range(image_j, 0, image.hsize() - 1))
			{
				fragment.at(i,j) = image.at(image_i,image_j);
			}
		}
	}
}

inline void GetFragmentSizes2D(const point2_ST &full_size, range2_ST	&coordinatesXY)
{
	coordinatesXY.y1() = GetUnsigned("Coordanate 1 first:", 0, 0, full_size.y()-1);
	coordinatesXY.y2() = GetUnsigned("Coordanate 1 last:", full_size.y(), coordinatesXY.y1(), full_size.y());
	coordinatesXY.x1() = GetUnsigned("Coordanate 2 first:", 0, 0, full_size.x() - 1);
	coordinatesXY.x2() = GetUnsigned("Coordanate 2 last:", full_size.x(), coordinatesXY.x1(), full_size.x());
}

template<class A2DT>
void DisplayFragment2D(const A2DT &image, range2_ST	&coordinatesXY, const wstring row_title)
{
	typename A2DT::variable image_fragment(coordinatesXY.delta().y(), coordinatesXY.delta().x(), zero_value(image.at(0,0)));
	GetFragment2D(image, coordinatesXY, image_fragment);
	DisplayMathFunction2D(image_fragment, row_title);
}

//TODO объедить с ReducedWidth, объявленным в ScanConverter.h, и сделать отдельный файл с такими аналогами
// при фильтрации целочисленных отсчетов часто происходят потери данных либо переполнения.
// поэтому для анализа размытых данных копируем в буфер с аналогичным типом с плавающей запятой.
// ради этого вся конструкция ниже. потом перенести куда-то уровнем повыше
template<class T> struct FloatingExtension {typedef T value_type;};

#define deffextension1(T) template<> struct FloatingExtension<T> {typedef float value_type;};

deffextension1(int32_t);
deffextension1(int16_t);
deffextension1(int8_t);
deffextension1(uint32_t);
deffextension1(uint16_t);
deffextension1(uint8_t);

#undef deffextension1
// дальше то же самое для цветных и комплексных

template<class DISPLAY_TAG>
struct DisplayExtended;



template<class A2DT>
void	DisplayMathFunction2DTemplate(const A2DT &img, const wstring &title, const ScanConverterOptions &sco)
{
	typedef A2DT	image_type;
	typedef DisplayTag_t<decltype(img.at(0,0))> display_tag;
	// универсальный шаблон и для действительных, и для комплексных данных
	size_t	row_no = img.vsize()/2;
	//size_t	first_row_no = img.vsize() / 2;
	//size_t	last_row_no = img.vsize() / 2;
	range2_ST	coordinatesXY;
	double	mean_h = 5;
	double	mean_v = 5;
	size_t	col_no = img.hsize()/2;
//	size_t	first_col_no = img.hsize() / 2;
//	size_t	last_col_no = img.hsize() / 2;
//	size_t	fragment_size = 50;
	size_t in_moments_size = 5;

	bool	no_scan_converter = sco.depth_range().cm() ? false : true;

	wstring	lateral_label;
	double	lateral_start, lateral_increment;
	double	axial_start, axial_increment;
	wstring	axial_label;

	if(no_scan_converter)
	{
		axial_start = 0;
		axial_increment = 1;
		axial_label = L"columns";
		lateral_start = 0;
		lateral_increment = 1;
		lateral_label = L"rows";
	}
	else
	{
		axial_increment = sco.depth_range().cm()/img.hsize();
		axial_label = L"cm";
		axial_start = 0;
		//if(sco.angle_range().radians()) axial_start = sco.scanning_trajectory_length().cm()/sco.angle_range().radians();
		// отсчет глубины идет от поверхности датчика. выше вариант, когда это происходит от центра кривизны его.
		// так менее удобно, но может пригодиться тоже. подумать

		if(sco.angle_range().degrees())
		{
			lateral_start = sco.start_angle().degrees();
			lateral_increment = sco.angle_range().degrees()/img.vsize();
			lateral_label = L"degrees";
		}
		else
		{
			lateral_start = 0;
			lateral_increment = sco.scanning_trajectory_length().cm()/img.vsize();
			lateral_label = L"cm";
		}
	}

	size_t	answer;

	enum
	{
		e_raster_scan_converter,
		e_raster_as_is,
		e_row,
		e_col,
		e_local_statistics,
		e_fragment,
		e_histogram,
		e_extended,
		e_exit
	};

	do
	{
		if(no_scan_converter)
		{
			answer = GetButtonDecision(convert_to_string(title),//e_exit,
				{
					"Raster 1:1",
					"Row",
					"Column",
					"Local statistics",
					"Fragment",
					"Histogram",
					"Extended options",
					"Exit"
				}) + 1;
		}
		else
		{
			answer = GetButtonDecision(convert_to_string(title), //e_exit+1,
			{
				"Raster (Scan Converter)",
				"Raster 1:1",
				"Row",
				"Column",
				"Local statistics",
				"Fragment",
				"Histogram",
				"Extended options",
				"Exit"
			});
		}

		try
		{
			switch(answer)
			{
				case e_row:
				{
					row_no = GetUnsigned("Row to display:", row_no, 0, img.vsize()-1);
					wstring	row_title = no_scan_converter ? ssprintf(L"Row # %d", row_no) : ssprintf(L"Row #%d at %g %Ls", row_no, lateral_start + row_no*lateral_increment, lateral_label.c_str());

					DisplayMathFunction(img.row(row_no), axial_start, axial_increment, row_title, L"signal", axial_label);
				}
				break;


				case e_local_statistics:
				{
					typedef  floating32_type<typename A2DT::value_type> mean_value_type;
					typedef MathFunction2D<MathFunction<mean_value_type, double, typename A2DT::field_tag>> mean_values_buffer_type;
					mean_values_buffer_type	m1(img);

					size_t	moment_no = GetUnsigned("Moment no", 1, 1, 10);
					size_t	smooth_type = GetButtonDecision("Select averaging filter", {"Gaussian", "Rect"});
					mean_v = GetFloating("Meaning window height:", mean_v, 1, img.hsize());
					mean_h = GetFloating("Meaning window width:", mean_h, 1, img.vsize());
					point_2<FilterKernelReal, double, FilterKernelReal::field_tag> filters;
					wstring	legend = title + ssprintf(L"<M%d; %dg%g>", int(moment_no), mean_v, mean_h);

					switch(smooth_type)
					{
						// выбирается фиксированная дисперсия гауссова окна.
						// при необходимости можно выбрать квадратное, в этом случае оно подбирается так, чтобы
						// взвешенная ширина его была такая же, как у гауссова
						default:
						case 0:
							InitFIRFilterGaussian(filters.y(), double(mean_v), 0.01);
							InitFIRFilterGaussian(filters.x(), double(mean_h), 0.01);
							break;
						case 1:
						{
							size_t	vs = mean_v*sqrt(12.);
							size_t	hs = mean_h*sqrt(12.);
							filters.y().realloc(vs, 1./vs);
							filters.y().SetSmoothingKernelNormalizer();
							filters.x().realloc(hs, 1./hs);
							filters.x().SetSmoothingKernelNormalizer();
						}
							break;
					}
					FilterArray2DSeparate(m1, filters);

					if(moment_no == 1)
					{
						DisplayMathFunction2D(m1, legend, sco);
					}
					else
					{
						mean_values_buffer_type	centered(img);
						centered -= m1;
						mean_values_buffer_type moment(centered);
						for(size_t m = 1; m < moment_no; ++m)
							Apply_AA_2D_F2(moment, centered, MF2DInterfaceAuxiliaries::componentwise_multiply_assign());

						FilterArray2DSeparate(moment, filters);
						ApplyFunction(moment, MF2DInterfaceAuxiliaries::componentwise_signed_pow<mean_value_type>(1./moment_no));

						DisplayMathFunction2D(moment, legend, sco);
					}
				}
				break;

				case e_fragment:
				{
					GetFragmentSizes2D(point2_ST(img.vsize(), img.hsize()), coordinatesXY);

					wstring	row_title = ssprintf(L"Image fragment (%d, %d) (%d, %d)", coordinatesXY.y1(), coordinatesXY.x1(), coordinatesXY.y2(), coordinatesXY.x2());

					DisplayFragment2D(img, coordinatesXY, row_title);
				}
				break;


				case e_col:
				{
					col_no = GetUnsigned("Column to display:", col_no, 0, img.hsize()-1);
					wstring	column_title = no_scan_converter ? ssprintf(L"Column # %d", col_no) : ssprintf(L"Column #%d at %g %Ls", col_no, axial_start + col_no*axial_increment, axial_label.c_str());

					DisplayMathFunction(img.col(col_no), lateral_start, lateral_increment, column_title, L"signal", lateral_label);
					break;
				}

				case e_raster_scan_converter:
					SelectRectifierAndDisplayRaster<display_tag>::Display(img,
													title,
													axis_legend(lateral_start, lateral_increment, lateral_label),
													axis_legend(axial_start, axial_increment, axial_label),
													value_legend(0, 255, 1, ""),
													sco);
					break;

				case e_raster_as_is:
					SelectRectifierAndDisplayRaster<display_tag>::Display(img,
													title,
													axis_legend(lateral_start, lateral_increment, lateral_label),
													axis_legend(axial_start, axial_increment, axial_label),
													value_legend(0, 255, 1, ""),
													ScanConverterOptions());

					break;

				case e_histogram:
					in_moments_size = GetUnsigned("Enter number of moments to calculate", in_moments_size, 1, 6);
					DisplayHistogram2D<display_tag>::Display(img, title + L" 'histogram'", in_moments_size);
					break;
				case e_extended:
					DisplayExtended<display_tag>::Display(img, title, sco);
					break;
			}
		}
		catch(canceled_operation &)
		{
		// нажатие кнопки cancel в любом диалоговом окне, включая прогресс.
		// (например, отмена ошибочного выбранной команды без необходимости
		// дожидаться ее завершения)
		}
		catch(quit_application &)
		{
		// команда принудительного выхода из программы
			throw;
		}
		catch(exception &ex)
		{
		// все остальное, включая нехватку памяти
			ShowString("An error occured", ex.what());
		}
	} while(answer != e_exit);
}

template<>
struct DisplayExtended<DisplayTagScalar>
{
	template<class A2DT>
	static void Display(const A2DT &image, const wstring &title, const ScanConverterOptions &sco)
	{
		using namespace XRAD_PixelNormalizers;
		real_data_extended_display_options option = GetExtendedRealDataDisplayOption(title);
		typedef	typename A2DT::value_type value_type;
		typedef	ComplexSample<typename std::remove_cv<typename A2DT::value_type>::type, double> complex_type;
		typedef ComplexFunction2D<complex_type, double> complex_buffer_type;
		complex_buffer_type	buffer;

		switch(option)
		{
			case display_as_real_part:
				MakeCopy(buffer, image);
				DisplayMathFunction2DTemplate(buffer, L"<" + title + L" (as a real part)", sco);
				break;

			case display_as_magnitude:
				MakeCopy(buffer, image, Functors::assign_f1(Functors::absolute_value()));
				DisplayMathFunction2DTemplate(buffer, L"<" + title + L" (as a magnitude)", sco);
				break;

			case display_as_radians_phase:
				MakeCopy(buffer, image, Functors::assign_f1(Functors::phasor_value()));
				DisplayMathFunction2DTemplate(buffer, L"<" + title + L" (as a radians phase)", sco);
				break;

			case display_as_degrees_phase:
				MakeCopy(buffer, image, Functors::assign_f1(Functors::phasord_value()));
				DisplayMathFunction2DTemplate(buffer, L"<" + title + L" (as a degrees phase)", sco);
				break;
		}
	}
};

template<>
struct DisplayExtended<DisplayTagComplex>
{
	template<class A2DT>
	static void Display(const A2DT &image, const wstring &title, const ScanConverterOptions &sco)
	{
		using namespace XRAD_PixelNormalizers;
		using value_type = typename A2DT::value_type;
		complex_data_extended_display_options option = GetExtendedComplexDataDisplayOption(title);

		typedef MathFunction2D<MathFunction<typename value_type::part_type, double, typename A2DT::part_type::field_tag> > real_buffer_type;

		switch(option)
		{
			case display_real_part:
			{
				real_buffer_type	part;
				MakeCopy(part, image, Functors::assign_f1(Functors::real_part()));
				DisplayMathFunction2D(part, L"<" + title + L" (real part)", sco);
			}
			break;

			case display_imaginary_part:
			{
				real_buffer_type	part;
				MakeCopy(part, image, Functors::assign_f1(Functors::imag_part()));
				DisplayMathFunction2D(part, L"<" + title + L" (imag part)", sco);
			}

			case display_envelope:
			{
				real_buffer_type	part;
				MakeCopy(part, image, Functors::assign_f1(Functors::absolute_value()));
				DisplayMathFunction2D(part, L"<" + title + L" (envelope)", sco);
			}
			break;
			default:
				break;
		}
	}
};

template<>
struct DisplayExtended<DisplayTagRGB>
{
	template<class A2DT>
	static void Display(const A2DT &image, const wstring &title, const ScanConverterOptions &sco)
	{
		using namespace XRAD_PixelNormalizers;
		using value_type = typename A2DT::value_type;
		color_data_extended_display_options	answer = e_red_plate;

		//TODO: следующие числа и запрос их переделать по образцу GetExtendedComplexDataDisplayOption(title)

		while(answer != e_exit_color_data_display)
		{
			answer = GetExtendedColorDataDisplayOption(title + L" 'extended'");
			switch(answer)
			{
				case e_red_plate:
					DisplayMathFunction2D(image.red(), title + L" 'red plate'", sco);
					break;
				case e_green_plate:
					DisplayMathFunction2D(image.green(), title + L" 'green plate'", sco);
					break;
				case e_blue_plate:
					DisplayMathFunction2D(image.blue(), title + L" 'blue plate'", sco);
					break;

				case e_hue_plate:
				{
					RealFunction2D_F64	hue;
					hue.MakeCopy(image, Functors::assign_f1(Functors::hue_functor()));
					DisplayMathFunction2D(hue, title + L" 'hue'", sco);
				}
				break;

				case e_saturation_plate:
				{
					RealFunction2D_F64	sat;
					sat.MakeCopy(image, Functors::assign_f1(Functors::saturation_functor()));
					DisplayMathFunction2D(sat, title + L" 'saturation'", sco);
				}
				break;

				case e_lightness_plate:
				{
					RealFunction2D_F64	L;
					L.MakeCopy(image, Functors::assign_f1(Functors::lightness_functor()));
					DisplayMathFunction2D(L, title + L" 'lightness'", sco);
				}
				break;
			}
		}
	}
};

// Type fork:
// Оборачивание некоторых слишком низкоуровневых типов данных (DataArray2D)
// в обертки "математических" типов. В вызываемых функциях
// выполняются математические операции с анализируемым массивом.
// Для упрощения кода конвертация типа производится здесь,
// хотя, в принципе, это можно делать и по месту.

template<class DISPLAY_TAG>
struct DisplayMathFunction2DTypeFork;

template<>
struct DisplayMathFunction2DTypeFork<DisplayTagScalar>
{
	// MathFunction2D и его наследников конвертировать не требуется.
	// Предоставляем специализацию, которая просто передает данные дальше.
	template<class FT>
	static inline void Display(const MathFunction2D<FT> &image, const wstring &title, const ScanConverterOptions &sco)
	{
		DisplayMathFunction2DTemplate(image, title, sco);
	}

	// Общий шаблон, делающий обертку MathFunction2D.
	template<class A2DT>
	static void Display(const A2DT &image, const wstring &title, const ScanConverterOptions &sco)
	{
		typedef typename std::remove_reference<decltype(image.at(0, 0))>::type value_type; // const не снимаем
		MathFunction2D<MathFunction<value_type, double, AlgebraicStructures::FieldTagScalar> > buffer;
		buffer.UseData(&image.at(0, 0), image.vsize(), image.hsize(), image.vstep_raw(), image.hstep_raw());
		DisplayMathFunction2DTemplate(buffer, title, sco);
	}
};

template<>
struct DisplayMathFunction2DTypeFork<DisplayTagComplex>
{
	// ComplexFunction2D и его наследников конвертировать не требуется.
	// Предоставляем специализацию, которая просто передает данные дальше.
	template<class VT, class ST>
	static inline void Display(const ComplexFunction2D<VT, ST> &image, const wstring &title, const ScanConverterOptions &sco)
	{
		DisplayMathFunction2DTemplate(image, title, sco);
	}

	// Общий шаблон, делающий обертку ComplexFunction2D.
	template<class A2DT>
	static void Display(const A2DT &image, const wstring &title, const ScanConverterOptions &sco)
	{
		typedef typename std::remove_reference<decltype(image.at(0, 0))>::type value_type; // const не снимаем
		ComplexFunction2D<value_type, double> buffer;
		buffer.UseData(&image.at(0, 0), image.vsize(), image.hsize(), image.vstep_raw(), image.hstep_raw());
		DisplayMathFunction2DTemplate(buffer, title, sco);
	}
};

template<>
struct DisplayMathFunction2DTypeFork<DisplayTagRGB>
{
	// ColorImage и его наследников конвертировать не требуется.
	// Предоставляем специализацию, которая просто передает данные дальше.
	template<class VT, class ST>
	static inline void Display(const ColorImage<VT, ST> &image, const wstring &title, const ScanConverterOptions &sco)
	{
		DisplayMathFunction2DTemplate(image, title, sco);
	}

	// Общий шаблон, делающий обертку ColorImage.
	template<class A2DT>
	static inline void Display(const A2DT &image, const wstring &title, const ScanConverterOptions &sco)
	{
		typedef typename std::remove_reference<decltype(image.at(0, 0))>::type value_type; // const не снимаем
		ColorImage<value_type, double> buffer;
		buffer.UseData(const_cast<value_type*>(&image.at(0, 0)), image.vsize(), image.hsize(), image.vstep_raw(), image.hstep_raw());
		DisplayMathFunction2DTemplate(buffer, title, sco);
	}
};

template <class A2DT>
void DisplayMathFunction2DTypeForkHelper(const A2DT &image, const wstring &title, const ScanConverterOptions &sco)
{
	DisplayMathFunction2DTypeFork<DisplayTag_t<decltype(image.at(0, 0))>>::Display(image, title, sco);
}

} //namespace MF2DInterfaceAuxiliaries

//--------------------------------------------------------------

template <class AT>
void DisplayMathFunction2D(const DataArray2D<AT> &image, const string &title,
		const ScanConverterOptions &sco)
{
	MF2DInterfaceAuxiliaries::DisplayMathFunction2DTypeForkHelper(image, convert_to_wstring(title), sco);
}

template <class AT>
void DisplayMathFunction2D(const DataArray2D<AT> &image, const wstring &title,
		const ScanConverterOptions &sco)
{
	MF2DInterfaceAuxiliaries::DisplayMathFunction2DTypeForkHelper(image, title, sco);
}

//--------------------------------------------------------------

XRAD_END
