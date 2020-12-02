/*!
	\file
	\date 10/18/2018 12:47:56 PM
	\author Kovbas (kovbas)
*/
#ifndef datasource_h__
#define datasource_h__

#include <XRADDicom/Sources/DicomClasses/dataelement.h>
#include <XRADBasic/Sources/Core/cloning_ptr.h>
#include <XRADDicom/Sources/Utils/file_info.h>
#include <XRADDicom/Sources/DicomClasses/Indexing/DicomFileIndex.h>

XRAD_BEGIN

namespace Dicom
{
	enum class e_request_t
	{
		cget,
		cmove
	};

	class pacs_params_t
	{
	public:
		pacs_params_t() = delete;
		pacs_params_t(const wstring &address_pacs_p, size_t port_pacs_p, const wstring &AETitle_pacs_p, const wstring &AETitle_local_p, size_t port_local_p, e_request_t request_type_p = e_request_t::cmove)
			:m_address_pacs(address_pacs_p), m_port_pacs(port_pacs_p), m_AETitle_pacs(AETitle_pacs_p), m_AETitle_local(AETitle_local_p), m_port_local(port_local_p), m_request_type(request_type_p)
		{}
		virtual ~pacs_params_t() {}

		const pacs_params_t &pacs_params() const { return *this; }

		void operator=(const pacs_params_t &val)
		{
			m_address_pacs = val.m_address_pacs;
			m_port_pacs = val.m_port_pacs;
			m_AETitle_pacs = val.m_AETitle_pacs;
			m_AETitle_local = val.m_AETitle_local;
		}

		wstring address_pacs() const { return m_address_pacs; }
		size_t port_pacs() const { return m_port_pacs; }
		wstring AETitle_pacs() const { return m_AETitle_pacs; }
		wstring AETitle_local() const { return m_AETitle_local; }
		virtual wstring print() const
		{
			return L"address: " + address_pacs() + L", port: " + to_wstring(port_pacs()) + L", AETitle: " + AETitle_pacs();
		}

		const e_request_t& request_type() const { return m_request_type; };
		void set_request_type(e_request_t val) { m_request_type = val; };

	private:
		wstring m_address_pacs;
		size_t m_port_pacs;
		wstring m_AETitle_pacs;
		wstring m_AETitle_local;
		size_t m_port_local;
		e_request_t m_request_type;
	};


	class datasource_t
	{
	public:
		datasource_t() {}
		virtual ~datasource_t() {}

		enum e_datasource { folder, file, pacs };
		virtual e_datasource type() const = 0;
		virtual datasource_t* clone() const = 0;
		virtual wstring print() const = 0;
	};

	class datasource_folder : public datasource_t
	{
	public:
		enum class mode_t
		{
			/*!
				\brief Использовать и обновлять ранее построенные индексы каталогов

				Данные сверяются с индексом по датам изменения файлов и по их размеру. При наличии изменений
				данные обновляются, изменения в индексе сохраняются.

				Ошибки работы с файлами индекса игнорируются (делаются записи в лог).
				Ошибки получения информации по измененным и новым файлам не игнорируются (исключение).
			*/
			read_and_update_index,

			/*!
				\brief Использовать ранее построенные индексы каталогов, обновлять данные, но не сохранять

				Данные сверяются с индексом по датам изменения файлов и по их размеру. При наличии изменений
				данные обновляются, но изменения в индексе не сохраняются (режим read-only).

				Ошибки работы с файлами индекса игнорируются (делаются записи в лог).
				Ошибки получения информации по измененным и новым файлам не игнорируются (исключение).
			*/
			read_only_index,

			/*!
				\brief Использовать ранее построенные индексы каталогов без обновления
				
				Используются данные из индекса. Производится проверка изменений файлов по размеру и датам.
				При обнаружении изменений выдается ошибка (исключение).
			*/
			read_index_as_is,

			//! \brief Не использовать индексирование каталогов
			no_index,

			default_mode = read_and_update_index
		};
	public:
		datasource_folder() = delete;
		datasource_folder(const wstring &folder_path, bool analyze_subfolders_in);
		datasource_folder(const wstring &folder_path, bool analyze_subfolders_in, mode_t mode);
		virtual ~datasource_folder() override {}

		virtual e_datasource type() const override { return folder; }

		virtual datasource_t* clone() const override;
		virtual wstring print() const override {return m_path;}

		wstring path() const { return m_path; }
		bool analyze_subfolders() const { return m_analyze_subfolders; }
		mode_t mode() const { return m_mode; }

	private:
		wstring m_path;
		bool m_analyze_subfolders;
		mode_t m_mode = mode_t::default_mode;
	};

	class datasource_pacs : public datasource_t, public pacs_params_t
	{
	public:
		datasource_pacs() = delete;
		datasource_pacs(const wstring &address_pacs_p, 
						size_t port_pacs_p, 
						const wstring &AETitle_pacs_p, 
						const wstring &AETitle_local_p, //note Kovbas: use XRAD_SCU by default
						size_t port_local_p,			// note Kovbas: use 104 by default
						e_request_t request_type_p);

		virtual ~datasource_pacs() {}

		virtual e_datasource type() const override { return pacs; }
		virtual datasource_pacs* clone() const override { return new datasource_pacs(*this); }
		virtual wstring print() const override { return pacs_params_t::print(); }
	};

