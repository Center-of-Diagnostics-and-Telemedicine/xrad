/*!
	* \file pacsTools.h
	* \date 10/10/2018 3:58:45 PM
	*
	* \author Kovbas (kovbas)
	*
	* \brief
	*
	* TODO: long description
	*
	* \note
*/
#ifndef pacsTools_h__
#define pacsTools_h__

#include <XRADDicom/DicomClasses/XRADDicomGeneral.h>
#include <XRADDicom/DicomClasses/DataContainers/datasource.h>
//#include <set>

//#include <XRADDicom/DicomClasses/Instances/LoadGenericClasses.h>

#undef dcmtklog

XRAD_BEGIN

namespace Dicom
{

	bool checkPACSAccessibility(const Dicom::pacs_params_t &src_pacs);


	bool studiesDownloader(const datasource_pacs &datasource, const list<elemsmap_t>&, study_id_type_t study_id_type, const wstring &destination_folder, ProgressProxy pp = VoidProgressProxy());




	bool sendInstancesMy();

} //namespace Dicom


XRAD_END

#endif // pacsTools_h__
