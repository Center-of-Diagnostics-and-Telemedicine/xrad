/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 9:7:2015 14:58
	\author MSU
*/
#ifndef DicomStorageAnalyze_h__
#define DicomStorageAnalyze_h__

#include "DataContainers/datasource.h"
#include "Instances/tomogram_slice.h"
#include "DicomFilenameFilter.h"
#include <XRADBasic/Containers.h>

XRAD_BEGIN

struct compare_frames_by_instance_number
{
	bool operator() (const Dicom::instance_ptr &first, const Dicom::instance_ptr &second)
	{
		XRAD_ASSERT_THROW(Dicom::is_modality_tomogram(first->modality()) && Dicom::is_modality_tomogram(second->modality()));
		return (first->instance_number() < second->instance_number());
	}
};

struct compare_frames_by_content_time
{
	bool operator() (const Dicom::instance_ptr &first, const Dicom::instance_ptr &second)
	{
		XRAD_ASSERT_THROW(Dicom::is_modality_tomogram(first->modality()) && Dicom::is_modality_tomogram(second->modality()));
		auto &fs = dynamic_cast<const Dicom::image &>(*first);
		auto &sn = dynamic_cast<const Dicom::image &>(*second);

		return (fs.content_time() < sn.content_time());
	}
};

//
//struct compare_frames_by_series_instance_uid
//{
//	bool operator() (const Dicom::instance_ptr &first, const Dicom::instance_ptr &second)
//	{
//		return (first->series_instance_uid() < second->series_instance_uid());
//	}
//};


//void Homogenize(Dicom::instances &dicom_folder_map, wstring modality_filter);

// вспомогательные функции: получение списка файлов и путей к ним,
// выделение из них списка дайкомов без разбивки на исследования/серии

//! \brief Фильтры для загрузки DICOM. Применяются в порядке: filter_t, dicom_instance_predicate,
//! dicom_acquisition_predicate
using DicomInstanceFilters_t = std::tuple<
		Dicom::filter_t,
		Dicom::dicom_instance_predicate//,
		//Dicom::dicom_acquisition_predicate
		>;

//! \brief Создать фильтр, пропускающий всё.
//! См. также перегруженные версии с разными наборами фильтров
inline DicomInstanceFilters_t MakeDicomInstanceFilters()
{
	return make_tuple<>(
			Dicom::filter_t(),
			Dicom::dicom_instance_predicate::true_predicate()//,
			//Dicom::dicom_acquisition_predicate::true_predicate()
		);
}

inline DicomInstanceFilters_t MakeDicomInstanceFilters(Dicom::dicom_instance_predicate instance_pred)
{
	return make_tuple<>(
			Dicom::filter_t(),
			instance_pred//,
			//Dicom::dicom_acquisition_predicate::true_predicate()
		);
}

inline DicomInstanceFilters_t MakeDicomInstanceFilters(Dicom::filter_t filter_p, Dicom::dicom_instance_predicate instance_pred)
{
	return make_tuple<>(
			filter_p,
			instance_pred
		);
}
/*
inline DicomInstanceFilters_t MakeDicomInstanceFilters(Dicom::dicom_instance_predicate instance_pred,
		Dicom::dicom_acquisition_predicate acquisition_pred)
{
	return make_tuple<>(
			Dicom::filter_t(),
			instance_pred//,
			//acquisition_pred
		);
}*/

/*!
	\brief Получение списка исследований с любого источника (каталога на файловой системе или PACS)
	\details С декабря 2018 г. функция претерпела значительные изменения.
	Привычный прототип, который до сих пор используется во многих проектах:
	GetDicomStudiesHeap(studies_heap, dicom_path, true, GUIProgressProxy());
	1. Результат помещался в заранее созданную ссылку.
	В новом прототипе результат возвращается.
	2. Вместо пути к каталогу первый аргумент представляет собой абстрактный источник данных
	(создаваемый на основе каталога или PACS). Для каталогов нужно использовать функцию
	Dicom::datasource_folder(dicom_path, true). Второй булев аргумент показывает, следует ли
	рекурсивно анализировать вложенные подкаталоги.
	3. Функция получает фильтры двух видов (...дописать, как их создавать...)
	Пример правильного вызова функции:
	auto studies_heap = GetDicomStudiesHeap(Dicom::datasource_folder(dicom_path, true), MakeDicomInstanceFilters(!instance_is_multiframe(), !tomogram_acquisition_is_auxiliary()), GUIProgressProxy());
*/


Dicom::patients_loader GetDicomStudiesHeap(const Dicom::datasource_t &dicom_src,
	//const Dicom::filter_t &filters,
	const DicomInstanceFilters_t &filters,
	ProgressProxy progress_proxy);

XRAD_END


#endif //DicomStorageAnalyze_h__
