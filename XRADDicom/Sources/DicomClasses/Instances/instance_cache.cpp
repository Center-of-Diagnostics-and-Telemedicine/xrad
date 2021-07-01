/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 2018/09/21 15:00
	\author kulberg
*/
#include "pre.h"
#include "instance_cache.h"

#include <XRADDicom/Sources/Utils/Utils.h>

XRAD_BEGIN

namespace Dicom
{


	//instance_cache--------------------------------------------------------------------------------------

	void instance_cache::set_element(const tag_e id, const wstring &value, dataelement_modify_condition c)
	{
		m_cache[id] = dataelement_t(value, c);
	}

	void instance_cache::append(const instance_cache &original)
	{
		for(const auto &o: original.m_cache)
		{
			m_cache[o.first] = o.second;
		}
	}


	void instance_cache::set_wstring(tag_e id, const wstring &new_value, bool set_only_if_exists)
	{
		if(set_only_if_exists)
		{
			auto it = m_cache.find(id);
			if(it != m_cache.end())
			{
				set_element(id, new_value, e_modify_dataelement);
			}
		}
		else
		{
			set_element(id, new_value, e_modify_dataelement);
		}
	}

	void instance_cache::set_wstring_values(tag_e id, const vector<wstring> &new_values, const wchar_t delim)
	{
		wstring buffer(L"");
		for(auto val : new_values)
		{
			buffer += val + delim;
		}

		set_wstring(id, buffer.substr(0, buffer.length() - 1));
	}


	void instance_cache::set_double(tag_e id, const double new_value)
	{
		set_wstring(id, check_float_value(new_value));
	}

	void instance_cache::set_double_values(tag_e id, const vector<double> &new_values, const wchar_t delim)
	{
		wstring buffer(L"");
		for(auto val : new_values)
			buffer += check_float_value(val) + wstring(1, delim);

		set_wstring(id, buffer.substr(0, buffer.length() - 1));
	}

	void instance_cache::set_int(tag_e id, const int new_value)
	{
		set_wstring(id, to_wstring(new_value));
	}

	void instance_cache::set_uint(tag_e id, const size_t new_value)
	{
		set_wstring(id, to_wstring(new_value));
	}

	void instance_cache::set_int_values(tag_e id, const vector<int> &new_values, const wchar_t delim)
	{
		wstring buffer(L"");
		for(auto val : new_values)
		{
			buffer += to_wstring(val) + delim;
		}

		set_wstring(id, buffer.substr(0, buffer.length() - 1));
	}

	wstring instance_cache::get_wstring(tag_e id, const wstring &default_value) const
	{
		// чтобы не создавал поле, которого нет, нужно использовать поиск
		auto it = m_cache.find(id);
		if(it != m_cache.end())
		{
			return it->second.value();
		}
		else
		{
			return default_value;
		}
	}


	vector<wstring> instance_cache::get_wstring_values(tag_e id, wchar_t delim) const
	{
		vector<wstring> v;

		wstring vector_buffer = get_wstring(id, L"");

		if(vector_buffer.length() == 0)
		{
			return v;
		}
		else
		{
			wstring buffer(L"");
			for(size_t i = 0; i <= vector_buffer.length(); i++)
			{
				if((i == vector_buffer.length()) || (vector_buffer[i] == delim))
				{
					v.push_back(buffer);
					buffer = L"";
				}
				else
					buffer += vector_buffer[i];
			}
			return v;
		}
	}


	double instance_cache::get_double(tag_e id, const double default_value) const
	{
		const wstring wstr = get_wstring(id);
		wchar_t *end;
		errno = 0;
		double num = wcstod(wstr.c_str(), &end);
		if((wstr.c_str() == end) || (errno != 0))
			return default_value;
		else
			return num;
	}

	vector<double> instance_cache::get_double_values(tag_e id, const wchar_t delim) const
	{
		vector<double> v;

		wstring vector_buffer = get_wstring(id, L"");

		if(vector_buffer.length() == 0)
		{
			return v;
		}
		else
		{
			wstring number_buffer(L"");
			for(size_t i = 0; i <= vector_buffer.length(); i++)
			{
				if((i == vector_buffer.length()) || (vector_buffer[i] == delim))
				{
					const wstring &wstr = number_buffer;
					wchar_t *end;
					errno = 0;
					double num = wcstod(wstr.c_str(), &end);
					if((wstr.c_str() == end) || (errno != 0))
						v.push_back(0);
					else
						v.push_back(num);

					number_buffer = L"";
				}
				else
					number_buffer += vector_buffer[i];
			}
			return v;
		}
	}

	size_t instance_cache::get_uint(tag_e id, const size_t default_value) const
	{
		const wstring wstr = get_wstring(id);
		wchar_t *end;
		errno = 0;
		size_t num = wcstoul(wstr.c_str(), &end, 10);
		if((wstr.c_str() == end) || (errno != 0))
			return default_value;
		else
			return num;
	}

	bool instance_cache::get_bool(tag_e id) const
	{
		return get_int(id) != 0;
	}

	int instance_cache::get_int(tag_e id, const int default_value) const
	{
		const wstring wstr = get_wstring(id);
		wchar_t *end;
		errno = 0;
		int num = wcstol(wstr.c_str(), &end, 10);
		if((wstr.c_str() == end) || (errno != 0))
			return default_value;
		else
			return num;
	}

	vector<int> instance_cache::get_int_values(tag_e id, const wchar_t delim) const
	{
		vector<int> v;

		wstring buffer = get_wstring(id, L"");

		if(buffer.length() == 0)
		{
			return v;
		}
		else
		{
			wstring tmpSs(L"");
			for(size_t i = 0; i <= buffer.length(); i++)
			{
				if((i == buffer.length()) || (buffer[i] == delim))
				{
					const wstring &wstr = tmpSs;
					wchar_t *end;
					errno = 0;
					int num = wcstol(wstr.c_str(), &end, 10);
					if((wstr.c_str() == end) || (errno != 0))
						v.push_back(0);
					else
						v.push_back(num);

					tmpSs = L"";
				}
				else
					tmpSs += buffer[i];
			}
			return v;
		}
	}



	void instance_cache::dump_to_dicom_file_image(Container &file, Container::error_process_mode epm)
	{
		for(auto el : m_cache)
		{
			try
			{
				//file.set_wstring(tag_e(el.first), el.second.value(), el.second.delete_condition());
				file.set_wstring(tag_e(el.first), el.second.value()); //todo (Kovbas) обращение сразу к контейнеру файла. Это плохо. Нужно через Dicom::instance
			}
			catch(...)
			{
				file.process_dataelement_error(tag_e(el.first), epm);
			}
		}
	// 		file.from_wstring(tag_e(0x00090010 /*Specific Character Set (0008,0005)*/), L"Kovbas was here! =)"); // это пасхальное яйцо =)
	}


} //end of namespace Dicom

XRAD_END
