/*!
	 * \file dcmtkElementsTools.h
	 * \date
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

#include "dcmtkElementsTools.h"

#include <XRADDicom/DicomClasses/dataelement.h>
#include <XRADDicom/DCMTKAccess/dcmtkUtils.h>

#include <XRADDicom/XRADDicomTools.h>

XRAD_BEGIN

namespace Dicom
{

	//конвертирует номер тега в формат DcmTag DCMTK
	DcmTag element_id_to_DcmTag(tag_t id)
	{
		Uint16 group = Uint16(id >> 16);
		Uint16 element = Uint16(id ^ (group << 16));

		return DcmTag(group, element);
	}

	//получение номера тега из DcmTag
	tag_e DcmTag_to_element_id(const DcmTag &dcmTag)
	{
		return tag_e(((dcmTag.getGroup() << 16) + dcmTag.getElement()));
	}

	//возвращает номер элемента (значение тега) в формате XRAD (id)
	tag_e get_element_id(const DcmObject* dobj)
	{
		return tag_e((dobj->getTag().getGroup() << 16) + dobj->getTag().getElement());
	}

	DcmDataset elemsmap_to_DcmDataset(const elemsmap_t &elemsmap)
	{
		DcmDataset dcmDataset;
		for (auto el : elemsmap)
			dcmDataset.putAndInsertOFStringArray(element_id_to_DcmTag(el.first), convert_to_string(el.second));

		return dcmDataset;
	}

	wstring remove_and_shrink(const wstring &val, wchar_t ch)
	{
		auto buf(val);
		buf.erase(remove(buf.begin(), buf.end(), ch), buf.end());
		return buf;
	}


	bool findAndDeleteElement(DcmFileFormat &dcmFile, uint id, bool all, bool into)
	{
		DcmTag dcmTag = element_id_to_DcmTag(id);
		if(dcmTag.getGroup() == e_meta_info_group)
		{
			DcmMetaInfo *currItem = dcmFile.getMetaInfo();
			return (currItem->findAndDeleteElement(dcmTag, all, into)).good();
		}
		else
		{
			DcmDataset *currItem = dcmFile.getDataset();
			return (currItem->findAndDeleteElement(dcmTag, all, into)).good(); //note Kovbas это удаление применимо для всех типов элементов, в т.ч. последовательностей. Поэтому для последовательностей ничего не добавил.
		}
	}
	/*
	bool putAndInsertString(DcmFileFormat &dcmFile, uint id, const string &val)
	{
		DcmTag dcmTag = element_id_to_DcmTag(id);

		if (dcmTag.getGroup() == e_meta_info_group)
		{
			DcmMetaInfo *currItem = dcmFile.getMetaInfo();
			return (currItem->putAndInsertString(dcmTag, val.c_str())).good();
		}
		else
		{
			DcmDataset *currItem = dcmFile.getDataset();
			return (currItem->putAndInsertString(dcmTag, val.c_str())).good();
		}
	}*/


	/*
		Функции для вывода содержимого DICOM-файла в строковом виде.
		Помещает информацию из объекта dicom (то, что находится внутри тега) в строковый поток.
	*/
	//! выводит заголовоки столбцов для элементов, значения которых в последствии выводит
	void putObjToStr_header(stringstream &msg)
	{
		//string abrakadabra = convert_to_string(L"Tag (кириллица,  թվականի)\n\n");
		//wstring a2 = convert_to_wstring(abrakadabra);
		//tag num
		msg << "Tag";

		//VR
		msg << "\t";
		msg << "VR";

		//VM
		msg << "\t";
		msg << "VM";

		//lenght
		msg << "\t";
		msg << "Element length";

		//value
		msg << "\t";
		msg << "Element value";

		//tag name
		msg << "\t";
		msg << "Tag name";
	}
	//! выводит содержимое элемента
	void putObjToStr(stringstream &msg, DcmObject *obj)
	{
		try
		{
	#if 0
			obj->print(msg);
	#else
			//DcmElement *delem = (DcmElement *)obj;
			DcmElement *delem = static_cast<DcmElement *>(obj);
			//			DcmElement *delem = static_cast<DcmElement *>(obj);

			DcmTag tag = obj->getTag();

			//get tag num
			string	str_temp = tag.toString();
			msg << str_temp;

			// get VR (Value Representation)
			msg << "\t";
			msg << tag.getVR().getVRName();

			// get VM
			msg << "\t";
			msg << obj->getVM();

			// get length
			msg << "\t";
			if (string(tag.getVR().getVRName()) != "SQ") // если не последовательность, то забираем размер
				msg << obj->getLengthField();

			// get value
			msg << "\t";
			if ((tag.getGroup() == e_pixel_data_group) || (tag.getGroup() == e_item_delimitation_tag_group)) //если не изображение и не разделитель, то забираем значение
				;
			else
			{
				string str;
				delem->getOFStringArray(str);
				msg << (str);
			}

			//get tag name
			msg << "\t";
			msg << tag.getTagName();
	#endif
		}

		catch (exception &/*ex*/)
		{
			//Error(ex.what());
			//throw;
		}
	}

	/*!
		\brief Функция вывода содержимого DICOM-файла в строковом виде

		Помещает информацию из объектов (элементов) DICOM-файла в строковый поток

		\note используется в DicomDataContainer, чтобы получать набор данных из файла
	*/
	wstring elements_to_wstring(DcmFileFormat &fileformat, bool byDCMTK)
	{
		std::stringstream msg;
		if (byDCMTK)
			// вывод всех данных посредством метода print() из dcmtk. Выводит всю информацию в строковый поток
			fileformat.print(msg);
		else
		{
			string indent = "";
			size_t numOfObj = 0;
			DcmStack stack;
			DcmObject *dobject = NULL;

			//вывод заголовка для получаемых данных
			//msg << "Num\t";
			putObjToStr_header(msg);
			msg << endl;

			// вывод MetaInfo из dicom файла
			msg << "Meta Info" << endl;
			DcmMetaInfo meta = *fileformat.getMetaInfo();
			OFCondition	status = meta.nextObject(stack, OFTrue);
			while (status.good())
			{
				numOfObj++;
				//msg << numOfObj << "\t";
				dobject = stack.top();
				putObjToStr(msg, dobject);
				msg << endl;
				status = meta.nextObject(stack, OFTrue);
			}

			// вывод данных из dataset
			msg << endl << "Dataset" << endl;
			DcmDataset dataset = *fileformat.getDataset();
			status = dataset.nextObject(stack, OFTrue);

			list<DcmObject*> listParObj;
			size_t prev;
			while (status.good())
			{

				dobject = stack.top();
				indent.clear();
				prev = listParObj.size();

				//не показываем (пропускаем) теги, обозначающие начало и конец последовательности
				/*if (dobject->getTag().getGroup() == 0xfffe)
				{
					status = dataset.nextObject(stack, OFTrue);
					continue;
				}*/
				// элементы последовательностей будут с отступом
				if (dobject->isNested())
				{
					size_t exist, index;

					exist = 0;
					for (auto el : listParObj)
					{
						if (el == dobject->getParent())
						{
							exist = 1;
							break;
						}
					}

					if (exist == 0)
					{
						listParObj.push_back(dobject->getParent());
					}

					index = 0;
					for (auto el : listParObj)
					{
						index++;
						if (el == dobject->getParent())
							break;
					}

					for (size_t i = 0; i < index; i++)
						indent.append("  ");

					if(index < prev)
						for(size_t i = 0; i < (prev - index); i++)
							listParObj.pop_back();

				}
				else
				{
					listParObj.clear();
				}

				if (dobject->getTag().getGroup() != e_item_delimitation_tag_group)
				{
					numOfObj++;
					//msg << numOfObj << "\t";
					msg << indent;
					//if (dobject->getTag().getGroup() == 0xfffe)
					//	msg << dobject->getTag().toString();
					//else
					putObjToStr(msg, dobject);

					msg << endl;
				}
				status = dataset.nextObject(stack, OFTrue);
			}
		}
		return string8_to_wstring(msg.str());
	}




	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// функции, используемые в других проектах

	//кладёт кодированное jpeg2000 изображение на место из файла, откуда был взят
	bool putJPEG2000DataInFile(DcmDataset* &dataset, const unique_ptr<char[]> &pixDataCompr, size_t pixDataLen)
	{
		OFCondition dcmRes;
		// находим элемент с изображением
		DcmElement* element = NULL;
		dcmRes = dataset->findAndGetElement(DCM_PixelData, element);

		checkOFResult(dcmRes, string("Get the picture:") + dcmRes.text());

		// забираем элемент с изображением
		DcmPixelData *dpix = NULL;
		dpix = OFstatic_cast(DcmPixelData*, element);
		/* Since we have compressed data, we must utilize DcmPixelSequence
		in order to access it in raw format, e. g. for decompressing it
		with an external library.
		*/
		// забираем последовательность байт жатого изображения
		DcmPixelSequence *dseq = NULL;
		E_TransferSyntax xferSyntax = EXS_LittleEndianExplicit;
		const DcmRepresentationParameter *rep = NULL;
		// Find the key that is needed to access the right representation of the data within DCMTK
		dpix->getOriginalRepresentationKey(xferSyntax, rep);
		// Access original data representation and get result within pixel sequence
		dcmRes = dpix->getEncapsulatedRepresentation(xferSyntax, rep, dseq);
		if (dcmRes != EC_Normal) throw runtime_error(string("Couldnt get encapsulated data"));
		DcmPixelItem* pixitem = NULL;

		// Access first frame (skipping offset table)
		dseq->getItem(pixitem, 1);
		if (pixitem == NULL) throw runtime_error(string("pixitem == null"));

		// в случае передачи указателя на NULL putUint8Array ничего не делает
		pixitem->putUint8Array(reinterpret_cast<const Uint8*>(pixDataCompr.get()), unsigned long(pixDataLen)); // приведение для отправки данных в DCMTK

		return true;
	}



	//===================================================================================================================
	// Общие инструменты. Обычно объявлены выше, определены здесь
	//===================================================================================================================
	wstring get_tag_as_string(tag_t tag_p)
	{
		return convert_to_wstring(element_id_to_DcmTag(tag_p).toString());
	}
	wstring get_tagname(tag_t tag_p)
	{
		if (dcmDataDict.isDictionaryLoaded())
		{
			auto val = dcmDataDict.rdlock().findEntry(Dicom::element_id_to_DcmTag(tag_p), nullptr);
			if (val == nullptr || val->getTagName() == nullptr) return L"no standard tag name";
			return convert_to_wstring(val->getTagName());
		}
		return L"";
	}
	wstring get_tagname(const wstring &tag_p)
	{
		auto buf = remove_and_shrink(remove_and_shrink(remove_and_shrink(remove_and_shrink(remove_and_shrink(tag_p, L' '), L'('), L')'), L','), L'\t');

		return get_tagname(wcstol(buf.c_str(), nullptr, 16));
	}

	wstring generateUUID(UUID_level id_type_p, const wstring &prefix_p)
	{
		char uid[70]; //note Kovbas DCMTK просит >=65 байт, сгенерированный UUID содержит не более 64 символов

		if (!prefix_p.length())
			switch (id_type_p)
			{
			case UUID_level::study:
				return convert_to_wstring(string(dcmGenerateUniqueIdentifier(uid, SITE_STUDY_UID_ROOT)));
			case UUID_level::series:
				return convert_to_wstring(string(dcmGenerateUniqueIdentifier(uid, SITE_SERIES_UID_ROOT)));
			case UUID_level::instance:
				return convert_to_wstring(string(dcmGenerateUniqueIdentifier(uid, SITE_INSTANCE_UID_ROOT)));
			case UUID_level::other:
				return convert_to_wstring(string(dcmGenerateUniqueIdentifier(uid)));
			default:
				return wstring();
			}

		switch (id_type_p)
		{
		case UUID_level::study:
			return convert_to_wstring(string(dcmGenerateUniqueIdentifier(uid, convert_to_string(prefix_p + L".1.2").c_str())));
		case UUID_level::series:
			return convert_to_wstring(string(dcmGenerateUniqueIdentifier(uid, convert_to_string(prefix_p + L".1.3").c_str())));
		case UUID_level::instance:
			return convert_to_wstring(string(dcmGenerateUniqueIdentifier(uid, convert_to_string(prefix_p + L".1.4").c_str())));
		case UUID_level::other:
			return convert_to_wstring(string(dcmGenerateUniqueIdentifier(uid, convert_to_string(prefix_p).c_str())));
		default:
			return{ L"" };
		}
	}

}//namespace Dicom


XRAD_END
