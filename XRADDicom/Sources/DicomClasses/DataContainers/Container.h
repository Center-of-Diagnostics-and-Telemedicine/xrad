/*!
	\file
	\date 2017/10/02 17:28
	\author kulberg
*/
#ifndef DicomDataContainer_h__
#define DicomDataContainer_h__

#include <vector>

#include <XRADBasic/Core.h>
#include <XRADBasic/Sources/Core/cloning_ptr.h>

#include <XRADDicom/Sources/DicomClasses/XRADDicomGeneral.h>
#include "datasource.h"
#include <XRADBasic/MathFunctionTypesMD.h>

XRAD_BEGIN

namespace Dicom
{

	class error : public xrad_exception
	{
	public:
		explicit error(const char* what_arg) : xrad_exception(what_arg) {}
		explicit error(const string& what_arg) : xrad_exception(what_arg) {}
	};



	class Container
	{
	protected:
		// Из того, как это поле используется, ясно, что оно просто хранит последнее имя файла, к которому было обращение
		// Чтобы не создавать путаницы, даю соответстенное название
		instancestorage_ptr m_last_used_instancestorage;

		size_t m_frames_number{0}; //кол-во фреймов в многофреймовом файле. По умолчанию 0, что также означает, что файл не мультифреймовый.
		bool m_is_dicomdir;
	public:

		//! Режим обработки ошибок при записи в dicom контейнер
		enum error_process_mode
		{
			e_delete_wrong_elements, e_ignore_error, e_throw_on_error
		};

		Container()
			: m_frames_number{0}
			, m_is_dicomdir{false}
			, m_last_used_instancestorage {nullptr}
		{}
		virtual ~Container() = default;

		const instancestorage_ptr &last_used_instancestorage() const { return m_last_used_instancestorage; }
		instancestorage_ptr &last_used_instancestorage() { return m_last_used_instancestorage; }

		void	process_dataelement_error(tag_e id, error_process_mode epm);


		virtual bool	try_open_instancestorage(const  instancestorage_ptr &instancestorage) = 0;
		virtual bool	create_empty_instancestorage(const  instancestorage_ptr &instancestorage) = 0;
		//! \details В случае ошибки вызывает исключение.
		virtual void	open_instancestorage() = 0;
		virtual bool is_opened() const = 0;
		//! \details Не кидает исключения.
		virtual void close_instancestorage() = 0;
		virtual bool	save_to_file(const wstring &full_file_path, e_compression_type_t encoding = e_uncompressed) = 0;
		//virtual bool	save_to_storage(const wstring &full_file_path, e_compression_type_t encoding = e_uncompressed) = 0;

		bool is_dicomdir() const { return m_is_dicomdir; }
		size_t	frames_number() const { return m_frames_number; }
		bool is_multiframe() const { return frames_number(); }

		virtual bool	delete_dataelement(tag_e id, bool all = true, bool into = true) = 0;
		virtual bool	exist_element(tag_e id) const = 0;
		
		//gets
		virtual wstring	get_wstring(tag_e id, size_t num_of_frame = 0, const wstring &default_value = L"") const = 0;
		//?virtual wstring get_wstring(tag_e id, const wstring &default_value = L"") const = 0;
		virtual vector<wstring> get_wstring_values(tag_e id, size_t num_of_frame = 0, wchar_t delimeter = L'\\') const = 0;
		virtual double get_double(tag_e id, size_t num_of_frame = 0, double default_value = 0.0) const = 0;
		virtual vector<double> get_double_values(tag_e id, size_t num_of_frame = 0, wchar_t delimeter = L'\\') const = 0;
		virtual size_t get_uint(tag_e id, size_t num_of_frame = 0, size_t default_value = 0) const = 0;
		virtual int get_int(tag_e id, size_t num_of_frame = 0, int default_value = 0) const = 0;
		virtual vector<int> get_int_values(tag_e id, size_t num_of_frame = 0, wchar_t delimeter = L'\\') const = 0;
		virtual bool get_bool(tag_e) const = 0;

		virtual bool	get_pixeldata(RealFunction2D_F32 &img_in, size_t &bpp, bool &is_signed, size_t &ncomp, size_t num_of_frame = 0) const = 0;
		virtual wstring get_elements_to_wstring(bool by_lib) const = 0;
		virtual elemsmap_t get_elements_list() const = 0;

		// modify
		//virtual void	set_wstring(tag_e id, const wstring &new_value, dataelement_delete_condition dc = force_preserve()) = 0;
		virtual void	set_pixeldata(const RealFunction2D_F32 &img_in, size_t bpp, bool is_signed, size_t ncomp) = 0;
		virtual void set_pixeldata_mf(const RealFunctionMD_F32 &img_in, size_t bpp, bool is_signed, size_t ncomp) = 0;

		virtual bool set_wstring(tag_e id, const wstring &new_value, size_t num_of_frame = 0, bool set_only_if_exist = false) = 0;
		virtual void set_wstring_values(tag_e id, const vector<wstring> &new_values, wchar_t delimeter = L'\\', size_t num_of_frame = 0, bool set_only_if_exist = false) = 0;
		virtual void set_double(tag_e id, double new_value, size_t num_of_frame = 0, bool set_only_if_exist = false) = 0;
		virtual void set_double_values(tag_e id, const vector<double> &new_values, wchar_t delimeter = L'\\', size_t num_of_frame = 0, bool set_only_if_exist = false) = 0;
		virtual void set_int(tag_e id, int new_value, size_t num_of_frame = 0, bool set_only_if_exist = false) = 0;
		virtual void set_uint(tag_e id, size_t new_value, size_t num_of_frame = 0, bool set_only_if_exist = false) = 0;
		virtual void set_int_values(tag_e id, const vector<int> &new_values, wchar_t delimeter = L'\\', size_t num_of_frame = 0, bool set_only_if_exist = false) = 0;
		virtual void delete_all_private_tags() = 0;
		
		//multiframe
		virtual vector<double> get_image_position(size_t frame_no) = 0;
		virtual  double get_slope_mf() = 0;
		virtual double get_intercept_mf() = 0;
		virtual vector<double> get_scales_xy_mf() = 0;
		virtual double get_thickness_mf() = 0;

	};

	shared_ptr<Container>	MakeContainer();
	shared_ptr<Container>	MakeContainer(const instancestorage_ptr &inst_src_in);

}//namespace Dicom

XRAD_END

#endif // DicomDataContainer_h__
