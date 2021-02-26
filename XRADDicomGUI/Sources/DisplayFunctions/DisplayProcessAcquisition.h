/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 22:12:2016 12:45
	\author kns
*/
#ifndef TomogramGUI_h__
#define TomogramGUI_h__

#include <XRADDicom/Sources/DicomClasses/ProcessContainers/ProcessAcquisition.h>

XRAD_BEGIN

void	DisplayProcessAcquisition(const ProcessAcquisition &acquisition, wstring title);

XRAD_END

#endif // TomogramGUI_h__
