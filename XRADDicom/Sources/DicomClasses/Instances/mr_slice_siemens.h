/*!
	\file
	\date 5/3/2018 12:17:49 PM
	\author kovbas
*/
#ifndef mr_slice_siemens_h__
#define mr_slice_siemens_h__

#include "mr_slice.h"

XRAD_BEGIN

namespace Dicom
{
	class mr_slice_siemens : public mr_slice
	{
		PARENT(mr_slice);
	public:
		//constructors
		mr_slice_siemens() {};

		//gets
		double diffusion_b_value_siemens() const { return dicom_container()->get_double(e_diffusion_b_value_siemens); };

		//sets
		//void set_diffusion_b_value_siemens(const double &val) { set_double(e_diffusion_b_value_siemens, val); };

		//methods
		//virtual void copy(const source &other) override { *this = dynamic_cast<mr_slice&>(const_cast<source&>(other)); };
		virtual	instance *clone() const override
		{
			return new mr_slice_siemens(*this);
		}// ранее существовавший метод copy не учитывал полиморфизма, во избежание путаницы переименовываю его в синоним

	protected:
		virtual void fill_tagslist_to_check_data(set<tag_e> &tagslist_to_check_data_p) override
		{
			parent::fill_tagslist_to_check_data(tagslist_to_check_data_p);

//			tagslist_to_check_data_p.insert(e_diffusion_b_value_siemens);
		}
	};
}

XRAD_END

#endif // mr_slice_siemens_h__