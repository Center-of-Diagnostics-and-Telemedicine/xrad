/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_color_sample_h
#define XRAD__File_color_sample_h

#include <XRADBasic/Sources/Core/Config.h>
#include <XRADBasic/Sources/Core/BasicMacros.h>
#include <XRADBasic/Sources/Core/BasicUtils.h>
#include <XRADBasic/Sources/Core/String.h>
#include <XRADBasic/Sources/Containers/LinearVectorFixedDimensions.h>
#include "HomomorphSamples.h"
#include <cstdint>

XRAD_BEGIN

//TODO: остается много неясного с альфа-каналом, когда он используется. ему не следует смешиваться с компонентами rgb-вектора при арифметических действиях, это очевидно. может, сделать более сложное наследование.

inline int	gray_pixel_range()
{
	return abs(int(gray_pixel_black) - int(gray_pixel_white));
}

inline int	gray_pixel_inversion()
{
	return sign(int(gray_pixel_white) - int(gray_pixel_black));
}



template<class ST>
class PlatformDependentRGBAColorTraits
{
	public:
		typedef rgb_pixel_component_type component_type;
		typedef	ST scalar_type;
		typedef AlgebraicStructures::FieldTagScalar component_field_tag;
		enum
		{
			n_pixel_components = 4
		};

#if XRAD_ENDIAN==XRAD_LITTLE_ENDIAN
		enum
		{
			blue_position, green_position, red_position, alpha_position
		};
#elif XRAD_ENDIAN==XRAD_BIG_ENDIAN
		enum
		{
			alpha_position, red_position, green_position, blue_position
		};
#else
#error "Unknown endianness"
#endif

#if gray_pixel_white>gray_pixel_black
		static component_type allowed_pixel_value(int v) {return v<gray_pixel_black ? gray_pixel_black : v>gray_pixel_white ? gray_pixel_white : v;}
#else
		static component_type allowed_pixel_value(int v) {return v<gray_pixel_white ? gray_pixel_white : v>gray_pixel_black ? gray_pixel_black : v;}
#endif

		static inline rgb_pixel_component_type inversion_mask(){return rgb_pixel_component_type(-1);}//0xFF
		static inline component_type invert(component_type v) {return inversion_mask()^v;}

		using reduced_width_type = PlatformDependentRGBAColorTraits<ST>; // Тип компонент изменить нельзя
};

template<class CT, class ST, class FIELD_TAG>
class PlatformIndependentRGBColorTraits
{
	public:
		typedef CT component_type;
		typedef	ST scalar_type;
		typedef FIELD_TAG component_field_tag;
		enum
		{
			n_pixel_components = 3
		};

		enum
		{
			red_position, green_position, blue_position
		};

		static component_type allowed_pixel_value(component_type v) {return v;}
		static component_type invert(component_type v) {return -v;}

		using reduced_width_type = PlatformIndependentRGBColorTraits<reduced_type<CT>, ST, FIELD_TAG>;
};

template<class CT, class ST, class FIELD_TAG>
class PlatformIndependentRGBAColorTraits
{
	public:
		typedef CT component_type;
		typedef	ST scalar_type;
		typedef FIELD_TAG component_field_tag;
		enum
		{
			n_pixel_components = 4
		};

		enum
		{
			red_position, green_position, blue_position, alpha_position
		};

		static component_type allowed_pixel_value(component_type v) {return v;}
		static component_type invert(component_type v) {return -v;}

		using reduced_width_type = PlatformIndependentRGBAColorTraits<reduced_type<CT>, ST, FIELD_TAG>;
};



