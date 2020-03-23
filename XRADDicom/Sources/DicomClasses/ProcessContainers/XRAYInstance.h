/*!
	* \file RGInstance.h
	* \date 4/19/2018 5:50:09 PM
	*
	* \author kovbas
	*
	* \brief
	*
	* TODO: long description
	*
	* \note
*/
#ifndef XRAYInstance_h__
#define XRAYInstance_h__


#include "ProcessAcquisition.h"

XRAD_BEGIN

class XRAYInstance
{
public:
	const RealFunction2D_F32 &image() const { return m_image; };
	RealFunction2D_F32 &image() { return m_image; };
	void set_image(const RealFunction2D_F32 &img);

private:
	RealFunction2D_F32 m_image;
};


typedef unique_ptr<XRAYInstance> XRAYInstance_ptr;

XRAD_END

#endif // XRAYInstance_h__