/*!
	\file

	\date 2018/04/11 17:21
	\author nicholas

	\brief
*/
#ifndef Hierarchy_h__
#define Hierarchy_h__


#include "Instances/instance.h" //for complete_study_id_t

XRAD_BEGIN

namespace Dicom
{
	//TODO эти функции используются в членах классов ниже, что неправильно.
	// должны превратиться во внешние утилиты и храниться в отдельном исходнике,
	// а не в заголовке
	wstring numbers_itm(size_t itm, bool hide_this_lev = false);
	wstring numbers_acq(size_t acq, size_t itm, bool hide_this_lev = false);
	wstring numbers_sta(size_t sta, size_t acq, size_t itm, bool hide_this_lev = false);
	wstring numbers_ser(size_t ser, size_t sta, size_t acq, size_t itm, bool hide_this_lev = false);
	wstring numbers_stu(size_t stu, size_t ser, size_t sta, size_t acq, size_t itm, bool hide_this_lev = false);



	template<class INSTANCE>
	class acquisition : public std::vector<INSTANCE> // использование list неудобно для параллельной обработки, а выигрыш от разницы контейнеров минимален
	{
	public:
		typedef	INSTANCE instance_t;
		typedef acquisition<INSTANCE> self;

		//constructors
		acquisition(const acquisition_id_t &in_acquisition_id, const wstring &in_description) : m_description(in_description), m_acquisition_id(in_acquisition_id) {}


		self(const self &s2) = default;
		self& operator=(const self &s2) = default;
		self(self &&s2) = default;
		self& operator=(self &&s2) = default;


		//gets
		wstring class_name() const { return wstring(L"acquisition"); }

		const acquisition_id_t &acquisition_id() const { return m_acquisition_id; }
		wstring id_string() const
		{
			auto result = ssprintf(L"no = %d", int(acquisition_id().number));
			if (acquisition_id().image_sizes.y() || acquisition_id().image_sizes.x())
			{
				result += ssprintf(L", image sizes = (%d,%d)", acquisition_id().image_sizes.y(), acquisition_id().image_sizes.x());
			}
			if (acquisition_id().orientation.size())
			{
				result += L", orientation = ";
				for (auto &c : acquisition_id().orientation) result += ssprintf(L"%g ", c);
			}
			return result;
		}

		size_t	elements_number() const { return size(); }
		size_t	n_instances() const { return elements_number(); }

		wstring items_number_string() const
		{
			//return (to_wstring(size()) + L" itm");
			return numbers_itm(size());
		};
		wstring description() const
		{
			wstring result;
			if (m_description.size() != 0)
				result = m_description;
			else
				result = id_string();

			return result += L" -- " + items_number_string();
		};

		//operators
		bool operator==(const acquisition& sn) const { return acquisition_id() == sn.acquisition_id(); };

	private:
		acquisition_id_t	m_acquisition_id;
		// 	size_t m_acquisition_number;
		// 	wstring m_orientation;
		// 	wstring m_position;

		wstring m_description;
	};


	template<class ACQUISITION>
	struct stack : public std::list<ACQUISITION>
	{
	public:
		typedef	ACQUISITION acquisition_t;
		typedef	typename acquisition_t::instance_t instance_t;
		typedef stack<ACQUISITION> self;

		//constructors
		stack(const wstring &in_stack_id, const wstring &in_description) : m_stack_id(in_stack_id), m_description(in_description) {}

		self(const self &s2) = default;
		self& operator=(const self &s2) = default;
		self(self &&s2) = default;
		self& operator=(self &&s2) = default;

		//gets
		wstring class_name() const { return wstring(L"stack"); };

		const wstring &stack_id() const { return m_stack_id; };
		wstring id_string() const { return stack_id(); };

		size_t	n_instances() const
		{
			size_t	result(0);
			for (auto &item : *this) result += item.n_instances();
			return result;
		}

		size_t	n_acquisitions() const
		{
			return size();
		}

		wstring items_number_string() const
		{
			return numbers_acq(n_acquisitions(), n_instances());
		};
		wstring description() const
		{
			wstring result;
			if (m_description.size() != 0)
				result = m_description;
			else
				result = id_string();

			return result += L" -- " + items_number_string();
		};

