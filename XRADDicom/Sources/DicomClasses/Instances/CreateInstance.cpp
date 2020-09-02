/*!
	\file
	\date 2018/03/02 15:32
	\author kulberg
*/
#include "pre.h"
#include "CreateInstance.h"

//tomogram
#include "tomogram_slice.h"
#include "ct_slice.h"
#include "mr_slice.h"
#include "mr_slice_siemens.h"

//image
#include "image.h"
#include "xray_image.h"
//#include "dx_image.h"

//instance

XRAD_BEGIN

namespace Dicom
{

	template <class T>
	instance_ptr	TryCreateInstance_template(const instance_ptr &instance_in, instance_ptr TryCreateAnother(const instance_ptr&))
	{
		instance_ptr result = unique_ptr<instance>(make_unique<T>());
		result->copy_container(*instance_in);
		if (result->get_m_frame_no() != 0)
		{
			return result;
		}
		else
		{
			if (result->exist_and_correct_essential_data())
				return result;
			else
				if (TryCreateAnother == nullptr)
					return nullptr;
				else
					return TryCreateAnother(instance_in);
		}
	}

	instance_ptr	TryCreateGenericInstance(const instance_ptr &instance_in) { return TryCreateInstance_template<instance>(instance_in, nullptr); }
	instance_ptr	TryCreateImage(const instance_ptr &instance_in) { return TryCreateInstance_template<image>(instance_in, TryCreateGenericInstance); }

	//tomogram
	instance_ptr	TryCreateGenericTomogram(const instance_ptr &instance_in) { return TryCreateInstance_template<tomogram_slice>(instance_in, TryCreateImage); }
	instance_ptr	TryCreateCT(const instance_ptr &instance_in) { return TryCreateInstance_template<ct_slice>(instance_in, TryCreateGenericTomogram); }
	instance_ptr	TryCreateMR(const instance_ptr &instance_in) { return TryCreateInstance_template<mr_slice>(instance_in, TryCreateGenericTomogram); }
	instance_ptr	TryCreateMRSiemens(const instance_ptr &instance_in) { return TryCreateInstance_template<mr_slice_siemens>(instance_in, TryCreateMR); }

	//xray
	instance_ptr	TryCreateGenericXRAY(const instance_ptr &instance_in) { return TryCreateInstance_template<xray_image>(instance_in, TryCreateImage); }
	//instance_ptr	TryCreateDX(const instance_ptr &instance_in) { return TryCreateInstance_template<dx_image>(instance_in, TryCreateGenericXRAY); }


	instance_ptr	CreateInstance(const instancestorage_ptr &instance_src_p, const dicom_instance_predicate &instance_predicate_p)
	{
		instance_ptr	prime(new instance);// = make_shared<instance>();// собирает модальность и данные для однозначной идентификации instance //note (Kovbas) иногда требуется название производителя. Возможно, лучше доработать source (добавить в него модальность, производителя), чтобы обходиться им на этом этапе
#if 1
		// TODO: Сделать механизм "владения признаком открытия файла", чтобы файл всегда закрывался
		// в той функции, которая его открыла, либо это владение передавалось другой функции, которая
		// его закроет. Это сделает ненужной функцию закрытия с флагом force и упростит логику
		// слежения за закрытием файла вовремя. [АБЕ]
		if (!prime->try_open_instancestorage(instance_src_p))
			return nullptr;
#else //todo Kovbas это попытка для сетевых инстансев собирать информацию не общим путём, а через C-FIND (это экономнее C-MOVE). Не катит, т.к. слишком мало данных так можно собрать. Возможно, прокатит, т.к. не так много данных нужно для того, чтобы нормально распарсить все инстансы
		switch (inst_src.type())
		{
		case instancestorage_t::file:
			prime->open_instancestorage(inst_src);
			break;

		case instancestorage_t::pacs:
			get_instance();
			break;

		default:
			throw runtime_error(~TODO~);
		}
#endif
		//проверяем на соответствие требованиям
		if (!instance_predicate_p(prime)) return nullptr;

		//создаём экземпляр соответствующего типа
		wstring mod = prime->modality();

		if (prime->dicom_container()->exist_element(e_number_of_frames))   prime->get_m_frame_no() = prime->dicom_container()->get_uint(e_number_of_frames, 0);
		else prime->get_m_frame_no() = 0;

		//tomograms
		//CT
		if (is_modality_ct(mod))
			return TryCreateCT(prime);
		//MR
		else if (is_modality_mr(mod))
		{
			if ((prime->manufacturer() == L"Siemens") ||
				(prime->manufacturer() == L"siemens") ||
				(prime->manufacturer() == L"SIEMENS"))
				return TryCreateMRSiemens(prime);
			else
				return TryCreateMR(prime);
		}

		else if (is_modality_tomogram(mod))
			return TryCreateGenericTomogram(prime);

		//xrays
		/*else if (is_modality_dx(mod))
		return TryCreateDX(prime);*/

		else if (is_modality_xray(mod))
			return TryCreateGenericXRAY(prime);

		//image
		else if (is_modality_image(mod))
			return TryCreateImage(prime);

		//generic instance
		return TryCreateGenericInstance(prime);
	}


