#ifndef __RasterImageFile_h
#define __RasterImageFile_h

#include <typeinfo>
//#include <string>

//#include <XRADBasic/Containers.h>
//#include <Q:/XRAD/XRADBasic/Sources/DataArrayIO/DataArrayIOEnum.h>
#include <XRADSystem/CFile.h>
#include <XRADBasic/MathFunctionTypes2D.h>



XRAD_BEGIN

using namespace std;

enum class color_channel
{
	red, green, blue, alpha, lightness, hue, saturation, cyan, magenta, yellow, black
};


namespace file
{
class raster_image
{
public:

	raster_image(std::string filename, std::string format = string()) { init(convert_to_wstring(filename), convert_to_wstring(format)); }
	raster_image(std::wstring filename, std::wstring format = wstring()) { init(convert_to_wstring(filename), convert_to_wstring(format)); }

	raster_image() = delete;

public:


	size_t get_bits_per_channel() const { return m_bits_per_channel; };
	size_t get_color_space() const { return m_color_space; };

	template <typename IMAGE_T = RealFunction2D_F32> IMAGE_T lightness();
	template <typename IMAGE_T = RealFunction2D_F32> IMAGE_T channel(color_channel ct);
	template<class RGB_IMAGE_T = ColorImageF32> RGB_IMAGE_T rgb();


private:
	template<class T> struct value_scalefactor_calculator;

	template<> struct value_scalefactor_calculator<int8_t> { static constexpr double get(int bpc) { return  double(1 << min(bpc, 7)) - 1; } };
	template<> struct value_scalefactor_calculator<uint8_t> { static constexpr double get(int bpc) { return double(1 << min(bpc, 8)) - 1; } };

	template<> struct value_scalefactor_calculator<int16_t> { static constexpr double get(int bpc) { return  double(1 << min(bpc, 15)) - 1; } };
	template<> struct value_scalefactor_calculator<uint16_t> { static constexpr double get(int bpc) { return double(1 << min(bpc, 16)) - 1; } };

	template<> struct value_scalefactor_calculator<int32_t> { static constexpr double get(int bpc) { return  double(min(std::numeric_limits<int32_t>::max(), int32_t(1u << bpc))); } };
	template<> struct value_scalefactor_calculator<uint32_t> { static constexpr double get(int bpc) { return double(min(std::numeric_limits<uint32_t>::max(), 1u<<bpc)); } };

	template<> struct value_scalefactor_calculator<float> { static constexpr double get(int bpc) { return double(1 << bpc) - 1; } };
	template<> struct value_scalefactor_calculator<double> { static constexpr double get(int bpc) { return double(1 << bpc) - 1; } };

	template<class RGB> struct scalefactor_calculator;
	template<class T> struct scalefactor_calculator<RGBColorSample<T>> { static constexpr double get(int bpc) { return value_scalefactor_calculator<typename T::component_type>::get(bpc); } };

	void	init(wstring filename, wstring extension);

	ColorImageF64	rgb_internal();
	RealFunction2D_F64 channel_internal(color_channel c);

	wstring	m_filename;
	string	m_format;// не юникод, довольно простого string

	enum { m_dimensions_number = 2 };
	size_t	m_sizes[m_dimensions_number];

	int	m_bits_per_channel;
	size_t m_color_space;
};

}




XRAD_END

#include "RasterImageFile.hh"

#endif //__RasterImageFile_h

