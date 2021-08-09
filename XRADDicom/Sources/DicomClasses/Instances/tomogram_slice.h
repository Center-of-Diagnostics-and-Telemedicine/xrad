/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 26:12:2016 13:15
	\author kns
*/
#ifndef tomogram_slice_h__
#define tomogram_slice_h__

#include "image.h"
#include <XRADBasic/Sources/Containers/SpaceCoordinates.h>

XRAD_BEGIN

/*!
	\brief Класс tomogram_slice params, обеспечивающий облегченный доступ к метаданным dicom_file, содержащего срез томограммы

	В него вынесены поля, необходимые для идентификации среза томограммы без обращения непосредственно к dicom_file.
	Наследником являются класс ct_slice_params и mr_slice_params.

	Класс не содержит никаких изображений, но только обеспечивает извлечение соответствующих данных в другую структуру, а также
	при необходимости позволяет сортировать срезы по различным признакам, чтобы выстроить томограмму в правильном порядке

*/

namespace Dicom
{

	class tomogram_slice : public image
	{
		PARENT(image);
	public:
		//constructors
		tomogram_slice() {}

		//gets
		virtual wstring get_description();
		double slice_thickness() const { return dicom_container()->get_double(e_slice_thickness); }
		vector<double> image_orientation_patient() const { return dicom_container()->get_double_values(e_image_orientation_patient); }
		vector<double> image_position_patient() const { return dicom_container()->get_double_values(e_image_position_patient); }
	
		vector<double> image_position_patient(size_t frame_no) { return dicom_container()->get_image_position(frame_no); }
		vector<double> scales_xy_mf() { return dicom_container()->get_scales_xy_mf(); }
		double thickness_mf() { return dicom_container()->get_thickness_mf(); }
		vector<double> currentss_mf() { return dicom_container()->get_currents_mf(); }
		
		//sets
		void set_slice_thickness(double val) { dicom_container()->set_double(e_slice_thickness, val); }
		void set_image_position_patient(const std::vector<double> &val) { dicom_container()->set_double_values(e_image_position_patient, val); };


		//methods


		virtual	instance *clone() const override
		{
	//		collect_everything();
			return new tomogram_slice(*this);
		}// ранее существовавший метод copy не учитывал полиморфизма, во избежание путаницы переименовываю его в синоним

	protected:

		virtual void fill_tagslist_to_check_data(set<tag_e> &tagslist_to_check_data_p) override
		{
			parent::fill_tagslist_to_check_data(tagslist_to_check_data_p);

			tagslist_to_check_data_p.insert(e_image_position_patient);
			tagslist_to_check_data_p.insert(e_image_orientation_patient);

			tagslist_to_check_data_p.insert(e_pixel_spacing);
			tagslist_to_check_data_p.insert(e_slice_thickness);

		}

		virtual bool check_values_internal() override
		{
			if (parent::check_values_internal())
			{
				if (dicom_container()->get_double_values(e_image_position_patient).size() != 3) return false;//throw invalid_argument(ssprintf("Invalid image position vector size = %d", dicom_container()->get_double_values(e_image_position_patient).size()));
				if (dicom_container()->get_double_values(e_image_orientation_patient).size() != 6) return false;// throw invalid_argument(ssprintf("Invalid image_cos size = %d", dicom_container()->get_double_values(e_image_orientation_patient).size()));

				return true;
			}
			return false;
		}
		virtual void fill_IDsTagsLst(set<tag_e> &IDsTagLst_p) override
		{
			parent::fill_IDsTagsLst(IDsTagLst_p);

			//common tomogram IDs
			IDsTagLst_p.insert(e_stack_id);
			IDsTagLst_p.insert(e_acquisition_number);
		}

	};

}//namespace Dicom


XRAD_END

#endif // tomogram_slice_h__
