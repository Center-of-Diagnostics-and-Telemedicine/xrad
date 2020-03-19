#include "pre.h"
#include "DistributionContainer.h"
#include <XRADBasic/Sources/Math/SpecialFunctions.h>
#include <XRADBasic/Sources/Containers/UniversalInterpolation.h>
#include "BSplines.h"

XRAD_BEGIN


GaussianDistributionContainer::GaussianDistributionContainer( double in_average, double in_sigma ) :average(in_average), sigma(in_sigma)
	{
	}

double GaussianDistributionContainer::pdf( double x ) const
	{
	return	gaussian_pdf(x,average,sigma);
	}

double GaussianDistributionContainer::cdf( double x ) const
	{
	return	gaussian_cdf(x,average,sigma);
	}

string GaussianDistributionContainer::name() const
	{
	return ssprintf("Gaussian, average=%g, sigma=%g", average,sigma);
	}


RayleighDistributionContainer::RayleighDistributionContainer( double in_sigma ) :sigma(in_sigma)
	{
	}

double RayleighDistributionContainer::pdf( double x) const
	{
	return rayleigh_pdf(x,sigma);
	}

double RayleighDistributionContainer::cdf( double x ) const
	{
	return	rayleigh_cdf(x,sigma);
	}

string RayleighDistributionContainer::name() const
	{
	return ssprintf("Rayleigh, sigma=%g", sigma);
	}

RicianDistributionContainer::RicianDistributionContainer(double in_nu, double in_sigma) :sigma(in_sigma), nu(in_nu)
	{
	}

double RicianDistributionContainer::pdf( double x ) const
	{
	return rician_pdf(x,nu,sigma);
	}

double RicianDistributionContainer::cdf( double x ) const
	{
	return rician_cdf(x, nu, sigma);
	}

string RicianDistributionContainer::name() const
	{
	return ssprintf("Rician, nu=%g, sigma=%g", nu, sigma);
	}

double IrwingHallDistribution::pdf( double x ) const
	{
	return b_spline(n_components, x - average + double(n_components)/2);
	}

double IrwingHallDistribution::cdf( double x ) const
	{
	return b_spline_integral(n_components, x - average + double(n_components)/2);
	}

xrad::string IrwingHallDistribution::name() const
	{
	return ssprintf("Irwing-Hall noise, average = %g, n_components = %d", average, n_components);
	}



//--------------------------------------------------------------
//
//	произвольное табличное распределение

void TableDistributionContainer::SetPDF(const RealFunctionF64 &in_pdf, double in_x0, double in_dx)
	{
	m_x0 = in_x0;
	m_dx = in_dx;
	// без проверки математической корректности введенных данных (положительность плотности, интеграл = 1 и т.п.)
	m_pdf.MakeCopy(in_pdf);
	m_cdf.MakeCopy(in_pdf);

	RealFunctionF64::iterator	it1 = m_cdf.begin(), it2 = m_cdf.begin()+1, ie = m_cdf.end();

	for(; it2<ie; ++it1, ++it2)
		{
		*it2 += *it1;
		}
	}

void TableDistributionContainer::SetCDF(const RealFunctionF64 &in_cdf, double in_x0, double in_dx)
	{
	m_x0 = in_x0;
	m_dx = in_dx;
	// без проверки математической корректности введенных данных (монотонность распределения, единица крайне правого значения и т.п.)
	m_pdf.MakeCopy(in_cdf);
	m_cdf.MakeCopy(in_cdf);

	RealFunctionF64::reverse_iterator	it1 = m_pdf.rbegin(), it2 = m_pdf.rbegin()+1, ie = m_pdf.rend();

	for(; it2<ie; ++it1, ++it2)
		{
		*it1 -= *it2;
		}
	}

double TableDistributionContainer::pdf(double x) const
	{
	double	position = (x-m_x0)/m_dx;
	return m_pdf.in(position, &interpolators::linear);
	}

double TableDistributionContainer::cdf(double x) const
	{
	double	position = (x-m_x0)/m_dx;
	return m_cdf.in(position, &interpolators::linear);
	}

xrad::string TableDistributionContainer::name() const
	{
	return m_name;
	}

XRAD_END
