/*!
	* \file CreateProcessAcquisition.cpp
	* \date 5/4/2018 12:30:02 PM
	*
	* \author kovbas
	*
	* \brief
	*
	* TODO: long description
	*
	* \note
*/
#include "pre.h"
#include "CreateProcessAcquisition.h"


#include <XRADDicom/DicomClasses/ProcessContainers/TomogramAcquisition.h>
#include <XRADDicom/DicomClasses/ProcessContainers/CTAcquisition.h>
#include <XRADDicom/DicomClasses/ProcessContainers/MRAcquisition.h>
#include <XRADDicom/DicomClasses/ProcessContainers/MRAcquisitionSiemens.h>

//image
#include <XRADDicom/DicomClasses/ProcessContainers/XRAYAcquisition.h>

//instance

XRAD_BEGIN


ProcessAcquisition_ptr CreateProcessAcquisition(const shared_ptr<Dicom::acquisition_loader> &acquisition_p, ProgressProxy pproxy)
{
	wstring	modality, manufacturer;

	auto first_frame = *acquisition_p->front();

	if(first_frame.dicom_container()->is_opened())
	{
		// если файл открыт, читаем информацию
		modality = first_frame.modality();
		manufacturer = first_frame.manufacturer();
	}
	else
	{
		//если закрыт, открываем временно
		Dicom::instance_open_close_class inst(first_frame);
		modality = first_frame.modality();
		manufacturer = first_frame.manufacturer();
	}

	//tomogram
	if (Dicom::is_modality_ct(modality))
		return make_unique<CTAcquisition>(acquisition_p);
	else if (Dicom::is_modality_mr(modality))
	{
		if ((manufacturer.compare(L"Siemens")) ||
			(manufacturer.compare(L"siemens")) ||
			(manufacturer.compare(L"SIEMENS")))
			return make_unique<MRAcquisitionSiemens>(acquisition_p);
		else
			return make_unique<MRAcquisition>(acquisition_p);
	}
	else if (Dicom::is_modality_tomogram(modality))
		return make_unique<TomogramAcquisition>(acquisition_p);

	//xray
	else if (Dicom::is_modality_xray(modality))
		return make_unique<XRAYAcquisition>(acquisition_p);

	else
		//note (Kovbas) здесь хотелось сделать создание объекта для набора инстансев либо для одного инстанса, чтобы можно было использовать функцию просмотра базовых вещей из дайкома
		//proc_acquisition.reset(new ProcessAcquisition);
		throw invalid_argument("Cannot create a process container for a modality '" + convert_to_string(modality) + "'");
}

ProcessAcquisition_ptr CreateProcessAcquisition(const Dicom::acquisition_loader &acquisition_p, ProgressProxy pproxy)
{
	auto acquisition_new = make_shared<Dicom::acquisition_loader>(acquisition_p);
	return CreateProcessAcquisition(acquisition_new, pproxy);
}


XRAD_END
