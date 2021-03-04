/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 2018/03/8 15:15
	\author nicholas
*/
#include "pre.h"
#include "source.h"

#include <XRADDicom/Sources/Utils/Utils.h>

XRAD_BEGIN

namespace Dicom
{

	//source--------------------------------------------------------------------------------------
	source::source()
	{
		init();
	}

	source::source(shared_ptr<Container> val)
	{
		init();
		set_dicom_container(val);
	}

	source::source(const instancestorage_ptr &other)
	{
		init();
		set_instancestorage(other);
		//todo (Kovbas) что-то ещё здесь должно быть, мне кажется =)
	}

	bool source::set_dicom_container(const shared_ptr<Container> &dcmFileContainer)
	{
#if 0
		if(dcmFileContainer)
		{
			m_dicom_container = dcmFileContainer;
			return bool(dicom_container());
		}
		else
			return bool(dicom_container());	//TODO логика неясна. Не должен ли объект после такого действия обнулять свой dicom_container();
#else
		m_dicom_container = dcmFileContainer;
		return bool(m_dicom_container);
#endif
	}
	/*+
	void source::create_new_dicom_container(const instancestorage_ptr &inst_src_in)
	{
		shared_ptr<Container> dcm_file(MakeContainer(inst_src_in));
		set_instancestorage(inst_src_in);
		set_dicom_container(dcm_file);
	}
	*/
	wstring	source::get_field(tag_e id) const
	{
		//note (Kovbas) будет работать только для однофреймовых файлов. В многофреймовых файлах будет отдан только элемент, найденный в данных, общих для всех. Чтобы искало среди всего возможного, нужно указывать номер фрейма
		auto f = dicom_container();
		if (!f)
			throw runtime_error("source::get_field(): DICOM file is not loaded.");
		return f->get_wstring(id);
	}

	wstring source::get_instance_content(bool byDCMTK)
	{
		return L"Instance: " + dicom_container()->last_used_instancestorage()->print() + L"\n\n" + dicom_container()->get_elements_to_wstring(byDCMTK);
	}

	bool source::save_to_file(const wstring &full_file_path_p, e_compression_type_t force_encoding)
	{
		// сохранить файл по указанному пути
		try
		{
			if (full_file_path_p != L"")
				return dicom_container()->save_to_file(full_file_path_p, force_encoding);
			else
				throw runtime_error("Full file path is empty!");
		}
		catch(...)
		{
			throw runtime_error("Can't save a new dicom file.\n" + GetExceptionStringOrRethrow());
		}
	}

	bool source::exist_and_correct_essential_data()
	{
		//общая схема работы:
		// 1. собрать теги, наличие которых нам нужно проверить
		// 2a. проверяем наличие тегов
		// 2b. проверяем корректность данных (от 4 зависит сможем ли мы создать нужного типа объект)
		// за то, что файл открыт, отвечает пользователь. Здесь это не рассматривается.

		//собираем список тегов, наличие которых нужно проверить
		set<tag_e> tagslist_to_check_data;
		fill_tagslist_to_check_data(tagslist_to_check_data);

		for (auto el : tagslist_to_check_data)
		{
			if(!dicom_container()->exist_element(el))
			{
				return false;
			}
		}

		//проверяем данные
		if (!check_values())
		{
			return false;
		}

		return true;
	}

	//note (Kovbas) Эту функцию использовать только с открытым файлом. Она не делает никаких проверок доступности данных, т.к. изначально предполагается как служебная и используется только в этом качестве.
	void source::collect_IDs()
	{
		set<tag_e> IDs_tags;
		fill_IDsTagsLst(IDs_tags);

		m_IDs_set.clear();
		for (auto el : IDs_tags)
		{
			m_IDs_set.insert({ el, dicom_container()->get_wstring(el, m_frame_no)});
		}
		// этот флаг был актуален, пока файл не был открыт.
		// он используется для быстрого индексирования каталогов.
		// если была вызвана функция collect_IDs, значит, получена
		// детальная информация из файла, а преиндекс перестал быть нужным.
		m_init_from_preindex = false;
	}


	// инициализировать source из внешнего индексированного источника данных
	bool source::InitFromPreindexed(DicomFileIndex &dcm_index)
	{
		//m_IDs_set.clear();
		for (size_t i = 0; i < dcm_index.get_dicom_tags_length(); i++)
			m_IDs_set[dcm_index.get_dicom_tags_ID(i)] =  dcm_index.get_dicom_tags_value(i);
		m_init_from_preindex = true;
		return true;
	}

	wstring source::get_field_from_IDs_set(tag_e id) const
	{
		if (m_IDs_set.find(id) == m_IDs_set.end())
		{
			throw runtime_error("source::get_field_from_IDs_set(): tag field is absent.");
		}
		return m_IDs_set.at(id);
	}


	/*
	void source::copy_generic_cache_(const source &other)
	{
		m_dicom_container_container = other.m_dicom_container_container;//возможно, копирует лишнее. Если так, вызов m_cache.reset(new instance_cache(*other.m_cache));
	}
	*/

	void source::open_instancestorage()
	{
		dicom_container()->open_instancestorage();

		collect_IDs();
	}

	bool source::try_open_instancestorage(const instancestorage_ptr &instance_src_p)
	{
		if (!dicom_container()->try_open_instancestorage(instance_src_p))
			return false;

		collect_IDs();
		return true;
	}

	/*
	bool source::open_instancestorage()
	{

		if (dicom_container()->open_instancestorage())
		{
			m_IDs_set.clear();
			collect_IDs();
			return true;
		}
		else
			return false;
	}*/

	void source::set_instancestorage(const instancestorage_ptr &instance_src_p)
	{
		dicom_container()->last_used_instancestorage() = instance_src_p;
	}

	void source::copy_container(const source &other)
	{
		m_dicom_container = other.m_dicom_container;
		m_frame_no = other.m_frame_no;
		if (dicom_container() && dicom_container()->is_opened())
			collect_IDs();
		else
			m_IDs_set = other.m_IDs_set;
	}

	void source::close_instancestorage()
	{
		dicom_container()->close_instancestorage();
	}

	void source::init()
	{
		m_frame_no = 1;
		m_dicom_container = MakeContainer();
		m_init_from_preindex = false;
	}

}//namespace Dicom

XRAD_END
