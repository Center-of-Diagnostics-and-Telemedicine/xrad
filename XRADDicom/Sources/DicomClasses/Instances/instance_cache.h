/*!
	\file
	\date 2018/09/21 15:01
	\author kulberg
*/
#ifndef instance_cache_h__
#define instance_cache_h__

#include <XRADDicom/Sources/DicomClasses/XRADDicomGeneral.h>
#include <XRADDicom/Sources/DicomClasses/DataContainers/Container.h>

XRAD_BEGIN

namespace Dicom
{

	// кэш используемых нами значений элементов файла.
	// также буфер для хранения наборов элементов,
	// используемых при редактировании instance

	class	instance_cache
	{
	private:
		using cache_t = map<tag_t, dataelement_t>;
		cache_t m_cache;

	public:
		virtual ~instance_cache() {}

		virtual instance_cache *clone() const { return new instance_cache(*this); }

		void	process_dataelement_error(Container &file, tag_e id, Container::error_process_mode epm);
		void	dump_to_dicom_file_image(Container &file, Container::error_process_mode epm);
		void	append(const instance_cache &original);// сделано для анонимизации, возможно, неудачно
		virtual void	clear() { m_cache.clear(); } //todo (Kovbas) Пока отключено, т.к. подразумеваем в работе, что эти данные сохраняются постоянно для нормальной работы всех частей. На данный момент их размен не столь критичен.

		void	set_element(const tag_e id, const wstring &value, dataelement_modify_condition c);


		void set_wstring(tag_e id, const wstring &new_value, bool set_only_if_exist = false);
		void set_wstring_values(tag_e id, const vector<wstring> &new_values, const wchar_t delimeter = L'\\');
		void set_double(tag_e id, const double new_value);
		void set_double_values(tag_e id, const vector<double> &new_values, const wchar_t delimeter = L'\\');
		void set_int(tag_e id, const int new_value);
		void set_uint(tag_e id, const size_t new_value);
		void set_int_values(tag_e id, const vector<int> &new_values, const wchar_t delimeter = L'\\');

		wstring get_wstring(tag_e id, const wstring &default_value = L"") const;
		vector<wstring> get_wstring_values(tag_e id, wchar_t delimeter = L'\\') const;
		double get_double(tag_e id, const double default_value = 0.0) const;
		vector<double> get_double_values(tag_e id, const wchar_t delimeter = L'\\') const;
		size_t get_uint(tag_e id, const size_t default_value = 0) const;
		int get_int(tag_e id, const int default_value = 0) const;
		vector<int> get_int_values(tag_e id, const wchar_t delimeter = L'\\') const;
		bool get_bool(tag_e) const;

		dataelement_t &operator[](tag_t n) { return m_cache[n]; }
		void fill(const dataelement_t &de) { for (auto &e : m_cache) e.second = de; }

		auto begin(){ return m_cache.begin(); }
		auto end(){ return m_cache.end(); }

		auto begin() const { return m_cache.begin(); }
		auto end() const { return m_cache.end(); }
		bool empty() const{ return m_cache.empty(); }
	};


	template<class IMAGE_T>
	class	image_cache : public instance_cache
	{
		PARENT(instance_cache);
		using self = image_cache<IMAGE_T>;
		IMAGE_T	m_image;
	public:
		virtual instance_cache *clone() const override { return new self(*this); }
		virtual void	clear()
		{
			parent::clear();
			m_image.realloc(0,0);
		}

		IMAGE_T	&image(){ return m_image; }
		const IMAGE_T	&image() const { return m_image; }
	};

	using gray_image_cache = image_cache<RealFunction2D_F32>;

}
XRAD_END

#endif // instance_cache_h__
