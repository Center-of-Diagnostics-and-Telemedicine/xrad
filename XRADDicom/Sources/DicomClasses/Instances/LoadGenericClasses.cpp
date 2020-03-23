/*!
	* \file LoadGenericClasses.cpp
	* \date 12/18/2017 2:46:43 PM
	*
	* \author kovbas
	*
	* \brief
	*
	* TODO: long description
	*
	* \note
*/
#include "pre.h"
#include "LoadGenericClasses.h"


#include <XRADBasic/Sources/Utils/ParallelProcessor.h>


XRAD_BEGIN


namespace Dicom
{
	/*!
		\note
		Также для решения вопроса выбора оси сортировки можно использовать следующие данные:
		- (0018,5100)	PatientPosition, значения описаны здесь:
			dicom/standard3/part03.html#sect_C.7.3.1.1.2
		- (0020, 0020)	CS	2	4	L\P	PatientOrientation
		- (0020, 0032)	DS	3	28 - 182.2261\ - 199.8043\2077.000	ImagePositionPatient
		- (0020, 0037)	DS	6	48	1.00000\0.00000\0.00000\0.00000\1.00000\0.00000	ImageOrientationPatient

		Подробная информация по этому поводу есть здесь:
		https://dicomiseasy.blogspot.ru/2013/06/getting-oriented-using-image-plane.html
	*/

		/*!
		\brief Используется для сортировки фреймов по осям. Для сортировки использует ось, номер которой получает в конструкторе.
		0 - x; 1 - y; 2 - z
		По умолчанию используется ось z.

		Используется только для томограмм.
	*/
	/*?
	struct compare_frames_by_coord
	{
		const size_t	axis_sort;

		compare_frames_by_coord(size_t in_axis_sort = 2) : axis_sort(in_axis_sort){}

		bool operator() (const Dicom::instance_ptr &first, const Dicom::instance_ptr &second)
		{
			try
			{
				return first->dicom_container()->get_double_values(Dicom::e_image_position_patient)[axis_sort] >
						second->dicom_container()->get_double_values(Dicom::e_image_position_patient)[axis_sort];
			}
			catch(...)
			{
				return false;
			}
		}
	};*/



	wstring	acquisition_loader::detect_root_folder_name() const
	{
		wstring	root_folder_name;

		if(size())
		{
			switch (front()->instance_storage()->type())
			{
			case Dicom::instancestorage_t::file:
				root_folder_name = (dynamic_cast<const Dicom::instancestorage_file &>(*front()->instance_storage())).folder_path();
				break;
			case Dicom::instancestorage_t::pacs:
				root_folder_name = L".";
				break;
			}
	//		root_folder_name.erase(root_folder_name.end() - front()->filename.size(), root_folder_name.end());//удаляется имя файла

		//ищем общее начало путей всех файлов серии
			for(auto &dcm : *this)
			{
				if (dcm->instance_storage()->type() == Dicom::instancestorage_t::file)
					root_folder_name = xrad::filesystem::detect_common_root_folder(root_folder_name, dynamic_cast<const Dicom::instancestorage_file &>(*dcm->instance_storage()).folder_path());
			}
		}

		return root_folder_name;
	}


	void acquisition_loader::filter(const dicom_instance_predicate &pred)
	{
		for (auto el = begin(); el != end();)
		{
			if(!pred(*el))
			{
				el = erase(el);
			}
			else
			{
				++el;
			}
		}
	}


	void acquisition_loader::open_instancestorages()
	{
		auto it = begin();
		try
		{
			for (auto last = end(); it != last; ++it)
				(*it)->open_instancestorage();
		}
		catch (...)
		{
			for (auto first = begin(); it != first; --it)
				(*it)->close_instancestorage();
			throw;
		}
	}


	void acquisition_loader::close_instancestorages()
	{
		for (auto el : *this)
			el->close_instancestorage();
	}

	void acquisition_loader::prepare()
	{
 		auto less_lbd = [](const instance_ptr &a1, const instance_ptr &a2) { return *a1 < *a2; };
		sort( begin(), end(), less_lbd );
		auto equal_lbd = [](const instance_ptr &a1, const instance_ptr &a2) { return *a1 == *a2; };
		erase( unique( begin(), end(), equal_lbd ), end() );
	}

	//stack---------------------------------------------------------------------------------
	wstring	stack_loader::detect_root_folder_name() const
	{
		wstring	root_folder_name;

		if(size())
		{
			root_folder_name = front().detect_root_folder_name();
	//		root_folder_name.erase(root_folder_name.end() - front()->filename.size(), root_folder_name.end());//удаляется имя файла

		//ищем общее начало путей всех файлов серии
			for(auto &acq : *this)
			{
				root_folder_name = xrad::filesystem::detect_common_root_folder(root_folder_name, acq.detect_root_folder_name());
			}
		}

		return root_folder_name;
	}


	//series---------------------------------------------------------------------------------
	wstring	series_loader::detect_root_folder_name() const
	{
		wstring	root_folder_name;

		if(size())
		{
			root_folder_name = front().detect_root_folder_name();
	//		root_folder_name.erase(root_folder_name.end() - front()->filename.size(), root_folder_name.end());//удаляется имя файла

		//ищем общее начало путей всех файлов серии
			for(auto &st : *this)
			{
				root_folder_name = xrad::filesystem::detect_common_root_folder(root_folder_name, st.detect_root_folder_name());
			}
		}

		return root_folder_name;
	}

	//study---------------------------------------------------------------------------------
	wstring	study_loader::detect_root_folder_name() const
	{
		wstring	root_folder_name;

		if(size())
		{
			root_folder_name = front().detect_root_folder_name();
	//		root_folder_name.erase(root_folder_name.end() - front()->filename.size(), root_folder_name.end());//удаляется имя файла

		//ищем общее начало путей всех файлов серии
			for(auto &sr : *this)
			{
				root_folder_name = xrad::filesystem::detect_common_root_folder(root_folder_name, sr.detect_root_folder_name());
			}
		}

		return root_folder_name;
	}

	//patient---------------------------------------------------------------------------------
	wstring	patient_loader::detect_root_folder_name() const
	{
		wstring	root_folder_name;

		if(size())
		{
			root_folder_name = front().detect_root_folder_name();
			for(auto &sr : *this)
			{
				root_folder_name = xrad::filesystem::detect_common_root_folder(root_folder_name, sr.detect_root_folder_name());
			}
		}

		return root_folder_name;
	}


	//patients---------------------------------------------------------------------------------
	wstring	patients_loader::detect_root_folder_name() const
	{
		wstring	root_folder_name;

		if(size())
		{
			root_folder_name = front().detect_root_folder_name();
			for(auto &sr : *this)
			{
				root_folder_name = xrad::filesystem::detect_common_root_folder(root_folder_name, sr.detect_root_folder_name());
			}
		}

		return root_folder_name;
	}

}//namespace Dicom

XRAD_END