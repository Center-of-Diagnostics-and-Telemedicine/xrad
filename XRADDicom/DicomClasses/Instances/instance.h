/*!
	\file

	\date 2018/03/8 15:00
	\author nicholas

	\brief
*/
#ifndef instance_h__
#define instance_h__


#include "Modality.h"
#include "source.h"

XRAD_BEGIN

namespace Dicom
{

	//!	Идентификатор сборки (acquisition)
	//! До марта 2019 г сборка идентифицировалась только по номеру. Были обнаружены исследования,
	//! в которых одна сборка содержит изображения разных размеров и разных ориентаций.
	//! При попытке загружать их возникают различные ошибки.
	struct acquisition_id_t
	{
		size_t number;
		point2_ST image_sizes;
		vector<double> orientation;

		acquisition_id_t(size_t in_number, point2_ST in_image_sizes, vector<double> in_orientation) : number(in_number), image_sizes(in_image_sizes), orientation(in_orientation)
		{
			for (auto &c : orientation) c = round_n(c, 1);//Косинусы округляются до десятых
		}

		bool operator == (const acquisition_id_t &id) const { return number == id.number && image_sizes == id.image_sizes && orientation == id.orientation; }
		bool operator != (const acquisition_id_t &id) const { return !operator==(id); }
	};

#if 0
	struct complete_study_id_t : public pair<wstring, wstring>
	{
		PARENT(pair<wstring, wstring>);

		complete_study_id_t(const wstring &in_study_id, const wstring &in_accession_number) { first = in_study_id; second = in_accession_number; }
		complete_study_id_t(const parent &p) : parent(p) {}
		complete_study_id_t() {}

		wstring	&study_id() { return first; }
		wstring	&accession_number() { return second; }

		const wstring	&study_id() const { return first; }
		const wstring	&accession_number() const { return second; }
	};
#else
	struct complete_study_id_t : public tuple<wstring, wstring, wstring>
	{
		PARENT(tuple<wstring, wstring, wstring>);
		using self = complete_study_id_t;

		complete_study_id_t(const wstring &study_instance_uid_p, const wstring &study_id_p, const wstring &accession_number_p)
			: tuple {study_instance_uid_p, study_id_p, accession_number_p}
		{}
		complete_study_id_t(const parent &p) : parent(p) {}
		complete_study_id_t() {}

		auto &study_instance_uid() { return get<0>(*this); }
		auto &study_id() { return get<1>(*this); }
		auto &accession_number() { return get<2>(*this); }

		const auto &study_instance_uid() const { return get<0>(*this); }
		const auto &study_id() const { return get<1>(*this); }
		const auto &accession_number() const { return get<2>(*this); }

		//! \brief Оператор ==, учитывающий возможность разных способов сравнения идентификаторов
		//!	\details Возможны три механизма, позволяющих различать исследования между собой:
		//!	1. Требовать полного совпадения всех идентификаторов. Наиболее типичный случай.
		//!	2. Часто требуется отбор исследований по паре study_id/accession_number. Для таких пар удобно составлять и редактировать
		//!	списки вручную, в этих списках study_instance_uid не включается из-за его неудобочитаемости.
		//!	3. Существуют исследования, в которых поля study_id и accession_number оставлены пустыми. Различить их можно только по
		//!	полю study_instance_uid.
		//!	Для случая 2 предлагается при загрузке списка пар study_id/accession_number заполнять поле study_instance_uid
		//!	звездочкой, чтобы показать, что это поле следует игнорировать при сравнении.

		bool operator == (const self &other) const
		{
			if(study_instance_uid() != L"*" && other.study_instance_uid() != L"*" && study_instance_uid() != other.study_instance_uid()) return false;
			if(study_id() != L"*" && other.study_id() != L"*" && study_id() != other.study_id()) return false;
			if(accession_number() != L"*" && other.accession_number() != L"*" && accession_number() != other.accession_number()) return false;

			return true;
		}
		bool operator != (const self &other) const{ return !operator==(other); }

	};
#endif