		//operators
		bool operator==(const stack& sn) const { return stack_id() == sn.stack_id(); };

	private:
		wstring m_stack_id; //stack_id;
		wstring m_description;
	};


	template<class STACK>
	struct series : public std::list<STACK>
	{
	public:
		typedef	STACK stack_t;
		typedef	typename stack_t::acquisition_t acquisition_t;
		typedef	typename stack_t::instance_t instance_t;
		typedef series<STACK> self;

		//constructors
		series(const complete_series_id_t &complete_series_id_p, const wstring &description_p)
			: m_complete_series_id(complete_series_id_p)
			, m_description(description_p)
		{}

		self(const self &s2) = default;
		self& operator=(const self &s2) = default;
		self(self &&s2) = default;
		self& operator=(self &&s2) = default;


		//gets
		wstring class_name() const { return wstring(L"series"); };

		const complete_series_id_t &complete_series_id() const { return m_complete_series_id; };
		//		complete_series_id_t &complete_series_id() { return m_complete_series_id; };

			//const wstring	&series_id() const { return complete_series_id().series_id(); }
			//const auto	&series_uid() const { return m_complete_series_id.series_instance_uid(); }
		//int	series_number() const { return m_complete_series_id.series_number(); }

		//wstring id_string() const { return series_id() + L", " + ssprintf(L"%l", series_number()); };
		wstring id_string() const { return ssprintf(L"%d", m_complete_series_id.series_number()) + L", UUID:" + m_complete_series_id.series_instance_uid(); };

		size_t	n_instances() const
		{
			size_t	result(0);
			for (auto &item : *this) result += item.n_instances();
			return result;
		}
		size_t	n_acquisitions() const
		{
			size_t	result(0);
			for (auto &item : *this) result += item.n_acquisitions();
			return result;
		}

		size_t	n_stacks() const
		{
			return size();
		}

		wstring items_number_string() const
		{
			return numbers_sta(n_stacks(), n_acquisitions(), n_instances());
		}

		wstring raw_description() const
		{
			return m_description;
		}

		wstring description() const
		{
			if (m_description.size() != 0)
			{
				return ssprintf(L"%s [SE#%s] -- %s",
					EnsureType<const wchar_t*>(m_description.c_str()),
					EnsureType<const wchar_t*>(id_string().c_str()),
					EnsureType<const wchar_t*>(items_number_string().c_str()));
			}
			else
			{
				return ssprintf(L"[SE#%s] -- %s",
					EnsureType<const wchar_t*>(id_string().c_str()),
					EnsureType<const wchar_t*>(items_number_string().c_str()));
			}
		};

		//operators
		bool operator==(const series& sn) const { return complete_series_id() == sn.complete_series_id(); };

	private:
		complete_series_id_t m_complete_series_id; //series_instance_uid and series_number
		wstring m_description;
	};


	template<class SERIES>
	struct study : public std::vector<SERIES>// Почему-то std::sort(study.begin(), study.end(), pred) дает ошибку при использовании list. Для быстрого решения list->vector, позже разобраться
	{
	public:
		typedef	SERIES series_t;
		typedef	typename series_t::stack_t stack_t;
		typedef	typename series_t::acquisition_t acquisition_t;
		typedef	typename series_t::instance_t instance_t;
		typedef study<SERIES> self;

		//constructors
		study(const complete_study_id_t &complete_study_id_p, const wstring &description_p)
			: m_complete_study_id{ complete_study_id_p }
			, m_description{ description_p }
		{}

		self(const self &s2) = default;
		self& operator=(const self &s2) = default;
		self(self &&s2) = default;
		self& operator=(self &&s2) = default;

		//gets
		wstring class_name() const { return wstring(L"study"); };

		const complete_study_id_t &complete_study_id() const { return m_complete_study_id; };
		wstring id_string() const { return m_complete_study_id.accession_number() + L", " + m_complete_study_id.study_id() + L", " + m_complete_study_id.study_instance_uid(); };
		void	set_complete_study_id(const complete_study_id_t &id) { m_complete_study_id = id; }

		const auto &study_uid() const { return m_complete_study_id; }

		size_t	n_instances() const
		{
			size_t	result(0);
			for (auto &item : *this) result += item.n_instances();
			return result;
		}

