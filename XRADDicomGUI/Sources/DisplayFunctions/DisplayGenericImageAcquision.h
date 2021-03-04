﻿/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 4/23/2018 5:47:27 PM
	\author kovbas
*/
#ifndef DisplayGenericImageAcquisition_h__
#define DisplayGenericImageAcquisition_h__

#include <XRADDicom/Sources/DicomClasses/ProcessContainers/XRAYAcquisition.h>

XRAD_BEGIN

void	DisplayGenericImageAcquisition(const GenericImageAcquisition &img, const wstring &title);

XRAD_END

#endif // DisplayAcquisitionXRay_h__