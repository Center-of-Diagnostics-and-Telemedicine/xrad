/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef SaveRasterImage_h__
#define SaveRasterImage_h__
/*!
	\file
	\date 2019/11/25
	\author kulberg
*/

#include <XRADBasic/MathFunctionTypes2D.h>

XRAD_BEGIN

//!	\brief Сохранение данных из массива DataArray2D в графический файл формата jpg, png, tif или bmp
//!	\param filename Имя сохраняемого файла с расширением. Расширением определяется формат документа
//!	\param image Двумерный массив сохраняемых данных
//! \param black, white. Значения исходного массива, соответствующие черному и белому цвету выходного изображения
template<class ROW_T>
void	SaveRasterImage(const wstring &filename, DataArray2D<ROW_T> &image, double black = 0, double white = 255);

XRAD_END

#include "SaveRasterImage.hh"

#endif //  SaveRasterImage_h__
