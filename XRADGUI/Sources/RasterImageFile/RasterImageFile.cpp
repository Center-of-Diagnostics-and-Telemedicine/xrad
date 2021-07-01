#include "pre.h"

#include <XRADSystem/CFile.h>
#include <XRADBasic/Sources/DataArrayIO/DataArrayIOEnum.h>
#include <XRADQt/QtStringConverters.h>
#include <XRADSystem/Sources/System/FileNameOperations.h>

#include "RasterImageFile.h"


#pragma warning(disable:4714)


#include <QtGui/QImage>

XRAD_BEGIN

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

	QImage local_qimage;

	XRAD_ASSERT_THROW_M(local_qimage.load(wstring_to_qstring(m_filename), m_format.c_str()),
		file_container_error,
		convert_to_string(ssprintf(L"File '%s' can not be opened with format '%s'", m_filename.c_str(), convert_to_wstring(m_format).c_str()), e_encode_literals));


	m_sizes[0] = local_qimage.height();
	m_sizes[1] = local_qimage.width();

	size_t format = local_qimage.format();

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


double	GetChannelValue(QColor c, color_channel ct)
{
	switch (ct)
	{
		case color_channel::red:
			return c.redF();
			break;
		case color_channel::green:
			return c.greenF();
			break;
		case color_channel::blue:
			return c.blueF();
			break;
		case color_channel::alpha:
			return c.alphaF();
			break;
		case color_channel::lightness:
			return c.lightnessF();
			break;
		case color_channel::hue:
			return c.hslHueF();
			break;
		case color_channel::saturation:
			return c.hslSaturationF();
			break;
		case color_channel::cyan:
			return c.cyanF();
			break;
		case color_channel::magenta:
			return c.magentaF();
			break;
		case color_channel::yellow:
			return c.yellowF();
			break;
		case color_channel::black:
			return c.blackF();
			break;
		default:
			throw invalid_argument("unknown color channel ");
	}
}


ColorImageF64 RasterImageFile::rgb_internal()
{
	ColorImageF64 result(m_sizes[0], m_sizes[1]);

	QImage local_qimage;
	local_qimage.load(wstring_to_qstring(m_filename), m_format.c_str());

	for(size_t i = 0; i < m_sizes[0]; i++)
	{
		for(size_t j = 0; j < m_sizes[1]; j++)
		{
			const QColor	&p = local_qimage.pixelColor(int(j), int(i));
			result.at(i, j) = ColorSampleF64(p.redF(), p.greenF(), p.blueF());
		}
	}
	return result;
}

RealFunction2D_F64 RasterImageFile::channel_internal(color_channel c)
{
	RealFunction2D_F64 result(m_sizes[0], m_sizes[1]);

	QImage local_qimage;
	local_qimage.load(wstring_to_qstring(m_filename), m_format.c_str());

	for (size_t i = 0; i < m_sizes[0]; i++)
	{
		for (size_t j = 0; j < m_sizes[1]; j++)
		{
			const QColor &p = local_qimage.pixelColor(int(j), int(i));
			result.at(i, j) = GetChannelValue(p, c);
		}
	}
	return result;
}



XRAD_END