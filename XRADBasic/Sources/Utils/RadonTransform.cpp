#include "pre.h"

#include <XRADBasic/Sources/Utils/PhysicalUnits.h>
#include <XRADBasic/FFT2D.h>

#include "RadonTransform.h"

/********************************************************************
	created:	2016/09/13
	created:	13:9:2016   13:35
	filename: 	q:\programs\CTDensityArtifact\sources\RadonTransform.cpp
	file path:	q:\programs\CTDensityArtifact\sources
	file base:	RadonTransform
	file ext:	cpp
	author:		kns
	
	purpose:	
*********************************************************************/

XRAD_BEGIN
namespace
{
RealInterpolator2D interpolator;

int	interpolator_size(32);
int	interpolators_amount(256);
}

void InitRadonTransform(size_t in_interpolator_size, size_t in_interpolators_amount)
{
	interpolator_size = int(in_interpolator_size);
	interpolators_amount = int(in_interpolators_amount);

	interpolator.InitFilters(interpolators_amount, interpolators_amount, SincFilterGenerator2D<FIRFilter2DReal>(interpolator_size, interpolator_size));
//	interpolator.InitFilters(interpolators_amount, interpolators_amount, BesselFilterGenerator<FIRFilter2DReal>(interpolator_size, interpolators2D::besselRadiusMIN_LOST));
//	interpolator.InitFilters(interpolators_amount, interpolators_amount, ISplineFilterGenerator2D<FIRFilter2DReal>(3));

	//	interpolator.InitFilters(interpolators_amount, interpolators_amount, BesselFilterGenerator<FIRFilter2DReal>(interpolator_size, interpolators2D::besselRadius_ISOTROPIC));
//	interpolator.InitFilters(interpolators_amount, interpolators_amount, SincFilterGenerator2D<FIRFilter2DReal>(interpolator_size, interpolator_size));
	interpolator.SetExtrapolationMethod(extrapolation::by_zero);
}

//	Обратное преобразование Радона (формирование таблицы коэффициентов затухания по набору проекционных рентгенограмм)
//	enlarge_factor не используется, нужно исследовать, даст ли он что
void RadonTransformReverse(RealFunction2D_F32 &restored, const RealFunction2D_F32 &radon, size_t /*enlarge_factor*/, ProgressProxy pp)
{
	size_t vs = restored.vsize();
	size_t hs = restored.hsize();
	XRAD_ASSERT_THROW(vs==hs);

	size_t n_angles(radon.vsize());
	size_t n_detectors(radon.hsize());

	double x0(hs/2);
	double y0(vs/2);

	physical_angle d_angle(degrees(180./n_angles));

	double s0(double(n_detectors/2));
	double ds(double(hs)/n_detectors);


	size_t	angle_expand = interpolator_size/2;
	ComplexFunction2D_F32 radonc(radon.vsize() + interpolator_size, n_detectors, complexF32(0));
	radonc.CopyData(radon);

	radonc.roll(angle_expand, 0);

	//виртуально увеличиваем угол поворота трубки 
	for(size_t i = 0; i < angle_expand; ++i)
	{
		size_t	source1 = i+radon.vsize()-angle_expand;
		size_t	target1 = i;
		size_t	source2 = i;
		size_t	target2 = i+radon.vsize()+angle_expand;

		for(size_t j = 0; j < radon.hsize(); ++j)
		{
			radonc.at(target1, n_detectors - j - 1) = radon.at(source1, (j+1)%radon.hsize());
			radonc.at(target2, n_detectors - j - 1) = radon.at(source2, (j+1)%radon.hsize());
		}
	}

	FFTf(radonc, fftRevRollBoth, fftNone, e_use_omp);
	
	RealFunction2D_F32 real_d(real(radonc));
	RealFunction2D_F32 imaginary_d(imag(radonc));
	ComplexFunction2D_F32 restoredc(vs, hs);

	ProgressBar	progress(pp);
	progress.start("Radon transform reverse", restoredc.vsize());
	for(size_t i = 0; i < vs; ++i)
	{
		double y = double(i) - y0;
		#pragma omp parallel for schedule (guided)
		for(ptrdiff_t j = 0; j < ptrdiff_t(hs); ++j)
		{
			double x = double(j) - x0;
			double radius = sqrt(square(y)+square(x));
			double s;
			double	angle = std::atan2(x, y);
			double angle_index = angle / d_angle.radians();
			if(angle_index>0)
			{
				s = s0+radius/ds;
			}
			else
			{
				s = s0 - radius/ds;
				angle_index+=n_angles;
			}
			if(!in_range(s, 0, radon.hsize()-1))
			{
				restoredc.at(i,j) = complexF64(0);
			}
			else
			{
				restoredc.at(i,j).re = real_d.in(angle_index + angle_expand, s, &interpolator);
				restoredc.at(i,j).im = imaginary_d.in(angle_index + angle_expand, s, &interpolator);
			}
		}
		++progress;
	}
	progress.end();

	FFTf(restoredc, fftRevRollBefore, fftRevRollBefore, e_use_omp);

	restoredc.roll(ptrdiff_t(restored.vsize()/2), ptrdiff_t(restored.hsize()/2));
	restored.CopyData(restoredc.real());
}

