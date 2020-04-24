#ifndef MultimodalFrameContainer_h__
#define MultimodalFrameContainer_h__
/********************************************************************
	created:	2016/01/27
	created:	27:1:2016   12:19
	author:		kns
*********************************************************************/

#include "FrameBitmapContainer.h"

namespace XRAD_GUI
{

XRAD_USING



template<class T>
struct	brightness_correction_functor
{
	private:
		typedef	T pixel_component_t;
		const double minval, maxval;
		const double gamma;

		double rangeval;
		double gamma_correction_factor;

		const pixel_component_t	max_sample_value, min_sample_value;

// 		mutable double	r,g,b;
		template<class T1>
		void transform(pixel_component_t &x, const T1 &y) const
		{
			gamma == 1 ?
				x = y < minval ? min_sample_value : y > maxval ? max_sample_value:  (y-minval)/rangeval :
				x = y < minval ? min_sample_value : y > maxval ? max_sample_value:  gamma_correction_factor * pow((y-minval)/rangeval, gamma);
		}

	public:
		brightness_correction_functor(const double in_min, const double in_max, const double in_gamma) :
			min_sample_value(0),
			max_sample_value(std::numeric_limits<T>::max()),
			minval(in_min),
			maxval(in_max),
			gamma(in_gamma)
		{
			rangeval = ((in_max-in_min)/double(max_sample_value));
			gamma_correction_factor = (pow(double(max_sample_value), 1.-gamma));
		}

		template<class T1>
		void operator()(pixel_component_t &x, const T1 &y) const {transform(x,y);}

		template<class T1>
		void operator()(ColorPixel &x, const RGBColorSample<T1> &y) const
		{
			transform(x.red(), y.red());
			transform(x.green(), y.green());
			transform(x.blue(), y.blue());
		}

		template<class T1, class ST>
		void operator()(pixel_component_t &x, const ComplexSample<T1,ST> &y) const {transform(x, cabs(y));}
};



class MultimodalFrameContainer
{
		xrad::DataArray2D<xrad::DataArray<ColorPixel> >		rgba_ui8;
		xrad::DataArray2D<xrad::DataArray<uint8_t> >		gray_ui8;
		xrad::DataArray2D<xrad::DataArray<int16_t> >		gray_i16;
		xrad::DataArray2D<xrad::DataArray<uint16_t> >		gray_ui16;
		xrad::DataArray2D<xrad::DataArray<int32_t> >		gray_i32;
		xrad::DataArray2D<xrad::DataArray<uint32_t> >		gray_ui32;
		xrad::DataArray2D<xrad::DataArray<float> >			gray_f32;
		xrad::DataArray2D<xrad::DataArray<float> >			gray_f64;// именно от float, для отображения точность неважна, а памяти может уйти много. см. тоже для complexF64
		xrad::DataArray2D<xrad::DataArray<ColorSampleF32> >	rgb_f32;

		xrad::DataArray2D<xrad::DataArray<complexF32> >		complex_f32;
		xrad::DataArray2D<xrad::DataArray<complexF32> >		complex_f64;

		xrad::DataArray2D<xrad::DataArray<complexI32> >		complex_i32;
		xrad::DataArray2D<xrad::DataArray<complexI16> >		complex_i16;

		display_sample_type	ContainerMode();

	public:

		QString	GetValueLegend(int row_no, int col_no);
		void	ImportFrame(const void *in_data, int n_rows, int n_columns, display_sample_type sample_type);
		void	GenerateBitmap(FrameBitmapContainer	&bitmap, double min_value, double max_value, double gamma, bool transpose);
		double	MaxComponentValue();
		double	MinComponentValue();
};



}

#include "MultimodalFrameContainer.hh"

#endif // MultimodalFrameContainer_h__
