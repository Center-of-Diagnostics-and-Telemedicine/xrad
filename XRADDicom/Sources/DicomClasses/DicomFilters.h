/*!
	\file
	\date 2018/02/20 12:21
	\author kulberg

	\brief  Фильтры для dicom наборов, основанные на объектах из dicom_predicate.h
*/
#ifndef dicom_filters_h__
#define dicom_filters_h__

#include "DicomFilterCore.h"

XRAD_BEGIN


//	Функции создания готовых фильтров

//! удаление "пилотных" наборов из КТ
inline auto	RemoveAuxiliaryTomogramImages(){ return	make_shared<AcquisitionFilter>(!tomogram_acquisition_is_auxiliary()); }

inline auto	RemoveMultiframeFilter(){ return	make_shared<InstanceFilter>(!instance_is_multiframe()); }

//inline auto	RemoveDICOMDIRFilter() { return	make_shared<InstanceFilter>(!instance_is_dicomdir()); }

inline auto	LeaveTomogramFilter(){ return	make_shared<InstanceFilter>(modality_is_tomogram()); }
inline auto	KeepOnlyCTFilter(){ return	make_shared<InstanceFilter>(modality_is(L"CT")); }
inline auto	KeepOnlyMRFilter(){ return	make_shared<InstanceFilter>(modality_is(L"MR")); }

inline auto	RemoveSRFilter(){ return	make_shared<InstanceFilter>(modality_is(L"SR", false)); }

inline auto KeepOnlyStudiesFromList(std::list<Dicom::complete_study_id_t> dicoms_list){return make_shared<InstanceFilter>(id_is_in_list(dicoms_list, true));}

//!	удаление instances по произвольному условию
inline auto	EachInstanceFilter(dicom_instance_predicate pred){ return	make_shared<InstanceFilter>(pred); }


XRAD_END

#endif // dicom_filters_h__
