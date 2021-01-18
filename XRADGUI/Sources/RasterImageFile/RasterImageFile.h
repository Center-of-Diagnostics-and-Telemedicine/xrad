#ifndef __RasterImageFile_h
#define __RasterImageFile_h

#include <typeinfo>
//#include <string>

//#include <XRADBasic/Containers.h>
//#include <Q:/XRAD/XRADBasic/Sources/DataArrayIO/DataArrayIOEnum.h>
#include <XRADSystem/CFile.h>




XRAD_BEGIN

using namespace std;

class RasterImageFile
{
public:

	RasterImageFile(std::string filename, std::string format = string()) { init(convert_to_wstring(filename), convert_to_wstring(format)); }
	RasterImageFile(std::wstring filename, std::wstring format = wstring()) { init(convert_to_wstring(filename), convert_to_wstring(format)); }

	RasterImageFile() = delete;

public:

	enum color_type
	{
		e_red, e_green, e_blue, e_alpha, e_L, e_H, e_S, e_cyan, e_magenta, e_yellow, e_black
	};

	int get_bits_per_channel() { return m_bits_per_channel; };
	size_t get_color_space() { return m_color_space; };
	RealFunction2D_F32 GetGrayscale();
	ColorImageF32 GetColor();

	template <typename T>
	void GetRGB(T& arg);
	
	template <typename ROW_T>
	void GetChannel(color_type c, MathFunction2D<ROW_T>& arg);


private:
	template<class T> struct sf_calculator;

	template<> struct sf_calculator<int8_t> { static constexpr double get(int bpc) { return  double(1 << min(bpc, 7)) - 1; } };
	template<> struct sf_calculator<uint8_t> { static constexpr double get(int bpc) { return double(1 << min(bpc, 8)) - 1; } };

//	template<> struct sf_calculator<int8_t> { static constexpr double get(int bpc) { return  double(min(std::numeric_limits<int8_t>::max(), int8_t(1u << bpc))); } };
//	template<> struct sf_calculator<uint8_t> { static constexpr double get(int bpc) { return double(min(std::numeric_limits<uint8_t>::max(), uint8_t(1u << bpc))); } };
	
	template<> struct sf_calculator<int16_t> { static constexpr double get(int bpc) { return  double(1 << min(bpc, 15)) - 1; }};
	template<> struct sf_calculator<uint16_t> { static constexpr double get(int bpc) { return double(1 << min(bpc, 16)) - 1; }};

//	template<> struct sf_calculator<int16_t> { static constexpr double get(int bpc) { return  double(min(std::numeric_limits<int16_t>::max(), int16_t(1 << bpc))); } };
//	template<> struct sf_calculator<uint16_t> { static constexpr double get(int bpc) { return double(min(std::numeric_limits<uint16_t>::max(), uint16_t(1u << bpc))); } };
	
	template<> struct sf_calculator<int32_t> { static constexpr double get(int bpc) { return  double(min(std::numeric_limits<int32_t>::max(), int32_t(1u << bpc)));}};
	template<> struct sf_calculator<uint32_t> { static constexpr double get(int bpc) { return double(min(std::numeric_limits<uint32_t>::max(),1u<<bpc)); }};

	template<> struct sf_calculator<float> { static constexpr double get(int bpc) { return double(1 << bpc) - 1; } };
	template<> struct sf_calculator<double> { static constexpr double get(int bpc) { return double(1 << bpc) - 1; } };

	template<class RGB> struct scalefactor_calculator;
	template<class T> struct scalefactor_calculator<RGBColorSample<T>> { static constexpr double get(int bpc) { return sf_calculator<typename T::component_type>::get(bpc); } };
	



	void	init(wstring filename, wstring extension);

	ColorImageF64	GetRGBInternal();
//	double	GetChannelValue(QColor c, RasterImageFile::color_type ct);
	RealFunction2D_F64 GetChannelInternal(color_type c);



	wstring	m_filename;
	string	m_format;// не юникод, довольно простого string

	enum { m_dimensions_number = 2 };
	size_t	m_sizes[m_dimensions_number];



//	size_t mVsize ;
//	size_t mHsize ;

	int	m_bits_per_channel;
	size_t m_color_space;
	};




XRAD_END

#include "RasterImageFile.hh"

#endif //__RasterImageFile_h

