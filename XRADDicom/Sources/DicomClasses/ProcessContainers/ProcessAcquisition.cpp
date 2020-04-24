/*!
	\file
	\date 4/19/2018 2:27:56 PM
	\author kovbas
*/
#include "pre.h"
#include "ProcessAcquisition.h"
#include <XRADDicom/Sources/DicomClasses/Instances/CreateInstance.h>

XRAD_BEGIN

// !!!!! эта функция приводит к выходу за рамки диапазона вектора [num_of_frame_p + 1] !!!!!
// похоже на то, что это тестирование работы мультифреймов
wstring ProcessAcquisition::get_dicom_file_content(size_t num_of_frame_p, bool byDCMTK) const
{
	return (*m_acquisition_loader)[num_of_frame_p + 1]->get_instance_content(byDCMTK);
}


//todo (Kovbas) функция нуждается в глубокой переработке, т.к. не учитывает то, что теперь нету никакого кэша в Dicom::instance, а значит и данные нужно сюда каким-то образом принести, чтобы они были сохранены в файлы
void ProcessAcquisition::save_to_file (const wstring &folder_path, e_saving_decision_options_t saving_decision_in, Dicom::e_compression_type_t compression, ProgressProxy pp) const
{
	if (saving_decision_in == e_saving_decision_options_exit) return;

	//проверяем и создаём папку для сохранения
	if (!DirectoryExists(folder_path))
	{
		CreatePath(folder_path);
	}
	//пока мы не умеем сохранять данные в мультифреймовые файлы, поэтому проверяем мультифреймовость исходника. Если он такой, то сохраняем серию в новые файлы
	//note (Kovbas) процедуру можно усложнить, сделав захват всех имеющихся по фрейму данных из исходного файла. Тогда сохранение в "старый" файл будет более правильным, т.к. будет содержать всю информацию по фрейму из старого файла
	// note (АБЕ) Проверку на мультифреймовость нужно делать не у generic_instance,
	// а непосредственно у того элемента instancesources(), в который сохраняем. Или проверить,
	// что хотя бы один из них мультифреймовый. Кроме того, сейчас у generic_instance
	// n_frames всегда равно 0.
	e_saving_decision_options_t saving_decision = saving_decision_in;
	if (m_acquisition_loader->front()->frames_number() != 0)
		saving_decision = e_save_to_new_file;

	ProgressBar	progress(pp);
	progress.start("Saving " + classname(), n_elements());
	for (size_t i = 0; i < n_elements(); i++)
	{
		//создаём объект, который поместит в себя данные и сохранит их в файл
  		//Dicom::instance_ptr instance(generic_instance);
		//Dicom::instance_ptr instance{ m_acquisition_loader->front() };
		Dicom::instance_ptr instance{ (*m_acquisition_loader)[i] };

		wstring	filename;
		if(saving_decision == e_save_to_new_file)
		{
			filename = folder_path + L"/IM" + ssprintf(L"%05d", int(i));
//			instance->clear();
			//+instance->create_new_dicom_container(new Dicom::instancestorage_file(filename));
			instance->dicom_container()->set_int(Dicom::e_instance_number, int(i));
		}
		else
		{
			//открываем старый файл, чтобы работать с ним
			instance->open_instancestorage();
			// открываем файл, чтобы в данные из него поместить наши данные
		}

		//наполняем его данными, соответствующими фрейму
		put_elements_to_instance(*instance, i);

		//instance->set_new_values_to_instance(Dicom::Container::e_delete_wrong_elements);


		if(instance->instance_storage()->type() == Dicom::instancestorage_t::file)
		{
			//filename = folder_path + L"/" + dynamic_cast<Dicom::instancestorage_file&>(*instance->instance_storage()).file_name();
			instance->save_to_file(filename, compression);
		}
		else
		{
			//filename = folder_path + L"/IM" + instance->get_field(Dicom::e_instance_number); //todo (Kovbas) ПЕРЕДЕЛАТЬ!!!!!!!! Нужно, чтобы генерировалось нормально имя для конечного файла: первые три символа - хэш от уникального номера серии и сборки в нём (один на всю сборку), второй - номер изображения в сборке (5 символов на номер)
		}

		//instance->clear();
		//instance->close_instancestorage(true); //todo (Kovbas) либо закрывает тот, кто открыл, либо использовать контейнер, который сам закрое при выходе из функции

		++progress;
	}
}
/*
void set_file_objects_for_multiframe(Dicom::acquisition_loader& source_tomogram)
{
	if (source_tomogram.begin()->get()->n_frames()) //(Kovbas) этот if фактически открывает мультифрейм dicom-файл для всех объектов instance из выбранной acquisition, чтобы далее не открывать/закрывать файл при каждом обращении за данными, т.к. это очень тяжело получается
	{
#if 0 //
		wstring tmp_path = source_tomogram.begin()->get()->full_file_path();
		//source_tomogram.begin()->get()->open_file(tmp_path);
		source_tomogram.begin()->get()->open_instancestorage(Dicom::instancestorage_file(tmp_path));
		auto curr_file_obj = source_tomogram.begin()->get();
		for (auto el : source_tomogram)
		{
			if (!tmp_path.compare(el.get()->full_file_path()))
				el.get()->set_dicom_file(curr_file_obj->dicom_file());
			else
			{
				tmp_path = el.get()->full_file_path();
				//el.get()->open_file(tmp_path);
				el.get()->open_instancestorage(Dicom::instancestorage_file(tmp_path));
				curr_file_obj = el.get();
			}
		}
#endif
		;
	}
}*/

XRAD_END