template <class RGB_TRAITS_T>
struct RGBColorSample : public RGB_TRAITS_T, public LinearVectorFixedDimensions<RGBColorSample<RGB_TRAITS_T>, typename RGB_TRAITS_T::component_type, typename RGB_TRAITS_T::scalar_type,
		AlgebraicStructures::FieldTagScalar,
		RGB_TRAITS_T::n_pixel_components>
{
	public:
		PARENT(LinearVectorFixedDimensions<RGBColorSample<RGB_TRAITS_T>, typename RGB_TRAITS_T::component_type, typename RGB_TRAITS_T::scalar_type,
				AlgebraicStructures::FieldTagScalar,
				RGB_TRAITS_T::n_pixel_components>);
		typedef	typename RGB_TRAITS_T::component_type component_type;
		typedef typename RGB_TRAITS_T::scalar_type	scalar_type;

		typedef	RGBColorSample<RGB_TRAITS_T>	self;

	private:
		template<size_t N>
		std::enable_if_t<N == 3> zero_alpha(){}
		template<size_t N>
		std::enable_if_t<N == 4> zero_alpha(){at(RGB_TRAITS_T::alpha_position) = 0;}

		using parent::operator++;
		using parent::operator--;
		using RGB_TRAITS_T::n_pixel_components;

	public:
		using parent::at;
		using RGB_TRAITS_T::invert;

		//
		//	constructors
		//
		RGBColorSample(){};

		explicit RGBColorSample(const component_type &v) : parent(v){zero_alpha<n_pixel_components>(); red() = v, green() = v, blue() = v;};
		RGBColorSample(const component_type& in_r, const component_type& in_g, const component_type& in_b) {zero_alpha<n_pixel_components>(); red() = in_r, green() = in_g, blue() = in_b;}

		template<class RGB2>
		RGBColorSample(const RGBColorSample<RGB2> &sample){zero_alpha<n_pixel_components>(); red() = sample.red(), green() = sample.green(), blue() = sample.blue();}
		RGBColorSample(const parent &sample) : parent(sample) {}

		template<class RGB2>
		self	&operator = (const RGBColorSample<RGB2> &sample){zero_alpha<n_pixel_components>(); red()=sample.red(); green()=sample.green(); blue()=sample.blue(); return *this;}
		self	&operator = (const self &sample){parent::operator=(sample); return *this;}

		self	&operator = (const component_type& value){zero_alpha<n_pixel_components>(); red()=green()=blue()=value; return *this;}
		self	&operator = (const parent &sample){parent::operator=(sample); return *this;}

		//
		//	доступ к компонентам цвета
		//
		component_type	&red(){return at(RGB_TRAITS_T::red_position);}
		const component_type	&red()const {return at(RGB_TRAITS_T::red_position);}
		component_type	&green(){return at(RGB_TRAITS_T::green_position);}
		const component_type	&green()const {return at(RGB_TRAITS_T::green_position);}
		component_type	&blue(){return at(RGB_TRAITS_T::blue_position);}
		const component_type	&blue()const {return at(RGB_TRAITS_T::blue_position);};
		component_type	&alpha(){return at(RGB_TRAITS_T::alpha_position);}
		const component_type	&alpha()const {return at(RGB_TRAITS_T::alpha_position);};

		component_type	lightness() const;
		double hue() const;
		double saturation() const;

		//
		//	оператор изменения насыщенности и инверсия.
		//	вся остальная арифметика общая с родителем-вектором
		//
		self &operator %= (const scalar_type& v);
		self operator % (const scalar_type& v)const;
		self operator ~() const {return self(invert(red()), invert(green()), invert(blue()));}

		//
		//	сравнение по яркости
		//
		template<class RGB_TRAITS_T2>
		bool	operator < (const RGBColorSample<RGB_TRAITS_T2> &sample) const;
		template<class RGB_TRAITS_T2>
		bool	operator <= (const RGBColorSample<RGB_TRAITS_T2> &sample) const;
		template<class RGB_TRAITS_T2>
		bool	operator > (const RGBColorSample<RGB_TRAITS_T2> &sample) const;
		template<class RGB_TRAITS_T2>
		bool	operator >= (const RGBColorSample<RGB_TRAITS_T2> &sample) const;

	public:
		component_type	min_color() const;
		component_type	max_color() const;

		self change_saturation_algorithm(const scalar_type&) const;

		static ptrdiff_t components_distance()
		{
			enum
			{
				self_size = sizeof(self),
				component_size = sizeof(component_type),
				distance = self_size/component_size
			};
			do_once
			{
				void	*dummy_p = distance*component_size  - self_size;
				// если компилятор здесь выдаст ошибку, размер цветного отсчета некратен размеру компонент.
				// этого никак не должно допускать. нужно искать причину и устранять.
				dummy_p;
				// фиктивное обращение к переменной, чтобы не было предупреждения
			}

			return distance;
		}
};



//--------------------------------------------------------------
//
// number traits definitions. see comment in number_traits.h
//
//--------------------------------------------------------------

template<class RGB_TRAITS_T>
RGBColorSample<RGB_TRAITS_T> zero_value(const RGBColorSample<RGB_TRAITS_T> &){return RGBColorSample<RGB_TRAITS_T>(0,0,0);}

template<class RGB_TRAITS_T>
void make_zero(RGBColorSample<RGB_TRAITS_T> &value){value=RGBColorSample<RGB_TRAITS_T>(0,0,0);}

template<class RGB_TRAITS_T>
inline number_complexity_e complexity_e(const RGBColorSample<RGB_TRAITS_T>&){return number_complexity_e::rgb;}

template<class RGB_TRAITS_T>
inline const number_complexity::rgb *complexity_t( const RGBColorSample<RGB_TRAITS_T> &) {return nullptr;}

template<class RGB_TRAITS_T>
inline size_t	n_components(const RGBColorSample<RGB_TRAITS_T> &) {return 3u;}

