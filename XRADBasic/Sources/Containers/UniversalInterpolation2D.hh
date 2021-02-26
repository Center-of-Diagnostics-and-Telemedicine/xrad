#ifndef XRAD__File_universal_interpolation_2d_cc
#define XRAD__File_universal_interpolation_2d_cc

XRAD_BEGIN

template<class FILTER>
void xrad::UniversalInterpolator2D<FILTER>::SetExtrapolationMethod(extrapolation::method em)
{
	for(int i = 0; i < n_filters_v; ++i)
	{
		for(int j = 0; j < n_filters_h; ++j)
		{
			InterpolationFilters.at(i, j).SetExtrapolationMethod(em);
		}
	}
}

template<class FILTER>
const FILTER	*UniversalInterpolator2D<FILTER>::GetNeededFilter(double v, double h) const
{
	if(!n_filters_v || !n_filters_h)
	{
		throw logic_error("UniversalInterpolator2D<FILTER>::GetNeededFilter. Interpolator not initialized. Init2DInterpolators() has not been called?");
	}

	int	dv = range(int(fractional_part(v)*n_filters_v), 0, n_filters_v-1);
	int	dh = range(int(fractional_part(h)*n_filters_h), 0, n_filters_v-1);

	// Интересное явление, связанное с потерей точности, реальный случай.
	// v = -2e-17, floor(v) = -1.
	// v-floor(v) в таком случае равно точно 1, а dv = n_filters.
	// Вероятность мала, но вот, возникло однажды.
	// Отсюда проверка диапазона.

	return &(InterpolationFilters.at(dv,dh));
}

XRAD_END

#endif //XRAD__File_universal_interpolation_2d_cc
