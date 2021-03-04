/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 2018/03/8 20:23
	\author nicholas
*/
#ifndef XRAD__File_ct_slice_h__
#define XRAD__File_ct_slice_h__

#include "tomogram_slice.h"

XRAD_BEGIN

namespace Dicom
{

	class ct_slice : public tomogram_slice
	{
		PARENT(tomogram_slice);
	public:
		//constructors
		ct_slice() {}

		double current() const { return dicom_container()->get_double(e_tube_current); }
		double voltage() const { return dicom_container()->get_double(e_tube_voltage_KVP); }
		double CTDIvol() const { return dicom_container()->get_double(e_CTDIvol); }

		virtual	instance *clone() const override
		{
			return new ct_slice(*this);
		}// ранее существовавший метод copy не учитывал полиморфизма, во избежание путаницы переименовываю его в синоним

	protected:
		virtual void fill_tagslist_to_check_data(set<tag_e> &tagslist_to_check_data_p) override
		{
			parent::fill_tagslist_to_check_data(tagslist_to_check_data_p);

// 			tagslist_to_check_data_p.insert(e_tube_current);
// 			tagslist_to_check_data_p.insert(e_tube_voltage_KVP);
			//tagslist_to_check_data_p.insert(e_CTDIvol);
		}

	private:
	};

}//namespace Dicom


XRAD_END

#endif // XRAD__File_ct_slice_h__
