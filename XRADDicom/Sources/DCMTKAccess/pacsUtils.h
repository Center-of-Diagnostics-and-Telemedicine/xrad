/*!
	\file
	\date 10/10/2018 3:58:45 PM
	\author Kovbas (kovbas)
*/
#ifndef pacsUtils_h__
#define pacsUtils_h__

#include <XRADDicom/Sources/DicomClasses/XRADDicomGeneral.h>
#include <XRADDicom/Sources/DicomClasses/DataContainers/datasource.h>
#include <XRADDicom/Sources/DicomClasses/Instances/LoadGenericClasses.h>

#include <set>

#undef dcmtklog

XRAD_BEGIN

namespace Dicom
{
	// our implementation of DcmSCU according to DCMTK developers' advice
	class DcmSCU_XRAD : public DcmSCU
	{
	public:
		DcmSCU_XRAD() {}
		virtual ~DcmSCU_XRAD() {}
	};

	// wrapper for DcmSCU_XRAD to automate initialization and aborting assotiation
	struct DcmSCU_XRAD_wrapper
	{
		DcmSCU_XRAD_wrapper() = delete;
		DcmSCU_XRAD_wrapper(DcmSCU_XRAD &scu)
			: scuRef(scu)
		{}

		bool initConnection()
		{
			//OFCondition cond = scu.initNetwork();
		//if (cond.bad())
		//{
		//	//OFLOG_FATAL(getscuLogger, DimseCondition::dump(temp_str, cond));
		//	//exit(1);
		//}
		//cond = scu.negotiateAssociation();
		//if (cond.bad())
		//{
		//	//OFLOG_FATAL(getscuLogger, "No Acceptable Presentation Contexts");
		//	//exit(1);
		//}
			OFCondition cond;
			cond = scuRef.initNetwork();
			if (cond.bad())
				return false;

			cond = scuRef.negotiateAssociation();
			if (cond.bad())
				return false;

			return true;
		}

		~DcmSCU_XRAD_wrapper()
		{
			if (scuRef.releaseAssociation().bad())
				scuRef.abortAssociation();
		}

	private:
		DcmSCU_XRAD &scuRef;
	};


	// our implementation of DcmStorageSCP according to DCMTK developers' advice
	class DcmStorageSCP_XRAD : public DcmStorageSCP
	{
		/*
		https://forum.dcmtk.org/viewtopic.php?t=2947
		Hi,

		no, sorry, we do not have a sample application for DcmSCP available online.

		However, it should work something like this. First initialize your SCP:
		Code:
		DcmSCP scp;
		scp.setPort(8888);
		scp.setAETitle("MYSYSTEM");
		scp.addPresentationContext(...);
		scp.listen();

		Then, overwrite DcmSCPs function "handleIncomingCommand(...)" and call your own handler for whatever DIMSE message you like to handle. As an example, look at the handler for echo, which is in the function "handleECHORequest()".

		Best regards,
		Michael
		*/
		PARENT(DcmStorageSCP);
	public:

		DcmStorageSCP_XRAD()
			: m_fl_stopAfterCurrentAssociation(false)//,
			//storescpLogger(OFLog::getLogger("dcmtk.apps." "XRAD_SCP"))
		{
			m_dcmFileFormat = nullptr;
			if (!dcmDataDict.isDictionaryLoaded())
			{
#ifdef dcmtklog

				OFLOG_WARN(storescpLogger, "no data dictionary loaded, check environment variable: "
					<< DCM_DICT_ENVIRONMENT_VARIABLE);
#endif // dcmtklog

			}
			else
			{
#ifdef dcmtklog
				OFLOG_WARN(storescpLogger, "yes data dictionary loaded, check environment variable: "
					<< DCM_DICT_ENVIRONMENT_VARIABLE);
#endif // dcmtklog
			}

		}

		~DcmStorageSCP_XRAD() {}

		bool m_fl_stopAfterCurrentAssociation;
		DcmFileFormat *m_dcmFileFormat;

		void setOutputDcmFileFormat(DcmFileFormat &dcmFileFormat)
		{
			setOutputDirectory("");
			m_dcmFileFormat = &dcmFileFormat;
		}

		void setStopAfterCurrentAssociation(bool stopAfterCurrentAssociation_in) { m_fl_stopAfterCurrentAssociation = stopAfterCurrentAssociation_in; }

		virtual OFBool stopAfterConnectionTimeout()
		{
			setConnectionTimeout(0);
			if (getConnectionTimeout() == 0)
				return true;
			else
				return false;
		}

		virtual OFBool stopAfterCurrentAssociation()
		{
			return m_fl_stopAfterCurrentAssociation;
		}

		OFCondition setOutputDirectory(const OFString &directory) //note Kovbas реализовано здесь, чтобы избежать путаницы в том, куда сохранять скачанный файл
		{
			m_dcmFileFormat = nullptr;
			return parent::setOutputDirectory(directory);
		}

