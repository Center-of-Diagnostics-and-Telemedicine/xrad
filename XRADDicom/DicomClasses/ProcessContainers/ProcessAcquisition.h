/*!
	* \file ProcessAcquisition.h
	* \date 4/19/2018 2:27:56 PM
	*
	* \author kovbas
	*
	* \brief
	*
	* TODO: long description
	*
	* \note
*/
#ifndef ProcessAcquisition_h__
#define ProcessAcquisition_h__


#include <XRADDicom/DicomClasses/XRADDicomGeneral.h>
#include <XRADDicom/DicomClasses/Instances/instance.h>
#include <XRADDicom/DicomClasses/Instances/tomogram_slice.h>

XRAD_BEGIN

class ProcessAcquisition
{
public:
	ProcessAcquisition(const shared_ptr<Dicom::acquisition_loader> &acquisition_loader_p)
		: m_acquisition_loader{ acquisition_loader_p }
	{
		//for(auto &instance: *m_acquisition_loader) instance->open_instancestorage();
	}
	//destructor
	virtual ~ProcessAcquisition()
	{
		//for(auto &instance: *m_acquisition_loader) instance->close_instancestorage();
	}

	shared_ptr<Dicom::acquisition_loader> get_loader() { return m_acquisition_loader; }
	Dicom::acquisition_loader &loader() { return *m_acquisition_loader; }
	const Dicom::acquisition_loader &loader() const { return *m_acquisition_loader; }

	//gets
	virtual std::string classname() const = 0;// { return "ProcessAcquisition"; }

	virtual size_t n_elements() const = 0;

	size_t	n_instances() const { return n_elements(); }

	//! \details При ошибке кидает исключение и оставляет инстансы закрытыми.
	virtual void open_instancestorages() { m_acquisition_loader->open_instancestorages(); }
		//! \details Не кидает исключения.
	virtual void close_instancestorages() { m_acquisition_loader->close_instancestorages(); }
	//-virtual void get_element_from_instance(const Dicom::instance &instance, const size_t element_num);
	virtual void put_elements_to_instance(Dicom::instance &instance, size_t element_number) const = 0;
	virtual void save_to_file(const wstring &fold_path, e_saving_decision_options_t saving_decision, Dicom::e_compression_type_t compression, ProgressProxy pp) const;


	virtual wstring modality() const final { return m_acquisition_loader->front()->modality(); }
	virtual wstring manufacturer() const final { return m_acquisition_loader->front()->manufacturer(); }
	virtual wstring accession_number() const final { return m_acquisition_loader->front()->accession_number(); }
	wstring get_dicom_file_content(size_t num_of_frame_p = 0, bool = false) const;
	/*
	const std::vector<wstring> &file_paths() const { return m_file_paths; }
	const std::vector<Dicom::instancestorage_ptr> &instancesources() const { return m_instancesources; }
	*/
	wstring series_description() const { return m_acquisition_loader->front()->series_description(); }
	//+void set_series_description(const wstring &val) { m_series_description = val; }
	//-virtual void collect_generic_data(Dicom::instance_ptr &instance_ptr);
	//wstring summary_info() const { return summary_info_string; };
	//wstring summary_info() const { return summary_info_string; };
	wstring summary_info() const { return m_acquisition_loader->front()->get_summary_info_string(); };

protected:
	shared_ptr<Dicom::acquisition_loader> m_acquisition_loader;

};

typedef unique_ptr<ProcessAcquisition> ProcessAcquisition_ptr;

class ProcessAcquisitionOpenClose
{
public:
	ProcessAcquisitionOpenClose() = delete;
	ProcessAcquisitionOpenClose(ProcessAcquisition &processAcquisition_p)
		: m_processAcquisition(processAcquisition_p)
	{
		m_processAcquisition.open_instancestorages();
	}
	ProcessAcquisitionOpenClose(const ProcessAcquisitionOpenClose &) = delete;
	ProcessAcquisitionOpenClose &operator=(const ProcessAcquisitionOpenClose &) = delete;
	// TODO: Копирование объекта невозможно, но move-семантику можно реализовать.

	~ProcessAcquisitionOpenClose()
	{
		m_processAcquisition.close_instancestorages();
	}
private:
	ProcessAcquisition &m_processAcquisition;
};

//void set_file_objects_for_multiframe(Dicom::acquisition_loader&);

XRAD_END

#endif // ProcessAcquisition_h__