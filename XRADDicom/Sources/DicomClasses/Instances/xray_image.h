/*!
	\file
	\date 4/23/2018 4:55:29 PM
	\author kovbas
*/
#ifndef xray_image_h__
#define xray_image_h__

#include "image.h"

XRAD_BEGIN

namespace Dicom
{

	class xray_image : public image
	{
		PARENT(image);
	public:
		virtual wstring get_description() {	return L"";	}
		vector<double> image_orientation_patient() const { return dicom_container()->get_double_values(e_image_orientation_patient); }
		vector<double> image_position_patient() const { return dicom_container()->get_double_values(e_image_position_patient); }

		//methods

		virtual	instance *clone() const override
		{
			return new xray_image(*this);
		}// ранее существовавший метод copy не учитывал полиморфизма, во избежание путаницы переименовываю его в синоним

	private:
		virtual void fill_tagslist_to_check_data(set<tag_e> &tagslist_to_check_data_p) override
		{
			parent::fill_tagslist_to_check_data(tagslist_to_check_data_p);

		}
	};

}//namespace Dicom

XRAD_END

#endif // xray_image_h__