	protected:
		//OFLogger storescpLogger;
		//const string locAETitle = "XRAD_SCP";
		const string localAETitle = "XRAD_SCU";


		virtual Uint16 checkAndProcessSTORERequest(const T_DIMSE_C_StoreRQ &reqMessage,
			DcmFileFormat &fileformat) override //note Kovbas переопределена практически полным копированием из исходника. Сделано для наших нужд: сохранение полученного файла не только как файла на диске, но и как объект в памяти для дальнейшей работы с ним
		{
			Uint16 statusCode = STATUS_STORE_Error_CannotUnderstand;
			if (m_dcmFileFormat != nullptr && getOutputDirectory() == "")
			{
				//DCMNET_DEBUG("checking and processing C-STORE request");

				DcmDataset *dataset = fileformat.getDataset();
				// perform some basic checks on the request dataset
				if ((dataset != NULL) && !dataset->isEmpty())
				{
					//OFString filename;
					//OFString directoryName;
					//OFString sopClassUID = reqMessage.AffectedSOPClassUID;
					//OFString sopInstanceUID = reqMessage.AffectedSOPInstanceUID;
					// generate filename with full path
					//OFCondition status = generateDirAndFilename(filename, directoryName, sopClassUID, sopInstanceUID, dataset);
					//if (status.good())
					{
						//DCMNET_DEBUG("generated filename for received object: " << filename);
						// create the output directory (if needed)
						//status = OFStandard::createDirectory(directoryName, OutputDirectory /* rootDir */);
						//if (status.good())
						{
							//if (OFStandard::fileExists(filename))
								//DCMNET_WARN("file already exists, overwriting: " << filename);
							// store the received dataset to file (with default settings)
							//status = fileformat.saveFile(filename);
							*m_dcmFileFormat = move(fileformat);
							//if (status.good())
							{
								// call the notification handler (default implementation outputs to the logger)
								//notifyInstanceStored(filename, sopClassUID, sopInstanceUID, dataset);
								statusCode = STATUS_Success;
							}
							//else {
								//DCMNET_ERROR("cannot store received object: " << filename << ": " << status.text());
								//statusCode = STATUS_STORE_Refused_OutOfResources;

								// delete incomplete file
								//OFStandard::deleteFile(filename);
							//}
						}
						//else {
							//DCMNET_ERROR("cannot create directory for received object: " << directoryName << ": " << status.text());
							//statusCode = STATUS_STORE_Refused_OutOfResources;
						//}
					}
					//else
						//DCMNET_ERROR("cannot generate directory or file name for received object: " << status.text());
				}
				return statusCode;
			}
			else
			{
				return parent::checkAndProcessSTORERequest(reqMessage, fileformat);
			}
		}

	};



	enum class e_initSCUPreset
	{
		verif,
		find,
		move,
		get,
	};
	bool initSCUAndCheckPACSAccessibility(DcmSCU_XRAD &scu_p, const pacs_params_t &src_pacs_p, e_initSCUPreset preset_p);

	//void getListOfObjects(DcmSCU_XRAD &scu, e_dcm_hierarchy_level_t dcmLev, const set<elem_t> &params, list<set<elem_t> > &elems); //todo (Kovbas) хочу избавиться от этого
	void fillListOfInstances(DcmSCU_XRAD &scu, const list<Dicom::elemsmap_t> &filter, list<Dicom::elemsmap_t> &insts);
	//void fillListOfInstances(DcmSCU_XRAD &scu, const list<Dicom::elemsmap_t> &filter, instances &insts);

	bool downloadInstance(DcmSCU_XRAD &scu, const instancestorage_pacs &inst_pacs_src_in, DcmFileFormat &dcmFileFormat);

	e_dcm_hierarchy_level_t recognizeInstanceLevel(const DcmDataset &instance);
	//void normalizeDcmDatasetLst(DcmSCU_XRAD &scu, e_dcm_hierarchy_level_t level, list<DcmDataset> &wrkLst);
	//list<DcmDataset> fillDcmDatasetLst(DcmSCU_XRAD &scu, e_dcm_hierarchy_level_t level, const list<elemsmap_t> &filter);
	//list<DcmDataset> prepareDcmDatasetLst(const DcmSCU_XRAD &scu, e_dcm_hierarchy_level_t level, const list<DcmDataset> &filter);
	list<DcmDataset> findAndPrepareDcmDatasetLst(const datasource_pacs &datasource_p, e_dcm_hierarchy_level_t level, const DcmDataset &filter, ProgressProxy pp_p);
	bool common_downloader(const datasource_pacs&, const DcmDataset &instance_info, const wstring &destination_folder);  //todo (Kovbas) при приведении этой функции к общей для использования всеми заменить destination_folder на ссылку на тип, родитель для типа папка и объект DcmFileFormat в памяти

	 Uint8 findUncompressedPC(const OFString& sopClass, const DcmSCU& scu);

	bool sendInstances();

} //namespace Dicom


XRAD_END

#endif // pacsUtils_h__
