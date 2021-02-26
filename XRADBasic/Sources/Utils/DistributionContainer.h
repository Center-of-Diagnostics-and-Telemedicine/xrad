#ifndef XRAD__File_distribution_container_h
#define XRAD__File_distribution_container_h

#include "StatisticUtils.h"

XRAD_BEGIN

class	DistributionContainer
	{
	public:
		virtual	double pdf(double x) const = 0;
		virtual double cdf(double x) const = 0;
		virtual string name() const = 0;
	};


class	GaussianDistributionContainer: public DistributionContainer
	{
	public:

		double	average, sigma;

		GaussianDistributionContainer(double in_average = 0, double in_sigma = 0);
		virtual double	pdf(double x) const;
		virtual	double	cdf(double x) const;
		virtual string	name() const;
	};


class	RayleighDistributionContainer: public DistributionContainer
	{
	public:

		double sigma;

		RayleighDistributionContainer(double in_sigma = 0);
		virtual double	pdf(double x) const;
		virtual	double	cdf(double x) const;
		virtual string	name() const;
	};


class	RicianDistributionContainer: public DistributionContainer
	{
	public:

		double sigma, nu;

		RicianDistributionContainer(double in_nu = 0, double in_sigma = 0);
		virtual double	pdf(double x) const;
		virtual	double	cdf(double x) const;
		virtual string	name() const;
	};


// распределение ирвинга-холла, сумма n равномерно распределенных величин
class	IrwingHallDistribution: public DistributionContainer
	{
	public:
		const int	n_components;
		const double	average;

		IrwingHallDistribution(int in_count, double in_average) : average(in_average), n_components(in_count){}

		double	pdf(double x) const;
		double	cdf(double x) const;
		string	name() const;
	};

//
//	произвольное табличное распределение
//

class	TableDistributionContainer: public DistributionContainer
	{
		double	m_x0, m_dx;
		RealFunctionF64	m_pdf;
		RealFunctionF64	m_cdf;
		string	m_name;

	public:
		void	SetPDF(const RealFunctionF64 &in_pdf, double in_x0, double in_dx);
		void	SetCDF(const RealFunctionF64 &in_cdf, double in_x0, double in_dx);

		const RealFunctionF64 &pdf_table() const {return m_pdf;}
		const RealFunctionF64 &cdf_table() const {return m_cdf;}
		double x0() const {return m_x0;}
		double dx() const {return m_dx;}


		double	pdf(double x) const;
		double	cdf(double x) const;
		string	name() const;
	};

XRAD_END

#endif // XRAD__File_distribution_container_h
