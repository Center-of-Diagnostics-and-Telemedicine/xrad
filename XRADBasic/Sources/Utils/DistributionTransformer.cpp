/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#include "pre.h"
#include "DistributionTransformer.h"

/********************************************************************
	created:	2015/11/16
	created:	16:11:2015   11:46
	author:		kns
*********************************************************************/

XRAD_BEGIN


void DistributionTransformer::GenerateTransformFunction()
	{
	size_t	j = 0;
	// i по функции преобразования, j по функции плотности
	for(size_t i = 0; i < n_levels; ++i)
		{
		double	x = double(i)/n_levels;
		while(cdf_table()[j] < x && j < cdf_table().size()) ++j;
		double	dj(0);

		if(j>0)
			{
			dj = (cdf_table()[j] - x)/(cdf_table()[j] - cdf_table()[j-1]);
			}

		m_transform_table[i] = x0() + (j - dj)*dx();
		}
//	m_transform_table[0] = 2*m_transform_table[1] - m_transform_table[2];
	}

void	DistributionTransformer :: CreateTransformFunctionCDF(const RealFunctionF64 &in_cdf, double x0, double dx)
	{
	SetCDF(in_cdf, x0, dx);
	GenerateTransformFunction();
	}



void	DistributionTransformer :: CreateTransformFunctionPDF(const RealFunctionF64 &pdf, double x0, double dx)
	{
	// эта функция только для тех случаев, когда непосредственное вычисление ФР затруднительно,
	// а ФП вычисляется легко
	SetPDF(pdf, x0, dx);
	GenerateTransformFunction();

// 	RealFunctionF64	cdf(pdf);
// 	for(int i = 1; i < n_levels; ++i)
// 		{
// 		cdf[i] += cdf[i-1];
// 		}
// 	cdf /= cdf[n_levels-1];
// 	CreateTransformFunctionCDF(cdf_table(), x0, dx);
	}



XRAD_END