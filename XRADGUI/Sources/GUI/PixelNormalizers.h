#ifndef XRAD__File_PixelNormalizers_h
#define XRAD__File_PixelNormalizers_h

#include <XRADBasic/Sources/Containers/SpaceCoordinates.h>
#include <XRADBasic/Sources/SampleTypes/ComplexSample.h>
#include <XRADBasic/Sources/SampleTypes/ColorSample.h>

//--------------------------------------------------------------
//
//	created:	2014/10/08
//	created:	8.10.2014   18:56
//	author:		kns
//
//--------------------------------------------------------------

XRAD_BEGIN

namespace XRAD_PixelNormalizers
{
//--------------------------------------------------------------
//
//	средства преобразования отсчетов данных
//	в типы, которые совместимы с экранными пикселями.
//
//	аргументы шаблона:
//	T тип отсчета исходных данных (чаще всего, действительных или комплексных).
//	PT тип выходного пикселя. для черно-белых данных uint8, для цветных ColorPixel
//	выходной пиксельный тип, но с плавающей запятой. т.о., для черно-белых это
//	float или double, a для цветных ColorSample<double>

//
template<class T, class PT>
class	pixel_normalizer
	{
	protected:
		typedef const range_1<double,double> range_type;

		const double	out_range() const {return double(255.);}

		double	rectify_linear(int x) const				{return x;}
		double	rectify_linear(float x) const				{return x;}
		double	rectify_linear(double x) const				{return x;}
		double	rectify_linear(const complexF64 &x) const	{return cabs(x);}
		double	rectify_linear(const complexF32 &x) const	{return cabs(x);}

		double	rectify_abs(int x) const				{return labs(x);}
		double	rectify_abs(float x) const				{return fabs(x);}
		double	rectify_abs(double x) const				{return fabs(x);}
		template<class T1, class ST1>
			double	rectify_abs(const ComplexSample<T1,ST1> &x) const	{return cabs(x);}
// 		double	rectify_abs(const complexF32 &x) const	{return cabs(x);}

		double	rectify_db(const int &x) const		{return amplitude_to_decibel(x);}
		double	rectify_db(const float &x) const	{return amplitude_to_decibel(x);}
		double	rectify_db(const double &x)	const	{return amplitude_to_decibel(x);}
		double	rectify_db(const complexF64 &x)const{return amplitude_to_decibel(cabs(x));}
		double	rectify_db(const complexF32 &x)const{return amplitude_to_decibel(cabs(x));}
		double	rectify_db(const complexI32 &x)const{return amplitude_to_decibel(cabs(x));}
		double	rectify_db(const complexI16 &x)const{return amplitude_to_decibel(cabs(x));}
		double	rectify_db(const complexI8 &x)const{return amplitude_to_decibel(cabs(x));}

		PT	pixel_value(double x) const {return PT(pow(range(x,0,out_range())/out_range(), gamma) * out_range());}
		PT	pixel_value(const ColorSampleF64 &x) const
			{
			PT result;
			result.red() = pow(range(x.red(),0,out_range())/out_range(), gamma) * out_range();
			result.green() = pow(range(x.green(),0,out_range())/out_range(), gamma) * out_range();
			result.blue() = pow(range(x.blue(),0,out_range())/out_range(), gamma) * out_range();
			return result;
			}

	public:
		range_type value_range;
		double	normalize_factor;
		double gamma;

		void	set_display_ranges(double in_min, double in_max, double in_gamma)
			{
			value_range = range_type(in_min, in_max);
			gamma = in_gamma;
			normalize_factor = out_range()/value_range.delta();
			}

	protected:
		pixel_normalizer(const double& min_value, const double& max_value, double in_gamma):value_range(min_value,max_value), gamma(in_gamma), normalize_factor(out_range()/value_range.delta()){}
		pixel_normalizer(const range_type &in_value_range, double in_gamma):value_range(in_value_range), gamma(in_gamma), normalize_factor(out_range()/value_range.delta()){}

	public:
		typedef	T input_sample_type;
		typedef	PT pixel_type;
		typedef double internal_type;
	};

//	универсальные для действительных, комплексных и цветных данных

template<class T, class PT>
class assign_identical : public pixel_normalizer<T,PT>
	{
	public:
		PARENT(pixel_normalizer<T, PT>);
		using typename parent::range_type;

