#ifndef __RandomNoiseGenerator_h
#define __RandomNoiseGenerator_h

//--------------------------------------------------------------
//
//	created:	2014/11/20
//	created:	20.11.2014   11:56
//	author:		kns
//
//--------------------------------------------------------------


#include "DistributionTransformer.h"

XRAD_BEGIN


//--------------------------------------------------------------
//
//	генераторы шумов по заданному закону
//


class	RandomNoiseGenerator
	{
		public:
		virtual	const DistributionContainer &GetDistributionContainer() = 0;
		virtual double	Generate() const = 0;
	};

class	GaussianNoiseGeneratorEssential: public RandomNoiseGenerator
	{
	public:
		const GaussianDistributionContainer	DistributionLaw;
		virtual	const DistributionContainer &GetDistributionContainer(){return DistributionLaw;}
		GaussianNoiseGeneratorEssential(double average, double sigma):DistributionLaw(average,sigma){}
		virtual double	Generate() const {return RandomGaussian(DistributionLaw.average,DistributionLaw.sigma);}
	};



class	GeneratorRicianEssential: public RandomNoiseGenerator
	{
	GaussianNoiseGeneratorEssential g;
	public:
		const RicianDistributionContainer	DistributionLaw;
		virtual	const DistributionContainer &GetDistributionContainer(){return DistributionLaw;}

		GeneratorRicianEssential(double in_nu, double in_sigma) : g(in_nu/sqrt(2.),in_sigma), DistributionLaw(in_nu, in_sigma){}
		virtual double	Generate() const
			{
			return sqrt(square(g.Generate()) + square(g.Generate()));
			}
	};

//	табличный генератор через обратное преобразование функции распределение

class	TableRandomNoiseGenerator: public RandomNoiseGenerator, public DistributionTransformer
	{
	private:

	protected:
		TableRandomNoiseGenerator(int nl) : DistributionTransformer(nl){}

	public:

		virtual double	Generate() const
			{
			double index = RandomUniformF64(0, 1);
			return FromUniform(index);
			}
	};

//--------------------------------------------------------------

class	GaussianNoiseGenerator: public TableRandomNoiseGenerator
	{
	public:
		const GaussianDistributionContainer	DistributionLaw;
		virtual	const DistributionContainer &GetDistributionContainer(){return DistributionLaw;}
		GaussianNoiseGenerator(double average, double sigma, int nl = 4096);
	};


class	RicianNoiseGenerator: public TableRandomNoiseGenerator
	{
	public:
		const RicianDistributionContainer	DistributionLaw;
		virtual	const DistributionContainer &GetDistributionContainer(){return DistributionLaw;}
		RicianNoiseGenerator(double nu, double sigma, int nl = 16384);
	};

class	RayleighNoiseGenerator: public TableRandomNoiseGenerator
	{
	public:
		const RayleighDistributionContainer	DistributionLaw;
		virtual	const DistributionContainer &GetDistributionContainer(){return DistributionLaw;}
		RayleighNoiseGenerator(double sigma, int nl = 4096);
	};

class	IrwingHallNoiseGenerator: public RandomNoiseGenerator
	{
	public:
		const IrwingHallDistribution	DistributionLaw;
		virtual	const DistributionContainer &GetDistributionContainer(){return DistributionLaw;}
		IrwingHallNoiseGenerator(double in_average, int in_n_components) : DistributionLaw(in_n_components, in_average){}
		virtual double	Generate() const
			{
			double result = 0;
			for(int j = 0; j < DistributionLaw.n_components; ++j)
				{
				result += RandomUniformF64(-0.5,0.5);
				}
			return result += DistributionLaw.average;
			}
	};

//--------------------------------------------------------------

XRAD_END

#endif //__RandomNoiseGenerator_h
