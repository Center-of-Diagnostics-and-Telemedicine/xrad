/*!
	\file

	\date 2018/03/8 15:00
	\author nicholas

	\brief
*/
#include "pre.h"
#include "instance.h"

XRAD_BEGIN

namespace Dicom
{

	wstring instance::get_summary_info_string() const
	{
		wstring	result;
		result += ssprintf(L"Patient name =\t%s\n", dicom_container()->get_wstring(e_patient_name).c_str());
		result += ssprintf(L"Patient sex =\t%s\n", dicom_container()->get_wstring(e_patient_sex).c_str());
		result += ssprintf(L"Patient birthdate =\t%s\n", dicom_container()->get_wstring(e_patient_birthdate).c_str());
		result += ssprintf(L"Patient age =\t%s\n", dicom_container()->get_wstring(e_patient_age).c_str());
		result += ssprintf(L"Patient id =\t%s\n", dicom_container()->get_wstring(e_patient_id).c_str());
		result += ssprintf(L"Study id =\t%s\n", study_id().c_str());
		result += ssprintf(L"Accession no =\t%s\n", accession_number().c_str());
		result += ssprintf(L"Series id =\t%s\n", dicom_container()->get_wstring(e_series_instance_uid).c_str());
		result += ssprintf(L"Modality =\t%s\n", dicom_container()->get_wstring(e_modality).c_str());
		result += ssprintf(L"Study date =\t%s\n", dicom_container()->get_wstring(e_study_date).c_str());
		result += ssprintf(L"Study time =\t%s\n", dicom_container()->get_wstring(e_study_time).c_str());
		result += ssprintf(L"Series time =\t%s\n", dicom_container()->get_wstring(e_series_time).c_str());
		result += ssprintf(L"Series description =\t%s\n", dicom_container()->get_wstring(e_series_description).c_str());
		result += ssprintf(L"Protocol name =\t%s\n", dicom_container()->get_wstring(e_protocol_name).c_str());
		result += ssprintf(L"Content time =\t%s\n", dicom_container()->get_wstring(e_content_time).c_str());
		result += ssprintf(L"Root datasource =\t%s\n", instance_storage()->print().c_str());
		return result;
	}
	/*
	bool instance::same_instance(const instance &inst) const
	{
		// Часто сравниваются instance, у которых отличаются только наиболее специфичные для
		// instance данные. Поэтому начинаем сравнение именно с них, заканчиваем общим patient_id.
		return
			(m_frame_no == inst.m_frame_no) &&
			(instance_number() == inst.instance_number()) &&
			(acquisition_number() == inst.acquisition_number()) &&
			(in_stack_position_number() == inst.in_stack_position_number()) &&
			(stack_id() == inst.stack_id()) &&
			(series_number() == inst.series_number()) &&
			(complete_study_id() == inst.complete_study_id()) &&
			(patient_id() == inst.patient_id());
	}*/

#if 0
	bool instance::collect_everything()
	{
		//check_opened_file();
		open_instancestorage(); //todo (Kovbas) проверить весь этот метод. Возможно, не нужно
		auto result = collect_everything_internal();
		//close_file();
		close_instancestorage();
		return result;
	}

	bool instance::collect_everything_internal()
	{ //todo (Kovbas) нужен ли этот метод?
		//dicom_container()->force_read_everything();
		return false;
	}
#endif


}//namespace Dicom


XRAD_END