	struct complete_series_id_t : public pair<wstring, size_t>
	{
		PARENT(pair<wstring, size_t>);

		complete_series_id_t(const wstring &series_instance_uid_p, size_t series_number_p)
			: pair {series_instance_uid_p, series_number_p}
		{}

		complete_series_id_t(const parent &p) : parent(p) {}
		complete_series_id_t() {}

		wstring	&series_instance_uid() { return first; }
		size_t	&series_number() { return second; }
		const wstring	&series_instance_uid() const { return first; }
		const size_t	&series_number() const { return second; }
	};


	class instance : public source
	{
		PARENT(source);

	public:
		void set_modality(const wstring &val) { dicom_container()->set_wstring(e_modality, val, m_frame_no); }
		wstring	modality() const {
			if (is_preindexed())
				return get_field_from_IDs_set(e_modality);
			return dicom_container()->get_wstring(e_modality);
		}

		// constructors
		instance() {}

		//gets
		wstring accession_number() const
		{
			if (is_preindexed())
				return get_field_from_IDs_set(e_accession_number);
			return dicom_container()->get_wstring(e_accession_number);
		}
		wstring study_id() const
		{
			if (is_preindexed())
				return get_field_from_IDs_set(e_study_id);
			return dicom_container()->get_wstring(e_study_id);
		}
		wstring study_instance_uid() const
		{
			if (is_preindexed())
				return get_field_from_IDs_set(e_study_instance_uid);
			return dicom_container()->get_wstring(e_study_instance_uid);
		}
		auto study_uid() const { return study_instance_uid(); }

		//gets
		wstring patient_id() const
		{
			if (is_preindexed())
				return get_field_from_IDs_set(e_patient_id);
			return dicom_container()->get_wstring(e_patient_id);
		}
		wstring patient_name() const
		{
			if (is_preindexed())
				return get_field_from_IDs_set(e_patient_name);
			return dicom_container()->get_wstring(e_patient_name);
		}
		wstring patient_birthdate() const { return dicom_container()->get_wstring(e_patient_birthdate); }


		complete_study_id_t complete_study_id() const
		{
			return complete_study_id_t(study_instance_uid(), study_id(), accession_number());
		};

		complete_series_id_t complete_series_id() const
		{
			return complete_series_id_t(series_instance_uid(), series_number());
		}

		acquisition_id_t acquisition_id() const
		{
			if (is_preindexed())
				return acquisition_id_t(
					acquisition_number(),
					point2_ST((size_t)0, (size_t)0),
					{ 0. }); // TODO
			return acquisition_id_t(
				acquisition_number(),
				point2_ST(dicom_container()->get_uint(e_rows, 0), dicom_container()->get_uint(e_columns, 0)),
				dicom_container()->get_double_values(e_image_orientation_patient));
		}

		wstring series_instance_uid() const
		{
			if (is_preindexed())
				return get_field_from_IDs_set(e_series_instance_uid);
			return dicom_container()->get_wstring(e_series_instance_uid);
		}
		size_t series_number() const
		{
			if (is_preindexed())
				return get_int(get_field_from_IDs_set(e_series_number), 0);
			return dicom_container()->get_uint(e_series_number);
		}

		wstring series_description() const
		{
			if (is_preindexed())
				return L"";  // get_field_from_IDs_set(e_series_description); // TODO
			return dicom_container()->get_wstring(e_series_description);
		}
		wstring stack_id() const
		{
			if (is_preindexed())
				return get_field_from_IDs_set(e_stack_id);
			return dicom_container()->get_wstring(e_stack_id);
		}

		int get_int(wstring wstr, int default_value) const
		{
			//const wstring wstr = get_wstring(id, num_of_frame, L"0");
			wchar_t *end;
			errno = 0;
			int num = wcstol(wstr.c_str(), &end, 10);
			if ((wstr.c_str() == end) || (errno != 0))
				return default_value;
			else
				return num;
		}

