#include "pre_GUI.h"
#include "SaveRasterImage.h"
#include "FileSaveUtils.h"

#include <XRADSystem/CFile.h>
#include <XRAD/PlatformSpecific/Qt/Internal/StringConverters_Qt.h>

/*!
\file
\date 2019/11/25
\author kulberg

\brief
*/



XRAD_BEGIN

using namespace XRAD_GUI;

BitmapContainerRGB	rgb_bmp;
BitmapContainerIndexed	indexed_bmp;



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


void	ssss()
{
 	RealFunction2D_F32	img(100, 100, 1);
//	ColorImageUI8	img(100, 100);
	QString	qfn;
	wstring	wfn;
//	save_bitmap(qfn, rgb_bmp);
	SaveRasterImage(wfn, rgb_bmp);
}


XRAD_END
