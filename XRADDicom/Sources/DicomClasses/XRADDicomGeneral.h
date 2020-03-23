/*!
	* \file XRADDicomGeneral.h
	* \date 2/16/2018 2:28:02 PM
	*
	* \author kovbas
	*
	* \brief
	*
	* TODO: long description
	*
	* \note
*/
#ifndef XRADDicomGeneral_h__
#define XRADDicomGeneral_h__


#include <XRADDicom/DicomClasses/tags_enum.h>
#include <XRADDicom/DicomClasses/dataelement.h>


XRAD_BEGIN

namespace Dicom
{

	enum class e_dcm_hierarchy_level_t
	{
		patient,
		study,
		series,
		stack,
		acquisition,
		instance,
		unknown
	};
	extern map<e_dcm_hierarchy_level_t, wstring> dcm_hierarchy_level_names;


	//находится здесь, т.к. может использоваться не только при работе с DCMTK, но и другими библиотеками, подключаемыми через классы, образованными от DicomDataContainer
	enum e_compression_type_t
	{
		// общие значения
		e_unknown,
		e_uncompressed,

		// используются для декомпрессии
		e_jpeg,
		e_jpeg_ls,
		e_jpeg2k,

		// используются для компрессии
		e_jpeg_lossless
	};


	enum class study_id_type_t
	{
		accession_number = e_accession_number,
		study_id = e_study_id,
		study_instance_uid = e_study_instance_uid,
	};

} //namespace Dicom

enum e_saving_decision_options_t
{
	e_save_to_new_file,
	e_save_to_old_file,
	e_saving_decision_options_exit
};
const vector<wstring> &saving_decision_options();




XRAD_END

#endif // XRADDicomGeneral_h__