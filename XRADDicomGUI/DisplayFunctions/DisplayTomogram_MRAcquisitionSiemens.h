/*!
* \file DisplayTomogram_MRAcquisitionSiemens.h
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
#ifndef DisplayTomogram_MRAcquisitionSiemens_h__
#define DisplayTomogram_MRAcquisitionSiemens_h__


#include <XRADDicom/DicomClasses/ProcessContainers/MRAcquisitionSiemens.h>

XRAD_BEGIN

void	DisplayTomogram_MRAcquisitionSiemens(const MRAcquisitionSiemens &mr, const wstring &title);

XRAD_END

#endif // DisplayTomogram_MRAcquisitionSiemens_h__