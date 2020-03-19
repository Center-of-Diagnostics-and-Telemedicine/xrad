/*!
	* \file XRADDicomUtils.h
	* \date 3/22/2018 12:10:31 PM
	*
	* \author kovbas
	*
	* \brief
	*
	* TODO: long description
	*
	* \note
*/
#ifndef XRADDicomUtils_h__
#define XRADDicomUtils_h__

#include <XRADDicom/DicomClasses/DataContainers/datasource.h>

XRAD_BEGIN

namespace Dicom
{

	std::wstring check_float_value(const double val);


} //namespace Dicom

XRAD_END

#endif // XRADDicomUtils_h__