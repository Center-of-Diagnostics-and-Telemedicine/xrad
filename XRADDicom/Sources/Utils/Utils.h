﻿/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 3/22/2018 12:10:31 PM
	\author kovbas
*/
#ifndef XRADDicomUtils_h__
#define XRADDicomUtils_h__

#include <XRADDicom/Sources/DicomClasses/DataContainers/datasource.h>

XRAD_BEGIN

namespace Dicom
{

	std::wstring check_float_value(const double val);

} //namespace Dicom

XRAD_END

#endif // XRADDicomUtils_h__