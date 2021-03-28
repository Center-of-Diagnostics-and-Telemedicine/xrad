/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#include "PixelNormalizersGUI.h"
#include "RasterImageSet.h"
#include "MathFunctionGUI2D.h"
#include <XRADBasic/Sources/ScanConverter/ScanConverter.h>
#include <XRADBasic/Sources/Containers/DataArrayAnalyzeMD.h>
#include <XRADBasic/Sources/Containers/ComplexArrayAnalyzeFunctors.h>
#include <XRADBasic/Sources/Utils/ExponentialBlurAlgorithms.h>
#include <XRADBasic/Sources/Utils/ParallelProcessor.h>

XRAD_BEGIN

//--------------------------------------------------------------

namespace MFMDInterfaceAuxiliaries
{

using namespace XRAD_PixelNormalizers;

enum class display_3D_options
{
	coord_0_animation,
	coord_1_animation,
	coord_2_animation,
	coord_12_analyzer,
	coord_02_analyzer,
	coord_01_analyzer,
	coord_12_all,
	coord_02_all,
	coord_01_all,
	histogram_analyzer,
	region_animation,
	local_statistics_analyzer,
	extended_analyzer,
	exit_3d_display
};

template<class Tag1, class Tag2, class T>
using if_tag = typename std::enable_if<std::is_same<Tag1, Tag2>::value, T>::type;

template<class A3DT>
class	MathFunction3DDisplayer
{
	private:
		typedef	A3DT array_3d_type;
		typedef	typename A3DT::value_type value_type;
		typedef typename A3DT::field_tag value_field_tag;
		typedef DisplayTag_t<value_type> display_tag;

		const array_3d_type &array_md;

		size_t	slice_number_0;
		size_t	slice_number_1;
		size_t	slice_number_2;

		size_t	slice_orientation_01;
		size_t	slice_orientation_12;
		size_t	slice_orientation_02;

		enum class CombineSamplesFunctorKind { Average, Max };
		CombineSamplesFunctorKind combine_samples_functor_kind = CombineSamplesFunctorKind::Average;
		using CombineSamplesFunctionArray = typename array_3d_type::row_type;
		using CombineSamplesFunctionType =
				floating64_type<typename CombineSamplesFunctionArray::value_type>
				(const CombineSamplesFunctionArray &data);
		using CombineSamplesFunctorType = pair<function<CombineSamplesFunctionType>, wstring>;

		wstring title;
		const ScanConverterOptions &sco_12;

		//	запрашивает ориентацию срезов
		size_t	DecideSliceOrientation_01();
		size_t	DecideSliceOrientation_12();
		size_t	DecideSliceOrientation_02();

		CombineSamplesFunctorType DecideCombineSamplesFunctor();

		//	внутри совершается формирование индексного вектора в зависимости от выбранного
		//	направления движения срезов и их ориентации
		template<class PT, class F>
			void	DisplayAnimations(const value_legend &vlegend, const F& functor,
					display_3D_options display_option, size_t slice_orientation);

		//	отображает двумерные срезы (с возможностью анализа)
		void	DisplaySliceAnalyzers_01(size_t slice_orientation_01);
		void	DisplaySliceAnalyzers_12(size_t slice_orientation_12);
		void	DisplaySliceAnalyzers_02(size_t slice_orientation_02);

		void	DisplaySliceRangeAnalyzers_01(size_t slice_orientation_12,
				CombineSamplesFunctorType combine_functor);
		void	DisplaySliceRangeAnalyzers_12(size_t slice_orientation_12,
				CombineSamplesFunctorType combine_functor);
		void	DisplaySliceRangeAnalyzers_02(size_t slice_orientation_12,
				CombineSamplesFunctorType combine_functor);

		//	функции делают запрос о способе преобразования чисел в пикселы,
		//	формируют анимацию по заданным ранее параметрам и отображают ее.
		//	для комплексных данных способы преобразования не такие, как для действительных.
		//	поэтому две реализации. для цветных понадобится третья
		template<class DISPLAY_TAG>
		if_tag<DISPLAY_TAG, DisplayTagScalar, void> GetRectifierAndDisplayAnimations(
				display_3D_options display_option, size_t slice_orientation);
		template<class DISPLAY_TAG>
		if_tag<DISPLAY_TAG, DisplayTagComplex, void> GetRectifierAndDisplayAnimations(
				display_3D_options display_option, size_t slice_orientation);
		template<class DISPLAY_TAG>
		if_tag<DISPLAY_TAG, DisplayTagRGB, void> GetRectifierAndDisplayAnimations(
				display_3D_options display_option, size_t slice_orientation);

		void DisplayLocalStatistics();

		template<class DISPLAY_TAG>
		if_tag<DISPLAY_TAG, DisplayTagScalar, void> DisplayHistogram();
		template<class DISPLAY_TAG>
		if_tag<DISPLAY_TAG, DisplayTagComplex, void> DisplayHistogram();
		template<class DISPLAY_TAG>
		if_tag<DISPLAY_TAG, DisplayTagRGB, void> DisplayHistogram();

		void DisplayFragment3D();

		template<class DISPLAY_TAG>
		if_tag<DISPLAY_TAG, DisplayTagScalar, void> DisplayExtended();
		template<class DISPLAY_TAG>
		if_tag<DISPLAY_TAG, DisplayTagComplex, void> DisplayExtended();
		template<class DISPLAY_TAG>
		if_tag<DISPLAY_TAG, DisplayTagRGB, void> DisplayExtended();

		//	вспомогательная функция, показывает анимацию срезов по заданному индексному вектору iv с заданным преобразованием F
		template<class PT, class F>
		void	DisplayAnimationBase(const index_vector &iv, const value_legend &vlegend, const F& functor, const wstring &title);

	public:
		//	единственный конструктор от отображаемого массива
		MathFunction3DDisplayer(const array_3d_type &in_cube, const wstring &in_title, const ScanConverterOptions &in_sco_12):
			array_md(in_cube),
			title(in_title),
			sco_12(in_sco_12)
		{
			slice_number_0 = array_md.sizes(0)/2;
			slice_number_1 = array_md.sizes(1)/2;
			slice_number_2 = array_md.sizes(2)/2;

			slice_orientation_01 = 0;//2015_08_12 изменено на 0, чтобы ориентация по умолчанию соответствовала виду "главной проекции" (1-2 вдоль 0)
			slice_orientation_12 = 1;
			slice_orientation_02 = 1;
		}

