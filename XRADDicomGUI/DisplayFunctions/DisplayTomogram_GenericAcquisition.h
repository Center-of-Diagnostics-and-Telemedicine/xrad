/*!
* \file DisplayTomogram_GenericAcquisition.h
* \date 4/23/2018 5:46:06 PM
*
* \author kovbas
*
* \brief
*
* TODO: long description
*
* \note
*/
#ifndef DisplayTomogram_GenericAcquisition_h__
#define DisplayTomogram_GenericAcquisition_h__


#include <XRADDicom/DicomClasses/ProcessContainers/TomogramAcquisition.h>

XRAD_BEGIN

void	DisplayTomogram_GenericAcquisition(const TomogramAcquisition &tm, const wstring &title);

XRAD_END

#endif // DisplayTomogram_GenericAcquisition_h__