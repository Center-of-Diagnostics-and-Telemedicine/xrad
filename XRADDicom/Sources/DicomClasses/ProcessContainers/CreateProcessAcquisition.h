/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 5/4/2018 12:30:02 PM
	\author kovbas
*/
#ifndef CreateProcessAcquisition_h__
#define CreateProcessAcquisition_h__

#include "ProcessAcquisition.h"

#include <XRADDicom/Sources/DicomClasses/Instances/instance.h>

XRAD_BEGIN

ProcessAcquisition_ptr CreateProcessAcquisition(const Dicom::acquisition_loader& acquisition_p, ProgressProxy pproxy);
ProcessAcquisition_ptr CreateProcessAcquisition(const shared_ptr<Dicom::acquisition_loader> &acquisition_p, ProgressProxy pproxy);

XRAD_END

#endif // CreateProcessAcquisition_h__