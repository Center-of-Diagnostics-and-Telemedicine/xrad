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

#include <XRADGUI.h>

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
	if (Dicom::is_modality_ct(tmpMod))
		DisplayTomogram_CTAcquisition(dynamic_cast<const CTAcquisition &>(acquisition), title);
	else if (Dicom::is_modality_mr(tmpMod))
	{
		if ((acquisition.manufacturer().compare(L"Siemens")) ||
			(acquisition.manufacturer().compare(L"siemens")) ||
			(acquisition.manufacturer().compare(L"SIEMENS")))
			DisplayTomogram_MRAcquisitionSiemens(dynamic_cast<const MRAcquisitionSiemens &>(acquisition), title);
		else
			DisplayTomogram_MRAcquisition(dynamic_cast<const MRAcquisition &>(acquisition), title);
	}
	else if (Dicom::is_modality_tomogram(tmpMod))
		DisplayTomogram_GenericAcquisition(dynamic_cast<const TomogramAcquisition &>(acquisition), title);

	//XRAY generic
	else if (Dicom::is_modality_xray(tmpMod))
		DisplayXRAYGeneric(dynamic_cast<const XRAYAcquisition &>(acquisition), title);

	//other
	else
		//DisplayProcessAcquisitionGeneric(acquisition, title);
		throw invalid_argument("Cannot display a modality: " + convert_to_string(tmpMod));
}

XRAD_END
