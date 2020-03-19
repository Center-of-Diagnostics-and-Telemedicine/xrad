// file Core.h
//--------------------------------------------------------------
#ifndef __Core_h
#define __Core_h
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
#endif // __Core_h
