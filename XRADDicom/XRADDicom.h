/*!
	* \file XRADDicom.h
	* \date 12/6/2017 11:43:53 AM
	*
	* \author kovbas
	*
	* \brief
	*
	* TODO: Было принято решение о том, что пользователям XRADDicom проще использовать все XRADDicom не подключая каждый компонент отдельно, а подключая только этот заголовочный файл.
	В связи с этим этот файл будет содержать в себе включения всех заголовочных файлов, необходимым для работы с инструментами библиотеки.
	*
	* \note
*/
#ifndef XRADDicom_h__
#define XRADDicom_h__

#include <XRAD/PlatformSpecific/MSVC/PC_XRADDicomLink.h>

#include <XRADDicom/DicomClasses/XRADDicomGeneral.h>

//tools
#include <XRADDicom/XRADDicomTools.h>

//containers
#include <XRADDicom/DicomClasses/DataContainers/Container.h>

//instances
#include <XRADDicom/DicomClasses/Instances/CreateInstance.h>
#include <XRADDicom/DicomClasses/DicomFilters.h>

//instance cache
#include <XRADDicom/DicomClasses/Instances/instance_cache.h>

//load classes and utils
#include <XRADDicom/DicomClasses/DicomStorageAnalyze.h>
#include <XRADDicom/DicomClasses/Instances/LoadGenericClasses.h>

//process classes
//#include <XRADDicom/DicomClasses/ProcessContainers/LoadProcessAcquisition.h>
#include <XRADDicom/DicomClasses/ProcessContainers/CreateProcessAcquisition.h>
//#include <DicomClasses/ProcessContainers/ProcessAcquisition.h>
//#include <DicomClasses/ProcessContainers/TomogramAcquisition.h>
#include <XRADDicom/DicomClasses/ProcessContainers/CTAcquisition.h>

//processors
#include <XRADDicom/DicomClasses/DicomProcessors.h>

//network tools
#include <XRADDicom/DCMTKAccess/pacsTools.h>


XRAD_BEGIN



XRAD_END

#endif // XRADDicom_h__