/*!
* \file DisplayTomogram_MRAcquisition.h
* \date 4/23/2018 5:46:58 PM
*
* \author kovbas
*
* \brief
*
* TODO: long description
*
* \note
*/
#ifndef DisplayTomogram_MRAcquisition_h__
#define DisplayTomogram_MRAcquisition_h__


#include <XRADDicom/DicomClasses/ProcessContainers/MRAcquisition.h>

XRAD_BEGIN

void	DisplayTomogram_MRAcquisition(const MRAcquisition &mr, const wstring &title);

XRAD_END

#endif // DisplayTomogram_MRAcquisition_h__