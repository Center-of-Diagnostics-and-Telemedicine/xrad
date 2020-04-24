/*!
	\file
	\date 5/11/2018 13:07:02
	\author kovbas
*/
#include "pre.h"
#include "Hierarchy.h"

XRAD_BEGIN

namespace Dicom
{


	wstring numbers_itm(size_t itm, bool hide_this_lev)
	{
		(void)hide_this_lev; //чтобы не появлялись варнинги, но при необходимости можно использовать
		return to_wstring(itm) + L" items";
	}

	wstring numbers_acq(size_t acq, size_t itm, bool hide_this_lev)
	{
		wstring result;
		if (acq > 1)
		{
			bool hide_next_lev(false);
			if (acq == itm)
				hide_next_lev = true;
			if (!hide_this_lev)
				result += to_wstring(acq) + L" acquisitions, ";
			result += numbers_itm(itm, hide_next_lev);
		}
		else
		{
			result += numbers_itm(itm);
		}
		return result;
	}

	wstring numbers_sta(size_t sta, size_t acq, size_t itm, bool hide_this_lev)
	{
		wstring result;
		if (sta > 1)
		{
			bool hide_next_lev(false);
			if (sta == acq)
				hide_next_lev = true;
			if (!hide_this_lev)
				result += to_wstring(sta) + L" stacks, ";
			result += numbers_acq(acq, itm, hide_next_lev);
		}
		else
		{
			result += numbers_acq(acq, itm);
		}
		return result;
	}

	wstring numbers_ser(size_t ser, size_t sta, size_t acq, size_t itm, bool hide_this_lev)
	{
		wstring result;
		if (ser > 1)
		{
			bool hide_next_lev(false);
			if (ser == sta)
				hide_next_lev = true;
			if (!hide_this_lev)
				result += to_wstring(ser) + L" series, ";
			result += numbers_sta(sta, acq, itm, hide_next_lev);
		}
		else
		{
			result += numbers_sta(sta, acq, itm);
		}
		return result;
	}


	wstring numbers_stu(size_t stu, size_t ser, size_t sta, size_t acq, size_t itm, bool hide_this_lev)
	{
		wstring result(L"");
		if (stu > 1)
		{
			bool hide_next_lev(false);
			if (stu == ser)
				hide_next_lev = true;
			if(!hide_this_lev)
				result += to_wstring(stu) + L" studies, ";
			result += numbers_ser(ser, sta, acq, itm, hide_next_lev);
		}
		else
		{
			result += numbers_ser(ser, sta, acq, itm);
		}
		return result;
	}



} //namespace Dicom

XRAD_END