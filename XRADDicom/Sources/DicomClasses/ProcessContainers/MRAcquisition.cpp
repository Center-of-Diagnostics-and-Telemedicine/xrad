/*!
	\file
	\date 30:9:2016 15:37
	\author kns
*/
#include "pre.h"
#include "MRAcquisition.h"

#include <XRADDicom/Sources/DicomClasses/DicomStorageAnalyze.h>
#include <XRADDicom/Sources/DicomClasses/Instances/mr_slice.h>

XRAD_BEGIN
/*?
MRAcquisition::MRAcquisition(size_t s0, size_t s1, size_t s2) : TomogramAcquisition(s0, s1, s2)
{
	realloc_local({s0, s1, s2});
}*/


void MRAcquisition::put_elements_to_instance(Dicom::instance &instance, size_t n) const
{
	//?TomogramAcquisition::put_element_to_instance(instance, n);

//	auto	&ct = dynamic_cast<Dicom::mr_slice &>(*tmp_obj);
	//todo (Kovbas) дописать
}

MRAcquisition &MRAcquisition::operator=(const MRAcquisition &original)
{
	TomogramAcquisition::operator=(original);
	return *this;
}

XRAD_END
