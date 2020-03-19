#include "DataArrayAnalyze.h"

XRAD_BEGIN


//--------------------------------------------------------------
//
//	FIRFilterKernel initializations
//
//--------------------------------------------------------------



//	Написать для всего этого комментарии!!!

template<XRAD__FilterKernel_template>
void	InitFIRFilterGaussian(FIRFilterKernel<XRAD__FilterKernel_template_args> &filter, double dispersion, double value_at_edge, extrapolation::method ex)
{
	filter.SetExtrapolationMethod(ex);
	filter.SetFilteringAlgorithm(fir_scan_data);

	if(!dispersion)
	{
		filter.realloc(1);
		filter[0] = 1;
		return;
	}

	size_t	filter_size = 1 + static_cast<size_t>(2.*dispersion*sqrt(-2.*log(value_at_edge)));
	if(filter_size < 3) filter_size = 3;
	if(!(filter_size%2)) ++filter_size;
	size_t	fs2 = filter_size/2;

	filter.realloc(filter_size);
	if(dispersion < 0.5)
	{
		double	h = square(dispersion)/2;
		filter[0] = filter[2] = h;
		filter[1] = 1.-2*h;
	}
	else
	{
		for(size_t i = 0; i < filter_size; ++i)
		{
//TODO	снова об опасности бездумного введения беззнаковых типов. строка ниже искажала данные.
// 			double	x = i-fs2;
			double	x = double(i)-double(fs2);
			double	filter_value = gauss(x, dispersion);
			filter[i] = filter_value;
		}
	}
	filter /= value_at_edge;
	filter.SetSmoothingKernelNormalizer();
}

template<XRAD__FilterKernel_template>
void	InitIIRFilterBiexponential(FIRFilterKernel<XRAD__FilterKernel_template_args> &filter, double dispersion, extrapolation::method ex)
{
	filter.SetExtrapolationMethod(ex);
	filter.SetFilteringAlgorithm(iir_bidirectional);
	filter.realloc(2);
	if(dispersion <= 0)
	{
		throw invalid_argument("InitFIRFilterBiexponential(), invalid dispersion value");
	}
	double filter_value = exp(-sqrt(2.)/dispersion);
	filter[0] = filter_value;
	filter[1] = -filter_value;
}


template<XRAD__FilterKernel_template>
void	InitFIRFilterUnsharpMask(FIRFilterKernel<XRAD__FilterKernel_template_args> &filter, double strength,
	double radius_unsharp, double radius_sharp,
	extrapolation::method ex)
{
	if(radius_unsharp <= radius_sharp)
	{
		throw invalid_argument(string("Invalid params: InitFIRFilterUnsharpMask(") + typeid(FIRFilterKernel<XRAD__FilterKernel_template_args>).name() +
			ssprintf(", strength = %g, radius_unsharp = %g, radius_sharp = %g, ex_method = %d)",
			strength, radius_unsharp, radius_sharp, ex));
	}

	FIRFilterKernel<double, double, AlgebraicStructures::FieldTagScalar>	sharp, unsharp;

	InitFIRFilterGaussian(sharp, radius_sharp);
	sharp /= ElementSum(sharp);
	sharp.SetSmoothingKernelNormalizer();
	InitFIRFilterGaussian(unsharp, radius_unsharp);
	unsharp /= ElementSum(sharp);
	unsharp.SetSmoothingKernelNormalizer();

	filter.resize(unsharp.size());
	for(size_t i = 0; i < unsharp.size(); ++i)
	{
		filter[i] = -unsharp[i]*strength;
	}
	size_t	i0 = unsharp.size()/2 - sharp.size()/2;
	for(size_t i = 0; i < sharp.size(); ++i)
	{
		filter[i0+i] += sharp[i]*(strength + 1);
	}

	filter.SetExtrapolationMethod(ex);
}



XRAD_END