	// функция для заполнения инстанса из индексированных структур
	instance_ptr	CreateInstancePreIndexed(const instancestorage_ptr &instance_src_p, const dicom_instance_predicate &instance_predicate_p)
	{
		instance_ptr result; // = cloning_ptr<ct_slice>();
		try
		{
			auto &pre_indexed_source = dynamic_cast<const instancestorage_file_pre_indexed &>(*instance_src_p);
			if (pre_indexed_source.pre_indexed()) // заполнять поля
			{
				DicomFileIndex &dcm_index = dynamic_cast<DicomFileIndex &>(*instance_src_p);

				if (pre_indexed_source.get_ImageType_mr_slice_siemens())
				{
					result = cloning_ptr<mr_slice_siemens>(new mr_slice_siemens());
				}
				else if (pre_indexed_source.get_ImageType_mr_slice())
				{
					result = cloning_ptr<mr_slice>(new mr_slice());
				}
				else if (pre_indexed_source.get_ImageType_ct_slice())
				{
					result = cloning_ptr<ct_slice>(new ct_slice());
				}
				else if (pre_indexed_source.get_ImageType_tomogram_slice())
				{
					result = cloning_ptr<tomogram_slice>(new tomogram_slice());
				}
				else if (pre_indexed_source.get_ImageType_xray_image())
				{
					result = cloning_ptr<xray_image>(new xray_image());
				}
				else if (pre_indexed_source.get_ImageType_image())
				{
					result = cloning_ptr<image>(new image());
				}
				else
				{
					throw runtime_error("Unknown image type."); // TODO
				}
				result->InitFromPreindexed(dcm_index);
				if (!instance_predicate_p(result)) // для проверки работы фильтра над инстансом, созданным индексированием
					return nullptr;
				result->dicom_container()->create_empty_instancestorage(instance_src_p);

			//	Dicom::instance_open_close_class openclose(*result);
				result->try_open_instancestorage(instance_src_p);
				if (result->dicom_container()->exist_element(e_number_of_frames))   result->get_m_frame_no() = result->dicom_container()->get_uint(e_number_of_frames, 0);
				else result->get_m_frame_no() = 0;
				
			//	result->set_num_of_frame(result->dicom_container()->frames_number());
			//	result->dicom_container()->frames_number();
			}
			else
			{
				result = CreateInstance(instance_src_p, instance_predicate_p);
			}

			return result;
		}
		//		catch (bad_cast &)
		catch (...)
		{
			result = CreateInstance(instance_src_p, instance_predicate_p);
			return result;
		}
	}

}//namespace Dicom

XRAD_END
