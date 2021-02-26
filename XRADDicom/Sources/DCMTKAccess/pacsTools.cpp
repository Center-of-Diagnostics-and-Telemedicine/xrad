/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
\file
\date 10/10/2018 3:58:45 PM
\author Kovbas (kovbas)
*/
#include "pre.h"
#include "pacsTools.h"

#include "pacsUtils.h"
#include "dcmtkElementsTools.h"
#include <XRADSystem/TextFile.h>

#include <thread>
#include <sstream>

XRAD_BEGIN

namespace Dicom
{

bool checkPACSAccessibility(const Dicom::pacs_params_t &src_pacs_p)
{
	DcmSCU_XRAD scu;
	return initSCUAndCheckPACSAccessibility(scu, src_pacs_p, e_initSCUPreset::verif);
}

size_t getNumberOfInstances(DcmDataset & dst)
{
	string result;
	dst.findAndGetOFStringArray(DCM_NumberOfStudyRelatedInstances, result);
	return stoi(result);
}

string getAccessionNumber(DcmDataset & dst)
{
	string result;
	dst.findAndGetOFStringArray(DCM_AccessionNumber, result);
	return result;
}

string getStudyInstanceUID(DcmDataset & dst)
{
	string result;
	dst.findAndGetOFStringArray(DCM_StudyInstanceUID, result);
	return result;
}

set<string> getStudyInstanceUIDSet(list<DcmDataset> & wrkLst)
{
	set<string> result;
	for (auto &dst : wrkLst)
	{
		string buffer;
		dst.findAndGetOFStringArray(DCM_StudyInstanceUID, buffer);
		result.insert(move(buffer));
	}
	return result;
}

bool findDatasetAcNo(const datasource_pacs &datasource_p, 
				string rec, 
				const string &modality, 
				list<DcmDataset> & wrkLst, 
				const wstring &destination_folder, size_t &count)
{
	DcmDataset request;

	request.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "STUDY");
	request.putAndInsertOFStringArray(DCM_ModalitiesInStudy, modality);

	request.putAndInsertOFStringArray(DCM_PatientName, "");
	request.putAndInsertOFStringArray(DCM_PatientID, "");

	request.putAndInsertOFStringArray(DCM_StudyDate, "");
	request.putAndInsertOFStringArray(DCM_StudyTime, "");

	request.putAndInsertOFStringArray(DCM_StudyInstanceUID, "");
	request.putAndInsertOFStringArray(DCM_StudyID, "");
	request.putAndInsertOFStringArray(DCM_AccessionNumber, rec);

	request.putAndInsertOFStringArray(DCM_NumberOfStudyRelatedInstances, "");


	wrkLst = findDataset(datasource_p, request, destination_folder, rec, count);
	if (wrkLst.empty())
		return false;
	return true;
}



bool findDatasetStID(const datasource_pacs &datasource_p,
	string rec,
	const string &modality,
	list<DcmDataset> & wrkLst,
	const wstring &destination_folder, size_t &count)
{
	DcmDataset request;

	request.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "STUDY");
	request.putAndInsertOFStringArray(DCM_ModalitiesInStudy, modality);

	request.putAndInsertOFStringArray(DCM_PatientName, "");
	request.putAndInsertOFStringArray(DCM_PatientID, "");

	request.putAndInsertOFStringArray(DCM_StudyDate, "");
	request.putAndInsertOFStringArray(DCM_StudyTime, "");

	request.putAndInsertOFStringArray(DCM_StudyInstanceUID, "");
	request.putAndInsertOFStringArray(DCM_StudyID, rec);
	request.putAndInsertOFStringArray(DCM_AccessionNumber, "");

	request.putAndInsertOFStringArray(DCM_NumberOfStudyRelatedInstances, "");


	wrkLst = findDataset(datasource_p, request, destination_folder, rec, count);
	if (wrkLst.empty())
		return false;
	return true;
}


bool findDatasetStUID(const datasource_pacs &datasource_p,
	string rec,
	const string &modality,
	list<DcmDataset> & wrkLst,
	const wstring &destination_folder, size_t &count)
{
	DcmDataset request;

	request.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "STUDY");
	request.putAndInsertOFStringArray(DCM_ModalitiesInStudy, modality);

	request.putAndInsertOFStringArray(DCM_PatientName, "");
	request.putAndInsertOFStringArray(DCM_PatientID, "");

	request.putAndInsertOFStringArray(DCM_StudyDate, "");
	request.putAndInsertOFStringArray(DCM_StudyTime, "");

	request.putAndInsertOFStringArray(DCM_StudyInstanceUID, rec);
	request.putAndInsertOFStringArray(DCM_StudyID, "");
	request.putAndInsertOFStringArray(DCM_AccessionNumber, "");

	request.putAndInsertOFStringArray(DCM_NumberOfStudyRelatedInstances, "");


	wrkLst = findDataset(datasource_p, request, destination_folder, rec, count);
	if (wrkLst.empty())
		return false;
	return true;
}