		size_t	n_acquisitions() const
		{
			size_t	result(0);
			for (auto &item : *this) result += item.n_acquisitions();
			return result;
		}

		size_t	n_stacks() const
		{
			size_t	result(0);
			for (auto &item : *this) result += item.size();
			return result;
		}

		size_t	n_series() const
		{
			return size();
		}

		wstring items_number_string() const
		{
			return numbers_ser(n_series(), n_stacks(), n_acquisitions(), n_instances());
		};

		wstring description() const
		{
			wstring result;
			if (m_description.size() != 0)
				result = m_description;
			else
				result = id_string();

			return result += L" -- " + items_number_string();
		};

		//operators
		bool operator==(const study& sn) const { return complete_study_id() == sn.complete_study_id(); };

	private:
		complete_study_id_t m_complete_study_id; // study_instance_uid, study_id and accession_number
		wstring m_description;
	};


	template<class STUDY>
	struct patient : public std::list<STUDY>
	{
	public:
		typedef	STUDY study_t;
		typedef	typename study_t::series_t series_t;
		typedef	typename study_t::stack_t stack_t;
		typedef	typename study_t::acquisition_t acquisition_t;
		typedef	typename study_t::instance_t instance_t;
		typedef patient<STUDY> self;

		//constructors
		patient(const wstring &in_patient_id, const wstring &in_description) : m_patient_id(in_patient_id), m_description(in_description) {};

		self(const self &s2) = default;
		self& operator=(const self &s2) = default;
		self(self &&s2) = default;
		self& operator=(self &&s2) = default;

		//gets
		wstring class_name() const { return wstring(L"patient"); };

		const wstring &patient_id() const { return m_patient_id; };
		void set_patient_id(const wstring &id) { m_patient_id = id; }
		wstring id_string() const { return patient_id(); };

		size_t	n_instances() const
		{
			size_t	result(0);
			for (auto &item : *this) result += item.n_instances();
			return result;
		}

		size_t	n_acquisitions() const
		{
			size_t	result(0);
			for (auto &item : *this) result += item.n_acquisitions();
			return result;
		}

		size_t	n_stacks() const
		{
			size_t	result(0);
			for (auto &item : *this) result += item.n_stacks();
			return result;
		}

		size_t	n_series() const
		{
			size_t	result(0);
			for (auto &item : *this) result += item.n_series();
			return result;
		}

		size_t	n_studies() const
		{
			return size();
		}


		wstring items_number_string() const
		{
			return numbers_stu(n_studies(), n_series(), n_stacks(), n_acquisitions(), n_instances());
		};

		wstring description() const
		{
			wstring result;
			if (m_description.size() != 0)
				result = m_description;
			else
				result = id_string();

			return result += L" -- " + items_number_string();
		};

		//operators
		bool operator==(const patient& sn) const { return patient_id() == sn.patient_id(); };

	private:
		wstring m_patient_id;
		wstring m_description;
	};


	template<class PATIENT>
	struct patients : public std::list<PATIENT>
	{
		typedef	PATIENT patient_t;
		typedef	typename patient_t::study_t study_t;
		typedef	typename patient_t::series_t series_t;
		typedef	typename patient_t::stack_t stack_t;
		typedef	typename patient_t::acquisition_t acquisition_t;
		typedef	typename patient_t::instance_t instance_t;

		size_t	n_instances() const
		{
			size_t	result(0);
			for (auto &item : *this) result += item.n_instances();
			return result;
		}

		size_t	n_acquisitions() const
		{
			size_t	result(0);
			for (auto &item : *this) result += item.n_acquisitions();
			return result;
		}

		size_t	n_stacks() const
		{
			size_t	result(0);
			for (auto &item : *this) result += item.n_stacks();
			return result;
		}

		size_t	n_series() const
		{
			size_t	result(0);
			for (auto &item : *this) result += item.n_series();
			return result;
		}

		size_t	n_studies() const
		{
			size_t	result(0);
			for (auto &item : *this) result += item.n_studies();
			return result;
		}

		size_t	n_patients() const
		{
			return size();
		}
	};

} //namespace Dicom

XRAD_END

#endif // Hierarchy_h__