		//	единственвный публичный метод
		void Display();
};

// универсальная процедура показа анимации.
// преобразует исходные данные в скалярный массив тех же размеров,
// в соответствии с заданным функтором, и отображает анимацию.

template<class A3DT>
template<class PT, class F>
void	MathFunction3DDisplayer<A3DT>::DisplayAnimationBase(const index_vector &iv, const value_legend &vlegend, const F& functor, const wstring &window_title)
{
	typedef	typename std::remove_cv<PT>::type pixel_type;
	typedef ScanConverter<DataArray2D<DataArray<pixel_type> > > scan_converter_type;


	if(iv.size() != 3 || array_md.n_dimensions() != 3)
		throw invalid_argument("DisplayAnimationBase(const DataArrayMD<>), invalid dimensions number");
	size_t	animation_coordinate(0);
	size_t	n_animation_coordinates(0);
	size_t	c1(1), c2(2);

	for(size_t i = 0; i < iv.size(); ++i)
	{
		if(!is_slice_mask(iv[i]))
		{
			animation_coordinate = i, ++n_animation_coordinates;
		}
		else
		{
			if(dimension_no(iv[i])==0) c1 = i;
			if(dimension_no(iv[i])==1) c2 = i;
		}
	}
	if(n_animation_coordinates != 1)
		throw invalid_argument("DisplayAnimationBase(const DataArrayMD<>), invalid index vector");

	size_t	n_slices = array_md.sizes()[animation_coordinate];
	size_t	size_coord_1 = array_md.sizes()[c1];
	size_t	size_coord_2 = array_md.sizes()[c2];

	bool	scan_conversion_possible = ((sco_12.depth_range().cm()!=0) && c1==1 && c2==2);
	bool	scan_conversion = scan_conversion_possible ?
		(Decide2(window_title + L": scan conversion options:", L"Raw data", L"Scan converter", SavedGUIValue<size_t>(1)) ? true:false) :
		false;

	bool	progress_needed = !IsProgressActive();

	shared_ptr<ProgressProxyCore> progress_proxy;
	if(progress_needed)
		progress_proxy = GUIProgressProxy();
	else
		progress_proxy = VoidProgressProxy();

	axis_legend zlegend(0, 1, "frame");
	axis_legend ylegend(0, 1, "row");
	axis_legend xlegend(0, 1, "col");
// 		value_legend vlegend(0, 255, 1, "");
	size_t	rvs;
	size_t	rhs;


	if(scan_conversion)
	{


		physical_length	vmin, vmax, hmin, hmax;
		scan_converter_type	SC(size_coord_1, size_coord_2);
		SC.CopyScanConverterOptions(sco_12);
		SC.InitScanConverter();

		SC.GetRasterDimensions(vmin, vmax, hmin, hmax);
		rvs = SC.GetConvertedImage().vsize();
		rhs = SC.GetConvertedImage().hsize();

		ylegend = axis_legend(vmin.cm(), (vmax-vmin).cm()/rvs, "cm");
		xlegend = axis_legend(hmin.cm(), (hmax-hmin).cm()/rhs, "cm");

		RasterImageSet	animation(window_title, rvs, rhs);
		bool	frames_added = animation.AddFrames(n_slices);

		animation.SetAxesScales(zlegend.min_value, zlegend.step,ylegend.min_value, ylegend.step, xlegend.min_value, xlegend.step);
		animation.SetLabels(window_title, zlegend.label, ylegend.label, xlegend.label, vlegend.label);
		animation.SetDefaultBrightness(vlegend.display_range.p1(), vlegend.display_range.p2(), vlegend.gamma);

		ParallelProcessor	proc;
 		auto	mode = frames_added ? ParallelProcessor::e_force_parallel:ParallelProcessor::e_force_plain;
		proc.init(n_slices, mode);

		vector<unique_ptr<scan_converter_type>> scs(proc.n_threads());
		for(auto &scp: scs)
		{
			scp = make_unique<scan_converter_type>(size_coord_1, size_coord_2);
			scp->CopyScanConverterOptions(sco_12);
			scp->InitScanConverter();
		}

		auto	lambda = [&](size_t slice_no)
		{
			auto	slice_iv(iv);
			slice_iv[animation_coordinate] = slice_no;
			size_t	thread_no = proc.thread_no(slice_no);

			scs[thread_no]->CopyData(array_md.GetSlice(slice_iv), Functors::assign_f1(functor));
			scs[thread_no]->BuildConvertedImage();
			animation.SetupFrame(int(slice_no), scs[thread_no]->GetConvertedImage());
		};
		proc.perform(lambda, L"Creating animation", progress_proxy);
		animation.Display(true);
	}
	else
	{
		index_vector	slice_iv_prototype(iv);
		typename A3DT::slice_type::invariable	slice_ref_prototype;
		slice_iv_prototype[animation_coordinate] = 0;
		array_md.GetSlice(slice_ref_prototype, slice_iv_prototype);

		rvs = slice_ref_prototype.vsize();
		rhs = slice_ref_prototype.hsize();

		RasterImageSet	animation(window_title, rvs, rhs);
		bool	frames_added = animation.AddFrames(n_slices);

		animation.SetAxesScales(zlegend.min_value, zlegend.step, ylegend.min_value, ylegend.step, xlegend.min_value, xlegend.step);
		animation.SetLabels(window_title, zlegend.label, ylegend.label, xlegend.label, vlegend.label);
		animation.SetDefaultBrightness(vlegend.display_range.p1(), vlegend.display_range.p2(), vlegend.gamma);

		ParallelProcessor	proc;
		auto	mode = frames_added ? ParallelProcessor::e_force_parallel:ParallelProcessor::e_force_plain;
		proc.init(n_slices, mode);

		auto	lambda = [&](size_t slice_no)
		{
			auto	slice_iv(iv);
			slice_iv[animation_coordinate] = slice_no;

			DataArray2D<DataArray<pixel_type> > raster;
			MakeCopy(raster, array_md.GetSlice(slice_iv), Functors::assign_f1(functor));
			animation.SetupFrame(int(slice_no), raster);
		};
		proc.perform(lambda, L"Creating animation", progress_proxy);
		animation.Display(true);
	}
}


template<class A3DT>
template<class DISPLAY_TAG>
typename std::enable_if<std::is_same<DISPLAY_TAG, DisplayTagScalar>::value, void>::type
		MathFunction3DDisplayer<A3DT>::GetRectifierAndDisplayAnimations(
				display_3D_options display_option, size_t slice_orientation)
{
	typedef	GrayPixel pixel_type;
	default_rectifier_id_t	rectifier_id = GetDefaultRectifierId();
	value_legend vlegend(0, 255, 1, "");

	switch(rectifier_id)
	{
		case default_rectifier_linear_auto:
		case default_rectifier_linear_manual:
			{
				range1_F64	recommended_range, absolute_range;
				ComputeDisplayRanges(array_md, recommended_range, absolute_range, Functors::identity());

				if(rectifier_id == default_rectifier_linear_manual)
				{
					GetDisplayRange(vlegend.display_range, vlegend.gamma, recommended_range, absolute_range);
				}
				else
				vlegend.display_range = recommended_range;

				DisplayAnimations<value_type>(vlegend, Functors::identity(),
						display_option, slice_orientation);
			}
			break;

		case default_rectifier_linear_abs_auto:
		case default_rectifier_linear_abs_manual:
			{
				range1_F64	recommended_range, absolute_range;
				ComputeDisplayRanges(array_md, recommended_range, absolute_range, Functors::absolute_value());
				if(rectifier_id == default_rectifier_linear_abs_manual)
				{
					GetDisplayRange(vlegend.display_range, vlegend.gamma, recommended_range, absolute_range);
				}
				else vlegend.display_range = recommended_range;
				DisplayAnimations<value_type>(vlegend, Functors::absolute_value(),
						display_option, slice_orientation);
	//				DisplayAnimations<pixel_type>(vlegend, GetDefaultRectifierLinearAbs<value_type, pixel_type>(rectifier_id, recommended_range, absolute_range));
			}
			break;

		case default_rectifier_logarithmic:
			{
				range1_F64	recommended_range, absolute_range;
				ComputeDisplayRanges(array_md, recommended_range, absolute_range, Functors::amplitude_to_decibel_value());
				vlegend.display_range = recommended_range;
				AdjustDynamicalRange(vlegend.display_range, absolute_range);
				vlegend.label = L"dB";


				DisplayAnimations<float>(vlegend, Functors::amplitude_to_decibel_value(),
						display_option, slice_orientation);
			}
			break;
	}
}

template<class A3DT>
template<class DISPLAY_TAG>
typename std::enable_if<std::is_same<DISPLAY_TAG, DisplayTagComplex>::value, void>::type
		MathFunction3DDisplayer<A3DT>::GetRectifierAndDisplayAnimations(
				display_3D_options display_option, size_t slice_orientation)
{
	typedef	GrayPixel pixel_type;
	complex_rectifier_id_t rectifier_id = GetComplexRectifierId();
	value_legend vlegend(0, 255, 1, "");

	switch(rectifier_id)
	{
		case complex_rectifier_linear_abs_auto:
		case complex_rectifier_linear_abs_manual:
			{
				range1_F64	recommended_range, absolute_range;
				ComputeDisplayRanges(array_md, recommended_range, absolute_range, Functors::absolute_value());
				if(rectifier_id == complex_rectifier_linear_abs_manual)
					GetDisplayRange(vlegend.display_range, vlegend.gamma, recommended_range, absolute_range);
				else
					vlegend.display_range = recommended_range;
				DisplayAnimations<value_type>(vlegend, Functors::identity(),
						display_option, slice_orientation);
			}
			break;
		case complex_rectifier_real_auto:
		case complex_rectifier_real_manual:
			{
				range1_F64	recommended_range, absolute_range;
				ComputeDisplayRanges(array_md, recommended_range, absolute_range, Functors::real_part());
				if(rectifier_id == complex_rectifier_real_manual)
					GetDisplayRange(vlegend.display_range, vlegend.gamma, recommended_range, absolute_range);
				else
					vlegend.display_range = recommended_range;
				DisplayAnimations<typename value_type::part_type>(vlegend, Functors::real_part(),
						display_option, slice_orientation);
	//				DisplayAnimations<typename value_type::component_type>(vlegend, real_functor<double,value_type>());
			}
			break;
		case complex_rectifier_imag_auto:
		case complex_rectifier_imag_manual:
			{
				range1_F64	recommended_range, absolute_range;
				ComputeDisplayRanges(array_md, recommended_range, absolute_range, Functors::imag_part());
				if(rectifier_id == complex_rectifier_imag_manual)
					GetDisplayRange(vlegend.display_range, vlegend.gamma, recommended_range, absolute_range);
				else
					vlegend.display_range = recommended_range;


				DisplayAnimations<typename value_type::part_type>(vlegend, Functors::imag_part(),
						display_option, slice_orientation);

			}
			break;
		case complex_rectifier_logarithmic:
			{
				range1_F64	recommended_range, absolute_range;
				ComputeDisplayRanges(array_md, recommended_range, absolute_range, Functors::amplitude_to_decibel_value());
				vlegend.display_range = recommended_range;
				AdjustDynamicalRange(vlegend.display_range, absolute_range);
				vlegend.label = L"dB";
				DisplayAnimations<float>(vlegend, Functors::amplitude_to_decibel_value(),
						display_option, slice_orientation);
			}
			break;
	}
}

template<class A3DT>
template<class DISPLAY_TAG>
typename std::enable_if<std::is_same<DISPLAY_TAG, DisplayTagRGB>::value, void>::type
		MathFunction3DDisplayer<A3DT>::GetRectifierAndDisplayAnimations(
				display_3D_options display_option, size_t slice_orientation)
{
	typedef	ColorPixel pixel_type;
	color_rectifier_id_t rectifier_type = GetColorRectifierId();
	value_legend vlegend(0, 255, 1, "");

	switch(rectifier_type)
	{
		case color_rectifier_as_is:
// 				DisplayAnimations<pixel_type>(vlegend, assign_identical<value_type, pixel_type>());
// 				break;

		case color_rectifier_fit_in_range_auto:
		case color_rectifier_fit_in_range_manual:
			{
				if(rectifier_type == color_rectifier_as_is)
				{
					vlegend.display_range = range1_F64(0,255);
					vlegend.gamma = 1;
				}
				else
				{
					range1_F64	recommended_range, absolute_range;
					range1_F64	recommended_range_r, absolute_range_r;
					range1_F64	recommended_range_g, absolute_range_g;
					range1_F64	recommended_range_b, absolute_range_b;
					ComputeDisplayRanges(array_md, recommended_range_r, absolute_range_r, Functors::red_functor());
					ComputeDisplayRanges(array_md, recommended_range_g, absolute_range_g, Functors::green_functor());
					ComputeDisplayRanges(array_md, recommended_range_b, absolute_range_b, Functors::blue_functor());

					recommended_range.x1() = min(recommended_range_r.x1(), min(recommended_range_g.x1(), recommended_range_b.x1()));
					recommended_range.x2() = max(recommended_range_r.x2(), max(recommended_range_g.x2(), recommended_range_b.x2()));
					absolute_range.x1() = min(absolute_range_r.x1(), min(absolute_range_g.x1(), absolute_range_b.x1()));
					absolute_range.x2() = max(absolute_range_r.x2(), max(absolute_range_g.x2(), absolute_range_b.x2()));

					if(rectifier_type == color_rectifier_fit_in_range_manual)
					{
						GetDisplayRange(vlegend.display_range, vlegend.gamma, recommended_range, absolute_range);
					}
					else
					{
						vlegend.display_range = recommended_range;
						vlegend.gamma = 1;
					}
				}

	//			DisplayAnimations<pixel_type>(vlegend, GetColorRectifierRange<value_type, pixel_type>(rectifier_type, recommended_range, absolute_range));
				DisplayAnimations<value_type>(vlegend, Functors::identity(),
						display_option, slice_orientation);
			}
			break;

		case color_rectifier_lightness_as_is:
			vlegend.display_range = range1_F64(0,255);
			DisplayAnimations<value_type>(vlegend, Functors::lightness_functor(),
					display_option, slice_orientation);
			break;

		case color_rectifier_lightness_fit_in_range_auto:
		case color_rectifier_lightness_fit_in_range_manual:
			range1_F64	recommended_range, absolute_range;
			ComputeDisplayRanges(array_md, recommended_range, absolute_range, Functors::lightness_functor());
			if(rectifier_type == color_rectifier_fit_in_range_manual)
			{
				GetDisplayRange(vlegend.display_range, vlegend.gamma, recommended_range, absolute_range);
			}
			else
			vlegend.display_range = recommended_range;

			DisplayAnimations<value_type>(vlegend, Functors::lightness_functor(),
					display_option, slice_orientation);
			break;
	}
}


template<class A3DT>
void MathFunction3DDisplayer<A3DT>::DisplayLocalStatistics()
{
	typedef	floating32_type<value_type> mean_value_type;
	typedef	MathFunctionMD<MathFunction2D<MathFunction<mean_value_type, double, value_field_tag>>>
			mean_values_buffer_type;
	mean_values_buffer_type m0_buffer(array_md);

	static point3_F64	radius(5, 5, 5);

	radius.z() = GetFloating("Coord 0 blur radius", radius.z(), 0, infinity());
	radius.y() = GetFloating("Coord 1 blur radius", radius.y(), 0, infinity());
	radius.x() = GetFloating("Coord 2 blur radius", radius.x(), 0, infinity());

	BiexpBlur3D(m0_buffer, radius);

	size_t	moment_no = GetUnsigned("Moment no", 1, 1, 10);
	wstring legend = ssprintf(L" <M%d %gx%gx%g>", int(moment_no), radius.z(), radius.y(), radius.x());

	if(moment_no==1)
	{
		DisplayMathFunction3D(m0_buffer, convert_to_wstring(title) + legend, sco_12);
	}
	else
	{
		mean_values_buffer_type centered(array_md);
		centered -= m0_buffer;
		mean_values_buffer_type moment(centered);

		for(size_t i = 1; i < moment_no; ++i) Apply_AA_MD_F2(moment, centered, MF2DInterfaceAuxiliaries::componentwise_multiply_assign());
		BiexpBlur3D(moment, radius);
		ApplyFunction(moment, MF2DInterfaceAuxiliaries::componentwise_signed_pow<mean_value_type>(1./moment_no));

		DisplayMathFunction3D(moment, convert_to_wstring(title) + legend, sco_12);
	}
}

template<class A3DT>
template<class DISPLAY_TAG>
if_tag<DISPLAY_TAG, DisplayTagScalar, void> MathFunction3DDisplayer<A3DT>::DisplayHistogram()
{
	size_t	histogram_type = Decide("Histogram type", {"Linear", "Log. absolute value"});
	size_t	n = GetUnsigned("Histogram size", int(sqrt(double(array_md.element_count()))), 3, array_md.element_count());
	RealFunctionF64	histogram(n);

	switch(histogram_type)
	{
		case 0:
			{
				range1_F64 absolute_range(MinValue(array_md), MaxValue(array_md));

				ComputeHistogram(array_md, histogram, absolute_range);
				double	step = absolute_range.delta()/n;
				DisplayMathFunction(histogram, absolute_range.p1() + step/2, step, title + L" 'histogram'", L"probability", L"value");
			}
			break;

		case 1:
			{
				range1_F64	recommended_range, absolute_range;
				ComputeDisplayRanges(array_md, recommended_range, absolute_range, Functors::amplitude_to_decibel_value());

				AdjustDynamicalRange(recommended_range, absolute_range);
				//RealFunctionF64	histogram(n);
				ComputeHistogramTransformed(array_md, histogram, recommended_range, Functors::amplitude_to_decibel_value());
				double	step = recommended_range.delta()/n;
				DisplayMathFunction(histogram, recommended_range.x1() + step/2,
					step, title + L" 'histogram (log. abs)'", L"probability", L"dB");
			}
			break;
	}
}

template<class A3DT>
template<class DISPLAY_TAG>
if_tag<DISPLAY_TAG, DisplayTagComplex, void> MathFunction3DDisplayer<A3DT>::DisplayHistogram()
{
	size_t	histogram_type = Decide("Histogram type", {"Real and imaginary separately", "Absolute value", "Log. absolute value"});
	size_t	n = GetUnsigned("Histogram size", int(sqrt(double(array_md.element_count()))), 3, array_md.element_count());
	switch(histogram_type)
	{
		case 0:
			{
				typedef typename ComplexFunctionMD<value_type, typename value_type::scalar_type>::invariable shell_type;
				shell_type	shell;
				shell.UseData(array_md);

				double	maxval = max(MaxValue(real(const_cast<const shell_type &>(shell))), MaxValue(imag(const_cast<const shell_type &>(shell))));
				double	minval = min(MinValue(real(const_cast<const shell_type &>(shell))), MinValue(imag(const_cast<const shell_type &>(shell))));

				RealFunction2D_F64	histogram(n_components(value_type()), n);

				ComputeComponentsHistogram(array_md, histogram, range1_F64(minval, maxval));
				double step = (maxval-minval)/n;
				GraphSet	gs(title + L" 'histogram'", L"probability", L"value");
				//TODO сделать уточнение масштаба гистограммы: форсировать минимум в нуле, диапазоны по x  растянуть до minval, maxval. то же в остальных гистограммах, включая многомерные данные
				gs.AddGraphUniform(histogram.row(0), minval+step/2, step, "real part");
				gs.AddGraphUniform(histogram.row(1), minval+step/2, step, "imag part");
				gs.Display();
			}
			break;

		case 1:
			{
				RealFunctionF64	histogram(n);
				Functors::absolute_value avf;
				range1_F64 absolute_range(MinValueTransformed(array_md, avf), MaxValueTransformed(array_md, avf));

				ComputeHistogramTransformed(array_md, histogram, absolute_range, avf);
				double step = absolute_range.delta()/n;
				DisplayMathFunction(histogram, absolute_range.x1() + step/2, step, title + L" 'histogram (abs)'", L"probability", L"abs. value");
			}
			break;

		case 2:
			{
				RealFunctionF64	histogram(n);
				range1_F64	recommended_range, absolute_range;
				ComputeDisplayRanges(array_md, recommended_range, absolute_range, Functors::amplitude_to_decibel_value());
				AdjustDynamicalRange(recommended_range, absolute_range);

				ComputeHistogramTransformed(array_md, histogram, recommended_range, Functors::amplitude_to_decibel_value());
				double step = recommended_range.delta()/n;
				DisplayMathFunction(histogram, recommended_range.x1() + step/2,
					step, title + L" 'histogram (log. abs)'", L"probability", L"dB");
			}
			break;
	}
}

template<class A3DT>
template<class DISPLAY_TAG>
if_tag<DISPLAY_TAG, DisplayTagRGB, void> MathFunction3DDisplayer<A3DT>::DisplayHistogram()
{
	using namespace XRAD_PixelNormalizers;
	size_t	histogram_type = Decide("Histogram type", {"RGB components separately", "Lightness"});
	size_t	data_count = array_md.element_count();
	size_t	n = GetUnsigned("Histogram size", int(sqrt(double(data_count))), 3, data_count);

	switch(histogram_type)
	{
		case 0:
			{
				//TODO: сделать MaxComponentValue, как в двумерном случае
				double	maxval = max(MaxValueTransformed(array_md, Functors::red_functor()),
										max(MaxValueTransformed(array_md, Functors::green_functor()),
										MaxValueTransformed(array_md, Functors::blue_functor())));

				double	minval = min(MinValueTransformed(array_md, Functors::red_functor()),
										min(MinValueTransformed(array_md, Functors::green_functor()),
										MinValueTransformed(array_md, Functors::blue_functor())));

				RealFunction2D_F64	histogram(n_components(value_type()), n);
				ComputeComponentsHistogram(array_md, histogram, range1_F64(minval, maxval));
				double	step = (maxval-minval)/n;

				GraphSet	gs(title + L" 'histogram (rgb)'", L"probability", L"value");
				gs.AddGraphUniform(histogram.row(1), minval+step/2, step, "green");
				gs.AddGraphUniform(histogram.row(2), minval+step/2, step, "blue");
				gs.AddGraphUniform(histogram.row(0), minval+step/2, step, "red");
				gs.Display();
			}
			break;

		case 1:
			{
				RealFunctionF64	histogram(n);
				Functors::lightness_functor lf;
				range1_F64	absolute_range(MinValueTransformed(array_md, lf), MaxValueTransformed(array_md, lf));
				ComputeHistogramTransformed(array_md, histogram, absolute_range, lf);

				double	step = absolute_range.delta()/n;

				DisplayMathFunction(histogram, absolute_range.x1() + step/2, step, title + L" 'histogram (abs)'", L"probability", L"abs. value");
			}
			break;
	}
}


template<class A3DT>
template<class DISPLAY_TAG>
if_tag<DISPLAY_TAG, DisplayTagScalar, void> MathFunction3DDisplayer<A3DT>::DisplayExtended()
{
	using namespace XRAD_PixelNormalizers;
	real_data_extended_display_options option = GetExtendedRealDataDisplayOption(title);
	ComplexFunctionMD_F64	buffer;

	switch(option)
	{
		case display_as_real_part:
			{
				buffer.MakeCopy(array_md);
				MathFunction3DDisplayer<ComplexFunctionMD_F64>	displayer(buffer, L"<" + title + L" (as a real part)", sco_12);
				displayer.Display();
			}
			break;

		case display_as_magnitude:
			{
				buffer.MakeCopy(array_md, Functors::assign_f1(Functors::absolute_value()));
				MathFunction3DDisplayer<ComplexFunctionMD_F64>	displayer(buffer, L"<" + title + L" (as a magnitude)", sco_12);
				displayer.Display();
			}
			break;

		case display_as_radians_phase:
			{
				buffer.MakeCopy(array_md, Functors::assign_f1(Functors::phasor_value()));
				MathFunction3DDisplayer<ComplexFunctionMD_F64>	displayer(buffer, L"<" + title + L" (as a radians phase)", sco_12);
				displayer.Display();
			}
			break;

		case display_as_degrees_phase:
			{
				buffer.MakeCopy(array_md, Functors::assign_f1(Functors::phasord_value()));
				MathFunction3DDisplayer<ComplexFunctionMD_F64>	displayer(buffer, L"<" + title + L" (as a degrees phase)", sco_12);
				displayer.Display();
			}
			break;
	}
}

template<class A3DT>
template<class DISPLAY_TAG>
if_tag<DISPLAY_TAG, DisplayTagComplex, void> MathFunction3DDisplayer<A3DT>::DisplayExtended()
{
	complex_data_extended_display_options option = GetExtendedComplexDataDisplayOption(title);
	typedef	MathFunctionMD<MathFunction2D<MathFunction<
				typename std::add_const<typename value_type::part_type>::type,
				typename value_type::scalar_type,
				AlgebraicStructures::FieldTagScalar>>>
			real_part_type;
	switch(option)
	{
		case display_real_part:
			{
				real_part_type	part;
	// 				array_md.GetReal(part);
				GetReal(part,array_md);
				MathFunction3DDisplayer<real_part_type>	displayer(part, L"<" + title + L" (real part)", sco_12);
				displayer.Display();
			}
			break;

		case display_imaginary_part:
			{
				real_part_type	part;
				GetImag(part, array_md);
	//				typename A3DT::part_type_invariable	part;
	//				array_md.GetImag(part);
				MathFunction3DDisplayer<real_part_type>	displayer(part, L"<" + title + L" (imag part)", sco_12);
	//				MathFunction3DDisplayer<typename A3DT::part_type_invariable>	displayer(part, L"<" + title + L" (imag part)", sco_12);
				displayer.Display();
			}
			break;

		case display_envelope:
			{
				typedef	MathFunctionMD<MathFunction2D<MathFunction<float, double, AlgebraicStructures::FieldTagScalar>>> part_type;
				part_type	part;
				MakeCopy(part, array_md, Functors::assign_f1(Functors::absolute_value()));
				MathFunction3DDisplayer<part_type>	displayer(part, L"<" + title + L" (Envelope)", sco_12);
				displayer.Display();
			}
			break;

		default:
			break;
	}
}

template<class A3DT>
template<class DISPLAY_TAG>
if_tag<DISPLAY_TAG, DisplayTagRGB, void> MathFunction3DDisplayer<A3DT>::DisplayExtended()
{
	//TODO доделать DisplayExtended для трехмерных данных
	Error("MathFunction3DDisplayer<RGBColorFunctionMD>::DisplayExtended not written");
}


template<class A3DT>
void	MathFunction3DDisplayer<A3DT>::DisplaySliceAnalyzers_01(size_t slice_orientation)
{
	slice_number_2 = GetUnsigned("Coord. 2 slice # for analyzing", slice_number_2,
			0, array_md.sizes(2)-1);
	if(slice_orientation)
	{
		auto slice = array_md.GetSlice({slice_mask(0), slice_mask(1), slice_number_2});
		DisplayMathFunction2D (slice, title + ssprintf(L", coord. 0-1 slice # %d", slice_number_2));
	}
	else
	{
		auto slice = array_md.GetSlice({slice_mask(1), slice_mask(0), slice_number_2});
		DisplayMathFunction2D (slice, title + ssprintf(L", coord. 1-0 slice # %d", slice_number_2));
	}
}

template<class A3DT>
void	MathFunction3DDisplayer<A3DT>::DisplaySliceAnalyzers_12(size_t slice_orientation)
{
	slice_number_0 = GetUnsigned("Coord. 0 slice # for analyzing", slice_number_0,
			0, array_md.sizes(0)-1);
	if(slice_orientation)
	{
		auto slice = array_md.GetSlice({slice_number_0, slice_mask(0),slice_mask(1)});
		DisplayMathFunction2D (slice, title + ssprintf(L", coord. 1-2 slice # %d", slice_number_0), sco_12);
	}
	else
	{
		auto slice = array_md.GetSlice({slice_number_0, slice_mask(1),slice_mask(0)});
		DisplayMathFunction2D (slice, title + ssprintf(L", coord. 2-1 slice # %d", slice_number_0));
	}
}

template<class A3DT>
void	MathFunction3DDisplayer<A3DT>::DisplaySliceAnalyzers_02(size_t slice_orientation)
{
	slice_number_1 = GetUnsigned("Coord. 1 slice # for analyzing", slice_number_1,
			0, array_md.sizes(1)-1);
	if(slice_orientation)
	{
		auto slice = array_md.GetSlice({slice_mask(0), slice_number_1,slice_mask(1)});
		DisplayMathFunction2D (slice, title + ssprintf(L", coord. 0-2 slice # %d", slice_number_1));
	}
	else
	{
		auto slice = array_md.GetSlice({slice_mask(1), slice_number_1,slice_mask(0)});
		DisplayMathFunction2D (slice, title + ssprintf(L", coord. 2-0 slice # %d", slice_number_1));
	}
}



template<class A3DT>
void	MathFunction3DDisplayer<A3DT>::DisplaySliceRangeAnalyzers_01(size_t slice_orientation,
		CombineSamplesFunctorType combine_functor)
{
	typename array_3d_type::slice_type::variable slice(array_md.sizes(0), array_md.sizes(1));
	for (size_t i = 0; i < array_md.sizes(0); ++i)
	{
		for (size_t j = 0; j < array_md.sizes(1); ++j)
		{
			slice.at(i, j) = combine_functor.first(array_md.GetRow({i, j, slice_mask(0)}));
		}
	}
	if(slice_orientation)
	{
		DisplayMathFunction2D(slice, title + ssprintf(L", coord. 0-1 slices, %ls",
				EnsureType<const wchar_t*>(combine_functor.second.c_str())));
	}
	else
	{
		slice.transpose();
		DisplayMathFunction2D(slice, title + ssprintf(L", coord. 1-0 slices, %ls",
				EnsureType<const wchar_t*>(combine_functor.second.c_str())));
	}
}

template<class A3DT>
void	MathFunction3DDisplayer<A3DT>::DisplaySliceRangeAnalyzers_12(size_t slice_orientation,
		CombineSamplesFunctorType combine_functor)
{
	typename array_3d_type::slice_type::variable slice(array_md.sizes(1), array_md.sizes(2));
	for (size_t i = 0; i < array_md.sizes(1); ++i)
	{
		for (size_t j = 0; j < array_md.sizes(2); ++j)
		{
			slice.at(i, j) = combine_functor.first(array_md.GetRow({slice_mask(0), i, j}));
		}
	}
	if(slice_orientation)
	{
		DisplayMathFunction2D(slice, title + ssprintf(L", coord. 1-2 slices, %ls",
				EnsureType<const wchar_t*>(combine_functor.second.c_str())), sco_12);
	}
	else
	{
		slice.transpose();
		DisplayMathFunction2D(slice, title + ssprintf(L", coord. 2-1 slices, %ls",
				EnsureType<const wchar_t*>(combine_functor.second.c_str())));
	}
}

template<class A3DT>
void	MathFunction3DDisplayer<A3DT>::DisplaySliceRangeAnalyzers_02(size_t slice_orientation,
		CombineSamplesFunctorType combine_functor)
{
	typename array_3d_type::slice_type::variable slice(array_md.sizes(0), array_md.sizes(2));
	for (size_t i = 0; i < array_md.sizes(0); ++i)
	{
		for (size_t j = 0; j < array_md.sizes(2); ++j)
		{
			slice.at(i, j) = combine_functor.first(array_md.GetRow({i, slice_mask(0), j}));
		}
	}
	if(slice_orientation)
	{
		DisplayMathFunction2D(slice, title + ssprintf(L", coord. 0-2 slices, %ls",
				EnsureType<const wchar_t*>(combine_functor.second.c_str())));
	}
	else
	{
		slice.transpose();
		DisplayMathFunction2D(slice, title + ssprintf(L", coord. 2-0 slices, %ls",
				EnsureType<const wchar_t*>(combine_functor.second.c_str())));
	}
}



template<class A3DT>
template<class PT, class F>
void	MathFunction3DDisplayer<A3DT>::DisplayAnimations(const value_legend &vlegend, const F& functor,
		display_3D_options display_option, size_t slice_orientation)
{
	typedef PT pixel_type;
	switch(display_option)
	{
		case display_3D_options::coord_0_animation:
			if(slice_orientation)
				DisplayAnimationBase<pixel_type>({0, slice_mask(0), slice_mask(1)}, vlegend, functor, title + L", 1-2 animation along 0 coord.");
			else
				DisplayAnimationBase<pixel_type>({0, slice_mask(1), slice_mask(0)}, vlegend, functor, title + L", 2-1 animation along 0 coord.");
			break;

		case display_3D_options::coord_1_animation:
			if(slice_orientation)
				DisplayAnimationBase<pixel_type>({slice_mask(0), 0, slice_mask(1)}, vlegend, functor, title + L", 0-2 animation along 1 coord.");
			else
				DisplayAnimationBase<pixel_type>({slice_mask(1), 0, slice_mask(0)}, vlegend, functor, title + L", 2-0 animation along 1 coord.");
			break;

		case display_3D_options::coord_2_animation:
			if(slice_orientation)
				DisplayAnimationBase<pixel_type>({slice_mask(0), slice_mask(1), 0}, vlegend, functor, title + L", 0-1 animation along 2 coord.");
			else
				DisplayAnimationBase<pixel_type>({slice_mask(1), slice_mask(0), 0}, vlegend, functor, title + L", 1-0 animation along 2 coord.");
			break;
	}
}


template<class A3DT>
size_t	MathFunction3DDisplayer<A3DT>::DecideSliceOrientation_01()
{
	return slice_orientation_01 = Decide2("Coord. 0-1 orientation:", "1=vertical, 0=horizontal", "0=vertical, 1=horizontal", slice_orientation_01);
}

template<class A3DT>
size_t	MathFunction3DDisplayer<A3DT>::DecideSliceOrientation_12()
{
	return slice_orientation_12 = Decide2("Coord. 1-2 orientation:", "2=vertical, 1=horizontal", "1=vertical, 2=horizontal", slice_orientation_12);
}

template<class A3DT>
size_t	MathFunction3DDisplayer<A3DT>::DecideSliceOrientation_02()
{
	return slice_orientation_02 = Decide2("Coord. 0-2 orientation:", "2=vertical, 0=horizontal", "0=vertical, 2=horizontal", slice_orientation_02);
}

template <class ST>
struct CombineSamplesMax
{
	public:
		template <class T>
		static T Combine(const DataArray<T> &data)
		{
			return MaxValue(data);
		}
};

template <class RGB>
struct CombineSamplesMax<RGBColorSample<RGB>>
{
	public:
		template <class T>
		static T Combine(const DataArray<T> &data)
		{
			typename ColorFunction<T, double>::ref cdata;
			cdata.UseData(data);
			return T(MaxValue(cdata.red()), MaxValue(cdata.green()),
					MaxValue(cdata.blue()));
		}
};

template<class A3DT>
auto MathFunction3DDisplayer<A3DT>::DecideCombineSamplesFunctor() -> CombineSamplesFunctorType
{
	combine_samples_functor_kind = Decide(L"Combine samples kind",
			{
				MakeButton(L"Average", CombineSamplesFunctorKind::Average),
				MakeButton(L"Max", CombineSamplesFunctorKind::Max)
			},
			SavedGUIValue(combine_samples_functor_kind));
	switch (combine_samples_functor_kind)
	{
		case CombineSamplesFunctorKind::Average:
			return CombineSamplesFunctorType([](const CombineSamplesFunctionArray &data)
				{
					return AverageValue(data);
				}, L"Average");
		case CombineSamplesFunctorKind::Max:
			return CombineSamplesFunctorType([](const CombineSamplesFunctionArray &data)
				{
					return CombineSamplesMax<std::remove_cv_t<
							typename CombineSamplesFunctionArray::value_type>>::Combine(data);
				}, L"Max");
		default:
			ForceDebugBreak();
			throw logic_error("MathFunction3DDisplayer<A3DT>::DecideCombineSamplesFunctor(): "
					"Invalid functor kind.");
	}
}



template<class A3DT>
void MathFunction3DDisplayer<A3DT>::DisplayFragment3D()
{
	range3_ST	coordinates;

	coordinates.z1() = GetUnsigned("Coordinate 0 first", 0, 0, array_md.sizes(0)-1);
	coordinates.z2() = GetUnsigned("Coordinate 0 last", array_md.sizes(0), coordinates.z1(), array_md.sizes(0));
	typename A3DT::slice_type::invariable	original_slice;
	typename A3DT::slice_type::variable	buffer_slice;

	range2_ST	coordinatesXY;

	array_md.GetSlice(original_slice, { 0, slice_mask(0), slice_mask(1) });
	MF2DInterfaceAuxiliaries::GetFragmentSizes2D(point2_ST(array_md.sizes(1), array_md.sizes(2)), coordinatesXY);

	coordinates.set_range_y(coordinatesXY.range_y());
	coordinates.set_range_x(coordinatesXY.range_x());

	wstring	fragment_title = ssprintf(L"{(%d-%d), (%d-%d), (%d-%d)} ", coordinates.z1(), coordinates.z2(), coordinates.y1(), coordinates.y2(), coordinates.x1(), coordinates.x2()) + title;

	typename A3DT::variable	display_buffer({ coordinates.delta().z(), coordinates.delta().y(), coordinates.delta().x()});

	for (size_t i = coordinates.z1(); i < coordinates.z2(); ++i)
	{
		array_md.GetSlice(original_slice, { i, slice_mask(0), slice_mask(1) });

		display_buffer.GetSlice(buffer_slice, { i-coordinates.z1(), slice_mask(0), slice_mask(1) });

		MF2DInterfaceAuxiliaries::GetFragment2D(original_slice, coordinatesXY, buffer_slice);
	}

	DisplayMathFunction3D(display_buffer, fragment_title);
}


template<class A3DT>
void	MathFunction3DDisplayer<A3DT>::Display()
{
	for (;;)
	{
		try
		{
			auto display_option = GetButtonDecision(title,
					{
						MakeButton(L"Coord 0 animation", display_3D_options::coord_0_animation),
						MakeButton(L"Coord 1 animation", display_3D_options::coord_1_animation),
						MakeButton(L"Coord 2 animation", display_3D_options::coord_2_animation),
						MakeButton(L"1-2 plane analyzer", display_3D_options::coord_12_analyzer),
						MakeButton(L"0-2 plane analyzer", display_3D_options::coord_02_analyzer),
						MakeButton(L"0-1 plane analyzer", display_3D_options::coord_01_analyzer),
						MakeButton(L"1-2 planes - all", display_3D_options::coord_12_all),
						MakeButton(L"0-2 planes - all", display_3D_options::coord_02_all),
						MakeButton(L"0-1 planes - all", display_3D_options::coord_01_all),
						MakeButton(L"Analyze histogram", display_3D_options::histogram_analyzer),
						MakeButton(L"Fragment", display_3D_options::region_animation),
						MakeButton(L"Local statistics", display_3D_options::local_statistics_analyzer),
						MakeButton(L"Extended options", display_3D_options::extended_analyzer),
						MakeButton(L"Exit display", display_3D_options::exit_3d_display)
					});
			if (display_option == display_3D_options::exit_3d_display)
				break;

			switch(display_option)
			{
				case display_3D_options::coord_01_analyzer:
					DisplaySliceAnalyzers_01(DecideSliceOrientation_01());
					break;
				case display_3D_options::coord_12_analyzer:
					DisplaySliceAnalyzers_12(DecideSliceOrientation_12());
					break;
				case display_3D_options::coord_02_analyzer:
					DisplaySliceAnalyzers_02(DecideSliceOrientation_02());
					break;

				case display_3D_options::coord_0_animation:
					GetRectifierAndDisplayAnimations<display_tag>(display_option,
							DecideSliceOrientation_12());
					break;
				case display_3D_options::coord_1_animation:
					GetRectifierAndDisplayAnimations<display_tag>(display_option,
							DecideSliceOrientation_02());
					break;
				case display_3D_options::coord_2_animation:
					GetRectifierAndDisplayAnimations<display_tag>(display_option,
							DecideSliceOrientation_01());
					break;

				case display_3D_options::coord_12_all:
					{
						auto slice_orientation = DecideSliceOrientation_12();
						auto combine_functor = DecideCombineSamplesFunctor();
						DisplaySliceRangeAnalyzers_12(slice_orientation, combine_functor);
					}
					break;

				case display_3D_options::coord_02_all:
					{
						auto slice_orientation = DecideSliceOrientation_02();
						auto combine_functor = DecideCombineSamplesFunctor();
						DisplaySliceRangeAnalyzers_02(slice_orientation, combine_functor);
					}
					break;

				case display_3D_options::coord_01_all:
					{
						auto slice_orientation = DecideSliceOrientation_01();
						auto combine_functor = DecideCombineSamplesFunctor();
						DisplaySliceRangeAnalyzers_01(slice_orientation, combine_functor);
					}
					break;

				case display_3D_options::histogram_analyzer:
					DisplayHistogram<display_tag>();
					break;

				case display_3D_options::region_animation:
					DisplayFragment3D();
					break;

				case display_3D_options::local_statistics_analyzer:
					DisplayLocalStatistics();
					break;

				case display_3D_options::extended_analyzer:
					DisplayExtended<display_tag>();
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
	}
}

template<class A3DT>
void	DisplayMathFunction3DTemplate(const A3DT &array_md, const wstring &title, ScanConverterOptions sco)
{
	MathFunction3DDisplayer<A3DT>	disp(array_md, title, sco);
	disp.Display();
}

// Type fork:
// Оборачивание некоторых слишком низкоуровневых типов данных (DataArrayMD)
// в обертки "математических" типов. В вызываемых функциях
// выполняются математические операции с анализируемым массивом.
template<class DISPLAY_TAG>
struct DisplayMathFunctionMDTypeFork;

template<>
struct DisplayMathFunctionMDTypeFork<DisplayTagScalar>
{
	template<class A3DT>
	static void Display(const A3DT &array, const wstring &title, const ScanConverterOptions &sco)
	{
		MathFunctionMD<MathFunction2D<MathFunction<typename A3DT::value_type_invariable, double,
				AlgebraicStructures::FieldTagScalar>>> buffer;
		buffer.UseData(array);
		DisplayMathFunction3DTemplate(buffer, title, sco);
	}
};

template<>
struct DisplayMathFunctionMDTypeFork<DisplayTagComplex>
{
	template<class A3DT>
	static void Display(const A3DT &array, const wstring &title, const ScanConverterOptions &sco)
	{
		ComplexFunctionMD<typename A3DT::value_type_invariable, double> buffer;
		buffer.UseData(array);
		DisplayMathFunction3DTemplate(buffer, title, sco);
	}
};

template<>
struct DisplayMathFunctionMDTypeFork<DisplayTagRGB>
{
	template<class A3DT>
	static void Display(const A3DT &array, const wstring &title, const ScanConverterOptions &sco)
	{
		ColorContainer<typename MathFunctionMD<MathFunction2D<MathFunction<
				typename A3DT::value_type, double, AlgebraicStructures::FieldTagScalar>>>::invariable,
				typename MathFunctionMD<MathFunction2D<MathFunction<
				typename A3DT::value_type::component_type, double,
				AlgebraicStructures::FieldTagScalar>>>::invariable> buffer;
		buffer.UseData(array);
		DisplayMathFunction3DTemplate(buffer, title, sco);
	}
};

template<class A3DT>
void	DisplayMathFunction3DHelper(const A3DT &array_md, const wstring &title, ScanConverterOptions sco)
{
	DisplayMathFunctionMDTypeFork<DisplayTag_t<decltype(array_md.at(index_vector()))>>::Display(array_md, title, sco);
}



}//namespace MFMDInterfaceAuxiliaries

//--------------------------------------------------------------

//TODO показ многомерных функций. 1. имеющееся подсократить; 2. добавить разбор произвольного числа размерностей

template<class A3DT>
void	DisplayMathFunction3D(const DataArrayMD<A3DT> &array_md, const wstring &title, ScanConverterOptions sco)
{
	MFMDInterfaceAuxiliaries::DisplayMathFunction3DHelper(array_md, title, sco);
}

template<class A3DT>
void	DisplayMathFunction3D(const DataArrayMD<A3DT> &array_md, const string &title, ScanConverterOptions sco)
{
	MFMDInterfaceAuxiliaries::DisplayMathFunction3DHelper(array_md, convert_to_wstring(title, e_decode_literals), sco);
}

//--------------------------------------------------------------

XRAD_END
