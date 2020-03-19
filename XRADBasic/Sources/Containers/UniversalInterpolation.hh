#ifndef __universal_interpolation_h
#error "#include this file only through UniversalInterpolation.h"
#endif

XRAD_BEGIN

template<class FILTER>
void	InterpolationFilterGenerator<FILTER>::SetOffsetCorrection(double &offset_correction) const
{
	if(filter_order & 0x00000001) offset_correction = 0.5;
		// При нечетном порядке фильтра возникает сдвиг на полотсчета,
		// который нужно скорректировать.
	else offset_correction = 0;
}

template<class FILTER>
const FILTER	*UniversalInterpolator<FILTER>::GetNeededFilter(double x) const
{
	int	dx = int(fractional_part(x)*n_filters);

	// Необходимость следующей проверки range неочивидна,
	// см. комментарий к аналогичной функции
	// в UniversalInterpolators2D.hh.
	dx = range(dx, 0, n_filters-1);

	return &(InterpolationFilters.at(dx));
}

XRAD_END