		size_t acquisition_number() const
		{
			if (is_preindexed())
				return get_int(get_field_from_IDs_set(e_acquisition_number), 0);
			return dicom_container()->get_uint(e_acquisition_number);
		}
		size_t instance_number() const { return dicom_container()->get_uint(e_instance_number); }
		wstring sop_instance_uid() const { return dicom_container()->get_wstring(e_sop_instance_uid); }
		auto instance_uid() const { return sop_instance_uid(); }
		size_t in_stack_position_number() const { return dicom_container()->get_uint(e_in_stack_position_number); }
		size_t frames_number() const { return dicom_container()->frames_number(); }
		size_t is_multiframe() const { return dicom_container()->is_multiframe(); }
		size_t frame_no() const { return m_frame_no; }
		wstring manufacturer() const { return dicom_container()->get_wstring(e_manufacturer); }
		wstring study_description() const
		{
			if (is_preindexed())
				return L""; // get_field_from_IDs_set(e_study_description); // TODO
			return dicom_container()->get_wstring(e_study_description);
		}


		//wstring get_series_description() const { return file_cache[e_series_description].value; }
		wstring get_series_description() const { return dicom_container()->get_wstring(e_series_description); }
		wstring protocol_name() const { return dicom_container()->get_wstring(e_protocol_name); }

		double content_time() const { return dicom_container()->get_double(e_content_time); }

		//sets
		void set_instance_number(const size_t val) { dicom_container()->set_uint(e_instance_number, val); }
		void set_in_stack_position_number(const size_t val) { dicom_container()->set_uint(e_in_stack_position_number, val); }
		//sets
		void set_patient_name(const wstring &val) { dicom_container()->set_wstring(e_patient_name, val, m_frame_no); }
		void set_patient_birthdate(const wstring &val) { dicom_container()->set_wstring(e_patient_birthdate, val, m_frame_no); }
		void set_patient_age(const wstring &val) { dicom_container()->set_wstring(e_patient_age, val, m_frame_no); }
		//methods

		wstring get_summary_info_string() const;

		//bool same_instance(const instance &inst) const;
		bool operator==(const instance &other_instance_p) const
		{
			//return same_instance(inst);
			// Часто сравниваются instance, у которых отличаются только наиболее специфичные для
		// instance данные. Поэтому начинаем сравнение именно с них, заканчиваем общим patient_id.
		return
			( (m_IDs_set == other_instance_p.m_IDs_set) &&
			(frame_no() == other_instance_p.frame_no()) );
		}
		bool operator<(const instance &other_instance_p) const
		{
			auto cmp = [](const wstring &s1, const wstring &s2) {if (s1.size() != s2.size()) return s1.size() <= s2.size(); return s1 <= s2; };
			auto fnd = [](const map<tag_e, wstring> &IDs_set, tag_e id_p) { auto search = IDs_set.find(id_p); if (search != IDs_set.end()) return search->second; else return wstring{L""};};
			auto cond_le = [this, &other_instance_p, &cmp, &fnd](tag_e id) {return cmp(fnd(m_IDs_set, id), fnd(other_instance_p.m_IDs_set, id));};

			//note (Kovbas) оставлена полная проверка, чтобы в случае сортировки внутри одной кучи для всех инстансов, сортировка проходила корректно
			// и сортировала инстансы во всей куче, а не только внутри заранее выделенных acquisition
			if (!cond_le(e_patient_id)) return false;
			if (!cond_le(e_study_instance_uid)) return false;
			if (!cond_le(e_series_instance_uid)) return false;
			if (!cond_le(e_stack_id)) return false;
			if (!cond_le(e_acquisition_number)) return false;
			if (atol(convert_to_string(fnd(m_IDs_set, e_instance_number)).c_str()) < atol(convert_to_string(fnd(other_instance_p.m_IDs_set, e_instance_number)).c_str()))
				return true;
			else
				if ((m_IDs_set == other_instance_p.m_IDs_set) &&
					(frame_no() < other_instance_p.frame_no()) )
					return true;
			return false;
		}

		virtual	instance *clone() const
		{
			return new instance(*this);
		}// ранее существовавший метод copy не учитывал полиморфизма, во избежание путаницы переименовываю его в синоним

