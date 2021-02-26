#ifndef XRAD__File_exponential_blur_algorithms_h
#define XRAD__File_exponential_blur_algorithms_h

#include <XRADBasic/Sources/Containers/SpaceCoordinates.h>
#include <XRADBasic/Sources/Algebra/FieldTraits.h>
#include <XRADBasic/Sources/Containers/DataArrayMD.h>

XRAD_BEGIN


double	ExponentialFlterCoefficient(double radius);

enum exponential_blur_direction
	{
	exponential_blur_none = 0,
	exponential_blur_forward = 1,
	exponential_blur_reverse = 2,
	biexponential_blur = exponential_blur_reverse | exponential_blur_forward,

	};


namespace	AlgebraicStructures
{
template<> struct FieldTraits<xrad::exponential_blur_direction>: FieldTagScalar {};
}

// TODO: No math on exponential_blur_direction!
typedef point_3<exponential_blur_direction,int,AlgebraicStructures::FieldTagScalar> blur_directions_3;
typedef point_2<exponential_blur_direction,int,AlgebraicStructures::FieldTagScalar> blur_directions_2;

// аргументом является коэффициент бих-фильтра второго порядка a. вычислять через вышеобъявленную функцию
// template<class ARR>
// inline void	BiexpBlur1D(ARR &f, typename ARR::value_type a);

// template<class ARR>
// inline void	ExponentialBlur1D(ARR &f, typename ARR::value_type a, exponential_blur_direction direction);

//! \brief Реализация биэкспоненциального фильтра на CPU. См. \sa BiexpBlur2D.
// template<class AR2D>
// void	BiexpBlur2D_cpu(AR2D &data, double radius_v, double radius_h);

/*!
	\brief Двумерный биэкспоненциальный фильтр

	При доступности OpenCL вызывает версию OpenCL.
	Аргументами являются радиусы фильтрации по нулевой и первой координате массива.
*/
//TODO 2016_11_11_ Надо подумать, нужны ли здесь объявления, если описания шаблонных функций все равно включаются в файле .hh?
//	Меняется описание, а про объявление забыли. При определенных условиях можно получить link error и
//	потом долго искать ее.

template<class AR2D>
void	BiexpBlur2D(AR2D &data, double radius_v, double radius_h);

template<class AR2D>
void	BiexpBlur2D(AR2D &data, point2_F64 &radius);

template<class AR2D>
void	ExponentialBlur2D(AR2D &data, const point2_F64 &radius, const blur_directions_2 &directions);



template <class F2DT>
void	BiexpBlur3D(DataArrayMD<F2DT> &data, const typename F2DT::scalar_type &radius_z, const typename F2DT::scalar_type &radius_y, const typename F2DT::scalar_type &radius_x, omp_usage_t omp_usage = e_dont_use_omp);

template <class F2DT>
void	BiexpBlur3D(DataArrayMD<F2DT> &data, const point3_F64 &radius, omp_usage_t omp_usage = e_dont_use_omp);


template <class F2DT>
void ExponentialBlur3D(DataArrayMD<F2DT> &data, const point3_F64 &radius, const blur_directions_3 &directions);

XRAD_END

#include "ExponentialBlurAlgorithms.hh"

#endif // XRAD__File_exponential_blur_algorithms_h
