/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 30:9:2016 15:35
	\author kns
*/
#ifndef ProcessCTSeries_h__
#define ProcessCTSeries_h__

#include "TomogramAcquisition.h"

XRAD_BEGIN

class CTAcquisition : public TomogramAcquisition
{
public:
	//constructors
	CTAcquisition(){}
	CTAcquisition(const shared_ptr<Dicom::acquisition_loader> &acquisition_loader_p)
		: TomogramAcquisition(acquisition_loader_p)
	{}
	//?CTAcquisition(size_t s0, size_t s1, size_t s2); //note этот конструктор только для создания новых сборок

	//gets
	//-const Dicom::instance_ptr get_generic_data_object() const { return generic_instance; }
	virtual std::string classname() const override { return "CTAcquisition"; }

	//sets

	//methods
	RealFunctionF32 voltages() const { return prepare_RealFunctionF32(Dicom::e_tube_voltage_KVP); }
	RealFunctionF32 currents() const { return load_ordered_currents(); };
	RealFunctionF32 CTDIvols() const { return prepare_RealFunctionF32(Dicom::e_CTDIvol); }

	virtual void put_elements_to_instance(Dicom::instance &instance, size_t num_frame) const override;

	//operators
	virtual CTAcquisition &operator=(const CTAcquisition &original);

//TODO ниже методы, с которыми нужно разобраться: оставить или удалить

	//--virtual void	PutFrameDataCreate(size_t frame, DicomDataContainer &dicom_file) const;

//	virtual void	PutFrameData(size_t frame, DicomDataContainer &df) const;
	//void PutFrameData(size_t frame, Dicom::tomogram_slice &params, DicomDataContainer &dicom_file) const;

	//virtual	generic_dicom_params_ptr GenerateSliceParams(size_t slice_no) const;
	//virtual Dicom::instance_ptr GenerateSliceParams(size_t slice_no) const;
	//virtual modality_t modality() const { return ct_slice; };

private:

	//-void realloc_local(const index_vector &sizes);
};



// ниже что-то старое. Нужно будет разбираться и удалять.
//TODO не на месте
//void GetSlicesCoords(RealFunctionF64 &coord_graph, dicom_params_list &data_chosen_series);
//void GetSlicesCoords(RealFunctionF64 &coord_graph, Dicom::acquisition &data_chosen_series);  //todo возможно, это уже не нужно

//void LoadMetaGraphs_CT(CTSeries &ct, dicom_params_list data_series);
//void GetDicomNumericMetaData_CT(DicomDataContainer *dicom_file, xray_ct_dicom_frame_params &params);

//--------------------------------------------------------------
//2016_12_22 added by kns

//std::vector<string> GetSeriesDescriptions(const dicom_study_t &dicom_study);
//+-std::vector<string> GetSeriesDescriptions(const dicom_study_t &dicom_study);

XRAD_END

#endif // ProcessCTSeries_h__
