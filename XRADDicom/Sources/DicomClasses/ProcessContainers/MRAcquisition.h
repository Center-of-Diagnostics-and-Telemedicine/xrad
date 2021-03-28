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
#ifndef ProcessMRAcquisition_h__
#define ProcessMRAcquisition_h__

#include "TomogramAcquisition.h"

XRAD_BEGIN

class MRAcquisition : public TomogramAcquisition
{
public:
	//constructors
	MRAcquisition(const shared_ptr<Dicom::acquisition_loader> &acquisition_loader_p)
		: TomogramAcquisition(acquisition_loader_p)
	{}
	//?MRAcquisition(size_t s0, size_t s1, size_t s2); //note этот конструктор только для создания новых сборок

	//gets
	//-const Dicom::instance_ptr get_generic_data_object() const { return generic_instance; }
	virtual std::string classname() const override { return "MRAcquisition"; }

	//sets

	//methods
	RealFunctionF32 repetition_times() const { return prepare_RealFunctionF32(Dicom::e_repetition_time); }
	RealFunctionF32 echo_times() const { return prepare_RealFunctionF32(Dicom::e_echo_time); }
	RealFunctionF32 flip_angles() const { return prepare_RealFunctionF32(Dicom::e_flip_angle); }
	RealFunctionF32 sars() const { return prepare_RealFunctionF32(Dicom::e_sar); }
	RealFunctionF32 number_of_averagess() const { return prepare_RealFunctionF32(Dicom::e_number_of_averages); }
	RealFunctionF32 spacing_between_slicess() const { return prepare_RealFunctionF32(Dicom::e_spacing_between_slices); }
	RealFunctionF32 echo_train_lengths() const { return prepare_RealFunctionF32(Dicom::e_echo_train_length); }
	RealFunctionF32 rf_echo_train_lengths() const { return prepare_RealFunctionF32(Dicom::e_rf_echo_train_length); }
	RealFunctionF32 parallel_acquisition_techniques() const { return prepare_RealFunctionF32(Dicom::e_parallel_acquisition_technique); }
	RealFunctionF32 pixel_bandwidths() const { return prepare_RealFunctionF32(Dicom::e_pixel_bandwidth); }
	RealFunctionF32 receive_coil_names() const { return prepare_RealFunctionF32(Dicom::e_receive_coil_name); }

	RealFunctionF32 diffusion_b_values() const { return prepare_RealFunctionF32(Dicom::e_diffusion_b_value); }
	RealFunctionF32 diffusion_directionalities() const { return prepare_RealFunctionF32(Dicom::e_diffusion_directionality); }

	RealFunctionF32 mr_velocity_encoding_sequences() const { return prepare_RealFunctionF32(Dicom::e_mr_velocity_encoding_sequence); }
	RealFunctionF32 velocity_encoding_directions() const { return prepare_RealFunctionF32(Dicom::e_velocity_encoding_direction); }
	RealFunctionF32 velocity_encoding_maximum_values() const { return prepare_RealFunctionF32(Dicom::e_velocity_encoding_maximum_value); }
	RealFunctionF32 velocity_encoding_minimum_values() const { return prepare_RealFunctionF32(Dicom::e_velocity_encoding_minimum_value); }
	RealFunctionF32 slab_thicknesses() const { return prepare_RealFunctionF32(Dicom::e_slab_thickness); }
	RealFunctionF32 slab_orientations() const { return prepare_RealFunctionF32(Dicom::e_slab_orientation); }

	RealFunctionF32 asl_crusher_flow_limits() const { return prepare_RealFunctionF32(Dicom::e_asl_crusher_flow_limit); }
	RealFunctionF32 asl_bolus_cut_off_flags() const { return prepare_RealFunctionF32(Dicom::e_asl_bolus_cut_off_flag); }
	RealFunctionF32 asl_bolus_cut_off_delay_times() const { return prepare_RealFunctionF32(Dicom::e_asl_bolus_cut_off_delay_time); }



	virtual void put_elements_to_instance(Dicom::instance &instance, size_t n) const override;

	//operators
	virtual MRAcquisition &operator=(const MRAcquisition &original);

};

XRAD_END

#endif // ProcessMRAcquisition_h__
