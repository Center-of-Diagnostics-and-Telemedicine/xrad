/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 2017/10/03 14:40
	\author kulberg
*/
#include "pre.h"
#include "ContainerDCMTK.h"

#include "dcmtk/dcmimage/diregist.h"

#ifdef XRAD_COMPILER_MSC
#if _MSC_VER >= 1900 // MSVC2015+
#ifndef XRAD_COMPILER_CMAKE
#pragma comment(lib, "dcmimage.lib")
#endif
#endif
#endif // XRAD_COMPILER_MSC

#include <XRADDicom/Sources/DCMTKAccess/dcmtkElementsTools.h>
#include <XRADDicom/Sources/DCMTKAccess/dcmtkCodec.h>
#include <XRADDicom/Sources/DCMTKAccess/dcmtkUtils.h>
#include <XRADDicom/Sources/Utils/Utils.h>

#include <XRADDicom/Sources/DCMTKAccess/pacsTools.h>
#include <XRADDicom/Sources/DCMTKAccess/pacsUtils.h>

#include <cwchar>

#include <XRADDicom/Sources/Utils/logger.h>

XRAD_BEGIN

namespace Dicom
{
	Dicom::Logger logger;

	shared_ptr<Container>	MakeContainerDCMTK()
	{
		return make_shared<ContainerDCMTK>();
	}

	shared_ptr<Container>	MakeContainer()
	{
		return MakeContainerDCMTK();
	}

	shared_ptr<Container>	MakeContainerDCMTK(const instancestorage_ptr &storage)
	{
		return make_shared<ContainerDCMTK>(storage);
	}

	shared_ptr<Container>	MakeContainer(const instancestorage_ptr &storage)
	{
		return MakeContainerDCMTK(storage);
	}




	ContainerDCMTK::ContainerDCMTK()
		: m_dicom_file(nullptr)
	{
		m_last_used_instancestorage = nullptr;
	}


	ContainerDCMTK::ContainerDCMTK(const instancestorage_ptr &storage)
		: m_dicom_file(nullptr)
	{
		//m_dicom_file = make_unique<DcmFileFormat>();
		m_last_used_instancestorage = storage;
	}


	/*?
	void ContainerDCMTK::set_instancestorage(const instancestorage_ptr &instance_src_p)
	{
		//dicom_container()->set_instancestorage(instance_src_p);
		m_las = inst_src_in;
	}
	*/

	namespace
	{
	OFFilename convert_to_OFFilename(const wstring &str)
	{
	// Проверяем поддержку конструктора OFFilename от const wchar_t*, используем его под Win32.
	// Код проверки взят из DCMTK: offile.h.
	#if (defined(WIDE_CHAR_FILE_IO_FUNCTIONS) || defined(WIDE_CHAR_MAIN_FUNCTION)) && defined(_WIN32)
		return OFFilename(EnsureType<const wchar_t*>(GetPathMachineReadable(str).c_str()));
	#else
		return OFFilename(convert_to_string(GetPathMachineReadable(str)));
	#endif
	}
	} // namespace

	//только открывает файл. Все остальные работы выполняются в open_instancestorage
	bool ContainerDCMTK::open_file(const instancestorage_file &instancestorage_file_p, bool use_exceptions)
	{
		try
		{
			auto dicom_file = make_unique<DcmFileFormat>();
			//открываем файл
			auto cond = dicom_file->loadFile(convert_to_OFFilename(
					instancestorage_file_p.full_file_path()));
			if (!cond.good())
			{
				if (!use_exceptions && cond != EC_InvalidFilename)
					return false;
				throw runtime_error(ssprintf("Error opening DICOM file \"%s\":\n%s",
						EnsureType<const char*>(convert_to_string(instancestorage_file_p.full_file_path()).c_str()),
						EnsureType<const char*>(cond.text())));
			}
			m_dicom_file = std::move(dicom_file);

			//После открытия файла сразу же проверяем кодировку строк и преобразуем ее в utf-8
			ForceUTF8Charset();

			return true;
		}
		catch (exception &ex)
		{
			logger.putLogMessage("Dicom file problem:\t" + string(ex.what()));
			throw;
		}
	}

	//только забирает с сервера. Все остальные работы выполняются в open_instancestorage
	bool ContainerDCMTK::open_pacs(const  instancestorage_pacs &instancestorage_pacs_p, bool use_exceptions)
	{
#if 1
		DcmSCU_XRAD dcmscu;
		//check accession of PACS
		if (checkPACSAccessibility(instancestorage_pacs_p))
			if (downloadInstance(dcmscu, instancestorage_pacs_p, *m_dicom_file))
			//+if (common_downloader(inst_pacs_src_in, ))
			{
				return true;
			}
#endif
		if (!use_exceptions)
			return false;
		throw runtime_error("ContainerDCMTK::open_pacs failed.");
	}

	bool ContainerDCMTK::try_open_instancestorage(const instancestorage_ptr &instancestorage_p)
	{
		//проверяем может уже открыт такой источник данных
		if (m_dicom_file)
		{
			if (*m_last_used_instancestorage == *instancestorage_p)
			{
				return true;
			}
			else
			{
				//если нет, то закрываем текущий источник данных, чтобы открыть запрашиваемый
				close_instancestorage();
			}
		}

		//всё подчищаем от возможного предыдущего открытия
		m_frames_number = 0;
		m_is_dicomdir = false;

		switch (instancestorage_p->type())
		{
		case instancestorage_t::file:
			if (!open_file(dynamic_cast<const instancestorage_file&>(*instancestorage_p), false))
				return false;
			break;

		case instancestorage_t::pacs:
			if (!open_pacs(dynamic_cast<const instancestorage_pacs&>(*instancestorage_p), false))
				return false;
			break;

		default:
			throw invalid_argument("ContainerDCMTK::open_instancestorage: instancestorage_t object contains incorrect data.");
		}

		m_last_used_instancestorage = instancestorage_p;

		try
		{
#if 0
			// выяснить является ли дайкомдиром или мультифреймом
			m_frames_number = get_uint(e_number_of_frames);
			//если m_frames_number > 0, значит, нужно подготовить всё внутри контейнера для быстрой работы с мультифреймом
			if (is_multiframe())
			{
				//parse_multiframe();
				m_dicom_file->getDataset()->findAndGetSequence(DCM_SharedFunctionalGroupsSequence, m_shared_frames_data_ptr);
				m_dicom_file->getDataset()->findAndGetSequence(DCM_PerFrameFunctionalGroupsSequence, m_per_frame_data_ptr);
			}
			else //в противном случае проверяем дайкомдир это или нет
#endif
				m_is_dicomdir = exist_element(e_file_set_id);

			return true;
		}
		catch (...)
		{
			close_instancestorage();
			m_frames_number = 0;
			m_is_dicomdir = false;
			throw;
		}
	}


