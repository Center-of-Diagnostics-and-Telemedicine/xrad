/*!
	\file
	\date 10/10/2018 3:58:45 PM
	\author Kovbas (kovbas)
*/
#ifndef pacsTools_h__
#define pacsTools_h__

#include <XRADDicom/Sources/DicomClasses/XRADDicomGeneral.h>
#include <XRADDicom/Sources/DicomClasses/DataContainers/datasource.h>

#undef dcmtklog

XRAD_BEGIN

namespace Dicom
{

	bool checkPACSAccessibility(const Dicom::pacs_params_t &src_pacs);

	bool sendInstancesMy();

} //namespace Dicom

XRAD_END

#endif // pacsTools_h__
