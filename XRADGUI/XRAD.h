/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__XRAD_h
#define XRAD__XRAD_h

#include <XRADBasic/Core.h>

#include <XRADBasic/MathFunctionTypesMD.h>
#include <XRADBasic/LinearVectorTypes.h>
#include <XRADBasic/MathMatrixTypes.h>
#include <XRADBasic/FFTMD.h>
#include <XRADSystem/System.h>

#include "Sources/GUI/XRADGUI.h"
#include "Sources/GUI/I18nSupport.h"
#include "Sources/GUI/TextDisplayer.h"
#include "Sources/GUI/DynamicDialog.h"
#include "Sources/GUI/SaveRasterImage.h"
#include "Sources/GUI/MathFunctionGUIMD.h"
#include "Sources/GUI/MatrixVectorGUI.h"
#include "Sources/Utils/AutoProgressIndicatorScheduler.h"

#ifdef XRAD_COMPILER_MSC
#include "Sources/PlatformSpecific/MSVC/MSVC_XRADLink.h"
#endif

#endif // XRAD__XRAD_h
