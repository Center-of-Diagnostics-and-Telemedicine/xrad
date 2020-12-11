#include "pre.h"
#include "DicomStorageAnalyze.h"

#include <XRADDicom/Sources/DCMTKAccess/dcmtkElementsTools.h>
#include <XRADDicom/Sources/DCMTKAccess/pacsTools.h>
#include "DicomProcessors.h"
#include "DicomFilters.h"
#include "Instances/CreateInstance.h"

#include "Indexing/DicomCatalogIndex.h"
#include "Indexing/DicomFileIndex.h"

#include <XRADSystem/System.h>
#include <XRADBasic/Sources/Utils/ParallelProcessor.h>

#include <set>
#include <mutex>

XRAD_BEGIN

namespace
{

	struct	subcontainer_exists
	{
		const Dicom::instance &instance;
		subcontainer_exists(const Dicom::instance &in_instance) : instance(in_instance) {}

		bool operator()(const Dicom::acquisition_loader &s) const { return s.acquisition_id() == instance.acquisition_id(); }
		bool operator()(const Dicom::stack_loader &s) const { return s.stack_id() == instance.stack_id(); }
		bool operator()(const Dicom::series_loader &s) const { return s.complete_series_id() == instance.complete_series_id(); }
		bool operator()(const Dicom::study_loader &s) const { return s.complete_study_id() == instance.complete_study_id(); }
		bool operator()(const Dicom::patient_loader &s) { return s.patient_id() == instance.patient_id(); }
	};


	template<class CONTAINER_T>
	void add_instance(const Dicom::instance_ptr &instance, CONTAINER_T &container)
	{
		typedef typename CONTAINER_T::value_type subcontainer_type;
		auto found = std::find_if(container.begin(), container.end(), subcontainer_exists(*instance));
		if (found == container.end())
		{
			container.push_back(subcontainer_type(*instance));
			found = std::find_if(container.begin(), container.end(), subcontainer_exists(*instance));
			add_instance(instance, *found);
		}
		else
		{
			add_instance(instance, *found);
		}
	}

	template<>
	void add_instance(const Dicom::instance_ptr &instance, Dicom::acquisition_loader &container)
	{
		container.push_back(instance);
	}


	void createAndAddInstance(Dicom::patients_loader &studies_heap_p, const Dicom::instancestorage_ptr &instancestoreage_p, const dicom_instance_predicate &instance_predicate_p, std::mutex &collector_mutex)
	{


		auto add_instance_lambda = [&collector_mutex, &studies_heap_p](Dicom::instance_ptr &dicom_instance_p)
		{
			std::lock_guard<std::mutex> lock(collector_mutex);
			add_instance(dicom_instance_p, studies_heap_p);
		};

		auto add_instance_lambda_preindexed = [&collector_mutex, &studies_heap_p, &instancestoreage_p, &instance_predicate_p]()
		{
			std::lock_guard<std::mutex> lock(collector_mutex);
			Dicom::instance_ptr dicom_instance = Dicom::CreateInstancePreIndexed(instancestoreage_p, instance_predicate_p); // создает полиморфный контейнер, опираясь на информацию о модальности в источнике Dicom
			if (!dicom_instance)
				return;
			add_instance(dicom_instance, studies_heap_p);
		};


		if (instancestoreage_p->pre_indexed())
		{
			add_instance_lambda_preindexed();
			/*
			Dicom::instance_ptr dicom_instance = Dicom::CreateInstancePreIndexed(instancestoreage_p, instance_predicate_p); // создает полиморфный контейнер, опираясь на информацию о модальности в источнике Dicom
			if (!dicom_instance)
				return;
			//if (!dicom_instance->dicom_container()->try_open_instancestorage(instancestoreage_p))
			//	return;
			add_instance_lambda(dicom_instance);
			*/
		}
		else
		{
			Dicom::instance_ptr dicom_instance;
			dicom_instance = Dicom::CreateInstance(instancestoreage_p, instance_predicate_p); // создает полиморфный контейнер, опираясь на информацию о модальности в источнике Dicom

			if (!dicom_instance) return;
			if (dicom_instance->is_dicomdir()) return; //note (Kovbas) пока пропускаем дайкомдиры
			//только после двух проверок выше что-то делаем с инстансем, иначе будут проблемы
			Dicom::instance_open_close_class inst(*dicom_instance); //note (Kovbas) чтобы при выходе файл был закрыт


			if (dicom_instance->is_multiframe()) //если мультифрейм, нужно создать соответствующее кол-во инстансев
			{
				//todo (Kovbas) работаем над тем, чтобы мультифреймы не пропускать
#if 0
				for (size_t i = 1; i <= dicom_instance->frames_number(); ++i)
				{
					auto dicom_instanceTmp(dicom_instance);
					dicom_instanceTmp->set_num_of_frame(i);
					add_instance_lambda(dicom_instanceTmp);
				}
#endif
			}
			else
			{
				add_instance_lambda(dicom_instance);
			}
		}
	}