	class datasource_file : public datasource_t, public xrad::filesystem::file_info
	{
	public:
		datasource_file() = delete;

		virtual e_datasource type() const override { return file; }

		virtual datasource_t* clone() const override
		{
			return new datasource_file(*this);
		}
		virtual wstring print() const override
		{
			return full_file_path();
		}
	};


	// instance storages ----------------------------------------------------------------------------
	class instancestorage_t
	{
	public:
		instancestorage_t(){}
		virtual ~instancestorage_t(){}

		enum e_instancestorage { file, pacs };
		virtual e_instancestorage type() const = 0;
		virtual instancestorage_t* clone() const = 0;
		virtual wstring print() const = 0;

		virtual bool operator==(const instancestorage_t &inst_src) = 0;
		virtual bool	pre_indexed() const { return false; }
	};


	class instancestorage_file : public instancestorage_t, public xrad::filesystem::file_info
	{
	public:
		instancestorage_file() = delete;
		instancestorage_file(const wstring &file_full_path_in)
			: file_info(file_full_path_in)
		{}

		virtual e_instancestorage type() const override { return file; }
		virtual instancestorage_t* clone() const override
		{
			return new instancestorage_file(*this);
		}

		virtual wstring print() const override
		{
			return full_file_path();
		}

		virtual bool operator==(const instancestorage_file &inst_src)
		{
			return (full_file_path() == inst_src.full_file_path());
		}

		virtual bool operator==(const instancestorage_t &inst_src) override
		{
			//const instancestorage_file &other = dynamic_cast<const instancestorage_file&>(inst_src);
			return operator==(dynamic_cast<const instancestorage_file&>(inst_src));
		}
	};


	class instancestorage_file_pre_indexed : public instancestorage_file, public DicomFileIndex
	{
		// сюда добавить информацию из индекса
		PARENT(instancestorage_file);
	public:
		instancestorage_file_pre_indexed() = delete;
		instancestorage_file_pre_indexed(const wstring &file_full_path_in) : parent(file_full_path_in)
		{}
		instancestorage_file_pre_indexed(const wstring &file_full_path_in, const DicomFileIndex& dcmFileIndex) : parent(file_full_path_in), DicomFileIndex(dcmFileIndex)
		{}
		virtual instancestorage_t* clone() const override
		{
			return new instancestorage_file_pre_indexed(*this);
		}
		virtual bool	pre_indexed() const { return is_dicom(); }
		//virtual bool	is_ct_slice() const { return get_ImageType_ct_slice(); }
	};

	//note (Kovbas) Используется для объединения параметров PACS и информации об инстансе. Используется как родитель для instancestorage_pacs
	class pacs_params_instance_t : public pacs_params_t
	{
	public:
		pacs_params_instance_t() = delete;
		pacs_params_instance_t(const wstring &address_pacs_p, size_t port_pacs_p, const wstring &AETitle_pacs_p, const wstring &AETitle_local_p, size_t port_local_p, const elemsmap_t &elems_p)
			:pacs_params_t(address_pacs_p, port_pacs_p, AETitle_pacs_p, AETitle_local_p, port_local_p), m_elems(elems_p)
		{}
		pacs_params_instance_t(const pacs_params_t &datasrc_pacs_in, const elemsmap_t &elems)
			:pacs_params_t(datasrc_pacs_in), m_elems(elems)
		{}

		const elemsmap_t& elems() const { return m_elems; }

	private:
		elemsmap_t m_elems;
	};

	class instancestorage_pacs: public instancestorage_t, public pacs_params_instance_t
	{
	public:
		instancestorage_pacs() = delete;
		instancestorage_pacs(const wstring &address_pacs_p, size_t port_pacs_p, const wstring &AETitle_pacs_p, const wstring &AETitle_local_p, size_t port_local_p, const elemsmap_t &elems_p)
			:pacs_params_instance_t(address_pacs_p, port_pacs_p, AETitle_pacs_p, AETitle_local_p, port_local_p, elems_p)
		{}
		instancestorage_pacs(const pacs_params_t &datasrc_pacs_in, const elemsmap_t &elems)
			:pacs_params_instance_t(datasrc_pacs_in, elems)
		{}

		virtual e_instancestorage type() const override { return pacs; }
		virtual instancestorage_t* clone() const override
		{
			return new instancestorage_pacs(*this);
		}

		virtual wstring print() const override
		{
			//return (L"Server:" + address_pacs() + L", port:" + to_wstring(port_pacs()) + L", AETitle:" + AETitle()); //todo (Kovbas) возможно, нужно дописать данные об инстансе (т.е. информацию из члена m_elems, которая однозначно идентифицирует инстанс на сервере)
			return (L"Server: " + pacs_params_t::print());
		}

		virtual bool operator==(const instancestorage_pacs &inst_pacs_in)
		{
			return (address_pacs() == inst_pacs_in.address_pacs() &&
				port_pacs() == inst_pacs_in.port_pacs() &&
				AETitle_pacs() == inst_pacs_in.AETitle_pacs() &&
				request_type() == inst_pacs_in.request_type() &&
				elems() == inst_pacs_in.elems());
		}

		virtual bool operator==(const instancestorage_t &inst_src) override
		{
			return operator==(dynamic_cast<const instancestorage_pacs&>(inst_src));
		}
	};

	using instancestorage_ptr = cloning_ptr<instancestorage_t>;

} //namespace Dicom

XRAD_END

#endif // datasource_h__
