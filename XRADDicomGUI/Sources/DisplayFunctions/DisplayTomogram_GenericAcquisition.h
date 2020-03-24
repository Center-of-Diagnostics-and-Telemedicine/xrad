/*!
	\file
	\date 4/23/2018 5:46:06 PM
	\author kovbas
*/
#ifndef DisplayTomogram_GenericAcquisition_h__
#define DisplayTomogram_GenericAcquisition_h__

#include <XRADDicom/Sources/DicomClasses/ProcessContainers/TomogramAcquisition.h>

XRAD_BEGIN

void	DisplayTomogram_GenericAcquisition(const TomogramAcquisition &tm, const wstring &title);

XRAD_END

#endif // DisplayTomogram_GenericAcquisition_h__