﻿/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 3/22/2018 12:10:31 PM
	\author kovbas
*/
#include "pre.h"
#include "Utils.h"

#include <XRADBasic/Sources/Utils/numbers_in_string.h>

XRAD_BEGIN

namespace Dicom
{

	std::wstring check_float_value(const double val)
	{
		//	Точность, используемая для перевода действительного числа в строку.
		//	Предполагаем, что этой длины хватит на все случаи.
		static const size_t double_value_precision = 50;
		string format = "%." + std::to_string(double_value_precision) + "g";

		std::wstring result(convert_to_wstring(ssprintf(format.c_str(), val)));
		smart_round(result);

		return result;
	}


}
XRAD_END
