/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 2/21/2018 3:39:35 PM
	\author kovbas
*/
#include "pre.h"
#include "image.h"

XRAD_BEGIN

namespace Dicom
{

	enum pixeldatatype
	{
		mono_int8, mono_uint8, mono_i16, mono_ui16, mono_i32, mono_ui32, mono_f32,
		rgb_int8, rgb_uint8, rgb_i16, rgb_ui16, rgb_i32, rgb_ui32, rgb_f32
	};
	/*-
	bool image::collect_image()
	{
		open_instancestorage();
		bool result = collect_image_internal();
		close_instancestorage();
		return result;
	}*/
	/*-
	bool image::collect_image_internal()
	{
		//получаем изображение из файла
		size_t bpp = dicom_container()->get_uint(e_bits_allocated);
		bool signedness = dicom_container()->get_uint(e_pixel_representation) != 0;
		//size_t ncomponents = 0;
		size_t ncomponents = dicom_container()->get_uint(e_samples_per_pixel);
		return dicom_container()->get_pixeldata(internal_image(), bpp, signedness, ncomponents, m_frame_no);
	}*/
	/*
	void image::set_to_instance_pixeldata(Container::error_process_mode epm)
	{
		try
		{
		//отдаём в объект файла данные изображения
			dicom_container()->set_pixeldata(internal_image(), bits_allocated(), signedness(), ncomponents());
		}
		catch(...)
		{
			dicom_container()->process_dataelement_error(e_pixel_data, epm);
		}
		dicom_container()->set_wstring(e_image_comments, L"RPCMR processed", m_frame_no);
	}*/
	/*
	void image::set_new_values_to_instance(Container::error_process_mode epm)
	{
		set_to_instance_pixeldata(epm);
		instance::set_new_values_to_instance(epm);
	}*/
	/*-
	image::image()
	{
		//init_cache();
	}*/


} //namespace Dicom

XRAD_END
