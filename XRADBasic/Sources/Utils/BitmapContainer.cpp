/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#include "pre.h"
#include "BitmapContainer.h"
#include <cstring>

XRAD_BEGIN

//--------------------------------------------------------------
//
//	хранение, обработка и запись растровой картинки в файл Windows bmp
//
//	вне зависимости от платформы здесь заново определяются
//	некоторые типы, заданные по умолчаню в windows.h
//	при компиляции под windows эти определения не должны входить
//	в конфликт с исходными определениями, т.к. заданы в другом
//	namespace


//	важно! во всех структурах нужно отключить выравнивание,
//	т.к. их sizeof должен в точности соответствовать сумме
//	размеров всех их членов

#pragma pack(push,1)

//--------------------------------------------------------------
//
//	вспомогательные средства для изменения порядка байт с BE на LE
//	для платформ с big endian
//
#if (XRAD_ENDIAN == XRAD_BIG_ENDIAN)

char	&byte(int32_t &n, int byte_no) { return ((char*)(&n))[byte_no]; }
const char &byte(const int32_t &n, int byte_no) { return ((const char*)(&n))[byte_no]; }

char	&byte(int16_t &n, int byte_no) { return ((char*)(&n))[byte_no]; }
const char &byte(const int16_t &n, int byte_no) { return ((const char*)(&n))[byte_no]; }

void	swap4(int32_t &n, const int32_t &i)
{
	byte(n, 0) = byte(i, 3);
	byte(n, 1) = byte(i, 2);
	byte(n, 2) = byte(i, 1);
	byte(n, 3) = byte(i, 0);
}

void	swap2(int16_t &n, const int16_t &i)
{
	byte(n, 0) = byte(i, 1);
	byte(n, 1) = byte(i, 0);
}

//--------------------------------------------------------------
//
//	имитация целочисленных типов windows; все присвоения к
//	встроенным типам делаются с изменением порядка байт
//
class WORD
{
	int16_t	n;

public:
	WORD(int16_t i=0){ swap2(n, i); }
	operator int16_t() const
	{
		int16_t i;
		swap2(i, n);
		return i;
	}
};

class DWORD
{
	int32_t	n;

public:
	DWORD(int i=0){ swap4(n, i); }

	operator int32_t() const
	{
		int32_t	i;
		swap4(i, n);
		return i;
	}
};

typedef	DWORD LONG;

//--------------------------------------------------------------

#elif (XRAD_ENDIAN == XRAD_LITTLE_ENDIAN)

//--------------------------------------------------------------
//
//	для LE нужно только точно задать размеры полей
//
typedef int16_t WORD;
typedef int32_t DWORD;
typedef	DWORD LONG;
//--------------------------------------------------------------
#else
#error "Unknown endianness"
#endif

enum
{
	//	идентификатор 	компрессия
	BI_RGB = 0,		//несжатое изображение
	BI_RLE8 = 1, 		//сжатие RLE для 8-битных изображений
	BI_RLE4 = 2, 		//сжатие RLE для 4-битных изображений
	BI_BITFIELDS = 3,	//изображение не сжато, палитра содержит три 4-байтные маски для красной, зелЮной и синей компонент цвета. €спользуется для 16- и 32-битных изображений
	BI_JPEG = 4,		//Win98/Me/2000/XP: JPEG-сжатие
	BI_PNG = 5,		//Win98/Me/2000/XP: PNG-сжатие
	BI_ALPHABITFIELDS = 6	//WinCE: изображение не сжато, палитра содержит четыре 4-байтные маски для красной, зелЮной, синей и прозрачной (альфа-канал) компонент цвета. €спользуется для 16- и 32-битных изображений
};

struct RGBQUAD
{
	uint8_t	rgbBlue, rgbGreen, rgbRed, reserved;
	RGBQUAD(){ reserved = rgbBlue = rgbGreen = rgbRed = 0; };
	RGBQUAD(const uint8_t &x){ reserved = 0; rgbBlue = rgbGreen = rgbRed = x; };
	RGBQUAD(const ColorPixel &x){ reserved = 0; rgbBlue = x.blue(); rgbGreen = x.green(); rgbRed = x.red(); };
};

