/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 30:9:2016 15:37
	\author kns
*/
#include "pre.h"
#include "CTAcquisition.h"

#include <XRADDicom/Sources/DicomClasses/DicomStorageAnalyze.h>
#include <XRADDicom/Sources/DicomClasses/Instances/ct_slice.h>
#include <XRADDicom/Sources/Utils/Utils.h>
#include <XRADBasic/Sources/Containers/DataArrayAnalyze2D.h>
#include <omp.h>

XRAD_BEGIN
/*?
xrad::CTAcquisition::CTAcquisition(size_t s0, size_t s1, size_t s2) : TomogramAcquisition(s0, s1, s2)
{
	//-CTAcquisition::realloc_local({s0, s1, s2});
}*/

void CTAcquisition::put_elements_to_instance(Dicom::instance &instance, size_t num_frame) const
{
	TomogramAcquisition::put_elements_to_instance(instance, num_frame);

//	auto	&ct = dynamic_cast<Dicom::ct_slice &>(instance);

	//todo (Kovbas) надо ли здесь это? Из-за этого dicom_file() публичный, может быть опасно.
	//?ct.dicom_container()->set_double(Dicom::e_tube_current, currents[num_frame]);
	//?ct.dicom_container()->set_double(Dicom::e_tube_voltage, voltages[num_frame]);
	//?ct.dicom_container()->set_double(Dicom::e_CTDIvol, CTDIvols[num_frame]);
}

CTAcquisition &CTAcquisition::operator=(const CTAcquisition &original)
{
	TomogramAcquisition::operator=(original);

	return *this;
}


XRAD_END