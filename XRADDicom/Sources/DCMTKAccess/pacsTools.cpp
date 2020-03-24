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

	class DownloadsRegistrator
	{
		const wstring	path;
		wstring	complete_filename;
		const study_id_type_t study_id_type;
		const bool	m_success;
		text_file_writer	log;
		set<wstring>	ids;
		const size_t	attempt_no;

		wstring	filename() const
		{
			wstring	result = ssprintf(L"%03zu_", attempt_no);

			result += m_success ? L"successful_" : L"failed_";
			switch(study_id_type)
			{
				case study_id_type_t::accession_number:
					result += L"accession_number";
					break;
				case study_id_type_t::study_id:
					result += L"study_id_list";
					break;
				case study_id_type_t::study_instance_uid:
					result += L"study_instance_uid";
					break;
			}
			return result += L"_list.txt";
		}

	public:
		DownloadsRegistrator(wstring in_path, study_id_type_t in_study_id_type, bool success, size_t in_attempt_no) : study_id_type(in_study_id_type), path(in_path), m_success(success), attempt_no(in_attempt_no)
		{
			start();
		}

		void	start()
		{
			complete_filename = path + L"/" + filename();
			if(!DirectoryExists(path)) CreatePath(path);
			if(FileExists(complete_filename) && m_success)
			{
				// список успешно загруженных дополняем
				text_file_reader	prime(complete_filename);
				wstring	buffer;
				prime.read(buffer);
				prime.close();

				auto	it = buffer.begin();
				while(it!=buffer.end())
				{
					auto	it2 = std::find(it, buffer.end(), '\n');
					wstring	new_id(it, it2);
					if(it2-it) ids.insert(new_id);
					it=it2;
					if(it!=buffer.end()) ++it;//пропуск найденного \n
				}
//				log.open_append(complete_filename);
				printf("\nFound log file %s", convert_to_string(complete_filename).c_str());
			}
			else
			{
				// список исследований с ошибками обнуляем каждый раз
				log.open_create(complete_filename, text_encoding::file_type::utf16_le);
				log.printf_(L"\n");
				log.close();
				printf("\nCreated log file %s", convert_to_string(complete_filename).c_str());
			}
		}

		void	append(const wstring id)
		{
			ids.insert(id);
			auto message = ssprintf(L"%s", id.c_str());

			log.open_append(complete_filename);
			log.printf_((message + L"\n").c_str());
			log.close();
		}

		bool	found(const wstring &id)
		{
			return(ids.find(id) != ids.end());
		}

	protected:
	private:
	};

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
	bool studiesDownloader(const datasource_pacs &datasource_p, const list<elemsmap_t> &in_inst_list, study_id_type_t study_id_type, const wstring &destination_folder, ProgressProxy pproxy)
	{
		list<elemsmap_t> inst_list(in_inst_list);
		if (!checkPACSAccessibility(datasource_p)) return false;
		//если пришёл пустой лист инстансев, то уходим
		if (inst_list.empty()) return false;

		//ищем соответствующие заданным идентификаторам исследований исследования на сервере и сохраняем уникальные идентификаторы для них вместе с соответствующим изначальным идентификатором из пришедшего листа
		multimap<wstring, DcmDataset> dcmDatasetMap;

		DownloadsRegistrator	successful_log(destination_folder, study_id_type, true, 0);

		size_t	preloaded_count(0);

		for(auto it = inst_list.begin(); it != inst_list.end();)
		{
			bool	preloaded(false);
			for(auto &id: *it)
			{
				if(id.first==size_t(study_id_type))
				{
					if(successful_log.found(id.second)) preloaded = true;
				}
			}
			if(preloaded)
			{
				it = inst_list.erase(it);
				++preloaded_count;
			}
			else
			{
				++it;
			}
		}

		printf("\nPreloaded studies number = %zu of %zu in list. %zu studies to download\n", preloaded_count, in_inst_list.size(), inst_list.size());

		ProgressBar	progress(pproxy);
		progress.start("Preparing dataset list", inst_list.size());

		for (auto el : inst_list)
		{
			auto dcmDatasetLst = findAndPrepareDcmDatasetLst(datasource_p, e_dcm_hierarchy_level_t::study,
					elemsmap_to_DcmDataset(el), progress.substep());
			//dcmDatasetLst = findAndPrepareDcmDatasetLst(datasource_p, e_dcm_hierarchy_level_t::instance, dcmDatasetLst, pp_p);

			for (auto el1 : dcmDatasetLst)
			{
				auto &id = (el.find(tag_t(study_id_type)))->second;
				dcmDatasetMap.emplace(id, el1);
			}
			++progress;
		}
		progress.end();


		//если ничего не нашлось, то выходим
		if (dcmDatasetMap.empty()) return false;
#if 0
		stringstream tmpStr{""};
		printf("---- normalized DCMs -----\n");
		for (auto el : dcmDatasetMap)
			el.second.print(tmpStr);
		printf("%s\n---- normalized DCMs -----\n\n", tmpStr.str().c_str());
#endif

		//закачиваем исследования
		size_t nTrials = 10;


		auto	started_time = std::chrono::high_resolution_clock::now();
		auto	ended_time = std::chrono::high_resolution_clock::now();


		vector<std::chrono::duration<long long>> delays = {0s, 10min, 30min, 1h, 1h, 1h, 1h, 1h, 1h, 1h, 1h};
		XRAD_ASSERT_THROW(delays.size()>=nTrials);

		size_t delay_no(0);

		for (size_t i = 0; i < nTrials; ++i)
		{
			auto	delay = delays[delay_no];
			if(i && ended_time - started_time < delay + dcmDatasetMap.size()*1min)
			{
				delay = delays[++delay_no];
				auto	t0 = std::chrono::high_resolution_clock::now();
				auto	te = t0+delay;
				auto	delay_message = [t0,te]()
				{
				cout << "\rWating " << std::chrono::duration_cast<std::chrono::minutes>(te-t0).count() << " minutes before next attempt, " <<
					std::chrono::duration_cast<std::chrono::seconds>(te-std::chrono::high_resolution_clock::now()).count() << " seconds remaining";
				fflush(stdout);
				};
				printf("\n");
				wait(delay, 1s, delay_message);

				//std::this_thread::sleep_for(delays[i]);
				cout << "OK\n";
			}
			else
			{
				delay_no = 0;
			}

			started_time = std::chrono::high_resolution_clock::now();

			DownloadsRegistrator	failed_log(destination_folder, study_id_type, false, i);
			wstring msg{ L"Downloading " + to_wstring(dcmDatasetMap.size()) + L" studies. Attempt " + to_wstring(i + 1) + L" of " + to_wstring(nTrials) };
			progress.start(msg, dcmDatasetMap.size());
			printf("%s\n", convert_to_string(msg).c_str());

			multimap<wstring, DcmDataset> tmpDcmDatasetMap;
			for (auto el : dcmDatasetMap)
			{

// 				auto dataset = el.second;
// 				DcmTagKey	study_id_tag(element_id_to_DcmTag(e_study_id));
// 				DcmTagKey	accession_number_tag(element_id_to_DcmTag(e_accession_number));
//
// 				dataset.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "STUDY");
//
// 				const wstring	study_id = element_value_to_wstring(dataset, study_id_tag);
// 				const wstring	accession_number = element_value_to_wstring(dataset, accession_number_tag);

				const wstring identifier(el.first);
				try
				{
// 					wstring	foldername(accession_number + L"_" + identifier);
 					wstring	foldername(identifier);
					//TODO использовать функцию make_proper_foldername
					std::replace(foldername.begin(), foldername.end(), L'*', L'_');
					wstring study_destination_folder = destination_folder + L"/" + foldername;
					printf("%s", convert_to_string(L"Downloading study " + identifier/* + L"\n"*/).c_str());
					if(common_downloader(datasource_p, el.second, study_destination_folder))
					{
						printf("%s", convert_to_string(L"\rStudy " + identifier + L" is downloaded.\n").c_str());
						successful_log.append(identifier);
					}
					else
					{
						throw runtime_error("common_downloader failed");
					}
				}
				catch(...)
				{
 					string message = GetExceptionStringOrRethrow();
//					string message = GetExceptionString();
					printf("%s", ("\rStudy " + convert_to_string(identifier) + " download failed. Reason = '" + message + "'\n").c_str());
					tmpDcmDatasetMap.insert(el);
					failed_log.append(identifier);
				}

				++progress;
			}
			progress.end();
			swap(dcmDatasetMap, tmpDcmDatasetMap);
			if (dcmDatasetMap.empty())
				break;

			//report about not downloaded strudies
			printf("These studies were not downloaded correctly (%zu item(s)). You should check them:\n", dcmDatasetMap.size());
#pragma вернуть
			//++for (auto el : dcmDatasetMap)
				//printf("%s\n", convert_to_string(element_value_to_wstring(el.second, element_id_to_DcmTag(tag_t(study_id_type)))).c_str());
		ended_time = std::chrono::high_resolution_clock::now();
		}

		return true;
	}



	bool sendInstancesMy()
	{
		return sendInstances();
	}

} //end namespace Dicom


XRAD_END