	// создаём незаполненный контейнер
	bool ContainerDCMTK::create_empty_instancestorage(const instancestorage_ptr &instancestorage_p)
	{
		m_frames_number = 0;
		m_is_dicomdir = false;

		// m_dicom_file = make_unique<DcmFileFormat>();
		m_last_used_instancestorage = instancestorage_p;
		return true;
	}


	void ContainerDCMTK::open_instancestorage()
	{
		if(!m_last_used_instancestorage)
		{
			throw runtime_error(
				"ContainerDCMTK::open_instancestorage: instance storage is not initialized.");
		}

		//проверяем может уже открыт такой источник данных
		if (m_dicom_file)
		{
			return;
		}
		auto &instancestorage_p = m_last_used_instancestorage;

		//всё подчищаем от возможного предыдущего открытия
		m_frames_number = 0;
		m_is_dicomdir = false;

		switch (instancestorage_p->type())
		{
		case instancestorage_t::file:
			open_file(dynamic_cast<const instancestorage_file&>(*instancestorage_p), true);
			break;

		case instancestorage_t::pacs:
			open_pacs(dynamic_cast<const instancestorage_pacs&>(*instancestorage_p), true);
			break;

		default:
			throw invalid_argument("ContainerDCMTK::open_instancestorage: instancestorage_t object contains incorrect data.");
		}

		m_last_used_instancestorage = instancestorage_p;

		try
		{
#if 0
			// выяснить является ли дайкомдиром или мультифреймом
			m_frames_number = get_uint(e_number_of_frames);
			//если m_frames_number > 0, значит, нужно подготовить всё внутри контейнера для быстрой работы с мультифреймом
			if (is_multiframe())
			{
				//parse_multiframe();
				m_dicom_file->getDataset()->findAndGetSequence(DCM_SharedFunctionalGroupsSequence, m_shared_frames_data_ptr);
				m_dicom_file->getDataset()->findAndGetSequence(DCM_PerFrameFunctionalGroupsSequence, m_per_frame_data_ptr);
			}
			else //в противном случае проверяем дайкомдир это или нет
#endif
				m_is_dicomdir = exist_element(e_file_set_id);
		}
		catch (...)
		{
			close_instancestorage();
			m_frames_number = 0;
			m_is_dicomdir = false;
			throw;
		}
	}

	void ContainerDCMTK::close_instancestorage()
	{
		m_dicom_file.reset();
	}




	void	ContainerDCMTK::SetTransferSyntax(E_TransferSyntax transfer_syntax)
	{
		// для данных jpeg2000 просто поменять transfer syntax не удается, следующая функция дает ошибку
		auto condition = m_dicom_file->getDataset()->chooseRepresentation(transfer_syntax, nullptr);

		if(condition.bad())
		{
			// Опытным путем установлено, что исправтиь это удается путем принудительной перезаписи
			// пиксельных данных. Видимо, это избыточное действие. Но как временное решение пригодно
			RealFunction2D_F32	image;
			size_t bpp;
			bool is_signed;
			size_t ncomp;
			size_t num_of_frame = 0;
			get_pixeldata(image, bpp, is_signed, ncomp, num_of_frame);
			set_pixeldata(image, bpp, is_signed, ncomp);

			auto condition2 = m_dicom_file->getDataset()->chooseRepresentation(transfer_syntax, nullptr);

			if(condition2.bad()) throw runtime_error(classname() + "Could not save the file. Condition " + condition.text());
		}
	}

	bool ContainerDCMTK::save_to_file(const wstring &full_file_path, e_compression_type_t encoding)
	{
		try
		{
			//разбор сжатия/несжатия изображений
			E_TransferSyntax transfer_syntax = EXS_LittleEndianExplicit;
			switch (encoding)
			{
			case e_uncompressed:
				transfer_syntax = EXS_LittleEndianExplicit;
				break;

			case e_jpeg_lossless:
				transfer_syntax = EXS_JPEGProcess14SV1;
				break;

			default:
				throw invalid_argument(classname() + "::save_to_file, invalid compression. You cannot use this codec. File was not saved. File " + convert_to_string(full_file_path));
			}

			// регистрируем кодер (через наш класс кодека, чтобы автоматически отключался)
			//lock_guard<mutex> lck{ dicom_file_mutex };
			Dicom::dcmtkCodec codec(Dicom::dcmtkCodec::e_encode, transfer_syntax, m_dicom_file->getDataset()->getOriginalXfer());

			// задаём тип компрессии в объекте файла
			// нужно обязательно задавать transfer_syntax, иначе не будет сохранять

			SetTransferSyntax(transfer_syntax);

			if (!m_dicom_file->getDataset()->canWriteXfer(transfer_syntax))
			{
				throw runtime_error(classname() + "Could not save the file. File " + convert_to_string(full_file_path));
			}

			//сохраняем файл
			m_last_used_instancestorage = make_unique<instancestorage_file>(instancestorage_file(full_file_path));

			return m_dicom_file->saveFile(convert_to_OFFilename(full_file_path), transfer_syntax).good();
		}
		catch (exception &ex)
		{
			logger.putLogMessage(ex.what());
			return false;
		}
	}

	bool ContainerDCMTK::exist_element(tag_e id) const
	{
		lock_guard<mutex> lck(dicom_file_mutex);
		return exist_element(element_id_to_DcmTag(id));
	}
	bool ContainerDCMTK::exist_element(const DcmTag &dcmTag) const
	{
		if (dcmTag.getGroup() == e_meta_info_group)
			return (m_dicom_file->getMetaInfo()->tagExists(dcmTag));
		else
			return (m_dicom_file->getDataset()->tagExists(dcmTag, OFTrue));
	}

