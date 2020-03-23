/*!
	* \file XRAYAcquisition.h
	* \date 4/23/2018 12:19:45 PM
	*
	* \author kovbas
	*
	* \brief
	*
	* TODO: long description
	*
	* \note
*/
#ifndef XRAYAcquisition_h__
#define XRAYAcquisition_h__

#include <list>
#include "XRAYInstance.h"
#include <XRADDicom/DicomClasses/Instances/LoadGenericClasses.h>

#include <XRADDicom/DicomClasses/Instances/xray_image.h>

XRAD_BEGIN

class XRAYAcquisition : public ProcessAcquisition
{
public:
	XRAYAcquisition(const shared_ptr<Dicom::acquisition_loader> &acquisition_loader_p);
	XRAYAcquisition(const size_t elements_amount); //note этот конструктор только для создания новых сборок

	//gets
	virtual std::string classname() const override { return "XRAYAcquisition"; }

	virtual size_t n_elements() const { return m_acquisition_loader->size(); }
	void put_elements_to_instance(Dicom::instance &instance, size_t num_element) const;

	RealFunction2D_F32 get_image(size_t no) const;
	index_vector	sizes() const
	{
		Dicom::image &first_slice = dynamic_cast<Dicom::image&>(*(m_acquisition_loader->front()));
		return{ (*m_acquisition_loader).size(), first_slice.vsize(), first_slice.hsize() };
	}

	
	RealFunction2D_F32	slice(size_t pos) const;		// получить 2D изображение среза с номером pos
	vector<RealFunction2D_F32>	slices() const;				// получить вектор 2D изображений 
};

XRAD_END

#endif // XRAYAcquisition_h__