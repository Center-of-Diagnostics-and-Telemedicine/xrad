/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 10/09/2020 12:19:45 PM
	\author sbp
*/
#include "pre.h"
#include "ProcessAcquisition.h"
#include "GenericImageAcquisition.h"
#include "XRayAcquisition.h"
#include <XRADDicom/Sources/DicomClasses/Instances/xray_image.h>

XRAD_BEGIN


XRayAcquisition::XRayAcquisition(const shared_ptr<Dicom::acquisition_loader> &acquisition_loader_p) : GenericImageAcquisition(acquisition_loader_p)
{
}

/*

void GenericImageAcquisition::put_elements_to_instance(Dicom::instance &instance, size_t element_num) const
{
	auto	&ts = dynamic_cast<Dicom::xray_image &>(instance);

	// кладём общие данные, чтобы можно было в последствии распарсить файлы
	if (ts.stack_id() == L"")
		ts.set_instance_number(element_num + 1);
	else
		ts.set_in_stack_position_number(element_num + 1);

	ts.set_image(get_image(element_num));
}



xrad::RealFunction2D_F32 GenericImageAcquisition::get_image(size_t no) const
{
	XRAD_ASSERT_THROW(no <= m_acquisition_loader->size()-1);

	auto	it = m_acquisition_loader->begin();
	advance(it, no);

	Dicom::image	&img(dynamic_cast<Dicom::image &>(**it));
	Dicom::instance_open_close_class control(img);
	return img.get_image();
}




// получить 2D изображение среза с номером pos
RealFunction2D_F32	GenericImageAcquisition::slice(size_t pos) const
{
	if (pos >= m_acquisition_loader->size())
		return RealFunction2D_F32();
	auto it = m_acquisition_loader->begin();
	advance(it, pos);
	Dicom::image &slice_container = dynamic_cast<Dicom::image&>(**it);
	RealFunction2D_F32 data_slice(slice_container.vsize(), slice_container.hsize());
	slice_container.get_image(data_slice);
	return data_slice;
}


// получить 3D изображение
vector<RealFunction2D_F32>	GenericImageAcquisition::slices() const
{
	vector<RealFunction2D_F32> slices;
	slices.reserve(m_acquisition_loader->size());
	for (auto el : *m_acquisition_loader)
	{
		Dicom::image &slice_container = dynamic_cast<Dicom::image&>(*el);
		RealFunction2D_F32 data_slice(slice_container.vsize(), slice_container.hsize());
		slice_container.get_image(data_slice);
		slices.push_back(std::move(data_slice));
	}
	return slices;
}

*/
XRAD_END
