/*!
	* \file dataelement.h
	* \date 5/16/2018 4:28:48 PM
	*
	* \author Kovbas (kovbas)
	*
	* \brief
	*
	* TODO: long description
	*
	* \note
*/
#ifndef dataelement_h__
#define dataelement_h__

#include <map>
#include <list>

XRAD_BEGIN

namespace Dicom
{

	using tag_t = size_t;

	//! Режимы записи/удаления полей при записи в dataelement в Dicom::Container по заданному тэгу
	enum dataelement_modify_condition
	{
		e_preserve_dataelement,	// команда Dicom::Container::set_wstring игнорируется, dataelement остается без изменений
		e_modify_dataelement, 	// команда Dicom::Container::set_wstring выполняется, dataelement при необходимости создается и принимает заданное значение
		e_delete_dataelement	// команда Dicom::Container::set_wstring удаляет dataelement из контейнера
	};


	struct dataelement_t
	{
	public:

		dataelement_t() : m_modify_condition(e_modify_dataelement) {};
		dataelement_t(const wstring &val, dataelement_modify_condition in_dc/*= e_modify_dataelement*/) : m_value(val), m_modify_condition(in_dc) {}

		wstring value() const { return m_value; }

		void	SetDeleteCondition(const dataelement_modify_condition &in_dc) { m_modify_condition = in_dc; }

		const auto	&modify_condition() const { return m_modify_condition; }

	private:
		dataelement_modify_condition m_modify_condition;
		wstring    m_value;
	};

	typedef pair<tag_t, wstring> elem_t;
	typedef map<tag_t, wstring> elemsmap_t;
	typedef list<elem_t> elemslist_t;

	typedef list<elemsmap_t> filter_t;

} //namespace Dicom

XRAD_END

#endif // dataelement_h__