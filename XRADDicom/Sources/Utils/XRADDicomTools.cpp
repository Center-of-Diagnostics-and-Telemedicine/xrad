/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 26/08/2019 13:10
	\author kovbas
*/
#include "pre.h"
#include "XRADDicomTools.h"

XRAD_BEGIN

namespace Dicom
{

// To avoid  warning LNK4221: This object file does not define any previously undefined public
// symbols, so it will not be used by any link operation that consumes this library.
// (MSVC 2015)
void xrad__dummy_XRADDicomTools() {}

} //end namespace Dicom

XRAD_END