template<class RGB_TRAITS_T>
inline typename RGBColorSample<RGB_TRAITS_T>::component_type	&component(RGBColorSample<RGB_TRAITS_T> &x, size_t n)
{
	switch(n)
	{
		case 0: return x.red();
		case 1: return x.green();
		case 2: return x.blue();

		default: throw invalid_argument(ssprintf("component(RGBColorSample<%s> &, size_t n ): invalid component number %d (0,1,2 are allowed values)",
			typeid(RGB_TRAITS_T).name(), n));
	};
}

template<class RGB_TRAITS_T>
inline const typename RGBColorSample<RGB_TRAITS_T>::component_type	&component(const RGBColorSample<RGB_TRAITS_T> &x, size_t n)
{
	switch(n)
	{
		case 0: return x.red();
		case 1: return x.green();
		case 2: return x.blue();
		default: throw invalid_argument(ssprintf("component(const RGBColorSample<%s> &, size_t n ): invalid component number %d (0,1,2 are allowed values)",
			typeid(RGB_TRAITS_T).name(), n));
	};
}

template<class RGB_TRAITS_T>
inline double	norma(const RGBColorSample<RGB_TRAITS_T> &x)
{
	return x.lightness();
}

template<class RGB_TRAITS_T>
inline double	fast_norma(const RGBColorSample<RGB_TRAITS_T> &x)
{
	return x.lightness();
}

template<class RGB_TRAITS_T>
inline double	quadratic_norma(const RGBColorSample<RGB_TRAITS_T> &x)
{
	// эта норма нужна только для формального соответствия стандарту,
	// заданному нами же в number_traits.h, от нее не требуется соответствия
	// каким-либо видимым субъективным характеристикам цвета
	return square(x.lightness());
}



//--------------------------------------------------------------
//
//	задание конкретных типов
//

typedef	rgb_pixel_component_type	GrayPixel;

typedef	RGBColorSample<PlatformIndependentRGBColorTraits<uint8_t, double, AlgebraicStructures::FieldTagScalar> >	ColorSampleUI8;
// для signed int8_t ничего не вводим, такие изображения не имеют смысла

typedef	RGBColorSample<PlatformIndependentRGBColorTraits<int16_t, double, AlgebraicStructures::FieldTagScalar> >	ColorSampleI16;
typedef	RGBColorSample<PlatformIndependentRGBColorTraits<uint16_t, double, AlgebraicStructures::FieldTagScalar> >	ColorSampleUI16;

typedef	RGBColorSample<PlatformIndependentRGBColorTraits<int32_t, double, AlgebraicStructures::FieldTagScalar> >	ColorSampleI32;
typedef	RGBColorSample<PlatformIndependentRGBColorTraits<uint32_t, double, AlgebraicStructures::FieldTagScalar> >	ColorSampleUI32;

typedef	RGBColorSample<PlatformIndependentRGBColorTraits<float, double, AlgebraicStructures::FieldTagScalar> >	ColorSampleF32;
typedef	RGBColorSample<PlatformIndependentRGBColorTraits<double, double, AlgebraicStructures::FieldTagScalar> >	ColorSampleF64;

typedef	RGBColorSample<PlatformDependentRGBAColorTraits<double> >	ColorPixel;

//--------------------------------------------------------------

//! \addtogroup gr_FloatingAnalog
//! @{

template<class T, class ST>
struct FloatingAnalog32<RGBColorSample<PlatformIndependentRGBColorTraits<T, ST, AlgebraicStructures::FieldTagScalar>>, typename enable_if<is_arithmetic_but_bool<T>::value>::type>
{ typedef ColorSampleF32  type; };

template<class T, class ST>
struct FloatingAnalog64<RGBColorSample<PlatformIndependentRGBColorTraits<T, ST, AlgebraicStructures::FieldTagScalar>>, typename enable_if<is_arithmetic_but_bool<T>::value>::type>
{ typedef ColorSampleF64  type; };

//! @} <!-- ^group gr_FloatingAnalog -->
//! \addtogroup gr_ReducedWidth
//! @{

template <class RGB_TRAITS_T>
struct	ReducedWidth<RGBColorSample<RGB_TRAITS_T>>
{ using type = RGBColorSample<typename RGB_TRAITS_T::reduced_width_type>; };

//! @} <!-- ^group gr_ReducedWidth -->

//--------------------------------------------------------------

check_if_number_traits_defined(ColorSampleF32)
check_if_number_traits_defined(ColorPixel)

//TODO: довести до ума цветные контейнеры РГБ. Довести до ума цветные пиксели не-РГБ

XRAD_END

#include "ColorSample.hh"

#endif //XRAD__File_color_sample_h
