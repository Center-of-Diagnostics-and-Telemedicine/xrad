/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#include "pre.h"
#include "RandomNoiseGenerator.h"

XRAD_BEGIN


//--------------------------------------------------------------
//
//	члены классов-генераторов шумов
//
//--------------------------------------------------------------




GaussianNoiseGenerator::GaussianNoiseGenerator(double average, double sigma, int nl) : TableRandomNoiseGenerator(nl), DistributionLaw(average, sigma)
{
	RealFunctionF64	cdf(nl);

	const	double	width = 10*sigma;
	const	double	x0 = average - width/2;
	const	double	dx = width/nl;
	for(int i = 0; i < nl; ++i)
	{
		double	x = x0 + i*dx;
		cdf[i] = DistributionLaw.cdf(x);
	}

	CreateTransformFunctionCDF(cdf, x0, dx);
}


RicianNoiseGenerator::RicianNoiseGenerator(double nu, double sigma, int nl) : TableRandomNoiseGenerator(nl), DistributionLaw(nu, sigma)
{
	RealFunctionF64	cdf(nl);
	const	double	peak_half_width = 5*sigma;
	const	double	x_max = nu + peak_half_width;
	const	double	x0 = (nu < peak_half_width) ? 0 : (nu - peak_half_width);

	const double	dx = (x_max-x0)/nl;
//	const double	sigma_square = sigma*sigma;

	for(int i = 0; i < nl; ++i)
	{
		double	x = x0 + i*dx;
//			cdf[i] = rician_pdf(x, nu, sigma);
		cdf[i] = DistributionLaw.cdf(x);
	}

	CreateTransformFunctionCDF(cdf, x0, dx);
}

RayleighNoiseGenerator::RayleighNoiseGenerator(double sigma, int nl) : TableRandomNoiseGenerator(nl), DistributionLaw(sigma)
{
	RealFunctionF64	cdf(nl);
	const	double	x_max = 5*sigma;
	const double	dx = x_max/nl;
//	double	sigma_square = sigma*sigma;

	for(int i = 0; i < nl; ++i)
	{
		double	x = i*dx;
		cdf[i] = DistributionLaw.cdf(x);
	}
	CreateTransformFunctionCDF(cdf, 0, dx);
}


XRAD_END