	protected:
		virtual void fill_tagslist_to_check_data(set<tag_e> &tagslist_to_check_data_p) override
		{
			parent::fill_tagslist_to_check_data(tagslist_to_check_data_p);

			//tagslist_to_check_data_p.insert(e_accession_number);
//			tagslist_to_check_data_p.insert(e_study_id);

			tagslist_to_check_data_p.insert(e_series_instance_uid);
//			tagslist_to_check_data_p.insert(e_series_number);
			//tagslist_to_check_data_p.insert(e_stack_id);
			//tagslist_to_check_data_p.insert(e_acquisition_number);
//			tagslist_to_check_data_p.insert(e_instance_number);
			//tagslist_to_check_data_p.insert(e_in_stack_position_number);

			//tagslist_to_check_data_p.insert(e_study_date);
			//tagslist_to_check_data_p.insert(e_study_time);
			//tagslist_to_check_data_p.insert(e_series_time);
			//tagslist_to_check_data_p.insert(e_instance_creation_time); //не является необходимым для обработки
			//tagslist_to_check_data_p.insert(e_content_time);//этот параметр может пригодиться при разборе смешанных серий
			// дополнительные даты для анонимизации
			//tagslist_to_check_data_p.insert(e_series_date);
			//tagslist_to_check_data_p.insert(e_acquisition_date);
			//tagslist_to_check_data_p.insert(e_content_date);
			//tagslist_to_check_data_p.insert(e_scheduled_procedure_step_start_date);
			//tagslist_to_check_data_p.insert(e_scheduled_procedure_step_end_date);
			//tagslist_to_check_data_p.insert(e_performed_procedure_step_start_date);

			//tagslist_to_check_data_p.insert(e_protocol_name);
			//tagslist_to_check_data_p.insert(e_study_description);
			//tagslist_to_check_data_p.insert(e_series_description);
			tagslist_to_check_data_p.insert(e_modality);
			//tagslist_to_check_data_p.insert(e_dimension_index_values);

			//tagslist_to_check_data_p.insert(e_manufacturer);
		}

		virtual void fill_IDsTagsLst(set<tag_e> &IDsTagLst_p) override
		{
			parent::fill_IDsTagsLst(IDsTagLst_p);

			//patient IDs
			IDsTagLst_p.insert(e_patient_id);

			//study IDs
			IDsTagLst_p.insert(e_study_id);
			IDsTagLst_p.insert(e_accession_number);
			IDsTagLst_p.insert(e_study_instance_uid);
			//series IDs
			IDsTagLst_p.insert(e_series_instance_uid);
			IDsTagLst_p.insert(e_series_number);
			IDsTagLst_p.insert(e_modality);
			//instance IDs
			IDsTagLst_p.insert(e_instance_number);
			IDsTagLst_p.insert(e_sop_instance_uid);
		}


	private:
//		void anonymization_instance(const anonymizer_settings::level &anonymize, const bool set_only_if_exist = true);

	};

// 	typedef std::shared_ptr<instance> instance_ptr;
	typedef cloning_ptr<instance> instance_ptr;

	/*!
		\brief Тип используется для создания кучи единичных дайком-компонент для дальнейшего разбора.

		\note Лучше больше ни для чего не использовать! =) Используйте acquisition для получения набора, с которым будет производиться последующая работа: вывод, изменение, сохранение и т.д.
	*/
	struct instances : public std::list<instance_ptr>{};


	struct instance_open_close_class
	{
		instance_open_close_class() = delete;
		instance_open_close_class(instance &instance_p)
			: m_instance(instance_p)
		{
			m_instance.open_instancestorage();
		}
		instance_open_close_class(const instance_open_close_class &) = delete;
		instance_open_close_class &operator=(const instance_open_close_class &) = delete;
		// TODO: Копирование объекта невозможно, но move-семантику можно реализовать.

		~instance_open_close_class()
		{
				m_instance.close_instancestorage();
		}
	private:
		instance &m_instance;
	};


}//namespace Dicom

XRAD_END

#endif // instance_h__
