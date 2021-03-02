/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 4/23/2018 5:46:58 PM
	\author kovbas
*/
#ifndef DisplayTomogram_MRAcquisition_h__
#define DisplayTomogram_MRAcquisition_h__

#include <XRADDicom/Sources/DicomClasses/ProcessContainers/MRAcquisition.h>

XRAD_BEGIN

void	DisplayTomogram_MRAcquisition(const MRAcquisition &mr, const wstring &title);

XRAD_END

#endif // DisplayTomogram_MRAcquisition_h__