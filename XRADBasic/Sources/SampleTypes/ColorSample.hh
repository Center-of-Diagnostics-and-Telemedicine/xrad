#include <algorithm>

XRAD_BEGIN



template<class RGB_TRAITS_T>
inline	RGBColorSample<RGB_TRAITS_T>	&RGBColorSample<RGB_TRAITS_T>::operator %=(const typename RGBColorSample<RGB_TRAITS_T>::scalar_type& v)
{
// оператор изменения насыщенности
	return *this = change_saturation_algorithm(v);
}

template<class RGB_TRAITS_T>
RGBColorSample<RGB_TRAITS_T> RGBColorSample<RGB_TRAITS_T>::operator %(const typename RGBColorSample<RGB_TRAITS_T>::scalar_type& v) const
{
	return change_saturation_algorithm(v);
}



//--------------------------------------------------------------
//
//	операции сравнения (члены класса)
//
//--------------------------------------------------------------

template<class RGB_TRAITS_T>
template<class RGB_TRAITS_T2>
bool	RGBColorSample<RGB_TRAITS_T>::operator < (const RGBColorSample<RGB_TRAITS_T2> &sample) const
{
	return fast_norma(*this) < fast_norma(sample);
}

template<class RGB_TRAITS_T>
template<class RGB_TRAITS_T2>
bool	RGBColorSample<RGB_TRAITS_T>::operator <= (const RGBColorSample<RGB_TRAITS_T2> &sample) const
{
	return fast_norma(*this) <= fast_norma(sample);
}

template<class RGB_TRAITS_T>
template<class RGB_TRAITS_T2>
bool	RGBColorSample<RGB_TRAITS_T>::operator > (const RGBColorSample<RGB_TRAITS_T2> &sample) const
{
	return fast_norma(*this) > fast_norma(sample);
}

template<class RGB_TRAITS_T>
template<class RGB_TRAITS_T2>
bool	RGBColorSample<RGB_TRAITS_T>::operator >= (const RGBColorSample<RGB_TRAITS_T2> &sample) const
{
	return fast_norma(*this) >= fast_norma(sample);
}



//--------------------------------------------------------------
//
//	нормировочные функции-члены
//
//--------------------------------------------------------------

template<class RGB_TRAITS_T>
typename RGBColorSample<RGB_TRAITS_T>::component_type	RGBColorSample<RGB_TRAITS_T>::min_color() const
{
	return min(norma(red()), min(norma(green()), norma(blue())));
}

template<class RGB_TRAITS_T>
typename RGBColorSample<RGB_TRAITS_T>::component_type	RGBColorSample<RGB_TRAITS_T>::max_color() const
{
	return max(norma(red()), max(norma(green()), norma(blue())));
}



//!	\brief Алгоритм изменения насыщенности цвета.
//! \param v Множитель, применяемый к насыщенности.
//	v<0 не допускается
//	При v==0 меняет цвет на нейтральный серый.
//	При v>0 & v<1 насыщенность уменьшается (цвет приближается к нейтральному серому.
//	При v==1 ничего не меняется.
//	При v>1 насыщенность увеличивается. При больших значениях возможны некорректные результаты (не отслеживается).

template<class RGB_TRAITS_T>
RGBColorSample<RGB_TRAITS_T> RGBColorSample<RGB_TRAITS_T>::change_saturation_algorithm(const typename RGBColorSample<RGB_TRAITS_T>::scalar_type &v) const
{
	XRAD_ASSERT_THROW_EX(v >= 0, invalid_argument);

	if(v == 0) return RGBColorSample<RGB_TRAITS_T>(max_color());

	component_type	min_c = min_color();
	component_type	max_c = max_color();

	if(min_c == max_c)
	{
		return *this;
	}

	RGBColorSample<RGB_TRAITS_T>	result(*this);

	result -= min_c;
	result *= v;
	component_type	min_c_new = max_c-(max_c-min_c)*v;
	return result += min_c_new;
}



