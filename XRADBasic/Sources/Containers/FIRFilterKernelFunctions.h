// file FIRFilterKernelFunctions.h
//--------------------------------------------------------------
#ifndef XRAD__File_FIRFilterKernelFunctions_h
#define XRAD__File_FIRFilterKernelFunctions_h
//--------------------------------------------------------------

#include "FIRFilterKernel.h"

XRAD_BEGIN

//--------------------------------------------------------------

template<XRAD__FilterKernel_template>
void	InitIIRFilterBiexponential(FIRFilterKernel<XRAD__FilterKernel_template_args> &filter, double dispersion, extrapolation::method ex = extrapolation::by_last_value);

template<XRAD__FilterKernel_template>
void	InitFIRFilterGaussian(FIRFilterKernel<XRAD__FilterKernel_template_args> &filter, double dispersion, double value_at_edge = 0.3, extrapolation::method ex = extrapolation::by_last_value);

template<XRAD__FilterKernel_template>
void	InitFIRFilterUnsharpMask(FIRFilterKernel<XRAD__FilterKernel_template_args> &filter, double strength, double radius_unsharp = 1, double radius_sharp = 0, extrapolation::method ex = extrapolation::by_last_value);

//--------------------------------------------------------------

XRAD_END

#include "FIRFilterKernelFunctions.hh"

//--------------------------------------------------------------
#endif // XRAD__File_FIRFilterKernelFunctions_h