bool findDataset(const datasource_pacs &datasource_p, string rec, const string &modality, list<DcmDataset> & wrkLst, const wstring &destination_folder, size_t &count)
{
	size_t pos = rec.find("_");
	string name = rec.substr(0, pos );
	string date = rec.substr(pos + 1, rec.length());

	pos = name.find(" ");
	name.replace(pos, 1, "^");
	pos = name.find(" ");
	name.replace(pos, 1, "^");

	pos = date.find("-");
	date.replace(pos, 1, "");
	pos = date.find("-");
	date.replace(pos, 1, "");
		
	DcmDataset request;

		request.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "STUDY");
		request.putAndInsertOFStringArray(DCM_ModalitiesInStudy, modality);

		request.putAndInsertOFStringArray(DCM_PatientName, name);
		request.putAndInsertOFStringArray(DCM_PatientID, "");

		request.putAndInsertOFStringArray(DCM_StudyDate, date);
		request.putAndInsertOFStringArray(DCM_StudyTime, "");

		request.putAndInsertOFStringArray(DCM_StudyInstanceUID, "");
		request.putAndInsertOFStringArray(DCM_StudyID, "");
		request.putAndInsertOFStringArray(DCM_AccessionNumber, "");

		request.putAndInsertOFStringArray(DCM_NumberOfStudyRelatedInstances, "");

			
	wrkLst = findDataset(datasource_p, request, destination_folder, rec, count);
	if (wrkLst.empty())
		return false;
	return true;
}

list<DcmDataset> findDataset(const datasource_pacs &datasource_p, DcmDataset & request, const wstring &destination_folder, const string &id, size_t &count)
{
	list<DcmDataset> wrkLst;
	count = 0;

	ofstream dtst_dump;
	dtst_dump.open(convert_to_string(destination_folder) + "/" + "search_dump_" +string8_to_string(id) + ".txt", ios::out | ios::trunc);

	try
	{
		DcmSCU_XRAD scu;
		if (!initSCUAndCheckPACSAccessibility(scu, datasource_p, e_initSCUPreset::find))
			throw runtime_error("PACS is not accessible");

		DcmSCU_XRAD_wrapper scu_wrap(scu);
		if (!scu_wrap.initConnection())
			throw runtime_error("Connection ininialization failed");

		T_ASC_PresentationContextID presID = findUncompressedPC(UID_FINDStudyRootQueryRetrieveInformationModel, scu);
		if (presID == 0)
			throw runtime_error("T_ASC_PresentationContextID is zero");


		OFList<QRResponse*> findResponses;

		if (scu.sendFINDRequest(presID, &request, &findResponses).bad())
			throw runtime_error("band find");

		for (auto el : findResponses)
		{
			if (el->m_dataset != NULL)
			{
				wrkLst.push_back(*el->m_dataset);

				dtst_dump << "\n\nReceived Dataset # " << count << ", dumped as follows:" << endl;
				el->m_dataset->print(dtst_dump);
				//fflush(stdout);

				//	string str;
				//	el->m_dataset->findAndGetOFStringArray(DCM_PatientName, str);
				//	dtst_dump << "Patient Name (декодирование utf8): [" << string8_to_string(str) << "]" << endl;
				//fflush(stdout);

				count++;
			}
		}
		dtst_dump << "\n\rTotal number of datasets for this request found = " << count << endl;
		dtst_dump.close();

		cout << "\n\rTotal number of datasets for this request found = " << count << endl;
		fflush(stdout);
	}
	catch (...)
	{
		dtst_dump.close();
		string message = GetExceptionStringOrRethrow();
		cout << "\rStudies search failed. Reason = " << message << endl;
	}

	return wrkLst;
}

//	wstring element_value_to_wstring(DcmItem &dcmItem_p, const DcmTagKey &dcmTag_p, bool searchIntoSub = false);

auto	wait(std::chrono::duration<long long>	t, std::chrono::duration<long long>	dt = 1s, std::function<void()> callback = function<void()>())
{
	while(t.count() > 0)
	{
		printf("!!!");
		callback();
		t -= dt;
		std::this_thread::sleep_for(dt);
	}
//		cout << "\nWating " << std::chrono::duration_cast<std::chrono::minutes>(delays[delay_no++]).count() << " minutes before next attempt...";
}

//TODO Эту функцию следует разделить на примитивные утилиты доступа к паксу (оставить здесь) и алгоритм выгрузки (перенести в DicomDownloader)




bool sendInstancesMy()
{
	return sendInstances();
}

} //end namespace Dicom


XRAD_END