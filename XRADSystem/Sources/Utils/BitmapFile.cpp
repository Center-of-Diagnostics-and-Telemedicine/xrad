/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#include "pre.h"
#include "BitmapFile.h"
#include <XRADBasic/Sources/Utils/BitmapContainer.h>
#include <XRADSystem/CFile.h>

XRAD_BEGIN

void	WriteBitmapFile(const char *filename, const uint8_t *bits, size_t n_rows, size_t n_cols)
	{
	BitmapContainerIndexed	data;
	data.SetSizes(n_rows, n_cols);

	for (size_t i=0; i<n_rows; ++i)
		{
		size_t	data_offset = (n_rows - 1 - i)*n_cols;
		const uint8_t *b = bits + data_offset;
		std::copy(b,  b+n_cols, data.row(i).begin());
		}

	shared_cfile bmp_file;
	bmp_file.open(filename, "wb");
	bmp_file.write(data.GetBitmapFile(), data.GetBitmapFileSize(), 1);
	}

void	WriteBitmapFile (const char *filename, const ColorPixel *bits, size_t n_rows, size_t n_cols)
	{
	BitmapContainerRGB	data;
	data.SetSizes(n_rows, n_cols);

	for (size_t i=0; i<n_rows; ++i)
		{
		size_t	data_offset = (n_rows - 1 - i)*n_cols;

		const ColorPixel *b = bits + data_offset;

		std::copy(b,  b+n_cols, data.row(i).begin());
		}

	shared_cfile bmp_file;
	bmp_file.open(filename, "wb");
	bmp_file.write(data.GetBitmapFile(), data.GetBitmapFileSize(), 1);
	}

XRAD_END
