/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 4/27/2018 10:28:54 AM
	\author kovbas
*/
#ifndef XRADDicomGUI_h__
#define XRADDicomGUI_h__
//--------------------------------------------------------------

#include "Sources/SelectSeriesInteractive.h"

//display functions
#include "Sources/DisplayFunctions/DisplayProcessAcquisition.h"

#include "Sources/dialogs.h"

//--------------------------------------------------------------

#ifndef XRAD_NO_LIBRARIES_LINKS
#ifdef XRAD_COMPILER_MSC
#include "Sources/PlatformSpecific/MSVC/MSVC_XRADDicomGUILink.h"
#endif // XRAD_COMPILER_MSC
#endif // XRAD_NO_LIBRARIES_LINKS

//--------------------------------------------------------------
#endif // XRADDicomGUI_h__
