/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_BitmapContainer_h
#define XRAD__File_BitmapContainer_h

#include <XRADBasic/Sources/SampleTypes/ColorSample.h>
#include <XRADBasic/Sources/Containers/DataArray2D.h>

XRAD_BEGIN

//	класс BitmapContainerRGB содержит структуру Windows bitmap
//	доступны:
//	данные попиксельно, через оператор [] и функцию at контейнера (порядок строк снизу вверх);
//	данные с заголовком tagInfoHeader, пригодные для передачи в функции типа StretchDIBits
//	данные с дополнительным заголовком для записи в bmp файл

template<class T>
class	BitmapContainer: public DataArray2D<DataArray<T> >
{
	PARENT(DataArray2D<DataArray<T> >);
	typedef T sample_type;
protected:
	DataArray<uint8_t> buffer;

	using parent::realloc;
	using parent::resize;
	using parent::UseData;
	using parent::UseDataFragment;
	using parent::MakeCopy;

public:
	const void* GetBitmapFile() const{ return &buffer[0]; }
	size_t	GetBitmapFileSize() const{ return buffer.size(); }

	~BitmapContainer()
	{
		//в деструкторе удаляем ссылки на буфер, прежде чем удаляется этот буфер.
		realloc(0, 0);
	}
};

class	BitmapContainerRGB: public BitmapContainer<ColorPixel>
{
	PARENT(BitmapContainer<ColorPixel>);
public:
	BitmapContainerRGB(){}
	void SetSizes(size_t n_rows, size_t n_cols);

	const void* GetBitmapData() const;
	size_t	GetBitmapSize() const;

	/*!
		\brief Загружает данные из образа файла. Кидает исключение в случае ошибки
	*/
	void ParseFileData(const void *data, size_t size);
};

class	BitmapContainerIndexed: public BitmapContainer<uint8_t>
{
	PARENT(BitmapContainer<uint8_t>);
	const size_t	n_palette_colors;
public:
	BitmapContainerIndexed(): n_palette_colors(256){}
	void SetSizes(size_t n_rows, size_t n_cols);
	DataArray<ColorPixel>	palette;

	const void* GetBitmapData() const;
	size_t	GetBitmapSize() const;
	size_t palette_size() const;
};

XRAD_END

#endif // XRAD__File_BitmapContainer_h
