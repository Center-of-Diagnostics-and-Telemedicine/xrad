/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_Core_h
#define XRAD__File_Core_h
//--------------------------------------------------------------

#include "Sources/Core/Config.h"
#include "Sources/Core/BasicMacros.h"
#include "Sources/Core/Exceptions.h"
#include "Sources/Core/String.h"
#include "Sources/Core/StringEncode.h"
#include "Sources/Core/BasicUtils.h"
#include "Sources/Core/FlowControl.h"
#include "Sources/Core/NumberTraits.h"
#include "Sources/Core/Limits.h"
#include "Sources/Core/MathConstants.h"
#include "Sources/Core/Functional.h"
#include "Sources/Core/PerformanceCounter.h"
#include "Sources/Core/ThreadSetup.h"
#include "Sources/Core/i18n.h"
#include "Sources/Core/Main.h"

//--------------------------------------------------------------

#ifndef XRAD_NO_LIBRARIES_LINKS
#ifdef XRAD_COMPILER_MSC
#include "Sources/PlatformSpecific/MSVC/MSVC_XRADBasicLink.h"
#endif // XRAD_COMPILER_MSC
#endif // XRAD_NO_LIBRARIES_LINKS

//--------------------------------------------------------------
#endif // XRAD__File_Core_h
