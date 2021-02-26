/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef DistributionTransformer_h__
#define DistributionTransformer_h__

/********************************************************************
	created:	2015/11/11
	created:	11:11:2015   13:30
	author:		kns
*********************************************************************/

#include "DistributionContainer.h"
#include <XRADBasic/Sources/Containers/UniversalInterpolation.h>


XRAD_BEGIN

class	DistributionTransformer : public TableDistributionContainer
	{
		void	GenerateTransformFunction();
		const size_t	n_levels;
		const double x_factor;
		RealFunctionF64	m_transform_table;

	public:

		DistributionTransformer(size_t nl): n_levels(nl), m_transform_table(nl), x_factor(nl-1){}
		void	CreateTransformFunctionPDF(const RealFunctionF64 &in_pdf, double x0, double dx);
		void	CreateTransformFunctionCDF(const RealFunctionF64 &in_cdf, double x0, double dx);

		const RealFunctionF64 &transform_table(){return m_transform_table;}

		double	ToUniform(double x) const
			{
			// из заданного таблицей преобразует в равномерное на [0,1]
//			return cdf_table().in((x-x0())/dx(), &interpolators::linear)
			return cdf(x);
			}

		double	FromUniform(double x) const
			{
			// из равномерного на [0,1] преобразует в распределение, заданное таблицей
			return m_transform_table.in(x*x_factor, &interpolators::linear);
			}
	};


XRAD_END

#endif // DistributionTransformer_h__