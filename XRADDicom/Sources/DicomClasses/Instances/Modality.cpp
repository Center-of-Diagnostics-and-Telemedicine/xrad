/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 2018/03/8 12:34
	\author nicholas
*/
#include "pre.h"
#include "Modality.h"

XRAD_BEGIN

namespace Dicom
{

	bool is_modality_ct(const wstring &modality_string)
	{
		return (modality_string == L"CT");
	}

	bool is_modality_mr(const wstring &modality_string)
	{
		return (modality_string == L"MR");
	}

	bool is_modality_tomogram(const wstring &modality_string)
	{
		if(is_modality_ct(modality_string)) return true;
		if(is_modality_mr(modality_string)) return true;

		if(modality_string == L"OCT") return true;
		if(modality_string == L"PET") return true;
		if(modality_string == L"ST") return true;

		return false;
	}

	bool is_modality_xray(const wstring &modality_string)
	{
		if (modality_string == L"CR") return true;
		if (modality_string == L"RF") return true;
		if (modality_string == L"DSA") return true;
		if (modality_string == L"DX") return true;
		if (modality_string == L"GM") return true;
		if (modality_string == L"MG") return true;
		if (modality_string == L"PX") return true;
		if (modality_string == L"XA") return true;
		if (modality_string == L"XC") return true;

		return false;
	}


	bool is_modality_image(const wstring &modality_string)
	{
		if (is_modality_tomogram(modality_string)) return true;
		if (is_modality_xray(modality_string)) return true;
		if (modality_string == L"ASMT") return true;

		return false;
	}

}//namespace Dicom


XRAD_END
