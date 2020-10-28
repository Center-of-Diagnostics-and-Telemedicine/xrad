/*!
	\file
	\date 10/10/2018 3:58:45 PM
	\author Kovbas (kovbas)
*/
#ifndef pacsTools_h__
#define pacsTools_h__

#include <XRADDicom/Sources/DicomClasses/XRADDicomGeneral.h>
#include <XRADDicom/Sources/DicomClasses/DataContainers/datasource.h>
#include <XRADDicom/Sources/DicomClasses/Instances/LoadGenericClasses.h>



#undef dcmtklog

XRAD_BEGIN

namespace Dicom
{

	bool checkPACSAccessibility(const Dicom::pacs_params_t &src_pacs);

	list<DcmDataset> findDataset(const datasource_pacs &datasource_p, DcmDataset & request, const wstring &destination_folder, const string &id, size_t &count);

	bool findDataset(const datasource_pacs &datasource_p, string rec, const string &modality, list<DcmDataset> & wrkLst, const wstring &destination_folder, size_t &count);

	string getStudyInstanceUID(DcmDataset & dst);

	string getAccessionNumber(DcmDataset & dst);

	size_t getNumberOfInstances(DcmDataset & dst);

	set<string> getStudyInstanceUIDSet(list<DcmDataset> & wrkLst);

	bool sendInstancesMy();

} //namespace Dicom

XRAD_END

#endif // pacsTools_h__
