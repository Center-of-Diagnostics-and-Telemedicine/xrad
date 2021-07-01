/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 2018/03/8 12:35
	\author nicholas
*/
#ifndef XRAD__File_Modality_h__
#define XRAD__File_Modality_h__

#include <XRADBasic/Core.h>

XRAD_BEGIN

namespace Dicom
{

	bool	is_modality_image(const wstring &modality_string);
	bool	is_modality_tomogram(const wstring &modality_string);
	bool	is_modality_ct(const wstring &modality_string);
	bool	is_modality_mr(const wstring &modality_string);
	bool	is_modality_xray(const wstring &modality_string);
	bool 	is_modality_mmg(const wstring &modality_string);

}//namespace Dicom

XRAD_END

#endif // XRAD__File_Modality_h__
