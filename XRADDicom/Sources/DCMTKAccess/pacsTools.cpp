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


	wstring element_value_to_wstring(DcmItem &dcmItem_p, const DcmTagKey &dcmTag_p, bool searchIntoSub = false);

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