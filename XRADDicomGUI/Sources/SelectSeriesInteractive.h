/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 22:12:2016 12:28
	\author kns
*/
#ifndef SelectSeriesInteractive_h__
#define SelectSeriesInteractive_h__

#include <XRADDicom/Sources/DicomClasses/Instances/LoadGenericClasses.h>

XRAD_BEGIN

/*!
	\brief функция выбора сборки, с которой будет дальнейшая работа
*/
Dicom::acquisition_loader &SelectSeriesInteractive(Dicom::patients_loader &studies_heap);

XRAD_END

#endif // SelectSeriesInteractive_h__