//--------------------------------------------------------------
//
//	rgb access nonmember

template<class RGB_TRAITS_T>
typename RGBColorSample<RGB_TRAITS_T>::component_type &red(RGBColorSample<RGB_TRAITS_T> &c)
{
	return c.red();
}

template<class RGB_TRAITS_T>
const typename RGBColorSample<RGB_TRAITS_T>::component_type &red(const RGBColorSample<RGB_TRAITS_T> &c)
{
	return c.red();
}

template<class RGB_TRAITS_T>
typename RGBColorSample<RGB_TRAITS_T>::component_type &green(RGBColorSample<RGB_TRAITS_T> &c)
{
	return c.green();
}

template<class RGB_TRAITS_T>
const typename RGBColorSample<RGB_TRAITS_T>::component_type &green(const RGBColorSample<RGB_TRAITS_T> &c)
{
	return c.green();
}

template<class RGB_TRAITS_T>
typename RGBColorSample<RGB_TRAITS_T>::component_type &blue(RGBColorSample<RGB_TRAITS_T> &c)
{
	return c.blue();
}

template<class RGB_TRAITS_T>
const typename RGBColorSample<RGB_TRAITS_T>::component_type &blue(const RGBColorSample<RGB_TRAITS_T> &c)
{
	return c.blue();
}

//	hls access member

template<class RGB_TRAITS_T>
typename RGBColorSample<RGB_TRAITS_T>::component_type RGBColorSample<RGB_TRAITS_T>::lightness() const
{
	//TODO алгоритмы hls перепроверить, как они написаны
	return max(red(), max(green(), blue()));
}

template<class RGB_TRAITS_T>
double RGBColorSample<RGB_TRAITS_T>::saturation() const
{
	double	L = max_color();
	if(L) return (L - min_color())/L;
	else return 0;
}

template<class RGB_TRAITS_T>
double RGBColorSample<RGB_TRAITS_T>::hue() const
{
	const	double	p3 = pi()/3.;

	if(red() < green() && red() < blue()){ // min color is red
		double g0 = green()-red();
		double b0 = blue()-red();
		return p3*(b0-g0)/max(g0, b0);
	}
	else if(green() < blue()){ // min color is green
		double r0 = red()-green();
		double b0 = blue()-green();
		return 2.*p3 + p3*(r0-b0)/max(r0, b0);
	}
	else{ // min color is blue
		double r0 = red()-blue();
		double g0 = green()-blue();
		return 4.*p3 + p3*(g0-r0)/max(r0, g0);
	}
}



//	nonmember

template<class RGB_TRAITS_T>
typename RGBColorSample<RGB_TRAITS_T>::component_type lightness(const RGBColorSample<RGB_TRAITS_T> &c)
{
	return c.lightness();
}

template<class RGB_TRAITS_T>
double saturation(const RGBColorSample<RGB_TRAITS_T> &c)
{
	return c.saturation();
}

template<class RGB_TRAITS_T>
double hue(const RGBColorSample<RGB_TRAITS_T> &c)
{
	return c.hue();
}



namespace Functors
{

//	functors for rgb access

struct	red_functor
{
	template <class T>
	auto operator()(const T &y) const { return y.red(); }
};

struct	green_functor
{
	template <class T>
	auto operator()(const T &y) const { return y.green(); }
};

struct	blue_functor
{
	template <class T>
	auto operator()(const T &y) const { return y.blue(); }
};

//	functors for hls access
struct	lightness_functor
{
	template <class T>
	auto operator()(const T &y) const { return lightness(y); }
};

struct	saturation_functor
{
	template <class T>
	auto operator()(const T &y) const { return saturation(y); }
};

struct	hue_functor
{
	template <class T>
	auto operator()(const T &y) const { return hue(y); }
};

} // namespace Functors



XRAD_END
