/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 22:12:2016 12:45
	\author kns
*/
#include "pre.h"
#include "DisplayProcessAcquisition.h"

#include "DisplayTomogram_GenericAcquisition.h"
#include "DisplayTomogram_CTAcquisition.h"
#include "DisplayTomogram_MRAcquisition.h"
#include "DisplayTomogram_MRAcquisitionSiemens.h"
#include "DisplayAcquisitionXRay.h"
#include "DisplayGenericImageAcquision.h"

XRAD_BEGIN

void DisplayProcessAcquisitionGeneric(const ProcessAcquisition_ptr &acquisition, const wstring &title)
{
	wstring tmpStr;
	tmpStr = L"Could not display acquisition consists next instances:\n\n";
	//for (auto el : acquisition->file_paths())
		//tmpStr += el + L"\n";
	/*?
	for (auto &el : acquisition->instancesources())
		tmpStr += el->print() + L"\n";
		*/
	ShowText(title, tmpStr);
}

void	DisplayProcessAcquisition(const ProcessAcquisition &acquisition, wstring title_in)
{
	//TODO разделение может быть более сложным, чем просто по модальности.
	//	Например, набор localizers будет иметь модальность КТ, но не будет
	//	содержать томограммы, а только набор срезов, для которых понадобится отдельная функция
	//	показа. Возможно, в классе ProcessAcquisition нужно будет предусмотреть список типов виде enum
	//	и пустую перегружаемую функцию, которая в своих реализациях будет возвращать тип конкретного объекта
	auto tmpMod{ acquisition.modality() };
	auto title{ tmpMod + L" " + title_in };
	//tomograms
	if (Dicom::is_modality_ct(tmpMod) && dynamic_cast<const CTAcquisition *>(&acquisition))
		DisplayTomogram_CTAcquisition(dynamic_cast<const CTAcquisition &>(acquisition), title);

	else if (Dicom::is_modality_mr(tmpMod) && dynamic_cast<const MRAcquisitionSiemens *>(&acquisition))
	{
		if ((acquisition.manufacturer().compare(L"Siemens")) ||
			(acquisition.manufacturer().compare(L"siemens")) ||
			(acquisition.manufacturer().compare(L"SIEMENS")))
			DisplayTomogram_MRAcquisitionSiemens(dynamic_cast<const MRAcquisitionSiemens &>(acquisition), title);
		else
			DisplayTomogram_MRAcquisition(dynamic_cast<const MRAcquisition &>(acquisition), title);
	}
	else if (Dicom::is_modality_tomogram(tmpMod) && dynamic_cast<const TomogramAcquisition *>(&acquisition))
		DisplayTomogram_GenericAcquisition(dynamic_cast<const TomogramAcquisition &>(acquisition), title);

	//XRAY generic
	else if (Dicom::is_modality_xray(tmpMod) && dynamic_cast<const XRayAcquisition *>(&acquisition))
		DisplayXRayGeneric(dynamic_cast<const XRayAcquisition &>(acquisition), title);

	//other

	else if ( dynamic_cast<const GenericImageAcquisition *>(&acquisition))
	{
		DisplayGenericImageAcquisition(dynamic_cast<const GenericImageAcquisition &>(acquisition),title);
	}
	//DisplayProcessAcquisitionGeneric(acquisition, title);
	else //throw invalid_argument("Cannot display a modality: " + convert_to_string(tmpMod));
		throw invalid_argument("Cannot display either modality: " + convert_to_string(tmpMod) +" or can't display generic image");
}

XRAD_END
