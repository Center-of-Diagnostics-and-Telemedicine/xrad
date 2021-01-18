#include "pre.h"

#include <XRADSystem/CFile.h>
#include <XRADBasic/Sources/DataArrayIO/DataArrayIOEnum.h>
#include <XRADQt/QtStringConverters.h>
#include <XRADSystem/Sources/System/FileNameOperations.h>

#include "RasterImageFile.h"


#pragma warning(disable:4714)


#include <QtGui/QImage>

XRAD_BEGIN

#if 1
using namespace std;


void	RasterImageFile::init(wstring in_filename, wstring in_format)
{
	m_filename = in_filename;
	if (in_format.size())
	{
		m_format = convert_to_string(in_format);
	}
	else
	{
		m_format = convert_to_string(file_extension(m_filename));
		m_format.erase(m_format.begin(), m_format.begin() + 1);
	}

	QImage mQImage;

	

	XRAD_ASSERT_THROW_M(mQImage.load(wstring_to_qstring(m_filename), m_format.c_str()),
		file_container_error,
		convert_to_string(ssprintf(L"File '%s' can not be opened with format '%s'", m_filename.c_str(), convert_to_wstring(m_format).c_str()), e_encode_literals));


	m_sizes[0] = mQImage.height();
	m_sizes[1] = mQImage.width();

	size_t format = mQImage.format();

//	printf("format is: %zu\n", format);

	switch (format)
	{
	case QImage::Format_RGB32://rgb_8.png
		m_bits_per_channel = 8;
		m_color_space = DataArrayIOAuxiliaries::ioNumberType::rgb_color;
		break;

	case QImage::Format_RGBX64://rgb_16.png
		m_bits_per_channel = 16;
		m_color_space = DataArrayIOAuxiliaries::ioNumberType::rgb_color;
		break;

	case QImage::Format_Grayscale16://gray_16.png
		m_bits_per_channel = 16;
		m_color_space = DataArrayIOAuxiliaries::ioNumberType::grayscale;
		break;

	case QImage::Format_Grayscale8://gray_8.png
		m_bits_per_channel = 8;
		m_color_space = DataArrayIOAuxiliaries::ioNumberType::grayscale;
		break;

	default:
		throw invalid_argument("unknown file format ");

	}
}
#endif


ColorImageF64 RasterImageFile::GetRGBInternal()
{
	ColorImageF64 result;

	result.resize(m_sizes[0], m_sizes[1]);

	QImage mQImage;
	mQImage.load(wstring_to_qstring(m_filename), m_format.c_str());

	for (size_t i = 0; i < m_sizes[0]; i++)
	{
		for (size_t j = 0; j < m_sizes[1]; j++)
		{
			const auto	&p = mQImage.pixelColor(int(j), int(i));
			result.at(i, j) = ColorSampleF64(p.redF(), p.greenF(), p.blueF());
			
		}
	}
	return result;
}

double	GetChannelValue(QColor c, RasterImageFile::color_type ct)
{
	switch (ct)
	{
	case RasterImageFile::color_type::e_red:
		return c.redF();
		break;
	case RasterImageFile::color_type::e_green:
		return c.greenF();
		break;
	case RasterImageFile::color_type::e_blue:
		return c.blueF();
		break;
	case RasterImageFile::color_type::e_alpha:
		return c.alphaF();
		break;
	case RasterImageFile::color_type::e_L:
		return c.lightnessF();
		break;
	case RasterImageFile::color_type::e_H:
		return c.hslHueF();
		break;
	case RasterImageFile::color_type::e_S:
		return c.hslSaturationF();
		break;
	case RasterImageFile::color_type::e_cyan:
		return c.cyanF();
		break;
	case RasterImageFile::color_type::e_magenta:
		return c.magentaF();
		break;
	case RasterImageFile::color_type::e_yellow:
		return c.yellowF();
		break;
	case RasterImageFile::color_type::e_black:
		return c.blackF();
		break;
	default:
		throw invalid_argument("unknown color channel ");

	}
	//e_red, e_green, e_blue, e_alpha, e_L, e_H, e_S, e_cyan, e_magenta, e_yellow, e_black
}



RealFunction2D_F64 RasterImageFile::GetChannelInternal(color_type c)
{
	RealFunction2D_F64 result;

	result.resize(m_sizes[0], m_sizes[1]);

	QImage mQImage;
	mQImage.load(wstring_to_qstring(m_filename), m_format.c_str());

	for (size_t i = 0; i < m_sizes[0]; i++)
	{
		for (size_t j = 0; j < m_sizes[1]; j++)
		{
			const QColor &p = mQImage.pixelColor(int(j), int(i));
			result.at(i, j) = GetChannelValue(p, c);
		}
	}
	return result;
}


RealFunction2D_F32 RasterImageFile::GetGrayscale()
{
	RealFunction2D_F32 imgA(m_sizes[0], m_sizes[1], 0);

	QImage mQImage;
	mQImage.load(wstring_to_qstring(m_filename), m_format.c_str());

	for (size_t i = 0; i < m_sizes[0]; i++)
	{
		for (size_t j = 0; j < m_sizes[1]; j++)
		{
			const auto	&p = mQImage.pixelColor(int(i), int(j));
			//TODO сделать функцию rgb->brightness с такими параметрами
			imgA.at(i,j) = 255*(p.redF()*0.2989 + p.greenF()*0.5870 + p.blueF()*0.1141);
		}
	}

	return imgA;
}

#if 1
ColorImageF32 RasterImageFile::GetColor()
{
	QImage mQImage;
	mQImage.load(wstring_to_qstring(m_filename), m_format.c_str());
	ColorImageF32 imgA(m_sizes[0], m_sizes[1], ColorSampleF32(0));

	for (size_t i = 0; i < m_sizes[0]; i++)
	{
		for (size_t j = 0; j < m_sizes[1]; j++)
		{
			const auto	&p = mQImage.pixelColor(int(j), int(i));
			imgA.at(i, j) = ColorSampleF32(p.redF(), p.greenF(), p.blueF())*255.;
		}
	}


	return imgA;
}


#endif


XRAD_END