		assign_identical() : parent(range_type(), 1){}
		PT& operator()(PT &x, const T &y) const
			{
			return x = y;
			}
	};


template<class T, class PT>
class assign_linear_normalized : public pixel_normalizer<T,PT>
	{
	public:
		PARENT(pixel_normalizer<T, PT>);
		using typename parent::range_type;
		using parent::normalize_factor;
		using parent::value_range;

		assign_linear_normalized(const range_type &in_value_range, double in_gamma) :
			parent(in_value_range, in_gamma){}
		PT& operator()(PT &x, const T &y) const
			{
			return x = pixel_value(normalize_factor*(rectify_linear(y)-value_range.x1()));
			}
	};


template<class T, class PT>
class assign_linear_abs_normalized : public pixel_normalizer<T,PT>
	{
	public:
		PARENT(pixel_normalizer<T, PT>);
		using typename parent::range_type;
		using parent::normalize_factor;
		using parent::value_range;

		assign_linear_abs_normalized(const range_type &in_value_range, double in_gamma) :
			parent(in_value_range, in_gamma){}
		PT& operator()(PT &x, const T &y) const
			{
			return x = pixel_value(normalize_factor*(rectify_abs(y)-value_range.x1()));
			}
	};

template<class T, class PT>
class assign_log_normalized: public pixel_normalizer<T,PT>
	{
	public:
		PARENT(pixel_normalizer<T, PT>);
		using parent::normalize_factor;
		using parent::value_range;

		assign_log_normalized(const range_1<double,double> &in_dynamic_range, double in_gamma) :
			parent(in_dynamic_range, in_gamma){}
		PT &operator()(PT &x, const T &y) const
			{
			return x = pixel_value(normalize_factor*(rectify_db(y) - value_range.x1()));
			}
	};

// комплексные
template<class T, class PT>
class assign_linear_real_normalized : public pixel_normalizer<T,PT>
	{
	public:
		PARENT(pixel_normalizer<T, PT>);
		using typename parent::range_type;
		using parent::normalize_factor;
		using parent::value_range;

		assign_linear_real_normalized(const range_type &in_value_range, double in_gamma) :
			parent(in_value_range, in_gamma){}
		PT& operator()(PT &x, const T &y) const
			{
			return x = pixel_value(normalize_factor*(rectify_linear(real(y))-value_range.x1()));
			}
	};

template<class T, class PT>
class assign_linear_imag_normalized : public pixel_normalizer<T,PT>
	{
	public:
		PARENT(pixel_normalizer<T, PT>);
		using typename parent::range_type;
		using parent::normalize_factor;
		using parent::value_range;

		assign_linear_imag_normalized(const range_type &in_value_range, double in_gamma) :
			parent(in_value_range, in_gamma){}
		PT& operator()(PT &x, const T &y) const
			{
			return x = pixel_value(normalize_factor*(rectify_linear(imag(y))-value_range.x1()));
			}
	};





// цветные

template<class T, class PT>
class assign_color_lightness_as_is: public pixel_normalizer<T,PT>
	{
	public:
		PARENT(pixel_normalizer<T, PT>);
		using typename parent::range_type;

		assign_color_lightness_as_is() : parent(range_type(), 1){}
		PT& operator()(PT &x, const T &y) const
			{
			return x = lightness(y);
			}
	};


template<class T, class PT>
class assign_color_normalized : public pixel_normalizer<T,PT>
	{
	public:
		PARENT(pixel_normalizer<T, PT>);
		using typename parent::range_type;
		using parent::normalize_factor;
		using parent::value_range;

		assign_color_normalized (const range_type &in_value_range, double in_gamma) :
			parent(in_value_range, in_gamma){}
		PT& operator()(PT &x, const T &y) const
			{
			return x = pixel_value(y*normalize_factor-value_range.x1());
			}
	};

template<class T, class PT>
class assign_color_lightness_normalized: public pixel_normalizer<T,PT>
	{
	public:
		PARENT(pixel_normalizer<T, PT>);
		using typename parent::range_type;
		using parent::normalize_factor;
		using parent::value_range;

		assign_color_lightness_normalized (const range_type &in_value_range, double in_gamma) :
			parent(in_value_range, in_gamma){}
		PT& operator()(PT &x, const T &y) const
			{
			return x = pixel_value(lightness(y)*normalize_factor-value_range.x1());
			}
	};

//--------------------------------------------------------------

}//namespace XRAD_PixelNormalizers

XRAD_END

#endif //XRAD__File_PixelNormalizers_h
