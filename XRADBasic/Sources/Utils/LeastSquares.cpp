#include "pre.h"
#include "LeastSquares.h"
#include "LeastSquaresAlgorithms.h"
#include "StatisticUtils.h"



XRAD_BEGIN


void	DetectLSPolynomUniformGrid(const RealFunctionF64 &samples, RealVectorF64 &coefficients)
	{
	least_squares_classes::LSDetectorUniformGrid<RealFunctionF64> detector(samples);
	DetectLSPolynomTemplate(detector, coefficients);
	}

void	DetectLSPolynomNonUniformGrid(const RealFunctionF64 &samples, const RealFunctionF64 &grid, RealVectorF64 &coefficients)
	{
	least_squares_classes::LSDetectorNonUniformGrid<RealFunctionF64> detector(samples, grid);
	DetectLSPolynomTemplate(detector, coefficients);
	}

void	DetectLSPolynomWeighted(const RealFunctionF64 &samples, const RealFunctionF64 &grid, const RealFunctionF64 &weights, RealVectorF64 &coefficients)
	{
	least_squares_classes::LSDetectorNonUniformGridWeighted<RealFunctionF64> detector(samples, grid, weights);
	DetectLSPolynomTemplate(detector, coefficients);
	}



void	DetectLSUniversalUniformGrid(const RealFunctionF64 &samples, const abstract_LS_basis_function&f,  RealVectorF64 &coefficients)
	{
	least_squares_classes::LSDetectorUniformGrid<RealFunctionF64> detector(samples);
	DetectLSUniversalTemplate(detector, f, coefficients);
	}

void	DetectLSUniversalNonUniformGrid(const RealFunctionF64 &samples, const abstract_LS_basis_function&f,  const RealFunctionF64 &grid, RealVectorF64 &coefficients)
	{
	least_squares_classes::LSDetectorNonUniformGrid<RealFunctionF64> detector(samples, grid);
	DetectLSUniversalTemplate(detector, f, coefficients);
	}

void	DetectLSUniversalWeighted(const RealFunctionF64 &samples, const abstract_LS_basis_function&f,  const RealFunctionF64 &grid, const RealFunctionF64 &weights, RealVectorF64 &coefficients)
	{
	least_squares_classes::LSDetectorNonUniformGridWeighted<RealFunctionF64> detector(samples, grid, weights);
	DetectLSUniversalTemplate(detector, f, coefficients);
	}



XRAD_END