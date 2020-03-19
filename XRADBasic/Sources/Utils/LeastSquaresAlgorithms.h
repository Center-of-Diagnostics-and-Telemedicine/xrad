#ifndef __least_squares_algorithms_h
#define __least_squares_algorithms_h

#include <XRADBasic/MathFunctionTypes.h>
#include <XRADBasic/MathMatrixTypes.h>
#include <XRADBasic/Sources/Containers/DataArrayAnalyze.h>
#include <XRADBasic/Sources/Containers/DataArrayAnalyze2D.h>

XRAD_BEGIN

namespace least_squares_classes
{

template<class ARR_T>
class	LSDetector
	{
		const ARR_T &samples;
		const size_t n_samples;
		RealMatrixF64 matrix;

		void	PrepareLinearSystem(size_t order, const x_power_function&);
		void	PrepareLinearSystem(size_t order, const abstract_LS_basis_function&);

		virtual double	grid(size_t k) = 0;
		virtual double	sample(size_t k) {return samples[k];}
		virtual	double	weight(size_t k) = 0;
	public:
		LSDetector(const ARR_T &in_samples) : samples(in_samples), n_samples(in_samples.size()){}

		void DetectLSConst(double &a0){a0 = AverageValue(samples);}
		void DetectLSLinear(double &a0, double &a1);
		void DetectLSSquare(double &a0, double &a1, double &a2);

		template<class function_t, XRAD__LinearVector_template>
			void DetectLSUniversal(LinearVector<XRAD__LinearVector_template_args> &coefficients, const function_t &f);
	};

//--------------------------------------------------------------
//
// класс с равномерной сеткой
//
template<class ARR_T>
class	LSDetectorUniformGrid : public LSDetector<ARR_T>
	{
		PARENT(LSDetector<ARR_T>);

		double	grid(size_t k){return double(k);}
		double	weight(size_t ) {return 1;}
	public:
		LSDetectorUniformGrid(const ARR_T &in_samples) : parent(in_samples){}
	};



//--------------------------------------------------------------
//
// класс с неравномерной сеткой
//
template<class ARR_T>
class	LSDetectorNonUniformGrid : public LSDetector<ARR_T>
	{
		PARENT(LSDetector<ARR_T>);
		double	grid(size_t k){return grid_points[k];}
		const ARR_T &grid_points;
		double	weight(size_t ) {return 1;}
	public:
		LSDetectorNonUniformGrid(const ARR_T &in_samples, const ARR_T &in_grid_points) : parent(in_samples), grid_points(in_grid_points){}
	};

//--------------------------------------------------------------
//
// класс со взвешенными отсчетами; сетка только неравномерная,
// чтобы не плодить версий
//
template<class ARR_T>
class	LSDetectorNonUniformGridWeighted : public LSDetector<ARR_T>
	{
		PARENT(LSDetector<ARR_T>);
		const ARR_T &weights;
		const ARR_T &grid_points;

		double	grid(size_t k){return grid_points[k];}
		double	weight(size_t k) {return weights[k];}

	public:
		LSDetectorNonUniformGridWeighted(const ARR_T &in_samples, const ARR_T &in_grid_points, const ARR_T &in_weights) : parent(in_samples), grid_points(in_grid_points), weights(in_weights){}
	};

} // namespace least_squares_classes

XRAD_END

#include "LeastSquaresAlgorithms.hh"

#endif //__least_squares_algorithms_h
