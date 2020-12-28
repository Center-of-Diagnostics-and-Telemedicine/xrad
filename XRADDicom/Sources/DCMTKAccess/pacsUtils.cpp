/*!
	\file
	\date 10/10/2018 3:58:45 PM
	\author Kovbas (kovbas)
*/
#include "pre.h"
#include "pacsUtils.h"

#include <XRADDicom/Sources/DicomClasses/dataelement.h>
#include "dcmtkElementsTools.h"

#include <XRADDicom/Sources/DicomClasses/DataContainers/ContainerDCMTK.h>
#include <XRADDicom/Sources/DicomClasses/Instances/ct_slice.h>

#include <thread>

#include <dcmtk/dcmdata/dcpath.h>


XRAD_BEGIN

namespace Dicom
{
	void printDcmDatasetToLog(DcmDataset *dcmDS, const string &comment)
	{
		ostringstream ss{ "" };
		dcmDS->print(ss);
		printf("---DcmDataset start---%s\n %s \n---DcmDataset end---\n", comment.c_str(),  ss.str().c_str());
	}

	 Uint8 findUncompressedPC(const OFString& sopClass, const DcmSCU& scu)
	{
		DcmSCU& scuRef = const_cast<DcmSCU&>(scu);
		Uint8 pc;
		pc = scuRef.findPresentationContextID(sopClass, UID_LittleEndianExplicitTransferSyntax);
		if (pc == 0)
			pc = scuRef.findPresentationContextID(sopClass, UID_BigEndianExplicitTransferSyntax);
		if (pc == 0)
			pc = scuRef.findPresentationContextID(sopClass, UID_LittleEndianImplicitTransferSyntax);
		return pc;
	}



	bool initSCUAndCheckPACSAccessibility(DcmSCU_XRAD &scu_p, const pacs_params_t &src_pacs_p, e_initSCUPreset preset_p)
	{
		//OFCondition result;
		scu_p.setAETitle(convert_to_string(src_pacs_p.AETitle_local()));
		scu_p.setPeerHostName(convert_to_string(src_pacs_p.address_pacs()));
		scu_p.setPeerPort(static_cast<Uint16>(src_pacs_p.port_pacs()));
		scu_p.setPeerAETitle(convert_to_string(src_pacs_p.AETitle_pacs()));

		// minimal presentation contexts for initialization
		OFList<OFString> ts;
		ts.push_back(UID_LittleEndianExplicitTransferSyntax);
		ts.push_back(UID_BigEndianExplicitTransferSyntax);
		ts.push_back(UID_LittleEndianImplicitTransferSyntax);

		scu_p.addPresentationContext(UID_VerificationSOPClass, ts);
		switch (preset_p)
		{
		case e_initSCUPreset::verif:
			break;
		case e_initSCUPreset::find:
			scu_p.addPresentationContext(UID_FINDPatientRootQueryRetrieveInformationModel, ts);
			scu_p.addPresentationContext(UID_FINDStudyRootQueryRetrieveInformationModel, ts);
			break;
		case e_initSCUPreset::move:
			scu_p.addPresentationContext(UID_MOVEPatientRootQueryRetrieveInformationModel, ts);
			scu_p.addPresentationContext(UID_MOVEStudyRootQueryRetrieveInformationModel, ts);
			break;
		case e_initSCUPreset::get:
			scu_p.addPresentationContext(UID_GETPatientRootQueryRetrieveInformationModel, ts);
			scu_p.addPresentationContext(UID_GETStudyRootQueryRetrieveInformationModel, ts);
			for (Uint16 j = 0; j < numberOfDcmLongSCUStorageSOPClassUIDs; j++)
			{
				//dcmLongSCUStorageSOPClassUIDs;
				scu_p.addPresentationContext(dcmLongSCUStorageSOPClassUIDs[j], ts, ASC_SC_ROLE_SCP);
			}
			break;
		default:
			break;
		}

		/* Initialize network */
		//result = scu_p.initNetwork();
		//if (result.bad())
		if (scu_p.initNetwork().bad())
		{
#ifdef dcmtklog
			DCMNET_ERROR("Unable to set up the network: " << result.text());
#endif // dcmtklog
			return false;
		}

		/* Negotiate Association */
		//result = scu_p.negotiateAssociation();
		//if (result.bad())
		if (scu_p.negotiateAssociation().bad())
		{
#ifdef dcmtklog
			DCMNET_ERROR("Unable to negotiate association: " << result.text());
#endif // dcmtklog
			return false;
		}

		/* Let's look whether the server is listening:
		Assemble and send C-ECHO request
		*/
		//result = scu_p.sendECHORequest(0);
		//if (result.bad())
		if (scu_p.sendECHORequest(0).bad())
		{
#ifdef dcmtklog
			DCMNET_ERROR("Could not process C-ECHO with the server: " << result.text());
#endif // dcmtklog
			return false;
		}

		//if (scu_p.abortAssociation().bad())
		if (scu_p.releaseAssociation().bad())
		//if (scu_p.closeAssociation(DCMSCU_RELEASE_ASSOCIATION).bad())
		{
#ifdef dcmtklog
			DCMNET_ERROR("Could not process C-ECHO with the server: " << result.text());
#endif // dcmtklog
			return false;
		}

		return true;

	}