	double ContainerDCMTK::get_slope_mf()
	{
		DcmDataset *Dataset = m_dicom_file->getDataset();

		DcmSequenceOfItems *dcmSequenceOfItems, *dcmSequenceOfItems1;
		double result;
		string str;
		try
		{
			if (Dataset->findAndGetSequence(DCM_SharedFunctionalGroupsSequence, dcmSequenceOfItems, true, true).good())
			{
				DcmItem* item1 = dcmSequenceOfItems->getItem(0);

				if (item1->findAndGetSequence(DCM_PixelValueTransformationSequence, dcmSequenceOfItems1, true, true).good())
				{
					DcmItem* item2 = dcmSequenceOfItems1->getItem(0);

					item2->findAndGetOFStringArray(DCM_RescaleSlope, str, true);
				}
				else throw logic_error("No tag of frame sequence in file");

				result = strtod(str.c_str(), NULL);
			}
			else throw logic_error("No tag of frame sequence in file");
		}
		catch (...)
		{
			throw;
		}

		return result;
	}

	double ContainerDCMTK::get_intercept_mf()
	{
		DcmDataset *Dataset = m_dicom_file->getDataset();

		DcmSequenceOfItems *dcmSequenceOfItems, *dcmSequenceOfItems1;
		double result;
		string str;
		try
		{
			if (Dataset->findAndGetSequence(DCM_SharedFunctionalGroupsSequence, dcmSequenceOfItems, true, true).good())
			{
				DcmItem* item1 = dcmSequenceOfItems->getItem(0);

				if (item1->findAndGetSequence(DCM_PixelValueTransformationSequence, dcmSequenceOfItems1, true, true).good())
				{
					DcmItem* item2 = dcmSequenceOfItems1->getItem(0);

					item2->findAndGetOFStringArray(DCM_RescaleIntercept, str, true);
				}
				else throw logic_error("No tag of frame sequence in file");

				result = strtod(str.c_str(), NULL);
			}
			else throw logic_error("No tag of frame sequence in file");
		}
		catch (...)
		{
			throw;
		}

		return result;
	}

	double ContainerDCMTK::get_thickness_mf()
	{
		DcmDataset *Dataset = m_dicom_file->getDataset();

		DcmSequenceOfItems *dcmSequenceOfItems, *dcmSequenceOfItems1;
		double result;
		string str;
		try
		{
			if (Dataset->findAndGetSequence(DCM_SharedFunctionalGroupsSequence, dcmSequenceOfItems, true, true).good())
			{
				DcmItem* item1 = dcmSequenceOfItems->getItem(0);

				if (item1->findAndGetSequence(DCM_PixelMeasuresSequence, dcmSequenceOfItems1, true, true).good())
				{

					DcmItem* item2 = dcmSequenceOfItems1->getItem(0);

					item2->findAndGetOFStringArray(DCM_SliceThickness, str, true);
				}
				else throw logic_error("No tag of frame sequence in file");

				result = strtod(str.c_str(), NULL);
						}
			else throw logic_error("No tag of frame sequence in file");
		}
		catch (...)
		{
			throw;
		}

		return result;
	}

	vector<double> ContainerDCMTK::get_scales_xy_mf()
	{
		DcmDataset *Dataset = m_dicom_file->getDataset();

		DcmSequenceOfItems *dcmSequenceOfItems, *dcmSequenceOfItems1;
		vector<double> result;
		string str;
		try
		{

			if (Dataset->findAndGetSequence(DCM_SharedFunctionalGroupsSequence, dcmSequenceOfItems, true, true).good())
			{
				DcmItem* item1 = dcmSequenceOfItems->getItem(0);

				if (item1->findAndGetSequence(DCM_PixelMeasuresSequence, dcmSequenceOfItems1, true, true).good())
				{
					DcmItem* item2 = dcmSequenceOfItems1->getItem(0);

					item2->findAndGetOFStringArray(DCM_PixelSpacing, str, true);
				}
				else throw logic_error("No tag of frame sequence in file");

				char* pEnd;
				double d1, d2;
				d1 = strtod(str.c_str(), &pEnd);
				d2 = strtod(pEnd + 1, NULL);

				result.push_back(d1);
				result.push_back(d2);
			}
			else throw logic_error("No tag of frame sequence in file");
		}
		catch (...)
		{
			throw;
		}

		return result;
	}



	vector<double> ContainerDCMTK::get_image_position(size_t frame_no)
	{
		DcmDataset *Dataset = m_dicom_file->getDataset();

		DcmSequenceOfItems *dcmSequenceOfItems;
		if (!Dataset->findAndGetSequence(DCM_PerFrameFunctionalGroupsSequence, dcmSequenceOfItems, true, false).good())
		{
			throw logic_error("No tag of frame sequence in file");
		}
		DcmItem* item1 = dcmSequenceOfItems->getItem(frame_no);
		DcmSequenceOfItems *dcmSequenceOfItems1;
		if (!item1->findAndGetSequence(DCM_PlanePositionSequence, dcmSequenceOfItems1, true, false).good())
		{
			throw logic_error("No tag of plane position in file");
		}
		DcmItem* item2 = dcmSequenceOfItems1->getItem(0);
		string str;
		item2->findAndGetOFStringArray(DCM_ImagePositionPatient, str, true);

		char* pEnd; char* pEnd2;
		double d1 = strtod(str.c_str(), &pEnd);
		if (*pEnd != '\\')
		{
			throw runtime_error("Invalid image position patient tag format");
		}
		double d2 = strtod(pEnd + 1, &pEnd2);
		if (*pEnd2 != '\\')
		{
			throw runtime_error("Invalid image position patient tag format");
		}
		double d3 = strtod(pEnd2 + 1, nullptr);

		return {d1, d2, d3};
	}

	namespace
	{

		inline string fix_string_ending(const string &str)
		{
			try
			{
				// удаляем неуместный \0 в конце некоторых string, пришедших из dicom файла
				return string(str.begin(), str.begin() + strlen(str.c_str()));
			}
			catch (...)
			{
				return "";
			}
		}


