/*!
	\file
	\date 10/18/2018 12:47:56 PM
	\author Kovbas (kovbas)
*/
#include "pre.h"
#include "datasource.h"

XRAD_BEGIN

namespace Dicom
{

// To avoid  warning LNK4221: This object file does not define any previously undefined public
// symbols, so it will not be used by any link operation that consumes this library.
// (MSVC 2015)
void xrad__dummy_datasource() {}

datasource_folder::datasource_folder(const wstring &folder_path, bool analyze_subfolders_in) :m_path(folder_path), m_analyze_subfolders(analyze_subfolders_in)
{
}

datasource_folder::datasource_folder(const wstring &folder_path, bool analyze_subfolders_in, mode_t mode) :m_path(folder_path), m_analyze_subfolders(analyze_subfolders_in), m_mode(mode)
{
}

xrad::Dicom::datasource_t* datasource_folder::clone() const
{
	return new datasource_folder(*this);
}

datasource_pacs::datasource_pacs(
	const wstring &address_pacs_p, 
	size_t port_pacs_p, 
	const wstring &AETitle_pacs_p, 
	const wstring &AETitle_local_p, //note Kovbas: use XRAD_SCU by default 
	size_t port_local_p,			//note Kovbas: use 104 by default 
	e_request_t request_type_p) : pacs_params_t(address_pacs_p, port_pacs_p, AETitle_pacs_p, AETitle_local_p, port_local_p, request_type_p)
{

}

} //namespace Dicom

XRAD_END
