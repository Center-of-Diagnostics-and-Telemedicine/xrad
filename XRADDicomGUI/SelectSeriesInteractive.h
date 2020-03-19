/********************************************************************
	created:	2016/12/22
	created:	22:12:2016   12:28
	file:	 	SelectSeriesInteractive.h

	author:		kns

	purpose:
*********************************************************************/
#ifndef SelectSeriesInteractive_h__
#define SelectSeriesInteractive_h__

#include <XRADDicom/DicomClasses/Instances/LoadGenericClasses.h>

XRAD_BEGIN

/*!
	\brief функция выбора сборки, с которой будет дальнейшая работа
*/
Dicom::acquisition_loader &SelectSeriesInteractive(Dicom::patients_loader &studies_heap);

XRAD_END

#endif // SelectSeriesInteractive_h__
