/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 2018/03/01 15:52
	\author kulberg
*/
#include "pre.h"
#include "DicomPredicates.h"

#include "DicomFilters.h"
#include "Instances/tomogram_slice.h"

XRAD_BEGIN


//! Фильтр, определяющий, не является аргумент вспомогательной сборкой томограммы КТ.
//!	Возвращает true для локализаторов и отчетов о дозовой нагрузке
//! В теперешнем состоянии это заготовка, требующая серьезной доработки.
//! Вероятное направление развития -- разделить на более простые условия, которые затем объединять через логические операции над предикатами
bool tomogram_localizer_condition::check(const Dicom::acquisition_loader &data) const
{
	if(!Dicom::is_modality_tomogram(data.front()->modality())) return false;
	for(auto &instance: data)
	{
		// Выявление локализаторов
		auto *slice = dynamic_cast<Dicom::tomogram_slice*>(instance.get());
		if(!slice)
			return false;
		auto image_type = slice->get_wstring_values(Dicom::e_image_type);
		if(image_type.size() < 3) return true;
		if(image_type[2]==L"LOCALIZER") return true;
	}

	return false;
}


bool auxiliary_tomogram_acquisition_condition::check(const Dicom::acquisition_loader &data) const
{
	if(Dicom::is_modality_ct(data.front()->modality()))
	{
		if (data.front()->get_m_frame_no())
			return false;

		for(auto &instance: data)
		{
			// Выявление не томограмм.
			// Любой instance внутри acquisition, не являющийся срезом томограммы,
			// делает весь acquisition "вспомогательным".
			auto *slice = dynamic_cast<Dicom::tomogram_slice*>(instance.get());
			if (!slice)
				return true;

			// Выявление локализаторов
			auto image_type = slice->dicom_container()->get_wstring_values(Dicom::e_image_type);
			if(image_type.size() < 3) return true;
			if(image_type[2]==L"LOCALIZER") return true;

			// Выявление вспомогательных изображений вроде dose report
			if(slice->dicom_container()->get_double_values(Dicom::e_image_position_patient).size() != 3)
					return true;//вектор положения отсутствует
			if(slice->dicom_container()->get_double_values(Dicom::e_image_orientation_patient).size() != 6)
					return true;//вектор ориентации отсутствует.

		}
	}

	return false;
}


bool id_in_list_condition::check(const Dicom::instance_ptr &dcm) const
{
	auto found = std::find(ids.begin(), ids.end(), dcm->complete_study_id());
	return found != ids.end();
}

XRAD_END
