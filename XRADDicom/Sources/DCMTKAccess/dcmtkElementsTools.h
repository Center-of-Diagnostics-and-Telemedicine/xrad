/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\author kovbas
*/
#ifndef dcmtkElementsTools_h__
#define dcmtkElementsTools_h__

#include <XRADDicom/Sources/DicomClasses/XRADDicomGeneral.h>

XRAD_BEGIN

namespace Dicom
{

	DcmTag element_id_to_DcmTag(tag_t id);
	tag_e DcmTag_to_element_id(const DcmTag &dcmTag);
	DcmDataset elemsmap_to_DcmDataset(const elemsmap_t &elemsmap);

	//получение тега элемента
	tag_e get_element_id(const DcmObject* dobj);

	//возвращает содержимое DICOM-файла в виде строки
	wstring elements_to_wstring(DcmFileFormat &fileformat, bool byDCMTK);

	bool putJPEG2000DataInFile(DcmDataset* &dataset, const unique_ptr<char[]> &pixDataCompr, size_t pixDataLen);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void putObjToStr(stringstream &msg, DcmObject *obj = NULL);

	//int strToDoubles(const string &str, vector<double> &v, char delimeter = '\\');

	//bool putAndInsertString(DcmFileFormat &dcmFile, uint id, const string &val);
	bool findAndDeleteElement(DcmFileFormat &dcmFile, uint id, bool all, bool into);



}//namespace Dicom

XRAD_END

#endif // dcmtkElementsTools_h__