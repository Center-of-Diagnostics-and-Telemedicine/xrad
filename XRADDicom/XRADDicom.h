/*!
	\file
	\date 12/6/2017 11:43:53 AM
	\author kovbas

	\brief Общий заголовочный файл библиотеки

	Было принято решение о том, что пользователям XRADDicom проще использовать весь XRADDicom,
	не подключая каждый компонент отдельно, а подключая только этот заголовочный файл.
	В связи с этим этот файл будет содержать в себе включения всех заголовочных файлов,
	необходимых для работы с инструментами библиотеки.
*/
#ifndef XRAD__File_XRADDicom_h
#define XRAD__File_XRADDicom_h
//--------------------------------------------------------------

#include "Sources/DicomClasses/XRADDicomGeneral.h"

//tools
#include "Sources/Utils/XRADDicomTools.h"
#include "Sources/Utils/logger.h"

//containers
#include "Sources/DicomClasses/DataContainers/Container.h"

//instances
#include "Sources/DicomClasses/Instances/CreateInstance.h"
#include "Sources/DicomClasses/DicomFilters.h"

//instance cache
#include "Sources/DicomClasses/Instances/instance_cache.h"

//load classes and utils
#include "Sources/DicomClasses/DicomStorageAnalyze.h"
#include "Sources//DicomClasses/Instances/LoadGenericClasses.h"

//process classes
#include "Sources/DicomClasses/ProcessContainers/CreateProcessAcquisition.h"
#include "Sources/DicomClasses/ProcessContainers/CTAcquisition.h"

//processors
#include "Sources/DicomClasses/DicomProcessors.h"

//network tools
#include "Sources/DCMTKAccess/pacsTools.h"

//--------------------------------------------------------------

#ifndef XRAD_NO_LIBRARIES_LINKS
#ifdef XRAD_COMPILER_MSC
#include "Sources/PlatformSpecific/MSVC/MSVC_XRADDicomLink.h"
#endif // XRAD_COMPILER_MSC
#endif // XRAD_NO_LIBRARIES_LINKS

//--------------------------------------------------------------
#endif // XRAD__File_XRADDicom_h
