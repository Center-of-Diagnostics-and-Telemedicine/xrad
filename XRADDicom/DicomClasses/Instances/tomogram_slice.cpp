/********************************************************************
	created:	2016/12/26
	created:	26:12:2016   13:15
	author:		kns
*********************************************************************/
#include "pre.h"
#include "tomogram_slice.h"


XRAD_BEGIN

namespace Dicom
{

	wstring tomogram_slice::get_description()
	{
		return dicom_container()->get_wstring(e_series_description) + L" [" + modality() + ssprintf(L"%dx%d", dicom_container()->get_uint(e_rows), dicom_container()->get_uint(e_columns)) + L"]";
	}


} //namespace Dicom

XRAD_END
