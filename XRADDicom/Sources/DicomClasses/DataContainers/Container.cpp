/*!
	 * \file DicomFile.cpp
	 * \date 2017/10/02 17:30
	 *
	 * \author kulberg
	 *
	 * \brief Возможно, этот файл и не нужен вовсе, надо подумать. кнс
	 *
	 * TODO: long description
	 *
	 * \note
*/
#include "pre.h"
#include "Container.h"


XRAD_BEGIN

namespace Dicom
{
	void Container::process_dataelement_error(tag_e id, error_process_mode epm)
	{
		switch (epm)
		{
		case e_throw_on_error:
			throw;

		case e_ignore_error:
			break;

		case e_delete_wrong_elements:
			delete_dataelement(id, true, true);
		}
	}
	/*
	size_t Container::n_frames() const
	{
		// функция, которая работает мимо кэша, чтобы этот параметр из multiframe контейнера по ошибке не сохранился в создаваемых нами однокадровых dicom-файлах
		wstring buf = get_wstring(e_number_of_frames, 0);
		return buf.size() ? wcstol(buf.c_str(), nullptr, 10) : 0;
	}*/

}

XRAD_END
