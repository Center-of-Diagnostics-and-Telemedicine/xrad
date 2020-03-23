/*!
	* \file DisplayTomogram_CTAcquisition.h
	* \date 4/23/2018 5:46:49 PM
	*
	* \author kovbas
	*
	* \brief
	*
	* TODO: long description
	*
	* \note
*/
#ifndef DisplayTomogram_CTAcquisition_h__
#define DisplayTomogram_CTAcquisition_h__

#include <XRADDicom/DicomClasses/ProcessContainers/CTAcquisition.h>

XRAD_BEGIN

void	DisplayTomogram_CTAcquisition(const CTAcquisition &ct, const wstring &title);

XRAD_END

#endif // DisplayTomogram_CTAcquisition_h__