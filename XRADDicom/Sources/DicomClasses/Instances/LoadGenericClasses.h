/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 12/18/2017 2:46:43 PM
	\author kovbas
*/
#ifndef LoadGenericClasses_h__
#define LoadGenericClasses_h__

#include "instance.h"
#include <XRADDicom/Sources/DicomClasses/Hierarchy.h>
#include <XRADBasic/Sources/Utils/Predicate.h>

XRAD_BEGIN

/*!
  Величины спрятаны, сделать геттеры и сеттеры для всех частей.
  Путь к файлу (имя фолдера) должно содержать в конце слэш, иначе возникает путаница. Поэтому во всех методах должны присутствовать проверки, устраняющие её.
*/

namespace Dicom
{
	// материал для отдельного исходника

	using dicom_instance_condition = predicate::condition<Dicom::instance_ptr>;
	using dicom_instance_predicate = predicate::checker<Dicom::instance_ptr>;

	struct acquisition_loader : public acquisition<instance_ptr>
	{
	public:
		void prepare();

	public:
		typedef acquisition_loader self;

		acquisition_loader(const size_t id_in, const wstring &in_description) : acquisition<instance_ptr>(acquisition_id_t(id_in, point2_ST(0, 0), vector<double>()), in_description) {}
		acquisition_loader(const instance &in_instance) : acquisition<instance_ptr>(in_instance.acquisition_id(), L"") {}

		acquisition_loader(const self &s2) = default;
		self& operator=(const self &s2) = default;
		acquisition_loader(self &&s2) = default;
		self& operator=(self &&s2) = default;

		wstring	detect_root_folder_name() const;
		void filter(const dicom_instance_predicate &pred);

		//! \details При ошибке кидает исключение и оставляет инстансы закрытыми.
		void open_instancestorages();

		//! \details Не кидает исключения.
		void close_instancestorages();
	};

	using dicom_acquisition_condition = predicate::condition<Dicom::acquisition_loader>;
	using dicom_acquisition_predicate = predicate::checker<Dicom::acquisition_loader>;

	class acquisition_loader_Open_Close_storages
	{
	public:
		acquisition_loader_Open_Close_storages() = delete;
		acquisition_loader_Open_Close_storages(acquisition_loader &acquisition_loader_p)
			: acq_loader(acquisition_loader_p)
		{
			acq_loader.open_instancestorages();
		}
		acquisition_loader_Open_Close_storages(const acquisition_loader_Open_Close_storages &) = delete;
		acquisition_loader_Open_Close_storages &operator=(const acquisition_loader_Open_Close_storages &) = delete;
		// TODO: Копирование объекта невозможно, но move-семантику можно реализовать.
		~acquisition_loader_Open_Close_storages()
		{
			acq_loader.close_instancestorages();
		}
	private:
		acquisition_loader &acq_loader;
	};

	struct stack_loader : public stack<acquisition_loader>
	{
		typedef stack_loader self;

		stack_loader(const wstring &id_in, const wstring &in_description) : stack<acquisition_loader>(id_in, in_description) {}
		stack_loader(const instance &in_instance) : stack<acquisition_loader>(in_instance.stack_id(), L"") {}
		wstring	detect_root_folder_name() const;

		stack_loader(const self &s2) = default;
		self& operator=(const self &s2) = default;
		stack_loader(self &&s2) = default;
		self& operator=(self &&s2) = default;
	};

	struct series_loader : public series<stack_loader>
	{
		typedef series_loader self;
		series_loader(const complete_series_id_t &id_in, const wstring &in_description) : series<stack_loader>(id_in, in_description) {}
		series_loader(const instance &in_instance) : series<stack_loader>(in_instance.complete_series_id(), in_instance.series_description()) {}

		series_loader(const self &s2) = default;
		self& operator=(const self &s2) = default;
		series_loader(self &&s2) = default;
		self& operator=(self &&s2) = default;

		wstring	detect_root_folder_name() const;
	};


	struct study_loader : public study<series_loader>
	{
		typedef study_loader self;
//		study_loader(const wstring &in_study_id, const wstring &in_accession_number, const wstring &in_description) : study<series_loader>(in_study_id, in_accession_number, in_description){}
		study_loader(const complete_study_id_t &id_in, const wstring &in_description) : study<series_loader>(id_in, in_description){}
		study_loader(const instance &in_instance) : study<series_loader>(in_instance.complete_study_id(), in_instance.study_description()){}

		study_loader(const self &s2) = default;
		self& operator=(const self &s2) = default;
		study_loader(self &&s2) = default;
		self& operator=(self &&s2) = default;

		wstring	detect_root_folder_name() const;
	};

	struct patient_loader : public patient<study_loader>
	{
		typedef patient_loader self;

		patient_loader(const wstring &id_in, const wstring &in_description) : patient<study_loader>(id_in, in_description) {}
		patient_loader(const instance &in_instance) : patient<study_loader>(in_instance.patient_id(), in_instance.patient_name()) {}
		wstring	detect_root_folder_name() const;

		patient_loader(const self &s2) = default;
		self& operator=(const self &s2) = default;
		patient_loader(self &&s2) = default;
		self& operator=(self &&s2) = default;
	};

	//typedef patient patient_loader;

	struct patients_loader: public patients<patient_loader>
	{
		wstring	detect_root_folder_name() const;
	};


}//namespace Dicom

XRAD_END

#endif // LoadGenericClasses_h__