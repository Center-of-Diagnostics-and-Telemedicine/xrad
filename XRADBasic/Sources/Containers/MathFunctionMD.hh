#ifndef __math_function_multidimensional_cc
#define __math_function_multidimensional_cc

XRAD_BEGIN

namespace FilterMDAuxiliaries
{
/*!
	\brief Возвращает номер оси

	Необходимо для ориентации среза/массива данных в пространстве.
	Т.е. при выборе нулевой координаты 0, возвратит значения координат 1 = 1.
	А второй координаты = 2.
*/
inline size_t GetCoordNo(size_t dimension, size_t n)
{
	if(n == 0)
	{
		switch(dimension)
		{
		case 0: return 1;
		case 1: return 0;
		case 2: return 0;
		default:
			ForceDebugBreak();
			throw invalid_argument("GetCoordNo(size_t dimension, size_t n), invalid argument");
		}
	}
	else if(n == 1)
	{
		switch(dimension)
		{
		case 0: return 2;
		case 1: return 2;
		case 2: return 1;
		default:
			{
				ForceDebugBreak();
				throw invalid_argument("GetCoordNo(size_t dimension, size_t n), invalid argument");
			}
		}
	}
	else
	{
		ForceDebugBreak();
		throw invalid_argument("GetCoordNo(size_t dimension, size_t n), invalid argument");
	}
}

} // namespace FilterMDAuxiliaries

//--------------------------------------------------------------
/*!
	\brief Трехмерный фильтр

	Обход по каждой из 2 осей среза по рядам (x, y), а координаты глубины (z) - по срезам.
*/
template <class F2DT, class FILTER_T1, class FILTER_T2, class FILTER_T3>
void FilterArray3DSeparate(DataArrayMD<F2DT> &data, const FILTER_T1 &filter_z, const FILTER_T2 &filter_y, const FILTER_T3 &filter_x)
{
	size_t coord0;
	MaxValue(data.steps(), &coord0);

	size_t	coord1 = FilterMDAuxiliaries::GetCoordNo(coord0, 0);
	size_t	coord2 = FilterMDAuxiliaries::GetCoordNo(coord0, 1);
	index_vector	access_v(3);
	if (data.n_dimensions() !=3)
	{
		ForceDebugBreak();
		throw invalid_argument("FilterArray3DSeparate(DataArrayMD<F2DT> &data, const FILTER_T1 &filter_x, const FILTER_T2 &filter_y, const FILTER_T3 &filter_z), invalid number of dimensions");
	}
	if(data.steps(coord1)>data.steps(coord2))
	{
		access_v[coord1] = slice_mask(0);
		access_v[coord2] = slice_mask(1);
	}
	else
	{
		access_v[coord1] = slice_mask(1);
		access_v[coord2] = slice_mask(0);
	}
	MathFunction<F2DT, typename F2DT::scalar_type, typename F2DT::field_tag> slices(data.sizes(coord0));

	for (size_t i = 0; i < data.sizes(coord0); ++i)
	{
		access_v[coord0] = i;
		data.GetSlice(slices[i],access_v);
		FilterArray2DSeparate(slices[i], filter_y, filter_x);
	}
	if (filter_z.size()>1)
	{
		slices.Filter(filter_z);
	}
}

template <class F2DT, class FILTER_T, class ST>
void FilterArray3DSeparate(DataArrayMD<F2DT> &data, const point_3<FILTER_T, ST, typename FILTER_T::field_tag> &filters)
{
	FilterArray3DSeparate(data, filters.z(), filters.y(), filters.x());
}



XRAD_END

#endif //__math_function_multidimensional_cc