	void fill_wrklst(DcmSCU_XRAD &scu, e_dcm_hierarchy_level_t dcmLev, list<elemsmap_t> &wrk_lst)
	{
		// if it is clear after this function it means that function did not get data or could not work correctly
		list<elemsmap_t> currListInsts;
		std::swap(currListInsts, wrk_lst);
		OFCondition result;
		string tmpStr;
		set<tag_e> tagsLst;

		auto it = currListInsts.begin();
		do
		{
			/* Assemble and send C-FIND request */
			OFList<QRResponse*> findResponses;
			DcmDataset req;
			switch (dcmLev)
			{
			case e_dcm_hierarchy_level_t::patient:
				req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "PATIENT");
				break;

			case e_dcm_hierarchy_level_t::study:
				req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "STUDY");
				break;

			case e_dcm_hierarchy_level_t::series:
				req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "SERIES");
				break;

			case e_dcm_hierarchy_level_t::stack:
				req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "IMAGE");
				break;

			case e_dcm_hierarchy_level_t::acquisition:
				req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "IMAGE");
				break;

			case e_dcm_hierarchy_level_t::instance:
				req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "IMAGE");
				break;

			default:
				return;
				break;
			}
			switch (dcmLev)
			{
			case e_dcm_hierarchy_level_t::instance:
				tagsLst.insert(e_instance_number);
				tagsLst.insert(e_sop_instance_uid);

			case e_dcm_hierarchy_level_t::acquisition:
				tagsLst.insert(e_acquisition_number);
				tagsLst.insert(e_acquisition_date);
				tagsLst.insert(e_acquisition_date_time);

			case e_dcm_hierarchy_level_t::stack:
				tagsLst.insert(e_stack_id);

			case e_dcm_hierarchy_level_t::series:
				tagsLst.insert(e_modality);
				tagsLst.insert(e_series_number);
				tagsLst.insert(e_series_instance_uid);

			case e_dcm_hierarchy_level_t::study:
				tagsLst.insert(e_study_id);
				tagsLst.insert(e_accession_number);
				tagsLst.insert(e_study_time);
				tagsLst.insert(e_study_date);
				tagsLst.insert(e_study_instance_uid);

			case e_dcm_hierarchy_level_t::patient:
				tagsLst.insert(e_patient_id);
				//note (Kovbas) думаю, теги ниже не нужны при скачивании исследований
				//?tagsLst.insert(e_patient_name);
				//?tagsLst.insert(e_issuer_of_patient_id);
				break;

			default:
				return;
				break;
			}
			for (auto el : tagsLst)
				req.putAndInsertOFStringArray(element_id_to_DcmTag(el), "");

			if (!currListInsts.empty())
				for (auto el : *it)
					req.putAndInsertOFStringArray(element_id_to_DcmTag(el.first), convert_to_string(el.second));

			//?dcmDatasetToLog(&req, "");

			T_ASC_PresentationContextID presID = findUncompressedPC(UID_FINDPatientRootQueryRetrieveInformationModel, scu);
			if (presID == 0)
			{
#ifdef dcmtklog
				DCMNET_ERROR("There is no uncompressed presentation context for Study Root FIND");
#endif
				//return 1;
				return;
			}
			result = scu.sendFINDRequest(presID, &req, &findResponses);
			if (result.bad())
				return;
			else
			{
				for (auto el : findResponses)
				{
					if (el->m_dataset != NULL)
					{
						//el->m_dataset->print(ss);
						//ShowText(L"", convert_to_wstring(ss.str()));
						elemsmap_t tmpLst;
						for (auto el1 : tagsLst)
						{
							el->m_dataset->findAndGetOFStringArray(element_id_to_DcmTag(el1), tmpStr);
							tmpLst.insert(elem_t(el1, convert_to_wstring(tmpStr)));
						}
						wrk_lst.push_back(tmpLst);
					}
				}
			}
			if (it == currListInsts.end() || ++it == currListInsts.end())
				break;
		} while (true);
	}
