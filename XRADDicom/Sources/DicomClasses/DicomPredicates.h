/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef dicom_predicates_h__
#define dicom_predicates_h__
/*!
	\file
	\date 2018/02/20 10:59
	\author kulberg

	\brief  предикаты для сортировки Dicom
*/

#include "Instances/LoadGenericClasses.h"
#include <XRADBasic/Sources/Utils/Predicate.h>

XRAD_BEGIN


//--------------------------------------------------------------
//
using Dicom::dicom_instance_condition;
using Dicom::dicom_instance_predicate;

using Dicom::dicom_acquisition_condition;
using Dicom::dicom_acquisition_predicate;

//	Вспомогательные классы



//! проверка идентификаторов исследования по списку (комментарий недописан)
class	id_in_list_condition : public dicom_instance_condition
{
	list<Dicom::complete_study_id_t>	ids;
public:
	id_in_list_condition(const list<Dicom::complete_study_id_t> &in_ids) : ids(in_ids){}
	bool	check(const Dicom::instance_ptr &dcm) const override;
	dicom_instance_condition *clone() const override { return new id_in_list_condition(ids); }
};


//! проверка модальности
class	modality_condition : public dicom_instance_condition
{
	wstring	mod;
public:
	modality_condition(wstring in_mod) : mod(in_mod){}
	bool	check(const Dicom::instance_ptr &dcm) const override{ return mod==dcm->modality(); }
	dicom_instance_condition *clone() const override { return new modality_condition(mod); }
};

//! выявление многокадровых dicom файлов
class	multiframe_condition : public dicom_instance_condition
{
public:
	multiframe_condition(){}
	bool	check(const Dicom::instance_ptr &dcm) const override{ return dcm->frames_number() >= 1; }
	dicom_instance_condition *clone() const override { return new multiframe_condition(); }
};


//! Выявление вспомогательных сборок (изображений, у которых модальность томограммы, но они не являются срезами объемной томограммы)
// Удаляет:
// Локализаторы - по тегу image_type
// Растровые текстовые материалы (dose-reports, заключения врачей) - по отсутствию в них вектора пространственной ориентации

class	tomogram_localizer_condition : public dicom_acquisition_condition
{
public:
	bool	check(const Dicom::acquisition_loader &a) const override;
	tomogram_localizer_condition	*clone() const override { return new tomogram_localizer_condition(); }
};


class	auxiliary_tomogram_acquisition_condition : public dicom_acquisition_condition
{
public:
	bool	check(const Dicom::acquisition_loader &a) const override;
	dicom_acquisition_condition	*clone() const override { return new auxiliary_tomogram_acquisition_condition(); }
};


//	Пользовательские функции


inline dicom_instance_predicate	id_is_in_list(const list<Dicom::complete_study_id_t> &dicoms_list, bool in_direct = true)
{
	return predicate::checker<Dicom::instance_ptr>(id_in_list_condition(dicoms_list), in_direct);
}



inline dicom_instance_predicate	modality_is(const wstring &x, bool in_direct = true)
{
	return predicate::checker<Dicom::instance_ptr>(modality_condition(x), in_direct);
}

inline dicom_instance_predicate	modality_is_ct()
{
	return modality_is(L"CT");
}

inline dicom_instance_predicate	modality_is_tomogram()
{
	return modality_is_ct() | modality_is(L"MR");
}

inline dicom_instance_predicate	instance_is_multiframe()
{
	return predicate::checker<Dicom::instance_ptr>(multiframe_condition());
}


inline dicom_acquisition_predicate	tomogram_acquisition_is_auxiliary(bool in_direct = true)
{
	return predicate::checker<Dicom::acquisition_loader>(auxiliary_tomogram_acquisition_condition(), in_direct);
}

inline dicom_acquisition_predicate	tomogram_acquisition_is_localizer(bool in_direct = true)
{
	return predicate::checker<Dicom::acquisition_loader>(tomogram_localizer_condition(), in_direct);
}


inline dicom_acquisition_predicate	any_acquisition(bool in_direct = true)
{
	return dicom_acquisition_predicate::true_predicate();
}





XRAD_END

#endif // dicom_predicates_h__
