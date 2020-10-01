﻿/*!
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



	list<DcmDataset> findDataset(const datasource_pacs &datasource_p, DcmDataset & request, const wstring &destination_folder, const wstring &id, size_t &count)
	{
		list<DcmDataset> wrkLst;
		count = 0;

		ofstream dtst_dump;
		dtst_dump.open(destination_folder + L"/" + L"search_dump" + id + L".txt", ios::out | ios::trunc);

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