/*
	string getStringFromDcmObj(DcmDataset &dcmDS, const DcmTagKey &dcmTagKey, const string &default_value = "")
	{
		OFString tmpStr;
		if (dcmDS.findAndGetOFStringArray(dcmTagKey, tmpStr).good())
			return tmpStr;
		else
			default_value;
	}
*/

	void fillListOfInstances(DcmSCU_XRAD &scu, const list<elemsmap_t> &filter, list<elemsmap_t> &wrkLst)
	{
		e_dcm_hierarchy_level_t currLev = e_dcm_hierarchy_level_t::patient;
		if (wrkLst.empty())
			for (auto el : filter)
				wrkLst.push_back(el);
		else
			for (auto el : filter)
				for (auto &el1 : wrkLst)
					for (auto el2 : el)
						el1.insert(el2);
		fill_wrklst(scu, currLev, wrkLst);

		currLev = e_dcm_hierarchy_level_t::study;
		if (wrkLst.empty())
			for (auto el : filter)
				wrkLst.push_back(el);
		else
			for (auto el : filter)
				for (auto &el1 : wrkLst)
					for (auto el2 : el)
						el1.insert(el2);
		fill_wrklst(scu, currLev, wrkLst);

		currLev = e_dcm_hierarchy_level_t::series;
		if (wrkLst.empty())
			for (auto el : filter)
				wrkLst.push_back(el);
		else
			for (auto el : filter)
				for (auto &el1 : wrkLst)
					for (auto el2 : el)
						el1.insert(el2);
		fill_wrklst(scu, currLev, wrkLst);

		currLev = e_dcm_hierarchy_level_t::instance;
		if (wrkLst.empty())
			for (auto el : filter)
				wrkLst.push_back(el);
		else
			for (auto el : filter)
				for (auto &el1 : wrkLst)
					for (auto el2 : el)
						el1.insert(el2);
		fill_wrklst(scu, currLev, wrkLst);
	}


	bool downloadInstance(DcmSCU_XRAD &scu, const instancestorage_pacs &inst_pacs_src_in, DcmFileFormat &dcmFileFormat)
	{
		DcmStorageSCP_XRAD scp;
		Uint8 presID;
		OFCondition result;

		// set folder for download
		//std::string dstPath("D:/_kovbas/tmp/_0");
		//if (!DirectoryExists(dstPath))
		//	if (!CreatePath(dstPath))
		//		throw ("\"" + dstPath + "\" is not exist. Cannot create the folder.");
		//scp.setOutputDirectory(dstPath);
		scp.setOutputDcmFileFormat(dcmFileFormat);
		//scp.setOutputDirectory("D:/_kovbas/tmp/_0");
		//scp.setDatasetStorageMode(DcmStorageSCP::DGM_StoreBitPreserving);

		// load cfg for SCP
		if (scp.loadAssociationCfgFile(GetApplicationDirectory() + "/storescp.cfg").bad())
			throw ("Cannot download Cfg for SCP from \"" + GetApplicationDirectory() + "/storescp.cfg" + "\"");

		// start listener for getting RQ and data from remote hosts
		auto startListener = [&scp](bool stopAfterCurrentAssociation)
		{
			scp.setStopAfterCurrentAssociation(stopAfterCurrentAssociation);
			scp.listen();
		};

		/* Assemble and send C-MOVE request, for each study identified above*/
		presID = findUncompressedPC(UID_MOVEPatientRootQueryRetrieveInformationModel, scu);

		if (presID == 0)
		{
#ifdef dcmtklog
			DCMNET_ERROR("There is no uncompressed presentation context for Study Root MOVE");
#endif // dcmtklog
			return false;
		}

		DcmDataset req;
		//list<set<elem_t> > objs;
		//getListOfObjects(scu, e_instance, params, objs);
		//if (!objs.empty())
		{
			//for (auto inst : objs)
			{
				//req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "IMAGE");
				for (auto el : inst_pacs_src_in.elems())
					req.putAndInsertOFStringArray(element_id_to_DcmTag(el.first), convert_to_string(el.second));
				req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "IMAGE");

				std::thread t1(startListener, true);

				result = scu.sendMOVERequest(presID, scu.getAETitle(), &req, NULL);//&resp);// /* we are not interested into responses*/);

				if (result.good())
				{
#ifdef dcmtklog
					DCMNET_INFO("Received study ttt#" << std::setw(7) << studyCount << ": " << instUID);
#endif
				}
				t1.join();
				return result.good();
				//dcmFileFormat = move(scp.dcmFileFormat());
			}
		}
	}












	// on DCMTK classes ----------------------------------------------------------------------------------------------------

	e_dcm_hierarchy_level_t recognizeInstanceLevel(const DcmDataset &instance_p)
	{
		e_dcm_hierarchy_level_t level{e_dcm_hierarchy_level_t::unknown};
		DcmDataset &req = const_cast<DcmDataset&>(instance_p); //note (Kovbas) const_cast, т.к. методы DcmDataset, не изменяющие объект, не имеют const (не соблюдена const-корректность)

		if (req.isEmpty()) return level;
		OFString tmpStr;
		if (req.findAndGetOFStringArray(element_id_to_DcmTag(e_patient_id), tmpStr).good())
			level = e_dcm_hierarchy_level_t::patient;

		if (req.findAndGetOFStringArray(element_id_to_DcmTag(e_study_instance_uid), tmpStr).good())
			level = e_dcm_hierarchy_level_t::study;
		else
			return level;

		if (req.findAndGetOFStringArray(element_id_to_DcmTag(e_series_instance_uid), tmpStr).good())
			level = e_dcm_hierarchy_level_t::series;
		else
			return level;

		if (req.findAndGetOFStringArray(element_id_to_DcmTag(e_stack_id), tmpStr).good())
			level = e_dcm_hierarchy_level_t::stack;

		if (req.findAndGetOFStringArray(element_id_to_DcmTag(e_acquisition_number), tmpStr).good())
			level = e_dcm_hierarchy_level_t::acquisition;

		if (req.findAndGetOFStringArray(element_id_to_DcmTag(e_sop_instance_uid), tmpStr).good())
			level = e_dcm_hierarchy_level_t::instance;

		return level;
	}


	void normalizeDcmDatasetLst(const datasource_pacs &datasource_p, e_dcm_hierarchy_level_t level_p, list<DcmDataset> &wrkLst)
	{
		//DcmSCU_XRAD &scuRef = const_cast<DcmSCU_XRAD&>(scu); //note (Kovbas) сделано так, т.к. DCMTK не const-корректна
		// if it is clear after this function it means that function did not get data or could not work correctly
		list<DcmDataset> currInstLst{ move(wrkLst) };

		auto it = currInstLst.begin();
		do
		{
			/* Assemble and send C-FIND request */
			set<tag_e> tagsLst;
			switch (level_p)
			{
			case e_dcm_hierarchy_level_t::instance:
				tagsLst.insert(e_instance_number);
				tagsLst.insert(e_sop_instance_uid);

			case e_dcm_hierarchy_level_t::acquisition:
				tagsLst.insert(e_acquisition_number);
				//tagsLst.insert(e_acquisition_date);
				//tagsLst.insert(e_acquisition_date_time);

			case e_dcm_hierarchy_level_t::stack:
				tagsLst.insert(e_stack_id);

			case e_dcm_hierarchy_level_t::series:
				tagsLst.insert(e_modality);
				tagsLst.insert(e_series_number);
				tagsLst.insert(e_series_instance_uid);

			case e_dcm_hierarchy_level_t::study:
				tagsLst.insert(e_study_id);
				tagsLst.insert(e_accession_number);
				tagsLst.insert(e_study_time);
				tagsLst.insert(e_study_date);
				tagsLst.insert(e_study_instance_uid);

			case e_dcm_hierarchy_level_t::patient:
				tagsLst.insert(e_patient_id);
				tagsLst.insert(e_patient_name);
				//tagsLst.insert(e_issuer_of_patient_id);
				break;

			default:
				return;
				break;
			}

			DcmDataset req;
			for (auto el : tagsLst)
				req.putAndInsertOFStringArray(element_id_to_DcmTag(el), "");

			/*if (!currInstLst.empty())
				for (auto el : it)
					req.putAndInsertOFStringArray(element_id_to_DcmTag(el.first), convert_to_string(el.second));*/
			DcmStack stack;
			while (it->nextObject(stack, OFTrue).good())
			{
#if 0
				req.insert(dynamic_cast<DcmElement*>(stack.top()->clone()), OFTrue);
#else
				if (level_p != e_dcm_hierarchy_level_t::instance)
					req.insert(dynamic_cast<DcmElement*>(stack.top()->clone()), OFTrue);
				else
					if (stack.top()->getTag() == DCM_SOPInstanceUID ||
						stack.top()->getTag() == DCM_AcquisitionNumber ||
						stack.top()->getTag() == DCM_StackID ||
						stack.top()->getTag() == DCM_SeriesInstanceUID ||
						stack.top()->getTag() == DCM_StudyInstanceUID ||
						stack.top()->getTag() == DCM_PatientID)
						req.insert(dynamic_cast<DcmElement*>(stack.top()->clone()), OFTrue);
#endif
			}

			switch (level_p)
			{
			case e_dcm_hierarchy_level_t::patient:
				req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "PATIENT");
				break;
			case e_dcm_hierarchy_level_t::study:
				req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "STUDY");
				break;
			case e_dcm_hierarchy_level_t::series:
				req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "SERIES");
				break;
			case e_dcm_hierarchy_level_t::stack:
				req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "IMAGE");
				break;
			case e_dcm_hierarchy_level_t::acquisition:
				req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "IMAGE");
				break;
			case e_dcm_hierarchy_level_t::instance:
				req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "IMAGE");
				break;
			default:
				return;
				break;
			}

			//?dcmDatasetToLog(&req, "query");
			DcmSCU_XRAD scu;
			if (!initSCUAndCheckPACSAccessibility(scu, datasource_p, e_initSCUPreset::find))
				return;
			DcmSCU_XRAD_wrapper scu_wrap(scu);
			if (!scu_wrap.initConnection())
				return;

			T_ASC_PresentationContextID presID;
			if (level_p == e_dcm_hierarchy_level_t::patient)
				presID = findUncompressedPC(UID_FINDPatientRootQueryRetrieveInformationModel, scu);
			else
				presID = findUncompressedPC(UID_FINDStudyRootQueryRetrieveInformationModel, scu);
			if (presID == 0)
			{
#ifdef dcmtklog
				DCMNET_ERROR("There is no uncompressed presentation context for Study Root FIND");
				return 1;
#else
				return;
#endif
			}
			OFList<QRResponse*> findResponses;
			if (scu.sendFINDRequest(presID, &req, &findResponses).bad())
				return;

			for (auto el : findResponses)
			{
				if (el->m_dataset != NULL)
				{
					//el->m_dataset->print(ss);
					//ShowText(L"", convert_to_wstring(ss.str()));
					//elemsmap_t tmpLst;
					/*for (auto el1 : tagsLst)
					{
						el->m_dataset->findAndGetOFStringArray(element_id_to_DcmTag(el1), tmpStr);
						tmpLst.insert(elem_t(el1, convert_to_wstring(tmpStr)));
					}*/
					//wrk_lst.push_back(tmpLst);
					wrkLst.push_back(*el->m_dataset);
					//?dcmDatasetToLog(el->m_dataset, "answer");
				}
			}

			//if (it == currInstLst.end() || ++it == currInstLst.end())
				//break;
		//} while (true);
		} while (!(it == currInstLst.end() || ++it == currInstLst.end()));
	}

	namespace
	{
		/*inline auto checkID(const list<DcmDataset> & wrkLst_p)
		{


		}*/

		inline auto surveyGottenID(const DcmDataset& currentDataset, const list<DcmDataset> & wrkLst_p, e_dcm_hierarchy_level_t currentLevel_p)
		{
			if (wrkLst_p.empty())
				printf("There is no %ls(s) with such ID.\n", dcm_hierarchy_level_names[currentLevel_p].c_str());
			else
			{
				if (wrkLst_p.size() > 1)
					printf("There are %zu %ls(s) with such ID.\n", wrkLst_p.size(), dcm_hierarchy_level_names[currentLevel_p].c_str());
				//checkID(wrkLst_p);
				//проверка на то, что ID остался прежним
				for (auto el : wrkLst_p)
					if (false) //todo (Kovbas) Здесь должна быть проверка на то, что ID остался тот же
						printf("");
			}
		}
	}

	list<DcmDataset> findAndPrepareDcmDatasetLst(const datasource_pacs &datasource_p, e_dcm_hierarchy_level_t level_p, const DcmDataset &dcmDataset_p, ProgressProxy pp_p)
	{
		list<DcmDataset> resLstTmp;
		ProgressBar	progress(pp_p);
		wstring msg{ L"Preparing studies to download."};
		//progress.start(msg, dcmDatasetLst_p.size());
		//list<DcmDataset> dcmDatasetLst_p;
		//for (auto el : dcmDatasetLst_p)
		auto lambda = [&resLstTmp, level_p, &datasource_p](const DcmDataset &dcmDataset_p)
		{
			//++progress;
			list<DcmDataset> wrkLst{ dcmDataset_p };

			e_dcm_hierarchy_level_t currentLevel{ e_dcm_hierarchy_level_t::patient };
			if (level_p == e_dcm_hierarchy_level_t::patient)
			{
				normalizeDcmDatasetLst(datasource_p, currentLevel, wrkLst);
				surveyGottenID(dcmDataset_p, wrkLst, currentLevel);

				if (currentLevel == level_p)
				{
					for (auto wrkEl : wrkLst)
					{
						resLstTmp.push_back(wrkEl);
					}
					return;
				}
			}

			currentLevel = e_dcm_hierarchy_level_t::study;
			normalizeDcmDatasetLst(datasource_p, currentLevel, wrkLst);
			surveyGottenID(dcmDataset_p, wrkLst, currentLevel);

			if (currentLevel == level_p)
			{
				for (auto wrkEl : wrkLst)
				{
					resLstTmp.push_back(wrkEl);
				}
				return;
			}

			currentLevel = e_dcm_hierarchy_level_t::series;
			normalizeDcmDatasetLst(datasource_p, currentLevel, wrkLst);
			if (currentLevel == level_p)
			{
				for (auto wrkEl : wrkLst)
				{
					resLstTmp.push_back(wrkEl);
				}
				return;
			}

			currentLevel = e_dcm_hierarchy_level_t::instance;
			normalizeDcmDatasetLst(datasource_p, currentLevel, wrkLst);
			for (auto wrkEl : wrkLst)
					resLstTmp.push_back(wrkEl);

		};
		//clear unnecessary elements from DcmDatasets
		lambda(dcmDataset_p);
		list<DcmDataset> resLst;
		for (auto el : resLstTmp)
		{
/*
			Patient's Name (0010,0010) R
			Patient ID(0010, 0020) U

			Study Date(0008, 0020) R
			Study Time(0008, 0030) R
			Accession Number(0008, 0050) R
			Study ID(0020, 0010) R
			Study Instance UID(0020, 000D) U

			Modality(0008, 0060) R
			Series Number(0020, 0011) R
			Series Instance UID(0020, 000E) U

			Stack ID(0020, 9056) O
			Acquisition Number(0020, 0012) O

			Instance Number(0020, 0013) R
			SOP Instance UID(0008, 0018) U
*/
			DcmStack tmpStack;
			DcmDataset tmpDcmDS;
			while (el.nextObject(tmpStack, OFTrue).good())
			{
				auto tmpTag{ tmpStack.top()->getTag() };
				if (tmpTag == DCM_PatientID ||
					tmpTag == DCM_StudyInstanceUID ||
					tmpTag == DCM_SeriesInstanceUID ||
					tmpTag == DCM_StackID ||
					tmpTag == DCM_AcquisitionNumber ||
					tmpTag == DCM_SOPInstanceUID)
					tmpDcmDS.insert(dynamic_cast<DcmElement*>(tmpStack.top()->clone()), OFTrue);
			}
			resLst.push_back(move(tmpDcmDS));
		}
		return resLst;
	}


	bool common_downloader_cget(const datasource_pacs& datasource_pacs_p, const DcmDataset &instance_info_p, const wstring &destination_folder)
	{
		wstring dstPath(destination_folder);
		if (!DirectoryExists(dstPath))
		{
			if(!CreatePath(dstPath))
			{
				throw invalid_argument("\"" + convert_to_string(dstPath) + "\" does not exist. Could not create the folder.");
			}
		}

		if (!dcmDataDict.isDictionaryLoaded())
		{
			return false;
		}
			//OFList<OFString> syntaxes;


			//DcmXfer xfer(EXS_Unknown);
		 //     if (xfer.isEncapsulated())
		 //     {
		 //       syntaxes.push_back(xfer.getXferID());
		 //     }
		 //     /* We prefer explicit transfer syntaxes.
		 //      * If we are running on a Little Endian machine we prefer
		 //      * LittleEndianExplicitTransferSyntax to BigEndianTransferSyntax.
		 //      */
		 //     if (gLocalByteOrder == EBO_LittleEndian)  /* defined in dcxfer.h */
		 //     {
		 //       syntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
		 //       syntaxes.push_back(UID_BigEndianExplicitTransferSyntax);
		 //     } else
		 //     {
		 //       syntaxes.push_back(UID_BigEndianExplicitTransferSyntax);
		 //       syntaxes.push_back(UID_LittleEndianExplicitTransferSyntax);
		 //     }
		 //     syntaxes.push_back(UID_LittleEndianImplicitTransferSyntax);


		DcmDataset req{ instance_info_p };
		switch(recognizeInstanceLevel(instance_info_p))
		{
		case e_dcm_hierarchy_level_t::patient:
			req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "PATIENT");
			break;
		case e_dcm_hierarchy_level_t::study:
			req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "STUDY");
			break;
		case e_dcm_hierarchy_level_t::series:
			req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "SERIES");
			break;
		case e_dcm_hierarchy_level_t::stack:
			req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "IMAGE");
			break;
		case e_dcm_hierarchy_level_t::acquisition:
			req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "IMAGE");
			break;
		case e_dcm_hierarchy_level_t::instance:
			req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "IMAGE");
			break;
		default:
			return false;
		}
		/*
		auto applyOverrideKeys = [](DcmDataset *dataset)
		{
			OFList<OFString> overrideKeys;
			// replace specific keys by those in overrideKeys
			OFListConstIterator(OFString) path = overrideKeys.begin();
			OFListConstIterator(OFString) endOfList = overrideKeys.end();
			DcmPathProcessor proc;
			proc.setItemWildcardSupport(OFFalse);
			proc.checkPrivateReservations(OFFalse);
			OFCondition cond;
			while (path != endOfList)
			{
				cond = proc.applyPathWithValue(dataset, *path);
				if (cond.bad())
				{
					//OFLOG_ERROR(getscuLogger, "Bad override key/path: " << *path << ": " << cond.text());
				}
				path++;
			}
		};
		applyOverrideKeys(&req);
		*/

		// minimal presentation contexts for initialization
		//OFList<OFString> ts;
		//ts.push_back(UID_LittleEndianExplicitTransferSyntax);
		//ts.push_back(UID_BigEndianExplicitTransferSyntax);
		//ts.push_back(UID_LittleEndianImplicitTransferSyntax);
		////note Kovbas: we add all necessary Presentation Contexts here. It is possible that the better solution is to add them where they are used but for adding another PresCont you have to disable InitNet and NegAssoc
		//scu.addPresentationContext(UID_GETPatientRootQueryRetrieveInformationModel, ts);
		//scu.addPresentationContext(UID_GETStudyRootQueryRetrieveInformationModel, ts);
		//initSCU();

		//for (Uint16 j = 0; j < numberOfDcmLongSCUStorageSOPClassUIDs; j++)
		//{
		//	dcmLongSCUStorageSOPClassUIDs;
		//	//scu.addPresentationContext(dcmLongSCUStorageSOPClassUIDs[j], ts, ASC_SC_ROLE_SCP);
		//}

		DcmSCU_XRAD scu;
		scu.setMaxReceivePDULength(ASC_DEFAULTMAXPDU);
		scu.setACSETimeout(30);
		scu.setDIMSEBlockingMode(DIMSE_BLOCKING);
		scu.setDIMSETimeout(0);
		scu.setVerbosePCMode(OFFalse);
		scu.setStorageMode(DCMSCU_STORAGE_DISK);
		scu.setStorageDir(convert_to_string(destination_folder));
		if (!initSCUAndCheckPACSAccessibility(scu, datasource_pacs_p, e_initSCUPreset::get))
			return false;

		DcmSCU_XRAD_wrapper scu_wrap(scu);
		if (!scu_wrap.initConnection())
			return false;

		//cond = EC_Normal;
		T_ASC_PresentationContextID pcid = scu.findPresentationContextID(UID_GETStudyRootQueryRetrieveInformationModel, "");
		if (pcid == 0)
		{
			// OFLOG_FATAL(getscuLogger, "No adequate Presentation Contexts for sending C-GET");
			 //exit(1);
		}

		//printDcmDatasetToLog(&req, "");
		OFList<RetrieveResponse*> responses;
		OFCondition cond = scu.sendCGETRequest(pcid, &req, &responses);
		if (responses.empty() ||
			(responses.back()->m_numberOfFailedSubops > 0) ||
			((responses.front()->m_numberOfRemainingSubops + responses.front()->m_numberOfCompletedSubops) != responses.back()->m_numberOfCompletedSubops))
			return false;

		return cond.good();
	}

	bool common_downloader_cmove(const datasource_pacs& datasource_pacs_p, const DcmDataset &instance_info, const wstring &destination_folder)
	{
		//провека того, какого уровня запрос делать. Если не определён, то выходим с ошибкой.
		e_dcm_hierarchy_level_t level(recognizeInstanceLevel(instance_info));
		if (level == e_dcm_hierarchy_level_t::unknown) return false;

		DcmSCU_XRAD scu;
		//initSCUandCheckPACSAccessibility(scu_p, datasource_pacs_p);
		initSCUAndCheckPACSAccessibility(scu, datasource_pacs_p, e_initSCUPreset::move);

		OFCondition result;

		// set destination place
#if 1 //folder
		wstring dstPath(destination_folder);
		if (!DirectoryExists(dstPath))
			if (!CreatePath(dstPath))
				throw ("\"" + convert_to_string(dstPath) + "\" is not exist. Could not create the folder.");
		DcmStorageSCP_XRAD scp;
		scp.setOutputDirectory(convert_to_string(dstPath));
#else //memory
		scp.setOutputDcmFileFormat(dcmFileFormat);
#endif
		//scp.setDatasetStorageMode(DcmStorageSCP::DGM_StoreBitPreserving);

		// load cfg for SCP
		if (scp.loadAssociationCfgFile(GetApplicationDirectory() + "/storescp.cfg").bad())
			throw ("Cannot download Cfg for SCP from \"" + GetApplicationDirectory() + "/storescp.cfg" + "\"");

		// start listener for getting RQ and data from remote hosts
		auto startListener = [&scp](bool stopAfterCurrentAssociation)
		{

			scp.setStopAfterCurrentAssociation(stopAfterCurrentAssociation);
			scp.listen();
		};

		/* Assemble and send C-MOVE request, for each study identified above*/
		Uint8 presID;
		//presID = findUncompressedPC(UID_MOVEPatientRootQueryRetrieveInformationModel, scu);
		presID = findUncompressedPC(UID_GETStudyRootQueryRetrieveInformationModel, scu);

		if (presID == 0)
		{
#ifdef dcmtklog
			DCMNET_ERROR("There is no uncompressed presentation context for Study Root MOVE");
#endif // dcmtklog
			return false;
		}


		//list<set<elem_t> > objs;
		//getListOfObjects(scu, e_instance, params, objs);
		//if (!objs.empty())
		{
			//for (auto inst : objs)
			{
				//req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "IMAGE");
				//for (auto el : filter)
					//req.putAndInsertOFStringArray(element_id_to_DcmTag(el.first), convert_to_string(el.second));
				DcmDataset req(instance_info);

				switch (level)
				{
				case e_dcm_hierarchy_level_t::patient:
					req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "PATIENT");
					break;
				case e_dcm_hierarchy_level_t::study:
					req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "STUDY");
					break;
				case e_dcm_hierarchy_level_t::series:
					req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "SERIES");
					break;
				case e_dcm_hierarchy_level_t::stack:
					req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "IMAGE");
					break;
				case e_dcm_hierarchy_level_t::acquisition:
					req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "IMAGE");
					break;
				case e_dcm_hierarchy_level_t::instance:
					req.putAndInsertOFStringArray(DCM_QueryRetrieveLevel, "IMAGE");
					break;
				default:
					return false;
				}


				std::thread scpListener(startListener, true);

				result = scu.sendMOVERequest(presID, scu.getAETitle(), &req, NULL);//&resp);// /* we are not interested into responses*/);
				//result = scu.sendCGETRequest(presID, &req, NULL);//&resp);// /* we are not interested into responses*/);

				if (result.good())
				{
#ifdef dcmtklog
					DCMNET_INFO("Received study ttt#" << std::setw(7) << studyCount << ": " << instUID);
#endif
				}
				scpListener.join();
				return result.good();
				//dcmFileFormat = move(scp.dcmFileFormat());
			}
		}
	}

	bool common_downloader(const datasource_pacs& datasource_pacs_p, const DcmDataset &instance_info, const wstring &destination_folder)
	{
		switch (datasource_pacs_p.request_type())
		{
		case e_request_t::cget:
			return common_downloader_cget(datasource_pacs_p, instance_info, destination_folder);
		case e_request_t::cmove:
			return common_downloader_cmove(datasource_pacs_p, instance_info, destination_folder);
		default:
			return false;
		}
	}







	// send
	bool sendInstances()
	{
		DcmStorageSCU storeSCU, scu;
		OFCondition result;
		pacs_params_t pacs_params(L"10.1.2.155", 5104, L"PACS_AE", L"XRAD_SCU", 104);
#if 0
		scu.setAETitle(convert_to_string(src_pacs.AETitle_loc()));
		scu.setPeerHostName(convert_to_string(src_pacs.address()));
		scu.setPeerPort(static_cast<Uint16>(src_pacs.port()));
		scu.setPeerAETitle(convert_to_string(src_pacs.AETitle()));
#else
		scu.setAETitle(convert_to_string(pacs_params.AETitle_local()));
		scu.setPeerHostName(convert_to_string(pacs_params.address_pacs()));
		scu.setPeerPort(Uint16(pacs_params.port_pacs()));
		scu.setPeerAETitle(convert_to_string(pacs_params.AETitle_pacs()));
#endif
		scu.setAssocConfigFileAndProfile(GetApplicationPath() + "/storescu.cfg", "Default");

		//// minimal presentation contexts for initialization
		//OFList<OFString> ts;
		//ts.push_back(UID_LittleEndianExplicitTransferSyntax);
		//ts.push_back(UID_BigEndianExplicitTransferSyntax);
		//ts.push_back(UID_LittleEndianImplicitTransferSyntax);
		////note Kovbas: we add all necessary Presentation Contexts here. It is possible that the better solution is to add them where they are used but for adding another PresCont you have to disable InitNet and NegAssoc
		//scu.addPresentationContext(UID_FINDPatientRootQueryRetrieveInformationModel, ts);
		//scu.addPresentationContext(UID_MOVEPatientRootQueryRetrieveInformationModel, ts);
		//scu.addPresentationContext(UID_VerificationSOPClass, ts);

		/* Initialize network */
		result = scu.initNetwork();
		if (result.bad())
		{
#ifdef dcmtklog
			DCMNET_ERROR("Unable to set up the network: " << result.text());
#endif // dcmtklog
			return false;
		}

		/* Negotiate Association */
		result = scu.negotiateAssociation();
		if (result.bad())
		{
#ifdef dcmtklog
			DCMNET_ERROR("Unable to negotiate association: " << result.text());
#endif // dcmtklog
			return false;
		}
//
//		/* Let's look whether the server is listening:
//		Assemble and send C-ECHO request
//		*/
//		result = scu.sendECHORequest(0);
//		if (result.bad())
//		{
//#ifdef dcmtklog
//			DCMNET_ERROR("Could not process C-ECHO with the server: " << result.text());
//#endif // dcmtklog
//			return false;
//		}
//


		scu.addDicomFile("D:/_kovbas/dicom_samples/_0 369 monoframes/1.2.392.200036.9116.2.5.1.37.2418751745.1500464052.61813.dcm");

		auto res = scu.sendSOPInstances();
		scu.createReportFile("D:/_kovbas/tmp/_storescu_tests/rep.txt");

		if (res.good())
			return true;
		else
			return false;



	}



} //end namespace Dicom


XRAD_END