	Dicom::patients_loader RawAnalyzeFolder(const Dicom::datasource_folder &src_folder,
		const DicomInstanceFilters_t &filter_p,//TODO этот фильтр уже давно здесь дает предупреждение, пора и разобраться с ним
		ProgressProxy pproxy)
	{
		RandomProgressBar	progress(pproxy);

		ProgressIndicatorScheduler	scheduler({ 5, 95 });
		progress.start("Analyzing Dicom folder", scheduler.n_steps());

		std::vector<wstring> filenames = GetDirectoryFiles(src_folder.path(), L"",
				src_folder.analyze_subfolders(), progress.subprogress(scheduler.operation_boundaries(0)));

		// Удаляем из списка заведомо не-DICOM файлы.
		filenames.erase(remove_if(filenames.begin(), filenames.end(), [](const wstring &name)
				{
					return !may_be_dicom_filename(name);
				}), filenames.end());

		ParallelProcessor	processor;
#ifdef XRAD_DEBUG
		//	В режиме отладки деление на порции по потокам принудительно отключается. Анализ каталога может занять значительное время. Индикатор прогресса с большими порциями
		//	обрабатываемых за раз данных становится при этом неинформативным (невозможно понять, сколько времени займет первый шаг, будет ли время обработки приемлемо).
		auto	mode = ParallelProcessor::e_force_plain;
#else
		auto	mode = ParallelProcessor::e_auto;
#endif
		processor.init(filenames.size(), mode, 32);
		//	Один поток создается для 32 файлов.
		//	Это обеспечит более плавную работу индикатора прогресса.
		//	По умолчанию ParallelProcessor укрупняет порцию одного потока,
		//	чтобы уменьшить относительную долю расходов по созданию потока.
		//	Но это актуально для множества быстрых операций, а открытие файла сюда не относится.
		Dicom::patients_loader studies_heap;
		std::mutex collector_mutex;
		auto	lambda = [&filenames, &studies_heap, &collector_mutex, &filter_p](size_t frame_no)
		{
			wstring full_file_path = filenames[frame_no];
			createAndAddInstance(studies_heap, new Dicom::instancestorage_file(full_file_path), get<dicom_instance_predicate>(filter_p), collector_mutex);
		};
		processor.perform(lambda, L"Parsing Dicom folder", progress.subprogress(scheduler.operation_boundaries(1)));

		return studies_heap;
	}

	Dicom::patients_loader RawAnalyzeFolderIndexing(
		const Dicom::datasource_folder &src_folder,
		const DicomInstanceFilters_t &filter_p,//TODO этот фильтр уже давно здесь дает предупреждение, пора и разобраться с ним
		ProgressProxy pproxy)
	{

		RandomProgressBar	progress(pproxy);
		ProgressIndicatorScheduler	scheduler({ 90, 2, 8 });
		progress.start(L"Analyzing DICOM folder", scheduler.n_steps());

		// индексировать все файлы в каталоге src_folder.path()

		bool b_show_stdout = true;//temporary
//		bool b_show_stdout = false;
		Dicom::DicomCatalogIndex dicom_catalog_index(b_show_stdout);
		dicom_catalog_index.PerformCatalogIndexing(src_folder, progress.subprogress(scheduler.operation_boundaries(0)));

		ProgressBar progress_b(progress.subprogress(scheduler.operation_boundaries(1)));
		progress_b.start(L"Fill instances", dicom_catalog_index.n_items());
		//  сформировать instancestorages вектор для Dicom файлов

		std::vector<Dicom::instancestorage_ptr> instancestorages;
		for (auto& el_dir : dicom_catalog_index.data()) // для каждой директории
		{
			auto path_to_dir = el_dir.get_path();
			for (const auto& el : el_dir) // для каждого файла
			{
				if ( el.is_dicom() ) // важное исправление, удалено условие && el.has_image_type():нужно индексировать все дайкомы, а не только изображения. Терялись SR при анонимизации
				{
					wstring filename = path_to_dir + wpath_separator() +
							convert_to_wstring(el.get_file_name());
					instancestorages.push_back(new Dicom::instancestorage_file_pre_indexed(filename, el));
				}
				++progress_b;
			}
			el_dir.clear(); // очистить занимаемую память для обработанной директории с индексами
			el_dir.shrink_to_fit();
		}
		progress_b.end();

		Dicom::patients_loader studies_heap_cpy;
#ifdef XRAD_DEBUG
		//	В режиме отладки деление на порции по потокам принудительно отключается. Анализ каталога может занять значительное время. Индикатор прогресса с большими порциями
		//	обрабатываемых за раз данных становится при этом неинформативным (невозможно понять, сколько времени займет первый шаг, будет ли время обработки приемлемо).
		auto	mode = ParallelProcessor::e_force_plain;
#else
		auto	mode = ParallelProcessor::e_auto;
#endif
		ParallelProcessor	processor;
		processor.init(instancestorages.size(), mode, 32);
		//	Один поток создается для 32 файлов.

		std::mutex collector_mutex;
		Dicom::patients_loader studies_heap;

		auto	lambda = [&instancestorages, &studies_heap, &collector_mutex, &filter_p](size_t frame_no)
		{
			createAndAddInstance(studies_heap, instancestorages[frame_no], get<dicom_instance_predicate>(filter_p), collector_mutex);
		};
		processor.perform(lambda, L"Parsing DICOM folder (with index)", progress.subprogress(scheduler.operation_boundaries(2)));

		return studies_heap;
	}

