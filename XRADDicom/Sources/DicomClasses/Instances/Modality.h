/*!
	\file

	\date 2018/03/8 12:35
	\author nicholas

	\brief
*/
#ifndef __Modality_h__
#define __Modality_h__


XRAD_BEGIN

namespace Dicom
{

	bool	is_modality_image(const wstring &modality_string);
	bool	is_modality_tomogram(const wstring &modality_string);
	bool	is_modality_ct(const wstring &modality_string);
	bool	is_modality_mr(const wstring &modality_string);
	bool	is_modality_xray(const wstring &modality_string);


}//namespace Dicom

XRAD_END

#endif // __Modality_h__
