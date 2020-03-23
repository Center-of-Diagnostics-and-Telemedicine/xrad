/********************************************************************
	created:	2016/12/22
	created:	22:12:2016   12:45
	file:	 	TomogramGUI.h

	author:		kns

	purpose:
*********************************************************************/
#ifndef TomogramGUI_h__
#define TomogramGUI_h__

#include <XRADDicom/DicomClasses/ProcessContainers/ProcessAcquisition.h>

XRAD_BEGIN

void	DisplayProcessAcquisition(const ProcessAcquisition &acquisition, wstring title);

XRAD_END

#endif // TomogramGUI_h__