		string element_value_to_hidden_ustring(DcmItem &dcmItem_p, const DcmTagKey &dcmTag, bool searchIntoSub = false)
		{
			try
			{
				string result("");

				if(dcmItem_p.tagExists(dcmTag, searchIntoSub))
				{
					dcmItem_p.findAndGetOFStringArray(dcmTag, result, searchIntoSub);
					return fix_string_ending(result);
				}
				return result;
			}
			catch(exception &)
			{
				return "Could not get an element value from DcmItem, DcmTag:" + dcmTag.toString();
			}
		}


	}

	//TODO эти функции перенести в отдельный исходник "Утиль"
	wstring element_value_to_wstring(DcmItem &dcmItem_p, const DcmTagKey &dcmTag_p, bool c_1251, bool searchIntoSub = false)
	{
		if(c_1251)
		{
			return string_to_wstring(element_value_to_hidden_ustring(dcmItem_p, dcmTag_p, searchIntoSub), e_decode_literals);
		}
		else
		{
			return string8_to_wstring(element_value_to_hidden_ustring(dcmItem_p, dcmTag_p, searchIntoSub));
		}
	}

	wstring ContainerDCMTK::get_wstring(tag_e id, size_t num_of_frame_p, const wstring &default_value_p) const
	{
		auto dcmTag(element_id_to_DcmTag(id));
		wstring result;
		if (dcmTag.getGroup() == e_meta_info_group)
			return result = element_value_to_wstring(*m_dicom_file->getMetaInfo(), dcmTag, cp_1251);

		result = element_value_to_wstring(*m_dicom_file->getDataset(), dcmTag, cp_1251);

		if (is_multiframe())
		{
			auto shared_frames_data_ptr = m_shared_frames_data_ptr->getItem(0);
			auto buf = element_value_to_wstring(*shared_frames_data_ptr, dcmTag, cp_1251, true);
			if (!buf.empty()) result = buf;

			uint32_t num_of_frame = uint32_t(num_of_frame_p);
			if (!num_of_frame) num_of_frame = 1;
			auto frame_data_ptr = m_per_frame_data_ptr->getItem(num_of_frame-1);
			buf = element_value_to_wstring(*frame_data_ptr, dcmTag, cp_1251, true);
			if (!buf.empty()) result = buf;
		}

		if (result.empty())
			return default_value_p;
		return result;
	}
	/*
	void ContainerDCMTK::set_wstring(tag_e id, const wstring &new_value, dataelement_delete_condition dc)
	{
		if(dc.check(new_value))
		{
			delete_dataelement(id, true, true);
			return;
		}
		add_dataelement(id);
		putAndInsertString(*m_dicom_file, id, convert_to_string8(new_value));
	}*/

	bool ContainerDCMTK::set_wstring(tag_e id, const wstring &new_value, size_t num_of_frame_p, bool set_only_if_exists)
	{
		if (set_only_if_exists)
		{
			if (!exist_element(id)) return true;
		}

		string	new_value_converted;

		if(cp_1251)
		{
			new_value_converted = convert_to_string(new_value);
		}
		else
		{
			new_value_converted = convert_to_string8(new_value);
		}


		DcmTag dcmTag(element_id_to_DcmTag(id));
		if (dcmTag.getGroup() == e_meta_info_group)
		{
			return (m_dicom_file->getMetaInfo()->putAndInsertOFStringArray(dcmTag, new_value_converted.c_str())).good();
		}
		else
		{
#if 1
			DcmDataset &currItem = *m_dicom_file->getDataset();
			return (currItem.putAndInsertString(dcmTag, new_value_converted.c_str())).good();
#else
			OFCondition result;
			auto &dcmDS = *m_dicom_file->getDataset();
			if (dcmDS.tagExists(dcmTag) || !set_only_if_exists)
				result = dcmDS.putAndInsertOFStringArray(dcmTag, new_value_converted.c_str());

			if (is_multiframe())
			{
				auto &shared_frames_data = *m_shared_frames_data_ptr->getItem(0);
				if (shared_frames_data.tagExists(dcmTag, true))
					result = shared_frames_data.putAndInsertOFStringArray(dcmTag, new_value_converted.c_str());

				auto num_of_frame(num_of_frame_p);
				if (!num_of_frame) num_of_frame = 1;
				auto &frame_data = *m_per_frame_data_ptr->getItem(num_of_frame - 1);
				if (frame_data.tagExists(dcmTag, true))
					result = frame_data.putAndInsertOFStringArray(dcmTag, new_value_converted.c_str());

			}
			return result.good();
#endif
		}
	}


	void ContainerDCMTK::set_wstring_values(tag_e id, const vector<wstring> &new_values, wchar_t delim, size_t num_of_frame, bool set_only_if_exist)
	{

		wstring buffer(L"");
		for(auto val : new_values)
		{
			buffer += val + delim;
		}

		set_wstring(id, buffer.substr(0, buffer.length() - 1), num_of_frame, set_only_if_exist);

	}

	void ContainerDCMTK::set_double(tag_e id, double new_value, size_t num_of_frame, bool set_only_if_exist)
	{
		set_wstring(id, check_float_value(new_value), num_of_frame, set_only_if_exist);
	}

	void ContainerDCMTK::set_double_values(tag_e id, const vector<double> &new_values, wchar_t delim, size_t num_of_frame, bool set_only_if_exist)
	{
		wstring buffer(L"");
		for(auto val : new_values)
			buffer += check_float_value(val) + wstring(1, delim);

		set_wstring(id, buffer.substr(0, buffer.length() - 1), num_of_frame, set_only_if_exist);
	}

	void ContainerDCMTK::set_int(tag_e id, int new_value, size_t num_of_frame, bool set_only_if_exist)
	{
		set_wstring(id, to_wstring(new_value), num_of_frame, set_only_if_exist);
	}

	void ContainerDCMTK::set_int_values(tag_e id, const vector<int> &new_values, wchar_t delim, size_t num_of_frame, bool set_only_if_exist)
	{
		wstring buffer(L"");
		for(auto val : new_values)
		{
			buffer += to_wstring(val) + delim;
		}

		set_wstring(id, buffer.substr(0, buffer.length() - 1), num_of_frame, set_only_if_exist);
	}