void RadonTransformForward(RealFunction2D_F32 &radon, const RealFunction2D_F32 &original, size_t enlarge_factor, ProgressProxy pp)
{
	size_t	vs = original.vsize();
	size_t	hs = original.hsize();
	XRAD_ASSERT_THROW(vs == hs);

	size_t n_angles(radon.vsize());
	size_t n_detectors(radon.hsize());

	size_t interpolated_vs = enlarge_factor*vs;
	size_t interpolated_hs = enlarge_factor*hs;

	double x0(interpolated_hs / 2);
	double y0(interpolated_vs / 2);
	double s0 = -double(interpolated_hs / 2);
	double ds(double(interpolated_hs) / (n_detectors));
	physical_angle d_angle = degrees(180. / n_angles);

	ComplexFunction2D_F32 radonc(radon);
	ComplexFunction2D_F32 enlarged_c(interpolated_vs, interpolated_hs);

	enlarged_c.CopyData(original);
	enlarged_c.roll(-ptrdiff_t(vs / 2), -ptrdiff_t(hs / 2));
	
	FFTf(enlarged_c, fftFwdRollAfter, fftFwdRollAfter, e_use_omp);

 	RealFunction2D_F32 real_d(real(enlarged_c));
 	RealFunction2D_F32 imaginary_d(imag(enlarged_c));

	ProgressBar progress(pp);
	progress.start("Radon transform forward", n_angles);
	for(size_t i = 0; i < n_angles; ++i)
	{
		#pragma omp parallel for schedule (guided)
		for(ptrdiff_t j = 0; j < ptrdiff_t(n_detectors); ++j)
		{
			double angle = i*d_angle.radians();
			double s = s0 + j*ds;
			double y = y0 + s*cos(angle);
			double x = x0 + s*sin(angle);
			radonc.at(i,j).re = real_d.in(y, x, &interpolator);
			radonc.at(i,j).im = imaginary_d.in(y, x, &interpolator);
		}
		++progress;
	}
	progress.end();

	FFTf(radonc, fftFwdRollBoth, fftNone, e_use_omp);

	radon.CopyData(radonc.real());
	radon *= enlarge_factor;
}



void MakeIsotropic(RealFunction2D_F32& original)
{
	size_t vs = original.vsize();
	size_t hs = original.hsize();

	size_t vs2 = vs/2;
	size_t hs2 = hs/2;

	ComplexFunction2D_F32 data_to_process(original);
	FFTf(data_to_process, fftFwdRollAfter, fftFwdRollAfter, e_use_omp);
	for(size_t i = 0; i< vs; ++i)
	{
		for(size_t j = 0; j< hs; ++j)
		{
			double radius = hypot(double(i)- vs2, double(j) - hs2);
			if(radius > vs2)
			{
				data_to_process.at(i, j) = 0;
			}
		}
	}
	FFTf(data_to_process, fftRevRollBefore, fftRevRollBefore, e_use_omp);

	original.CopyData(data_to_process.real());

}


XRAD_END
