/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 5/4/2018 1:12:18 PM
	\author Kovbas (kovbas)
*/
#ifndef MRAcquisitionSiemens_h__
#define MRAcquisitionSiemens_h__

#include "MRAcquisition.h"

XRAD_BEGIN

class MRAcquisitionSiemens : public MRAcquisition
{
public:
	MRAcquisitionSiemens(const shared_ptr<Dicom::acquisition_loader> &acquisition_loader_p)
		: MRAcquisition(acquisition_loader_p)
	{}
	//-MRAcquisitionSiemens(size_t s0, size_t s1, size_t s2); //note этот конструктор только для создания новых сборок

	//-RealFunctionF32 diffusion_b_values_siemens;

	//-virtual void get_element_from_instance(const Dicom::instance&, const size_t element_number) override;
	//-virtual	void realloc(const index_vector &sizes) override;
	virtual std::string classname() const override { return "MRAcquisition_Siemens"; }
	RealFunctionF32 diffusion_b_values_siemens() const
	{
		return prepare_RealFunctionF32(Dicom::e_diffusion_b_value_siemens);
	}

protected:


private:
	//-void realloc_local(const index_vector &sizes);

};

XRAD_END

#endif // MRAcquisitionSiemens_h__
