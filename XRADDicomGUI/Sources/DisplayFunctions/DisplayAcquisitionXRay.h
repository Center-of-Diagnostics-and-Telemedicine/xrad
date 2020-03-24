/*!
	\file
	\date 4/23/2018 5:47:27 PM
	\author kovbas
*/
#ifndef DisplayAcquisitionXRay_h__
#define DisplayAcquisitionXRay_h__

#include <XRADDicom/Sources/DicomClasses/ProcessContainers/XRAYAcquisition.h>

XRAD_BEGIN

void	DisplayXRAYGeneric(const XRAYAcquisition &rg, const wstring &title);

XRAD_END

#endif // DisplayAcquisitionXRay_h__