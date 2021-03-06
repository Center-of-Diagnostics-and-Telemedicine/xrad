﻿/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 4/19/2018 5:50:09 PM
	\author kovbas
*/
#include "pre.h"
#include "XRayInstance.h"

XRAD_BEGIN

void XRAYInstance::set_image(const RealFunction2D_F32 &img)
{
	//m_image.realloc(img.vsize(), img.hsize());
	//m_image.CopyData(img);
	m_image.MakeCopy(img);
}

XRAD_END
