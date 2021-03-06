﻿/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 4/23/2018 5:46:58 PM
	\author kovbas
*/
#ifndef DisplayTomogram_MRAcquisitionSiemens_h__
#define DisplayTomogram_MRAcquisitionSiemens_h__

#include <XRADDicom/Sources/DicomClasses/ProcessContainers/MRAcquisitionSiemens.h>

XRAD_BEGIN

void	DisplayTomogram_MRAcquisitionSiemens(const MRAcquisitionSiemens &mr, const wstring &title);

XRAD_END

#endif // DisplayTomogram_MRAcquisitionSiemens_h__