	void ContainerDCMTK::set_uint(tag_e id, const size_t new_value, size_t num_of_frame, bool set_only_if_exist)
	{
		set_wstring(id, to_wstring(new_value), num_of_frame, set_only_if_exist);
	}

	void ContainerDCMTK::delete_all_private_tags()
	{
		//https://support.dcmtk.org/redmine/projects/dcmtk/wiki/Howto_RemovePrivateData
		DcmDataset &dset = *m_dicom_file->getDataset();
		DcmStack stack;
		DcmObject *dobj = NULL;
		DcmTagKey tag;
		OFCondition status = dset.nextObject(stack, OFTrue);
		while (status.good())
		{
			dobj = stack.top();
			tag = dobj->getTag();
			if (tag.getGroup() & 1) // private tag ?
			{
				stack.pop();
				delete ((DcmItem *)(stack.top()))->remove(dobj);
			}
			status = dset.nextObject(stack, OFTrue);
		}
	}

	vector<wstring> ContainerDCMTK::get_wstring_values(tag_e id, size_t num_of_frame, wchar_t delim) const
	{
		vector<wstring> v;

		wstring vector_buffer = get_wstring(id, num_of_frame);

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


	double ContainerDCMTK::get_double(tag_e id, size_t num_of_frame, double default_value) const
	{
		const wstring wstr = get_wstring(id, num_of_frame, L"0.0");
		wchar_t *end;
		errno = 0;
		double num = wcstod(wstr.c_str(), &end);
		if((wstr.c_str() == end) || (errno != 0))
			return default_value;
		else
			return num;
	}

	vector<double> ContainerDCMTK::get_double_values(tag_e id, size_t num_of_frame, wchar_t delim) const
	{
		vector<double> v;

		wstring vector_buffer = get_wstring(id, num_of_frame);

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

	size_t ContainerDCMTK::get_uint(tag_e id, size_t num_of_frame, size_t default_value) const
	{
		const wstring wstr = get_wstring(id, num_of_frame, L"0");
		wchar_t *end;
		errno = 0;
		size_t num = wcstoul(wstr.c_str(), &end, 10);
		if((wstr.c_str() == end) || (errno != 0))
			return default_value;
		else
			return num;
	}

	bool ContainerDCMTK::get_bool(tag_e id) const
	{
		return get_int(id) != 0;
	}

	int ContainerDCMTK::get_int(tag_e id, size_t num_of_frame, int default_value) const
	{
		const wstring wstr = get_wstring(id, num_of_frame, L"0");
		wchar_t *end;
		errno = 0;
		int num = wcstol(wstr.c_str(), &end, 10);
		if((wstr.c_str() == end) || (errno != 0))
			return default_value;
		else
			return num;
	}

	vector<int> ContainerDCMTK::get_int_values(tag_e id, size_t num_of_frame, wchar_t delim) const
	{
		vector<int> v;

		wstring buffer = get_wstring(id, num_of_frame);

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


	bool ContainerDCMTK::get_pixeldata(RealFunction2D_F32 &img_in, size_t &bpp, bool &is_signed, size_t &ncomp, size_t num_of_frame) const
	{
		try
		{
			unique_lock<mutex> lck{ dicom_file_mutex };

			if(m_dicom_file->isEmpty()) throw invalid_argument("File is empty.");
			//создаём объект для декодирования

			//Dicom::dcmtkCodec	dcmCodec(dcmtkCodec::e_decode, element_value_to_hidden_ustring(*m_dicom_file, DCM_TransferSyntaxUID));

			//if (tmpstr == "")
			//E_TransferSyntax ts = m_dicom_file->getDataset()->getCurrentXfer();
			//stringstream ss;
			//m_dicom_file->print(ss);
			//printf("--file--\n %s \n ---file--- \n", ss.str().c_str());
			//ShowText(L"", convert_to_wstring(ss.str()));
			unique_ptr<dcmtkCodec> dcmCodec_ptr;
			//string xferstr = element_value_to_hidden_ustring(*m_dicom_file, DCM_TransferSyntaxUID);
			string xferstr = convert_to_string(get_wstring(DcmTag_to_element_id(DCM_TransferSyntaxUID))); //(*m_dicom_file, DCM_TransferSyntaxUID);
			if (xferstr == "")
				dcmCodec_ptr = make_unique<dcmtkCodec>(dcmtkCodec::e_decode, m_dicom_file->getDataset()->getCurrentXfer());
			else
				dcmCodec_ptr = make_unique<dcmtkCodec>(dcmtkCodec::e_decode, xferstr);

			//забираем данные
			DcmDataset *dcmDataset = m_dicom_file->getDataset();
			if(dcmDataset == NULL) throw runtime_error("DcmDataset is NULL");

			//забираем изображение со всеми параметрами
			unique_ptr<char[]> pixeldata;
			//size_t pixeldata_length;
			//bool signedness = true;
			//size_t bpp = 0;
			size_t vs = 0;
			size_t hs = 0;
			//size_t ncomp = 1;
			dcmCodec_ptr->getPixelData(*dcmDataset, pixeldata, vs, hs, bpp, is_signed, ncomp, num_of_frame);

			// Разблокируем dicom_file_mutex, дальше блокировка не требуется.
			lck.unlock();

			if (!pixeldata) throw logic_error("pixeldata is empty!");
			if (!vs || !hs) throw logic_error(ssprintf("Wrong size value  = %d x %d", vs, hs));

			size_t bytes_per_pixel = (bpp + (CHAR_BIT - 1)) / CHAR_BIT;


			if(img_in.empty()) img_in.realloc(vs, hs);

			//инициализируем наш массив, в который будут положены данные
			if (vs != img_in.vsize() || hs != img_in.hsize())
			{
				ForceDebugBreak();
				throw invalid_argument("Image object has incorrect size(s).");
			}
			//img_in.realloc(vs, hs);

			//переносим изображение в нашу переменную
			if (is_signed)
			{
				switch (bytes_per_pixel)
				{
				case(1):
					img_in.CopyData(reinterpret_cast<int8_t*>(pixeldata.get()), 1);
					break;
				case(2):
					img_in.CopyData(reinterpret_cast<int16_t*>(pixeldata.get()), 1);
					break;
				case(4):
					img_in.CopyData(reinterpret_cast<int32_t*>(pixeldata.get()), 1);
					break;
				default:
					throw logic_error(ssprintf("Wrong reinterpret value  = %d", is_signed));
				}
			}
			else
			{
				switch (bytes_per_pixel)
				{
				case(1):
					img_in.CopyData(reinterpret_cast<uint8_t*>(pixeldata.get()), 1);
					break;
				case(2):
					img_in.CopyData(reinterpret_cast<uint16_t*>(pixeldata.get()), 1);
					break;
				case(4):
					img_in.CopyData(reinterpret_cast<uint32_t*>(pixeldata.get()), 1);
					break;
				default:
					throw logic_error(ssprintf("Wrong reinterpret value  = %d", is_signed));
				}
			}

			return true;
		}
		catch(exception &ex)
		{
			logger.putLogMessage(classname() + "::get_pixeldata problem =\t'" + string(ex.what()) + "'\tFile:\t"/* + convert_to_string(m_last_opened_file_path)*/);//m_last_opened_file_path в функции не задействован
			return false;
		}
	}


	bool ContainerDCMTK::get_color_pixeldata(ColorImageF32& img) const
	{

		unique_ptr<unsigned char[]> pixelData;
		unique_ptr<unsigned char[]> redPixels;
		unique_ptr<unsigned char[]> greenPixels;
		unique_ptr<unsigned char[]> bluePixels;

		//	Uint32 bpp = 0;
		bool is_signed = false;
		Uint32 ncomp = 0;
		size_t vs = 0;
		size_t hs = 0;

		string str;

		try
		{
			DcmFileFormat fileformat1;// = *m_dicom_file.get();
			OFCondition status = fileformat1.loadFile("D:/june/1/1.2.40.0.13.1.110623347169722461097686482239455076524/DICOM/1.2.840.113619.2.261.4.2147483647.1596114745.155502.1006.1.dcm");
			DcmDataset* dcmDataset1 = fileformat1.getDataset();

			OFString xferstr;

			if (fileformat1.getMetaInfo()->tagExists(DCM_TransferSyntaxUID, false))
			{
				OFCondition condition = fileformat1.getMetaInfo()->findAndGetOFString(DCM_TransferSyntaxUID, xferstr, false);//DCM_TransferSyntaxUID//DCM_ImplementationClassUID
				cout << " DCM_TransferSyntaxUID found = " << xferstr << endl;
			}
			else	cout << "no DCM_TransferSyntaxUID found" << endl;

			xrad::Dicom::e_compression_type_t codec_type;

			if (xferstr.length())
			{
				codec_type = recognizeCodecType(xferstr);
			}

			else
			{
				codec_type = recognizeCodecType(fileformat1.getDataset()->getCurrentXfer());
			}

			//switch (codec_type)
			//{
			//case e_jpeg: case e_jpeg_lossless:
			//{
			//	std::lock_guard<std::mutex> guard(DCMTKDecoderJPEGInitialize);
			//	if (numOpenedDCMTKDecoderJPEG++ == 0)
			//		DJDecoderRegistration::registerCodecs(); // register JPEG decoder
			//}
			//break;

			//case e_jpeg_ls:
			//{
			//	std::lock_guard<std::mutex> guard(DCMTKDecoderJPEGLSInitialize);
			//	if (numOpenedDCMTKDecoderJPEGLS++ == 0)
			//		DJLSDecoderRegistration::registerCodecs(); // register JPEG-LS decoder
			//}
			//}

			switch (codec_type)
			{
			case e_unknown:	throw runtime_error("I dont know this codec. Cant decompress it.");

			case e_jpeg2k:	throw runtime_error("I dont like this Jpeg2000. Cant decompress it.");//getPixelsJpeg2000(*dcmDataset, pixeldata, vs, hs, bpp, signedness, ncomp, 0);

			default:
			{
				E_TransferSyntax xfer = dcmDataset1->getOriginalXfer();

				OFCondition condition = dcmDataset1->chooseRepresentation(EXS_LittleEndianExplicit, nullptr);

				//	unique_ptr<DicomImage> image1(new DicomImage(dcmDataset1, xfer, CIF_UsePartialAccessToPixelData, 1, 1 /* fcount */));
				unique_ptr<DicomImage> image1 = make_unique<DicomImage>(dcmDataset1, xfer, CIF_UsePartialAccessToPixelData, 1, 1 /* fcount */);//CIF_UsePartialAccessToPixelData | CIF_IgnoreModalityTransformation

				//cerr << "some text 1" << "\n";
				//unique_ptr<DicomImage> image1(new DicomImage(dcmDataset1, xfer, CIF_UsePartialAccessToPixelData, 1, 1 /* fcount */));
				//unique_ptr<DicomImage> image1 = make_unique<DicomImage>(dcmDataset1, EXS_Unknown, CIF_UsePartialAccessToPixelData , 0, 0 /* fcount */);//CIF_UsePartialAccessToPixelData | CIF_IgnoreModalityTransformation
				//cerr << "some text 2" << "\n";
				vs = image1->getHeight();
				hs = image1->getWidth();
				ncomp = image1->isMonochrome() ? 1 : 3;

//				if (image1->getStatus() != EIS_Normal) throw runtime_error("There is no pixel data in dataset!");

				unsigned char* ptr;
				cerr << image1->getOutputDataSize() << "\n";
				pixelData = make_unique<unsigned char[]>(image1->getOutputDataSize());

				redPixels = make_unique<unsigned char[]>(vs * hs);
				greenPixels = make_unique<unsigned char[]>(vs * hs);
				bluePixels = make_unique<unsigned char[]>(vs * hs);

				image1->getOutputData(pixelData.get(), image1->getOutputDataSize(), 0, 0, 0);

				ptr = pixelData.get();

				for (size_t i = 0; i < vs * hs; i++)
				{
					redPixels[i] = *ptr;
					ptr++;
					greenPixels[i] = *ptr;
					ptr++;
					bluePixels[i] = *ptr;
					ptr++;
				}
			}
			}

			//		if (!pixeldata) throw logic_error("pixeldata is empty!");
			if (!vs || !hs) throw logic_error(ssprintf("Wrong size value  = %d x %d", vs, hs));

			//size_t bytes_per_pixel = (bpp + (CHAR_BIT - 1)) / CHAR_BIT;


			if (img.empty()) img.realloc(vs, hs);

			//инициализируем наш массив, в который будут положены данные
			if (vs != img.vsize() || hs != img.hsize())
			{
				ForceDebugBreak();
				throw invalid_argument("Image object has incorrect size(s).");
			}

			is_signed = false;

			img.red().CopyData(redPixels.get());
			img.green().CopyData(greenPixels.get());
			img.blue().CopyData(bluePixels.get());
			return true;
		}
		catch (...)
		{
			cout << "some exception happened" << endl;
			return false;
		}

	}


	wstring ContainerDCMTK::get_elements_to_wstring(bool byDCMTK) const
	{
		lock_guard<mutex> lck{ dicom_file_mutex };
		return elements_to_wstring(*m_dicom_file, byDCMTK);
	}

	elemsmap_t ContainerDCMTK::get_elements_list() const
	{
		return elemsmap_t(); //todo (Kovbas) подлежит доработке
	}

	bool ContainerDCMTK::delete_dataelement(tag_e id, bool all, bool into)
	{
		static_assert(sizeof(tag_e) <= sizeof(uint), "Invalid type bit depth.");
		// findAndDeleteElement берет id в виде uint.
		return findAndDeleteElement(*m_dicom_file, id, all, into);
	}


	void ContainerDCMTK::add_dataelement(tag_e id)
	{
		if(!exist_element(element_id_to_DcmTag(id)))
		{
			//putAndInsertString(*m_dicom_file, id, "");
			set_wstring(id, L"");
		}
		if(!exist_element(element_id_to_DcmTag(id)))
		{
			throw runtime_error(ssprintf("Cannot add dataelement 0x%X to dicomfile", id));
		}
	}





	//! \breif Копирует данные в буфер с меньшей точностью (разрядностью). При этом ограничивает принудительно значения входного массива, чтобы при они не вызвали переполнения
	//! \details Такое может произойти в результате какой-либо сложной фильтрации изображения, которое содержало пиксели, близкие к предельным
	//!	значениям для разрядности. Небольшое усиление границ привращает, например, +1 в -1 (unsigned int) или 32767 в 32800 (signed short).
	//! При экспорте в Dicom в этих местах появляются контрастные кромки.
	template<class RESULT_ARR, class ARGUMENT_ARR>
	RESULT_ARR	CopyProperPixelValues(const ARGUMENT_ARR &data)
	{
		typedef	typename RESULT_ARR::value_type out_sample;
		typedef	typename ARGUMENT_ARR::value_type in_sample;

		out_sample	min_value = numeric_limits<out_sample>::min();
		out_sample	max_value = numeric_limits<out_sample>::max();

		RESULT_ARR	result;
 		result.MakeCopy(data, [&min_value, &max_value](out_sample &y, const in_sample &x)
			{
				y = static_cast<out_sample>(range(x, min_value, max_value));
			});
		return result;
	}

	void ContainerDCMTK::set_pixeldata_mf(const RealFunctionMD_F32 &img_in, size_t bpp, bool is_signed, size_t ncomp)
	{
		(void)ncomp; //note (Kovbas) нужно будет для сохранения цветных изображений
					 //разбираем и отдаём в объект файла данные изображения
					 //todo (Kovbas) попробовать переделать так, чтобы было в одну строку (возможно, достаточно одного реинтерпрета). 20180601 - не знаю как это можно сделать и возможно ли.
		unsigned long pixDataLen = (unsigned long)(img_in.sizes()[0] * img_in.sizes()[1] * img_in.sizes()[2] * bpp / CHAR_BIT); //явное преобразование для передачи в DCMTK
																																// отправляем в объект файла сырое изображение. При сохранении оно будет жато в зависимости от выбора.
		if (is_signed)
		{
			switch (bpp)
			{
			case 8:
			{
				auto buffer = CopyProperPixelValues<RealFunctionMD_I8>(img_in);
				m_dicom_file->getDataset()->putAndInsertUint8Array(DCM_PixelData, reinterpret_cast<Uint8*>(&(buffer.at({ 0,0,0 }))), pixDataLen);
			}
			break;
			case 16:
			{
				auto buffer = CopyProperPixelValues<RealFunctionMD_I16>(img_in);
				m_dicom_file->getDataset()->putAndInsertUint8Array(DCM_PixelData, reinterpret_cast<Uint8*>(&(buffer.at({ 0,0,0 }))), pixDataLen);
			}
			break;
			case 32:
			{
				auto buffer = CopyProperPixelValues<RealFunctionMD_I32>(img_in);
				m_dicom_file->getDataset()->putAndInsertUint8Array(DCM_PixelData, reinterpret_cast<Uint8*>(&(buffer.at({ 0,0,0 }))), pixDataLen);
			}
			break;
			default:
				throw logic_error(ssprintf("Wrong reinterpret value  = %d", is_signed));
			}
		}
		else
		{
			switch (bpp)
			{
			case 8:
			{
				auto buffer = CopyProperPixelValues<RealFunctionMD_UI8>(img_in);
				m_dicom_file->getDataset()->putAndInsertUint8Array(DCM_PixelData, reinterpret_cast<Uint8*>(&(buffer.at({ 0,0,0 }))), pixDataLen);
			}
			break;
			case 16:
			{
				auto buffer = CopyProperPixelValues<RealFunctionMD_UI16>(img_in);
				m_dicom_file->getDataset()->putAndInsertUint8Array(DCM_PixelData, reinterpret_cast<Uint8*>(&(buffer.at({ 0,0,0 }))), pixDataLen);
			}
			break;
			case 32:
			{
				auto buffer = CopyProperPixelValues<RealFunctionMD_UI32>(img_in);
				m_dicom_file->getDataset()->putAndInsertUint8Array(DCM_PixelData, reinterpret_cast<Uint8*>(&(buffer.at({ 0,0,0 }))), pixDataLen);
			}
			break;
			default:
				throw logic_error(ssprintf("Wrong reinterpret value  = %d", is_signed));
			}
		}
	}

	void ContainerDCMTK::set_pixeldata(const RealFunction2D_F32 &img_in, size_t bpp, bool is_signed, size_t ncomp)
	{
		(void)ncomp; //note (Kovbas) нужно будет для сохранения цветных изображений
		//разбираем и отдаём в объект файла данные изображения
		//todo (Kovbas) попробовать переделать так, чтобы было в одну строку (возможно, достаточно одного реинтерпрета). 20180601 - не знаю как это можно сделать и возможно ли.
		unsigned long pixDataLen = (unsigned long)(img_in.vsize()*img_in.hsize()*bpp / CHAR_BIT); //явное преобразование для передачи в DCMTK
		// отправляем в объект файла сырое изображение. При сохранении оно будет жато в зависимости от выбора.
		if (is_signed)
		{
			switch (bpp)
			{
			case 8:
			{
				auto buffer = CopyProperPixelValues<RealFunction2D_I8> (img_in);
				m_dicom_file->getDataset()->putAndInsertUint8Array(DCM_PixelData, reinterpret_cast<Uint8*>(buffer.data()), pixDataLen);
			}
			break;
			case 16:
			{
				auto buffer = CopyProperPixelValues<RealFunction2D_I16>(img_in);
				m_dicom_file->getDataset()->putAndInsertUint8Array(DCM_PixelData, reinterpret_cast<Uint8*>(buffer.data()), pixDataLen);
			}
			break;
			case 32:
			{
				auto buffer = CopyProperPixelValues<RealFunction2D_I32>(img_in);
				m_dicom_file->getDataset()->putAndInsertUint8Array(DCM_PixelData, reinterpret_cast<Uint8*>(buffer.data()), pixDataLen);
			}
			break;
			default:
				throw logic_error(ssprintf("Wrong reinterpret value  = %d", is_signed));
			}
		}
		else
		{
			switch (bpp)
			{
			case 8:
			{
				auto buffer = CopyProperPixelValues<RealFunction2D_UI8>(img_in);
				m_dicom_file->getDataset()->putAndInsertUint8Array(DCM_PixelData, reinterpret_cast<Uint8*>(buffer.data()), pixDataLen);
			}
			break;
			case 16:
			{
				auto buffer = CopyProperPixelValues<RealFunction2D_UI16>(img_in);
				m_dicom_file->getDataset()->putAndInsertUint8Array(DCM_PixelData, reinterpret_cast<Uint8*>(buffer.data()), pixDataLen);
			}
			break;
			case 32:
			{
				auto buffer = CopyProperPixelValues<RealFunction2D_UI32>(img_in);
				m_dicom_file->getDataset()->putAndInsertUint8Array(DCM_PixelData, reinterpret_cast<Uint8*>(buffer.data()), pixDataLen);
			}
			break;
			default:
				throw logic_error(ssprintf("Wrong reinterpret value  = %d", is_signed));
			}
		}
	}


	list<int32_t>	GetTagList(Container &dcm_generic)
	{
		auto &dcm(dynamic_cast<ContainerDCMTK&>(dcm_generic));
		list<int32_t>	result;
		DcmStack stack;

		DcmDataset &dataset = *dcm.m_dicom_file->getDataset();
		OFCondition	status = dataset.nextObject(stack, OFTrue);

		while (status.good())
		{

			uint16_t gtag = stack.top()->getGTag();
			uint16_t etag = stack.top()->getETag();
			bool	nested = stack.top()->isNested();

			if (!nested && gtag != e_item_delimitation_tag_group && gtag != e_pixel_data_group)
			{
				uint32_t val1 = uint32_t(gtag)<<16;
				uint32_t val = val1+etag;
				result.push_back(val);
			}
			status = dataset.nextObject(stack, OFTrue);
		}

		return result;
	}

	void	ContainerDCMTK::ForceUTF8Charset()
	{
		// делаем проверку кодировки и в случаях, когда она отличается от utf-8, форсированно переводить ее
		const string	charset_utf8 = "ISO_IR 192";

		DcmTagKey charset_tag(element_id_to_DcmTag(e_specific_character_set));
		auto dcmItem_p = m_dicom_file->getDataset();
		try
		{
			string charset("");
			// Отрабатываем ситуацию, когда тэг (0008,0005) прописан не в корне дайком файла, а в одной из его вложенных веток
			// Некоторые приборы так записывают. Из-за этого вьюверы (Radiant, Osirix, Horos) пытаются отобразить utf-8 строки
			// в локальной 8-битной кодировке. Для исправления ошибки дублируем тэг в корне дайком-файла.
			// Сделанные изменения влияют только на дайком-файлы, которые мы записываем из своей программы
			// (например, при анонимизации)

			bool	tag_exists_in_root = dcmItem_p->tagExists(charset_tag, false);
			bool	tag_exists_in_sub = dcmItem_p->tagExists(charset_tag, true);

			bool	b_exists = tag_exists_in_root || tag_exists_in_sub;
			bool	b_sub = tag_exists_in_sub && !tag_exists_in_root;

			if(b_exists)
			{
				dcmItem_p->findAndGetOFStringArray(charset_tag, charset, b_sub);
				charset = fix_string_ending(charset);

				if(charset != charset_utf8)
				{
					auto status = m_dicom_file->convertCharacterSet(charset_utf8, 0);
					if (!status.good())
					{
						//включаем этот режим при любом сбое кодировке, а не только в частном случае if(charset == "ISO 2022 IR 6\\IS")
						cp_1251 = true;
						//	TODO костыльное исправление ошибочного тэга. В норме должно быть принудительное преобразование кодировки к utf-8
						//	Изменение тэга кодировки -- раскомментировать две следующие строки.
						//	charset = charset_utf8;
						//	auto status = dcmItem_p->putAndInsertString(charset_tag, charset.c_str());
						//	изменение самих полей -- найти существующий цикл по всем полям dcm файла и скопировать его сюда.
						//	с помощью функций convert_to_string8 преобразовать все строковые поля и записать новые значения
					}
				}

				dcmItem_p->findAndGetOFStringArray(charset_tag, charset, b_sub);
				if(b_sub)
				{
					// если тэг был записан только во вложенной ветке, дублируем его в корне
					set_wstring(e_specific_character_set, convert_to_wstring(charset));
				}
			}
		}
		catch(...)
		{
			//ignore
		}
	}


}//namespace Dicom

XRAD_END
