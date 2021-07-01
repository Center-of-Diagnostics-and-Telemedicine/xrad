/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 4/23/2018 5:46:49 PM
	\author kovbas
*/
#ifndef DisplayTomogram_CTAcquisition_h__
#define DisplayTomogram_CTAcquisition_h__

#include <XRADDicom/Sources/DicomClasses/ProcessContainers/CTAcquisition.h>

XRAD_BEGIN

void	DisplayTomogram_CTAcquisition(const CTAcquisition &ct, const wstring &title);

XRAD_END

#endif // DisplayTomogram_CTAcquisition_h__