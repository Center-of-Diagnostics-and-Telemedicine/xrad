//TableFunction.hh


XRAD_BEGIN


//--------------------------------------------------------------
//
//	табличная функция
//
//--------------------------------------------------------------

template<class MATH_F, class F>
TableFunction<MATH_F, F> ::TableFunction(const F &f, size_t in_size, double in_x0, double in_dx) :
	functor(f), parent(in_size),
	x0(in_x0), dx(in_dx), x_max(in_x0 + dx*(in_size-1))
{
	iterator it = begin();
	for(size_t i = 0; i < size(); ++i, ++it)
	{
		*it = x0 + double(i)*dx;
	}
	ApplyFunction(*this, functor);
	base = &at(0);
}

template<class MATH_F, class F>
TableFunction<MATH_F, F> ::TableFunction(size_t in_size, double in_x0, double in_dx) :
	parent(in_size),
	x0(in_x0), dx(in_dx), x_max(in_x0 + dx*(in_size-1))
{
	base = &at(0);
}


template<class MATH_F, class F>
void TableFunction<MATH_F, F> ::SetFunctor(const F &f)
{
	functor = f;
	iterator it = begin();
	for(size_t i = 0; i < size(); ++i, ++it)
	{
		*it = x0 + i*dx;
	}
	ApplyFunction(*this, functor);
}



template<class MATH_F, class F>
inline typename MATH_F::value_type TableFunction<MATH_F, F> :: operator() (double x) const
{
	double	a = (x-x0)/dx;
	ptrdiff_t		index = integral_part(a);
	double	da = fractional_part(a);

	if(in_range(index, 0, ptrdiff_t(size()-2)))
	{
		//	универсальный интерполятор in(index) исключаем: у этой функции на первом месте быстродействие
		// 	поэтому делается "в лоб" линейная интерполяция. по сравнению с "ближайшим соседом"
		//	потеря скорости минимальна, зато точность возрастает существенно
		return (1.-da)*base[index] + da*base[index+1];
	}
	else
	{
		return functor(x);
	}


}


XRAD_END