	Dicom::patients_loader RawAnalyzeFolderSelector(
		const Dicom::datasource_folder &src_folder,
		const DicomInstanceFilters_t &filter_p,
		ProgressProxy pproxy)
	{
		switch (src_folder.mode())
		{
			default:
			case decltype(src_folder.mode())::read_and_update_index:
			case decltype(src_folder.mode())::read_only_index:
			case decltype(src_folder.mode())::read_index_as_is:
				return RawAnalyzeFolderIndexing(src_folder, filter_p, pproxy);

			case decltype(src_folder.mode())::no_index:
				return RawAnalyzeFolder(src_folder, filter_p, pproxy);
		}
	}

	Dicom::patients_loader RawAnalyzePACS(const Dicom::datasource_pacs &src_pacs,
		const DicomInstanceFilters_t &filter_p,
		ProgressProxy pproxy)
	{
		Dicom::patients_loader studies_heap;
		// инициализируем сеть
		//проверяем доступность сервера
		//-Dicom::DcmSCU_XRAD scu;
		if (!Dicom::checkPACSAccessibility(src_pacs))
			return studies_heap;

		// собираем информацию обо всех необходимых нам исследованиях, которые мы хотим забрать с сервера
		list<Dicom::elemsmap_t> work_list;
		//Dicom::fillListOfInstances(scu, filter, wrkLst);

		std::mutex collector_mutex;
		//TODO Kovbas добавляем в source данные о инстансах (источник данных и минимальные данные по каждому (то, что даёт запрос))
		// а из сорса через Dicom::Container уже открываем это дело и всё с этим делаем.
		for (auto el : work_list)
		{
			//Dicom::instance_ptr instTmp = Dicom::CreateInstance(new Dicom::instancestorage_pacs(src_pacs, el));
			//todo (Kovbas) решить вопрос с тем, что не все инстансы были получены с сервера
			createAndAddInstance(studies_heap, new Dicom::instancestorage_pacs(src_pacs, el), get<dicom_instance_predicate>(filter_p), collector_mutex);
		}
		return studies_heap;
	}

	//! \brief 	Возвращает кучу инстансов, разложенных по иерархическому дереву "patient-study-series-stack-acquisition".
	//			Инстансы содержат только информацию, позволяющую определить принадлежность к определенной ветви этого дерева.
	// 			Источники данных инстансов (dicom files) на выходе из функции закрыты для экономии памяти.
	//			Пользователь может открывать и закрывать их по мере необходимости.
	Dicom::patients_loader RawAnalyzeDicomSource(const Dicom::datasource_t &dicom_datasource,
		//const Dicom::filter_t &filter,
		const DicomInstanceFilters_t &filter_p,
		ProgressProxy pproxy)
	{
		switch (dicom_datasource.type())
		{
		case Dicom::datasource_t::folder:
			return RawAnalyzeFolderSelector(
				dynamic_cast<const Dicom::datasource_folder&>(dicom_datasource),
				filter_p,
				pproxy);

		case Dicom::datasource_t::pacs:
			return RawAnalyzePACS(
				dynamic_cast<const Dicom::datasource_pacs&>(dicom_datasource),
				filter_p,
				pproxy);
		}

		throw invalid_argument("Dicom::patients_loader RawAnalyzeDicomSource, invalid Dicom source type");
	}


	// Применение фильтров и удаление дубликатов в каждой сборке
	class AcquisitionPrepare : public Dicom::AcquisitionProcessor<Dicom::acquisition_loader>
	{
	public:
		AcquisitionPrepare() = default;
	private:
		virtual	void Apply(Dicom::acquisition_loader &data, ProgressProxy )
		{
			data.prepare(); // здесь производится сортировка и удаление дубликатов
		}
	};

} // namespace


Dicom::patients_loader GetDicomStudiesHeap(
	const Dicom::datasource_t &dicom_src,
	const DicomInstanceFilters_t  &filters_p,
	ProgressProxy progress_proxy)
{
	// Загружаем все дайкомы из указанного источника (папки или PACS),
	// разбираем по сборкам, с которыми в последствии и будет работа.
	RandomProgressBar	progress(progress_proxy);
	progress.start("Retrieving Dicom studies list", 1);
	Dicom::patients_loader studies_heap = RawAnalyzeDicomSource(dicom_src, filters_p, progress.subprogress(0, 0.5));

	if (studies_heap.empty()) return studies_heap; // дайкомов не обнаружено, уходим

	// Здесь делается предварительная обработка acquisition:
	// сортировка instances внутри acquisition, удаление дубликатов и пустых элементов studies_heap на всех уровнях
	Dicom::PatientsProcessorRecursive<Dicom::patients_loader>	processor(make_shared<AcquisitionPrepare>());
	processor.Apply(studies_heap, progress.subprogress(0.5, 1));

	return studies_heap;
}



XRAD_END