/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 2019/11/25
	\author kulberg
*/
#include "pre.h"
#include "SaveRasterImage.h"

#include "FileSaveUtils.h"

#include <XRADSystem/CFile.h>
#include <XRADQt/QtStringConverters.h>

XRAD_BEGIN

using namespace XRAD_GUI;

template<class BITMAP>
void	SaveImageTemplate(wstring filename, const BITMAP &bitmap)
{
	QString qfilename = wstring_to_qstring(filename);

//	char *type = "png (*.png);;jpeg (*.jpg);;bmp (*.bmp);;pdf (*.pdf)";
	if (qfilename.isEmpty()) return;
	if (FormatByFileExtension(qfilename) == "bmp")
	{
		shared_cfile	file;
		file.open(qstring_to_wstring(qfilename), L"wb");
		file.write(bitmap.GetBitmapFile(), bitmap.GetBitmapFileSize(), 1);
	}
	else
	{
		QPixmap pixmap(int(bitmap.vsize()), int(bitmap.hsize()));
		pixmap.loadFromData((const uint8_t*)bitmap.GetBitmapFile(), int(bitmap.GetBitmapFileSize()), "BMP");

		pixmap.save(qfilename);
	}
}

void	SaveImageUtil(wstring filename, const BitmapContainerRGB	&bitmap)
{
	SaveImageTemplate(filename, bitmap);
}

void	SaveImageUtil(wstring filename, const BitmapContainerIndexed &bitmap)
{
	SaveImageTemplate(filename, bitmap);
}



void	SaveRasterImage__dummy()
{
	BitmapContainerRGB	rgb_bmp;
 	//RealFunction2D_F32	img(100, 100, 1);
	//ColorImageUI8	img(100, 100);
	wstring	wfn;
	SaveRasterImage(wfn, rgb_bmp);
}

XRAD_END
