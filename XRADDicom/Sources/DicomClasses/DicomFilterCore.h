#ifndef DicomFilterCore_h__
#define DicomFilterCore_h__
/*!
	\file
	\date 2018/03/01 15:44
	\author kulberg
*/

#include "DicomPredicates.h"
#include "DicomProcessors.h"

XRAD_BEGIN



//!	Класс, предназначенный для удаления из Dicom набора instances, для которых m_predicate() = false
class InstanceFilter : public Dicom::AcquisitionProcessor<Dicom::acquisition_loader>
{
	dicom_instance_predicate	m_predicate;

public:

	virtual void	Apply(Dicom::acquisition_loader &acq, ProgressProxy pp) override
	{
		ProgressBar	progress(pp);
		progress.start("", acq.size());
		for(auto it = acq.begin(); it != acq.end();)
		{
			if(!m_predicate(*it))
			{
				it = acq.erase(it);
			}
			else ++it;
			++progress;
		}
	}

	InstanceFilter(const dicom_instance_predicate &in_pred) : m_predicate(in_pred){}
};



//!	Класс, предназначенный для удаления из Dicom набора acquisitions, для которых m_predicate() = false
class AcquisitionFilter : public Dicom::SeriesProcessor<Dicom::series_loader>
{
	dicom_acquisition_predicate	m_predicate;

	void	Apply(Dicom::series_loader &series, ProgressProxy pp)
	{
		ProgressBar	progress(pp);
		progress.start("", series.size());
		for(auto &stack: series)
		{

			for(auto it = stack.begin(); it != stack.end();)
			{
				if(!m_predicate(*it))
				{
					it = stack.erase(it);
					//it=stack.begin();
					//i=0;
				}
				else ++it;
			}
			++progress;
		}
	}

public:
	AcquisitionFilter(const dicom_acquisition_predicate &in_pred) : m_predicate(in_pred){}
};







//!	фильтрация полных наборов разных уровней (от списка пациентов до одного acquisition)
template<class FILTER_T, class PATIENT>
void	FilterDicoms(Dicom::patients<PATIENT> &studies_heap, shared_ptr<FILTER_T> filter, ProgressProxy pp = VoidProgressProxy())
{
	Dicom::PatientsProcessorRecursive<Dicom::patients<PATIENT>> processor(filter);
	processor.Apply(studies_heap, pp);
	//удаление пустых подсписков выполняется в processor.Apply
}

template<class FILTER_T, class STUDY>
void	FilterDicoms(Dicom::patient<STUDY> &in_patient, shared_ptr<FILTER_T> filter, ProgressProxy pp = VoidProgressProxy())
{
	Dicom::PatientProcessorRecursive<STUDY> processor(filter);
	processor.Apply(in_patient, pp);
	//удаление пустых подсписков выполняется в processor.Apply
}

template<class FILTER_T, class SERIES>
void	FilterDicoms(Dicom::study<SERIES> &study, shared_ptr<FILTER_T> filter, ProgressProxy pp = VoidProgressProxy())
{
	Dicom::StudyProcessorRecursive<SERIES> processor(filter);
	processor.Apply(study, pp);
	//удаление пустых подсписков выполняется в processor.Apply
}

template<class FILTER_T, class STACK>
void	FilterDicoms(Dicom::series<STACK> &series, shared_ptr<FILTER_T> filter, ProgressProxy pp = VoidProgressProxy())
{
	Dicom::SeriesProcessorRecursive processor(filter);
	processor.Apply(series, pp);
	//удаление пустых подсписков выполняется в processor.Apply
}

template<class FILTER_T, class ACQUISITION>
void	FilterDicoms(Dicom::stack<ACQUISITION> &stack, shared_ptr<FILTER_T> filter, ProgressProxy pp = VoidProgressProxy())
{
	Dicom::StackProcessorRecursive processor(filter);
	processor.Apply(stack, pp);
	//удаление пустых подсписков выполняется в processor.Apply
}

template<class FILTER_T, class INSTANCE>
void	FilterDicoms(Dicom::acquisition<INSTANCE> &acq, shared_ptr<FILTER_T> filter, ProgressProxy pp = VoidProgressProxy())
{
	filter->Apply(acq, pp);
	//удаление пустых подсписков не требуется, т.к. acquisition содержит только простые элементы
}

//TODO ниже пришлось временно делать еще одну реализацию, не сошлись концы где-то в коде. Исправить
template<class FILTER_T>
void	FilterDicoms(Dicom::acquisition_loader &acq, shared_ptr<FILTER_T> filter, ProgressProxy pp = VoidProgressProxy())
{
	filter->Apply(acq, pp);
	//удаление пустых подсписков не требуется, т.к. acquisition содержит только простые элементы
}



XRAD_END

#endif // DicomFilterCore_h__