typedef struct tagBITMAPINFOHEADER
{
	DWORD  biSize;
	LONG   biWidth;
	LONG   biHeight;
	WORD   biPlanes;
	WORD   biBitCount;
	DWORD  biCompression;
	DWORD  biSizeImage;
	LONG   biXPelsPerMeter;
	LONG   biYPelsPerMeter;
	DWORD  biClrUsed;
	DWORD  biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;


typedef struct tagBITMAPFILEHEADER
{
	WORD    bfType;
	DWORD   bfSize;
	WORD    bfReserved1;
	WORD    bfReserved2;
	DWORD   bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

#pragma pack(pop)

//
//	все структуры заданы.
//	возвращаем выравнивание как было
//
//--------------------------------------------------------------

namespace
{

size_t	file_header_size()
{
	return sizeof(BITMAPFILEHEADER);
}

size_t	info_header_size()
{
	return sizeof(BITMAPINFOHEADER);
}

size_t	full_header_size()
{
	return file_header_size() + info_header_size();
}

size_t	rgb_pixel_size()
{
	return sizeof(ColorPixel);
}


static_assert(sizeof(RGBQUAD)==sizeof(ColorPixel), "RGBQUAD and ColorPixel size mismatch.");

const WORD BMP_Type = WORD('B' + ('M' << 8)); // "BM"

} // namespace

void	BitmapContainerRGB::SetSizes(size_t n_rows, size_t n_cols)
{
	if (vsize()==n_rows && hsize() == n_cols) return;

	buffer.realloc(full_header_size() + n_rows*n_cols*rgb_pixel_size());
	BITMAPFILEHEADER &file_header(*reinterpret_cast<BITMAPFILEHEADER*>(&buffer[0]));
	BITMAPINFOHEADER &info_header(*reinterpret_cast<BITMAPINFOHEADER*>(&buffer[file_header_size()]));
	UseData(reinterpret_cast<ColorPixel*>(&buffer[full_header_size()]), n_rows, n_cols);

	info_header.biSize=DWORD(info_header_size());
	info_header.biWidth=LONG(n_cols);
	info_header.biHeight=LONG(n_rows);
	info_header.biPlanes=1;
	info_header.biBitCount=32;
	info_header.biCompression=BI_RGB;
	info_header.biSizeImage=0;
	info_header.biXPelsPerMeter=0;
	info_header.biYPelsPerMeter=0;
	info_header.biClrUsed=0;
	info_header.biClrImportant=0;

	file_header.bfType = BMP_Type;
	file_header.bfSize = DWORD(file_header_size() + GetBitmapSize());
	file_header.bfReserved1 = 0;
	file_header.bfReserved2 = 0;
	file_header.bfOffBits = DWORD(full_header_size());
}



void BitmapContainerRGB::ParseFileData(const void *data, size_t size)
{
	if (size < sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER))
	{
		throw runtime_error(convert_to_string(L"Неверный формат файла BMP."));
	}
	const BITMAPFILEHEADER *file_header = reinterpret_cast<const BITMAPFILEHEADER*>(data);
	if (file_header->bfType != BMP_Type ||
			file_header->bfReserved1 ||
			file_header->bfReserved2)
	{
		throw runtime_error(convert_to_string(L"Неверный формат файла BMP."));
	}
	if (size_t(file_header->bfSize) > size)
	{
		throw runtime_error(convert_to_string(L"Неверный формат файла BMP (неверная длина файла)."));
	}
	const BITMAPINFOHEADER *info_header = reinterpret_cast<const BITMAPINFOHEADER*>(
			reinterpret_cast<const char*>(file_header) + sizeof(BITMAPFILEHEADER));
	if (info_header->biSize != sizeof(BITMAPINFOHEADER) || // Может быть другой размер для расширенных BMP (v4, v5)
			info_header->biWidth < 0)
	{
		throw runtime_error(convert_to_string(L"Данный формат файла BMP не поддерживается."));
	}
	if (info_header->biPlanes != 1)
	{
		throw runtime_error(convert_to_string(L"Неверный формат файла BMP."));
	}
	if (info_header->biCompression != BI_RGB)
	{
		throw runtime_error(convert_to_string(L"Данный формат файла BMP не поддерживается. Данные должны быть несжатыми."));
	}

	size_t image_width = info_header->biWidth;
	size_t image_height;
	bool top_down_bitmap;
	if (info_header->biHeight > 0)
	{
		image_height = info_header->biHeight;
		top_down_bitmap = false;
	}
	else
	{
		image_height = -info_header->biHeight;
		top_down_bitmap = true;
	}

	if (top_down_bitmap)
	{
		throw runtime_error(convert_to_string(L"Данный формат файла BMP не поддерживается (top-down bitmap)."));
	}

	switch (info_header->biBitCount)
	{
		case 24:
		{
			size_t alignedRowSizeBytes = (image_width * 3 + 3) & ~0x3; // выравниваем на DWORD
			size_t pixelDataSize = alignedRowSizeBytes * image_height;
			size_t pixelOffset = file_header->bfOffBits;
			if (pixelOffset + pixelDataSize > size_t(file_header->bfSize))
			{
				throw runtime_error(convert_to_string(L"Неверный формат файла BMP (неверный размер данных)."));
			}

			SetSizes(image_height, image_width);

			auto *src_pixel_data = reinterpret_cast<const unsigned char*>(file_header) + pixelOffset;
			auto *dest_pixel_data = &at(0, 0);
			for (size_t row = 0; row < image_height; ++row)
			{
				const auto *src_row = src_pixel_data + row * alignedRowSizeBytes;
				for (size_t col = 0; col < image_width; ++col)
				{
					auto r = src_row[col * 3];
					auto g = src_row[col * 3 + 1];
					auto b = src_row[col * 3 + 2];
					*dest_pixel_data++ = ColorPixel(r, g, b);
				}
			}
			break;
		}
		case 32:
		{
			size_t pixelDataSize = image_width * image_height * sizeof(ColorPixel);
			size_t pixelOffset = file_header->bfOffBits;
			if (pixelOffset + pixelDataSize > size_t(file_header->bfSize))
			{
				throw runtime_error(convert_to_string(L"Неверный формат файла BMP (неверный размер данных)."));
			}

			SetSizes(image_height, image_width);

			const void *pixel_data = reinterpret_cast<const char*>(file_header) + pixelOffset;
			memcpy(&at(0, 0), pixel_data, pixelDataSize);
			break;
		}
		default:
			throw runtime_error(convert_to_string(ssprintf(
				L"Данный формат файла BMP не поддерживается. Количество бит на пиксель должно быть равно 24 или 32, в данном файле: %i.",
				(int)info_header->biBitCount)));
	}
}



const void* BitmapContainerRGB::GetBitmapData() const
{
	return &buffer[file_header_size()];
}

size_t BitmapContainerRGB::GetBitmapSize() const
{
	return info_header_size() + vsize()*hsize()*rgb_pixel_size();
}


void BitmapContainerIndexed::SetSizes(size_t n_rows, size_t n_cols)
{
	if (vsize()==n_rows && hsize() == n_cols) return;

	size_t	n_cols4;
	if (n_cols%4) n_cols4 = n_cols - (n_cols%4) + 4;
	else n_cols4 = n_cols;

	buffer.realloc(full_header_size() + palette_size() + n_rows*n_cols4);
	BITMAPFILEHEADER &file_header(*reinterpret_cast<BITMAPFILEHEADER*>(&buffer[0]));
	BITMAPINFOHEADER &info_header(*reinterpret_cast<BITMAPINFOHEADER*>(&buffer[file_header_size()]));
	palette.UseData(reinterpret_cast<ColorPixel*>(&buffer[full_header_size()]), n_palette_colors);
	UseData(&buffer[full_header_size() + palette_size()], n_rows, n_cols4);

	for (size_t i=0; i<256; i++)
	{
		palette[i].red() = palette[i].green() = palette[i].blue() = uint8_t(i);
	}

	info_header.biSize=DWORD(info_header_size());
	info_header.biWidth=LONG(n_cols);//4;
	info_header.biHeight=LONG(n_rows);
	info_header.biPlanes=1;
	info_header.biBitCount=8;
	info_header.biCompression=BI_RGB;
	info_header.biSizeImage=0;
	info_header.biXPelsPerMeter=0;
	info_header.biYPelsPerMeter=0;
	info_header.biClrUsed=0;
	info_header.biClrImportant=0;

	file_header.bfType = BMP_Type;
	file_header.bfSize = DWORD(file_header_size() + GetBitmapSize());
	file_header.bfReserved1 = 0;
	file_header.bfReserved2 = 0;
	file_header.bfOffBits = DWORD(full_header_size() + palette_size());
}

size_t BitmapContainerIndexed::palette_size() const
{
	return n_palette_colors*rgb_pixel_size();
}

const void* BitmapContainerIndexed::GetBitmapData() const
{
	return &buffer[file_header_size()];
}

size_t BitmapContainerIndexed::GetBitmapSize() const
{
	return info_header_size() + palette_size() + vsize()*hsize();
}



XRAD_END
