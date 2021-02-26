/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 4/2/2018 12:58:26 PM
	\author kovbas
*/
#ifndef mr_slice_h__
#define mr_slice_h__

#include "tomogram_slice.h"

XRAD_BEGIN

namespace Dicom
{

	class mr_slice : public tomogram_slice
	{
		PARENT(tomogram_slice);
	public:
		//constructors
		mr_slice() {};

		//gets
		double repetition_time() const { return dicom_container()->get_double(e_repetition_time); };
		double echo_time() const { return dicom_container()->get_double(e_echo_time); };
		double flip_angle() const { return dicom_container()->get_double(e_flip_angle); };
		double sar() const { return dicom_container()->get_double(e_sar); };
		double number_of_averages() const { return dicom_container()->get_double(e_number_of_averages); };
		double spacing_between_slices() const { return dicom_container()->get_double(e_spacing_between_slices); };
		double echo_train_length() const { return dicom_container()->get_double(e_echo_train_length); };
		double rf_echo_train_length() const { return dicom_container()->get_double(e_rf_echo_train_length); };
		double parallel_acquisition_technique() const { return dicom_container()->get_double(e_parallel_acquisition_technique); };
		double pixel_bandwidth() const { return dicom_container()->get_double(e_pixel_bandwidth); };
		double receive_coil_name() const { return dicom_container()->get_double(e_receive_coil_name); };

		double diffusion_b_value() const { return dicom_container()->get_double(e_diffusion_b_value); };
		double diffusion_directionality() const { return dicom_container()->get_double(e_diffusion_directionality); };

		double mr_velocity_encoding_sequence() const { return dicom_container()->get_double(e_mr_velocity_encoding_sequence); };
		double velocity_encoding_direction() const { return dicom_container()->get_double(e_velocity_encoding_direction); };
		double velocity_encoding_maximum_value() const { return dicom_container()->get_double(e_velocity_encoding_maximum_value); };
		double velocity_encoding_minimum_value() const { return dicom_container()->get_double(e_velocity_encoding_minimum_value); };
		double slab_thickness() const { return dicom_container()->get_double(e_slab_thickness); };
		double slab_orientation() const { return dicom_container()->get_double(e_slab_orientation); };

		double asl_crusher_flow_limit() const { return dicom_container()->get_double(e_asl_crusher_flow_limit); };
		double asl_bolus_cut_off_flag() const { return dicom_container()->get_double(e_asl_bolus_cut_off_flag); };
		double asl_bolus_cut_off_delay_time() const { return dicom_container()->get_double(e_asl_bolus_cut_off_delay_time); };


		//sets
		////void set_repetition_time(const double &val) { set_double(e_repetition_time, val); };
		//void set_repetition_time(const double &val) { set_double(e_repetition_time, val); };
		//void set_echo_time(const double &val) { set_double(e_echo_time, val); };
		//void set_flip_angle(const double &val) { set_double(e_flip_angle, val); };
		//void set_sar(const double &val) { set_double(e_sar, val); };
		//void set_number_of_averages(const double &val) { set_double(e_number_of_averages, val); };
		//void set_spacing_between_slices(const double &val) { set_double(e_spacing_between_slices, val); };
		//void set_echo_train_length(const double &val) { set_double(e_echo_train_length, val); };
		//void set_rf_echo_train_length(const double &val) { set_double(e_rf_echo_train_length, val); };
		//void set_parallel_acquisition_technique(const double &val) { set_double(e_parallel_acquisition_technique, val); };
		//void set_pixel_bandwidth(const double &val) { set_double(e_pixel_bandwidth, val); };
		//void set_receive_coil_name(const double &val) { set_double(e_receive_coil_name, val); };

		//void set_diffusion_b_value(const double &val) { set_double(e_diffusion_b_value, val); };
		//void set_diffusion_directionality(const double &val) { set_double(e_diffusion_directionality, val); };

		//void set_mr_velocity_encoding_sequence(const double &val) { set_double(e_mr_velocity_encoding_sequence, val); };
		//void set_velocity_encoding_direction(const double &val) { set_double(e_velocity_encoding_direction, val); };
		//void set_velocity_encoding_maximum_value(const double &val) { set_double(e_velocity_encoding_maximum_value, val); };
		//void set_velocity_encoding_minimum_value(const double &val) { set_double(e_velocity_encoding_minimum_value, val); };
		//void set_slab_thickness(const double &val) { set_double(e_slab_thickness, val); };
		//void set_slab_orientation(const double &val) { set_double(e_slab_orientation, val); };

		//void set_asl_crusher_flow_limit(const double &val) { set_double(e_asl_crusher_flow_limit, val); };
		//void set_asl_bolus_cut_off_flag(const double &val) { set_double(e_asl_bolus_cut_off_flag, val); };
		//void set_asl_bolus_cut_off_delay_time(const double &val) { set_double(e_asl_bolus_cut_off_delay_time, val); };


		//methods
//		virtual void anonymization(const anonymizer_settings &anonymize, const bool set_only_if_exist = true) override;

		virtual	instance *clone() const override
		{
			return new mr_slice(*this);
		}

	protected:
		virtual void fill_tagslist_to_check_data(set<tag_e> &tagslist_to_check_data_p) override
		{
			parent::fill_tagslist_to_check_data(tagslist_to_check_data_p);

/*
			tagslist_to_check_data_p.insert(e_repetition_time);
			tagslist_to_check_data_p.insert(e_echo_time);
			tagslist_to_check_data_p.insert(e_flip_angle);
			tagslist_to_check_data_p.insert(e_sar);
			tagslist_to_check_data_p.insert(e_number_of_averages);
			tagslist_to_check_data_p.insert(e_spacing_between_slices);
			tagslist_to_check_data_p.insert(e_echo_train_length);
			tagslist_to_check_data_p.insert(e_rf_echo_train_length);
			tagslist_to_check_data_p.insert(e_parallel_acquisition_technique);
			tagslist_to_check_data_p.insert(e_pixel_bandwidth);
			tagslist_to_check_data_p.insert(e_receive_coil_name);

			tagslist_to_check_data_p.insert(e_diffusion_b_value);
			tagslist_to_check_data_p.insert(e_diffusion_directionality);

			tagslist_to_check_data_p.insert(e_mr_velocity_encoding_sequence);
			tagslist_to_check_data_p.insert(e_velocity_encoding_direction);
			tagslist_to_check_data_p.insert(e_velocity_encoding_maximum_value);
			tagslist_to_check_data_p.insert(e_velocity_encoding_minimum_value);
			tagslist_to_check_data_p.insert(e_slab_thickness);
			tagslist_to_check_data_p.insert(e_slab_orientation);

			tagslist_to_check_data_p.insert(e_asl_crusher_flow_limit);
			tagslist_to_check_data_p.insert(e_asl_bolus_cut_off_flag);
			tagslist_to_check_data_p.insert(e_asl_bolus_cut_off_delay_time);
*/
		}

	private:
		//удаление технических данных об исследовании
//		void anonymization_mr_slice(const anonymizer_settings::level &anonymize, const bool set_only_if_exist = true);
	};

}

XRAD_END

#endif // mr_slice_h__