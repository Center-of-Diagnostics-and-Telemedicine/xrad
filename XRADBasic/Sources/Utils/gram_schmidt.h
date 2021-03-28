/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef __gram_schmidt_h
#define __gram_schmidt_h

//------------------------------------------------------------------
//
//	created:	2021/03/09	10:38
//	filename: 	gram_schmidt.h
//	file path:	Q:\XRAD\XRADBasic\Sources\Utils
//	author:		kns
//	
//	purpose:	
//
//------------------------------------------------------------------

namespace xrad
{


//! \brief Алгоритм ортогонализации Грама-Шмидта для произвольных контейнеров
//!	\param arr Контейнер, содержащий ортогонализируемые элементы.
//! Условие, налагаемое на контейнер: возможность получить ссылку на элемент по номеру.
//! Условие, налагаемое на элемент: должны быть определены действия векторной алгебры (умножение на скаляр, сложенме между собой, скалярное произведение).
//! \param lambda Функтор с аргументами (ARR &arr, size_t i), возвращающий ссылку на i-й вектор контейнера
//! \param n_vectors Количество векторов в контейнере, подлежащих ортогонализации

template<class ARR, class F>
void	gram_schmidt_orthogonalization(ARR& arr, const F& lambda, size_t n_vectors)
{
	for(size_t i = 0; i < n_vectors; ++i)
	{
		for(size_t j = 0; j < i; ++j)
		{
			auto	coefficient = sp(lambda(arr, i), lambda(arr, j));
			lambda(arr, i) -= lambda(arr, j) * coefficient;
		}
		double divisor = sqrt(norma(sp(lambda(arr, i), lambda(arr, i))));
		if(divisor) lambda(arr, i) /= divisor;
	}
}

//!	\brief Ортогонализация Грама-Шмидта столбцов контейнера
//! Рассчитано на наследников DataArray2D. Можно использовать любой контейнер, в котором определен тип row_type и функция col(size_t)
template<class ARR2D>
void	gso_matrix_columns(ARR2D& arr)
{
	auto lambda = [](ARR2D& a, size_t i)->typename ARR2D::row_type& {return a.col(i);};
	gram_schmidt_orthogonalization(arr, lambda, arr.hsize());
}

//!	\brief Ортогонализация Грама-Шмидта строк контейнера
//! Рассчитано на наследников DataArray2D. Можно использовать любой контейнер, в котором определен тип row_type и функция row(size_t)
template<class ARR2D>
void	gso_matrix_rows(ARR2D& arr)
{
	auto lambda = [](ARR2D& a, size_t i)->typename ARR2D::row_type& {return a.row(i);};
	gram_schmidt_orthogonalization(arr, lambda, arr.hsize());
}

//!	\brief Ортогонализация Грама-Шмидта элементов произвольного массива (например, вектор-функции)
//! Для элементов контейнера должны быть заданы действия векторной алгебры
template<class VEC>
void	gso_vector(VEC& arr)
{
	auto lambda = [](ARR2D& a, size_t i)->typename VEC::value_type& {return a.at(i);};
	gram_schmidt_orthogonalization(arr, lambda, arr.size());
}

//!	\brief Ортогонализация Грама-Шмидта срезов трехмерного массива
//! Для элементов контейнера должны быть заданы действия векторной алгебры
//! \param coordinate_no Номер координаты от 0 до 2
template<class ARRMD>
void	gso_3D(ARRMD& arr, size_t coordinate_no)
{
	XRAD_ASSERT_THROW(arr.n_dimensions() == 3);
	index_vector	iv(3, 0);
	size_t	j = 0;
	for(size_t i = 0; i < 3; ++i)
	{
		if(i!= coordinate_no) iv[i] = slice_mask(j++);
	}
	auto lambda = [&iv, coordinate_no](ARRMD& a, size_t i)//->typename ARRMD::slice_type&
	{
		iv[coordinate_no] =i;
		return a.GetSlice(iv);
	};
	gram_schmidt_orthogonalization(arr, lambda, arr.sizes(coordinate_no));
}

// TODO maybe
//!	\brief Ортогонализация Грама-Шмидта подмножеств многомерного массива
//! Для элементов контейнера должны быть заданы действия векторной алгебры
//	Не сделана функция sp для MathFunctionMD
// template<class ARRMD>
// void	gso_MD(ARRMD& arr, size_t coordinate_no)
// {
// 	index_vector	iv(arr.n_dimensions(), 0);
// 	size_t	j = 0;
// 	for(size_t i = 0; i < arr.n_dimensions(); ++i)
// 	{
// 		if(i!= coordinate_no) iv[i] = slice_mask(j++);
// 	}
// 	auto lambda = [&iv, coordinate_no](ARRMD& a, size_t i)//->typename ARRMD::slice_type&
// 	{
// 		iv[coordinate_no] =i;
// 		return a.GetSubset(iv);
// 	};
// 	gram_schmidt_orthogonalization(arr, lambda, arr.sizes(coordinate_no));
// }


}//namespace xrad

#endif //__gram_schmidt_h
