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
#ifndef source_h__
#define source_h__

#include <XRADDicom/Sources/DicomClasses/XRADDicomGeneral.h>
#include <XRADDicom/Sources/DicomClasses/DataContainers/datasource.h>
#include <XRADDicom/Sources/DicomClasses/DataContainers/Container.h>
#include <XRADBasic/Sources/Core/cloning_ptr.h>
#include <set>

XRAD_BEGIN

namespace Dicom
{
//TODO Большинство описаний функций, неоправданно включенных в тело класса, перенесено в cpp
//	Теперь следует окинуть взглядом класс на предмет возможных объединений каких-либо пересекающихся по смыслу функций

	//! source класс, который обеспечивает доступ к dicom_containerformat
	class source
	{
	public:
		//constructors
		source();
		//source(wstring);
		source(shared_ptr<Container>);
		//source(const source &other) : file_info(other){ m_cache = other.cache().clone(); }
		source(const instancestorage_ptr &other);
		void init();

		//destructors
		virtual ~source() {}//обязан быть объявлен виртуальный деструктор, иначе возможны утечки памяти

		//sets
		//void set_num_of_frame() { num_of_frame = dicom_container()->to_int(e_number_of_frame); }
		void set_num_of_frame(const size_t val) { m_frame_no = val; }

		//methods
		//+void create_new_dicom_container(const instancestorage_ptr &storage);

		//! \details Не кидает исключения.
		void close_instancestorage();

		void set_instancestorage(const instancestorage_ptr &inst_src_in);

		const instancestorage_ptr &instance_storage() const { return dicom_container()->last_used_instancestorage(); }

		//! \details При ошибке кидает исключение и оставляет инстанс закрытым.
		void open_instancestorage();

		bool try_open_instancestorage(const instancestorage_ptr &inst_src_in);

		bool exist_and_correct_essential_data();

		bool InitFromPreindexed(DicomFileIndex &dcm_index);						//  заполнить значение дайком тагов из индексированной DicomFileIndex
		bool is_preindexed() const { return m_init_from_preindex; }				//	вернуть  true, если объект был получен заполнением InitFromPreindexed
		wstring	get_field_from_IDs_set(tag_e id) const;							//  вернуть значение поля, соответствующее входному тэгу из m_IDs_set
	protected:
		bool check_values()	{return check_values_internal();}

		virtual bool check_values_internal() { return true; } //note (Kovbas) не во всех классах требуются проверки, поэтому присутствует только при необходимости
		void collect_IDs();

		virtual void fill_tagslist_to_check_data(set<tag_e> &tagslist_to_check_data_p) { (void)tagslist_to_check_data_p; return; }
		virtual void fill_IDsTagsLst(set<tag_e> &IDsTagLst_p) { (void)IDsTagLst_p; return; }

	public:
		wstring	get_field(tag_e id) const;
		bool delete_element(tag_e id_p) { return dicom_container()->delete_dataelement(id_p); }

		wstring get_wstring(tag_e id_p, const wstring &default_value_p = L"") const { return dicom_container()->get_wstring(id_p, m_frame_no, default_value_p); }
		auto get_wstring_values(tag_e id, wchar_t delimeter = L'\\') const { return dicom_container()->get_wstring_values(id, m_frame_no, delimeter); }
		double get_double(tag_e id_p, double default_value_p = 0.0) const { return dicom_container()->get_double(id_p, m_frame_no, default_value_p); }

		void set_wstring(tag_e id_p, const wstring &value_p) { dicom_container()->set_wstring(id_p, value_p, m_frame_no); }
		void set_double(tag_e id_p, double value_p = 0.0) { dicom_container()->set_double(id_p, value_p, m_frame_no); }

		/*!
			\brief Возвращает содержимое DICOM-файла в виде строки
			\todo сделать возврат в виде списка элементов с формальными полями, очень важно
		*/
		wstring get_instance_content(bool byDCMTK = false);
		//virtual void set_new_values_to_instance(Container::error_process_mode epm);

		virtual bool save_to_file(const wstring &fold_path, e_compression_type_t force_encoding = e_uncompressed) final;

		//gets
		bool is_dicomdir() { return dicom_container()->is_dicomdir(); } //note Kovbas используется при проходе. Если дир, то не обрабатывается

		//sets
		bool set_dicom_container(const shared_ptr<Container>&);

		void copy_container(const source &other);

		void delete_all_private_tags() { dicom_container()->delete_all_private_tags(); }

		size_t & get_m_frame_no() { return m_frame_no; }
		vector<double> get_currents_mf() { return m_dicom_container->get_currents_mf(); }
	public:
		shared_ptr<Container> dicom_container() { return m_dicom_container; }
		shared_ptr<const Container> dicom_container() const { return m_dicom_container; }

	private:
		//-virtual void	init_cache() { m_cache = make_unique<instance_cache>(); }
		//! Образ dicom-файла, хранящийся в памяти. Не имеет жесткой связи с каким-либо файлом на диске, поэтому в имени _image
		//! Привязка к физическому файлу на диске только через file_info
		//note какая-то связь с файлом DCMTK все-таки остается, надо установить, какая именно.
		//Kovbas связь остаётся до тех пор, пока файл не закрыт (существуют указатели на объекты файла)

	protected:
		bool m_init_from_preindex;
		size_t m_frame_no; //номер фрейма в мультифрейме. Имеет значение от 1. В однофрейме имеет также значение 1. В функциях используется значение по умолчанию =0 для забора данных, не зависящих от номера фрейма, например, имя пациента.
		map<tag_e, wstring> m_IDs_set; //todo (Kovbas) реализовать наполнение его при открытии файла, наверное, правильнее это сделать при создании объекта или при открытии файла из него.

	private:
		shared_ptr<Container> m_dicom_container;
	};

}//namespace Dicom


XRAD_END

#endif // source_h__
