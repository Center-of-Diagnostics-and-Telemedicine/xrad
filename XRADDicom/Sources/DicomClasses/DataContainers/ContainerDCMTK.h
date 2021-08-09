/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 2017/10/03 14:43
	\author kulberg
*/
#ifndef DicomDataContainerDCMTK_h__
#define DicomDataContainerDCMTK_h__

#include "Container.h"
#include <mutex>

XRAD_BEGIN

namespace Dicom
{

	class ContainerDCMTK : public Container
	{
	public:
		//constructors
		ContainerDCMTK();
 		ContainerDCMTK(const instancestorage_ptr &storage);

		//destructors
		//virtual ~ContainerDCMTK() override {}

	public:
		virtual bool	try_open_instancestorage(const  instancestorage_ptr &instancesource) override;
		virtual bool create_empty_instancestorage(const instancestorage_ptr &instancestorage_p) override;
		virtual void	open_instancestorage() override;
		virtual bool is_opened() const override { return (m_dicom_file && 1); }
		void set_storage(const instancestorage_pacs &inst_str, DcmFileFormat &dcmFF) //todo (Kovbas) нужно для тестов Файнд. Если не нужно, удалить
		{
			m_dicom_file = make_unique<DcmFileFormat>(dcmFF);
			m_last_used_instancestorage.reset(new instancestorage_pacs(inst_str));
		}
		//void set_instancestorage(const instancestorage_ptr &instance_src_p);
		virtual void close_instancestorage() override;
		virtual bool	save_to_file(const wstring &full_file_path, e_compression_type_t encoding = e_uncompressed) override;

		//gets
		virtual wstring	get_wstring(tag_e id, size_t num_of_frame = 0, const wstring &default_value = L"") const override;
		virtual vector<wstring> get_wstring_values(tag_e id, size_t num_of_frame = 0, wchar_t delimeter = L'\\') const  override;
		virtual double get_double(tag_e id, size_t num_of_frame = 0, double default_value = 0.0) const  override;
		virtual vector<double> get_double_values(tag_e id, size_t num_of_frame = 0, wchar_t delimeter = L'\\') const  override;
		virtual size_t get_uint(tag_e id, size_t num_of_frame = 0, size_t default_value = 0) const  override;
		virtual int get_int(tag_e id, size_t num_of_frame = 0, int default_value = 0) const  override;
		virtual vector<int> get_int_values(tag_e id, size_t num_of_frame = 0, wchar_t delimeter = L'\\') const  override;
		virtual bool get_bool(tag_e) const;

		virtual bool get_pixeldata(RealFunction2D_F32 &img_in, size_t &bpp, bool &is_signed, size_t &ncomp, size_t num_of_frame = 0) const override;
		virtual bool get_color_pixeldata(ColorImageF32& img) const override;
		virtual wstring get_elements_to_wstring(bool byDCMTK) const override;
		virtual elemsmap_t get_elements_list() const override;

		// modify
		virtual bool set_wstring(tag_e id, const wstring &new_value, size_t num_of_frame = 0, bool set_only_if_exist = false) override;
		virtual void set_wstring_values(tag_e id, const vector<wstring> &new_values, wchar_t delimeter = L'\\', size_t num_of_frame = 0, bool set_only_if_exist = false) override;
		virtual void set_double(tag_e id, double new_value, size_t num_of_frame = 0, bool set_only_if_exist = false) override;
		virtual void set_double_values(tag_e id, const vector<double> &new_values, wchar_t delimeter = L'\\', size_t num_of_frame = 0, bool set_only_if_exist = false) override;
		virtual void set_int(tag_e id, int new_value, size_t num_of_frame = 0, bool set_only_if_exist = false) override;
		virtual void set_uint(tag_e id, size_t new_value, size_t num_of_frame = 0, bool set_only_if_exist = false) override;
		virtual void set_int_values(tag_e id, const vector<int> &new_values, wchar_t delimeter = L'\\', size_t num_of_frame = 0, bool set_only_if_exist = false) override;


		/*!
			\brief Кладёт изображение в объект файла
			\note Здесь не устанавливается то, как будет кодировано изображение. Это делается только при сохранении файла
		*/
		virtual void set_pixeldata(const RealFunction2D_F32 &img_in, size_t bpp, bool is_signed, size_t ncomp)  override;
		virtual void set_pixeldata_mf(const RealFunctionMD_F32 &img_in, size_t bpp, bool is_signed, size_t ncomp) override;
		virtual bool exist_element(tag_e id) const override;
		bool exist_element(const DcmTag &dcmTag) const;

		virtual void delete_all_private_tags() override;

		virtual vector<double>  get_image_position(size_t frame_no) override;
		virtual  double get_slope_mf() override;
		virtual double get_intercept_mf() override;
		virtual vector<double> get_scales_xy_mf() override;
		vector<double> get_currents_mf() override;
		virtual double get_thickness_mf() override;

	private:
		//read/save
		//! \details При ошибке возвращает false или вызывает исключение в зависимости от use_exceptions.
		virtual bool	open_file(const instancestorage_file &instancestorage_in, bool use_exceptions);
		//! \details При ошибке возвращает false или вызывает исключение в зависимости от use_exceptions.
		virtual bool	open_pacs(const  instancestorage_pacs &instancestorage_in, bool use_exceptions);

		//для работы с мультифреймом
		DcmSequenceOfItems *m_shared_frames_data_ptr;
		DcmSequenceOfItems *m_per_frame_data_ptr;

		void	SetTransferSyntax(E_TransferSyntax transfer_syntax);
		void	ForceUTF8Charset();

		unique_ptr<DcmFileFormat> m_dicom_file;

		virtual bool	delete_dataelement(tag_e id, bool all = true, bool into = true) override;
		virtual void	add_dataelement(tag_e id) /*override*/;

		string classname() const { return "ContainerDCMTK"; };
		friend list<int32_t>	GetTagList(Container &dcm_generic);

		mutable mutex dicom_file_mutex;

		//!	Временное решение для Dicom-файлов, содержащих в себе кодировку 1251.
		//!	Стандарт Dicom не допускает использование такой кодировки, однако
		//!	некоторые разработчики de facto пишут в ней текстовые данные.
		//!	Если в файле выявлено некорректное значение по тэгу 0x00080005,
		//!	этот флаг устанавливается в true. При чтении и записи данные
		//!	принудительно интерпретируются как закодированные в 1251.
		//!	Предлагаемое лучшее решение см. комментарии в теле функции
		//! ContainerDCMTK::ForceUTF8Charset
		bool	cp_1251 = false;
	};




}//namespace Dicom

XRAD_END

#endif // DicomDataContainerDCMTK_h__
